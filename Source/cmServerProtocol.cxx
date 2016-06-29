/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmServerProtocol.h"

#include "cmCommand.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmServer.h"
#include "cmServerCompleter.h"
#include "cmServerDiff.h"
#include "cmServerParser.h"
#include "cmSourceFile.h"
#include "cmVersionMacros.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cm_jsoncpp_reader.h"
#include "cm_jsoncpp_value.h"
#endif

cmServerProtocol::cmServerProtocol(cmMetadataServer* server,
                                   std::string buildDir)
  : Server(server)
  , CMakeInstance(0)
  , m_buildDir(buildDir)
{
}

cmServerProtocol::~cmServerProtocol()
{
  delete this->CMakeInstance;
}

void cmServerProtocol::Error(std::string const& error = "unknown")
{
  Json::Value obj = Json::objectValue;
  obj["error"] = error;
  this->Server->WriteResponse(obj);
}

// return true if server should quit, false otherwise.
bool cmServerProtocol::processRequest(const std::string& json)
{
   Json::Value value;
   if (!Json::Reader{}.parse(json, value)) {
     this->Error("json parse failed on.\n" + json);
     return false;
   }

  auto const type = value["type"].asString();
  if (type == "quit") {
    return true;
  }

  if (this->Server->GetState() == cmMetadataServer::Started) {
    if (type == "handshake") {
      this->ProcessHandshake(value["protocolVersion"].asString());
    } else {
      this->Error("unknown query type " + type);
    }
  } else if (this->Server->GetState() == cmMetadataServer::ProcessingRequests) {
    if (type == "version") {
      this->ProcessVersion();
    } else if (type == "buildsystem") {
      this->ProcessBuildsystem();
    } else if (type == "cmake_variables") {
      this->ProcessCMakeVariables();
    } else if (type == "target_info") {
      const char* language = 0;
      if (value.isMember("language")) {
        language = value["language"].asCString();
      }
      this->ProcessTargetInfo(value["target_name"].asString(),
                              value["config"].asString(), language);
    } else if (type == "file_info") {
      this->ProcessFileInfo(value["target_name"].asString(),
                            value["config"].asString(),
                            value["file_path"].asString());
    } else if (type == "content") {
      auto diff = cmServerDiff::GetDiff(value);
      this->ProcessContent(value["file_path"].asString(),
                           value["file_line"].asInt(), diff,
                           value["matcher"].asString());
    } else if (type == "parse") {
      auto diff = cmServerDiff::GetDiff(value);
      this->ProcessParse(value["file_path"].asString(), diff);
    } else if (type == "contextual_help") {
      this->ProcessContextualHelp(
        value["file_path"].asString(), value["file_line"].asInt(),
        value["file_column"].asInt(), value["file_content"].asString());
    } else if (type == "content_diff") {
      auto diffs = cmServerDiff::GetDiffs(value);
      this->ProcessContentDiff(
        value["file_path1"].asString(), value["file_line1"].asInt(),
        value["file_path2"].asString(), value["file_line2"].asInt(), diffs);
    } else if (type == "code_complete") {
      auto diff = cmServerDiff::GetDiff(value);
      this->ProcessCodeComplete(value["file_path"].asString(),
                                value["file_line"].asInt(),
                                value["file_column"].asInt(), diff);
    } else if (type == "context_writers") {
      auto diff = cmServerDiff::GetDiff(value);

      this->ProcessContextWriters(value["file_path"].asString(),
                                  value["file_line"].asInt(),
                                  value["file_column"].asInt(), diff);
    } else {
      this->Error("unknown query type " + type);
    }
  } else {
    this->Error("unknown query type " + type);
  }

  return false;
}

void cmServerProtocol::ProcessHandshake(std::string const& protocolVersion)
{
  // TODO: Handle version.
  (void)protocolVersion;

  this->Server->SetState(cmMetadataServer::Initializing);
  this->CMakeInstance = new cmake;
  this->CMakeInstance->SetWorkingMode(cmake::SNAPSHOT_RECORD_MODE);
  std::set<std::string> emptySet;
  if (!this->CMakeInstance->GetState()->LoadCache(m_buildDir.c_str(), true,
                                                  emptySet, emptySet)) {
    this->Error("LoadCache failed.");
    return;
  }

  const char* genName =
    this->CMakeInstance->GetState()->GetInitializedCacheValue(
      "CMAKE_GENERATOR");
  if (!genName) {
    this->Error("CMAKE_GENERATOR not found in cache.");
    return;
  }

  const char* sourceDir =
    this->CMakeInstance->GetState()->GetInitializedCacheValue(
      "CMAKE_HOME_DIRECTORY");
  if (!sourceDir) {
    this->Error("CMAKE_HOME_DIRECTORY not found in cache.");
    return;
  }

  this->CMakeInstance->SetHomeDirectory(sourceDir);
  this->CMakeInstance->SetHomeOutputDirectory(m_buildDir);
  this->CMakeInstance->SetGlobalGenerator(
    this->CMakeInstance->CreateGlobalGenerator(genName));

  this->CMakeInstance->LoadCache();
  this->CMakeInstance->SetSuppressDevWarnings(true);
  this->CMakeInstance->SetWarnUninitialized(false);
  this->CMakeInstance->SetWarnUnused(false);
  this->CMakeInstance->PreLoadCMakeFiles();

  Json::Value obj = Json::objectValue;
  obj["progress"] = "initialized";

  this->Server->WriteResponse(obj);

  // First not? But some other mode that aborts after ActualConfigure
  // and creates snapshots?
  this->CMakeInstance->Configure();

  obj["progress"] = "configured";

  this->Server->WriteResponse(obj);

  if (!this->CMakeInstance->GetGlobalGenerator()->Compute()) {
    this->Error("GlobalGenerator::Compute failed.");
    return;
  }

  obj["progress"] = "computed";

  this->Server->WriteResponse(obj);

  cmState* state = this->CMakeInstance->GetState();

  auto snps = state->TraceSnapshots();
  for (auto is = snps.begin(); is != snps.end(); ++is) {
    this->Snapshots[is->first] = is->second;
  }

  auto srcDir = this->CMakeInstance->GetState()->GetSourceDirectory();

  Json::Value idleObj = Json::objectValue;
  idleObj["progress"] = "idle";
  idleObj["source_dir"] = srcDir;
  idleObj["binary_dir"] =
    this->CMakeInstance->GetState()->GetBinaryDirectory();
  idleObj["project_name"] = this->CMakeInstance->GetGlobalGenerator()
                              ->GetLocalGenerators()[0]
                              ->GetProjectName();

  this->Server->SetState(cmMetadataServer::ProcessingRequests);

  this->Server->WriteResponse(idleObj);
}

void cmServerProtocol::ProcessVersion()
{
  Json::Value obj = Json::objectValue;
  obj["version"] = CMake_VERSION;

  this->Server->WriteResponse(obj);
}

void cmServerProtocol::ProcessBuildsystem()
{
  Json::Value root = Json::objectValue;
  Json::Value& obj = root["buildsystem"] = Json::objectValue;

  auto mf = this->CMakeInstance->GetGlobalGenerator()->GetMakefiles()[0];
  auto lg = this->CMakeInstance->GetGlobalGenerator()->GetLocalGenerators()[0];

  obj["version"  ] = mf->GetSafeDefinition("CMAKE_VERSION");
  obj["generator"] = mf->GetSafeDefinition("CMAKE_GENERATOR");

  Json::Value& configs = obj["configs"] = Json::arrayValue;

  std::vector<std::string> configsVec;
  mf->GetConfigurations(configsVec);
  for (auto const& config : configsVec) {
    configs.append(config);
  }

  Json::Value& globalTargets = obj["globalTargets"] = Json::arrayValue;
  Json::Value& targets = obj["targets"] = Json::arrayValue;
  auto gens = this->CMakeInstance->GetGlobalGenerator()->GetLocalGenerators();

  auto firstMf = this->CMakeInstance->GetGlobalGenerator()->GetMakefiles()[0];
  auto firstTgts = firstMf->GetTargets();
  for (auto const& tgt : firstTgts) {
    if (tgt.second.GetType() == cmState::GLOBAL_TARGET) {
      globalTargets.append(tgt.second.GetName());
    }
  }

  for (auto const& gen : gens) {
    for (auto const& tgt : gen->GetGeneratorTargets()) {
      if (tgt->IsImported()) {
        continue;
      }
      if (tgt->GetType() == cmState::GLOBAL_TARGET) {
        continue;
      }
      Json::Value target = Json::objectValue;
      target["name"] = tgt->GetName();
      target["type"] = cmState::GetTargetTypeName(tgt->GetType());

      if (tgt->GetType() <= cmState::UTILITY) {
        auto lfbt = tgt->GetBacktrace();
        Json::Value bt = Json::arrayValue;
        for (auto const& lbtF : lfbt.FrameContexts()) {
          Json::Value fff = Json::objectValue;
          fff["path"] = lbtF.FilePath;
          fff["line"] = (int)lbtF.Line;
          bt.append(fff);
        }
        target["backtrace"] = bt;
        if (tgt->GetType() < cmState::OBJECT_LIBRARY) {
          //          std::string fp = (*ittgt)->GetFullPath(config, false,
          //          true);
          //          targetValue["target_file"] = fp;
        }
      }
      // Should be list?
      target["projectName"] = lg->GetProjectName();
      targets.append(target);
    }
  }
  this->Server->WriteResponse(root);
}

void cmServerProtocol::ProcessCMakeVariables()
{
  Json::Value root = Json::objectValue;
  Json::Value& obj = root["cmake_variables"] = Json::objectValue;

  auto const state = this->CMakeInstance->GetState();
  for (auto const& key : state->GetCacheEntryKeys()) {
    obj[key] = state->GetCacheEntryValue(key);
  }
  this->Server->WriteResponse(root);
}

void cmServerProtocol::ProcessTargetInfo(std::string tgtName,
                                         std::string config,
                                         const char* language)
{
  Json::Value obj = Json::objectValue;
  Json::Value& root = obj["target_info"] = Json::objectValue;

  auto tgt =
    this->CMakeInstance->GetGlobalGenerator()->FindGeneratorTarget(tgtName);

  if (!tgt) {
    this->Error(tgtName + " not found.");
    return;
  }

  root["target_name"] = tgt->GetName();

  if (tgt->GetType() != cmState::GLOBAL_TARGET &&
      tgt->GetType() != cmState::UTILITY &&
      tgt->GetType() != cmState::OBJECT_LIBRARY) {
    root["build_location"] = tgt->GetLocation(config);
    if (tgt->HasImportLibrary()) {
      root["build_implib"] = tgt->GetFullPath(config, true);
    }
  }

  std::vector<const cmSourceFile*> files;

  tgt->GetObjectSources(files, config);

  Json::Value& object_sources = root["object_sources"] = Json::arrayValue;
  Json::Value& generated_object_sources = root["generated_object_sources"] =
    Json::arrayValue;
  for (auto const& sf : files) {
    std::string filePath = sf->GetFullPath();
    if (sf->GetProperty("GENERATED")) {
      generated_object_sources.append(filePath);
    } else {
      object_sources.append(filePath);
    }
  }

  files.clear();

  tgt->GetHeaderSources(files, config);

  Json::Value& header_sources = root["header_sources"] = Json::arrayValue;
  Json::Value& generated_header_sources = root["generated_header_sources"] =
    Json::arrayValue;
  for (auto const& sf : files) {
    std::string filePath = sf->GetFullPath();
    if (sf->GetProperty("GENERATED")) {
      generated_header_sources.append(filePath);
    } else {
      header_sources.append(filePath);
    }
  }

  Json::Value& target_defines = root["compile_definitions"] = Json::arrayValue;

  std::string lang = language ? language : "C";

  std::vector<std::string> cdefs;
  tgt->GetCompileDefinitions(cdefs, config, lang);
  for (auto const& cdef : cdefs) {
    target_defines.append(cdef);
  }

  Json::Value& target_features = root["compile_features"] = Json::arrayValue;

  std::vector<std::string> features;
  tgt->GetCompileFeatures(cdefs, config);
  for (auto const& feature : features) {
    target_features.append(feature);
  }

  Json::Value& target_options = root["compile_options"] = Json::arrayValue;

  std::vector<std::string> options;
  tgt->GetCompileOptions(cdefs, config, lang);
  for (auto const& option : options) {
    target_options.append(option);
  }

  Json::Value& target_includes = root["include_directories"] =
    Json::arrayValue;

  std::vector<std::string> dirs;
  tgt->GetLocalGenerator()->GetIncludeDirectories(dirs, tgt, lang, config);
  for (auto const& dir : dirs) {
    target_includes.append(dir);
  }
  this->Server->WriteResponse(obj);
}

void cmServerProtocol::ProcessFileInfo(std::string tgtName, std::string config,
                                       std::string file_path)
{
  auto tgt =
    this->CMakeInstance->GetGlobalGenerator()->FindGeneratorTarget(tgtName);

  if (!tgt) {
    this->Error(tgtName + " not found.");
    return;
  }

  std::vector<const cmSourceFile*> files;
  tgt->GetObjectSources(files, config);

  const cmSourceFile* file = 0;
  for (auto const& sf : files) {
    if (sf->GetFullPath() == file_path) {
      file = sf;
      break;
    }
  }

  if (!file) {
    this->Error(file_path + " not found");
    return;
  }

  Json::Value obj = Json::objectValue;
  Json::Value& root = obj["file_info"] = Json::objectValue;

  root["targetName"] = tgtName;
  root["filePath"] = file_path;

  auto lg = tgt->GetLocalGenerator();

  std::string lang = file->GetLanguage();

  std::vector<std::string> includes;
  lg->GetIncludeDirectories(includes, tgt, lang, config);

  Json::Value& include_dirs = root["include_directories"] = Json::arrayValue;
  for (auto const& dir : includes) {
    include_dirs.append(dir);
  }

  std::set<std::string> defines;
  lg->AppendDefines(defines, file->GetProperty("COMPILE_DEFINITIONS"));
  {
    std::string defPropName = "COMPILE_DEFINITIONS_";
    defPropName += cmSystemTools::UpperCase(config);
    lg->AppendDefines(defines, file->GetProperty(defPropName));
  }

  // Add the export symbol definition for shared library objects.
  if (const char* exportMacro = tgt->GetExportMacro()) {
    lg->AppendDefines(defines, exportMacro);
  }

  // Add preprocessor definitions for this target and configuration.
  lg->AddCompileDefinitions(defines, tgt, config, lang);

  Json::Value& compile_defs = root["compile_definitions"] = Json::arrayValue;
  for (auto const& def : defines) {
    compile_defs.append(def);
  }

  std::string flags;
  lg->AddLanguageFlags(flags, lang, config);

  lg->AddArchitectureFlags(flags, tgt, lang, config);

  lg->AddVisibilityPresetFlags(flags, tgt, lang);

  lg->AddCompileOptions(flags, tgt, lang, config);

  root["compile_flags"] = flags;

  this->Server->WriteResponse(obj);
}

std::pair<cmState::Snapshot, long> cmServerProtocol::GetSnapshotAndStartLine(
  std::string filePath, long fileLine, DifferentialFileContent diff)
{
  assert(fileLine > 0);

  const auto& chunks = diff.Chunks;

  auto it = std::lower_bound(
    chunks.begin(), chunks.end(), fileLine,
    [](Chunk const& lhs, long rhs) { return lhs.NewStart < rhs; });
  if (it == chunks.end() || it->NewStart != fileLine) {
    --it;
  }
  auto theLine = it->NewStart;
  while (it->NumCommon + it->NumAdded == 0) {
    ++it;
  }
  assert(theLine == it->NewStart);

  // it is the chunk which contains the line request.

  auto searchStart = 0;
  bool isNotCommon = it->NumAdded != 0 || it->NumRemoved != 0;
  if (isNotCommon) {
    if (it != chunks.begin()) {
      auto it2 = it;
      --it2;
      searchStart = it2->OrigStart + it2->NumCommon;
    } else {
      searchStart = 1;
    }
  } else {
    auto newLinesDistance = fileLine - it->NewStart;
    searchStart = it->OrigStart + newLinesDistance;
  }

  auto ctx = this->GetSnapshotContext(filePath, searchStart);

  auto mostRecentSnapshotLine = ctx.second;

  // We might still have commands we can't execute in the dirty set. We
  // will skip over them.

  auto itToExecFrom = std::lower_bound(
    chunks.begin(), chunks.end(), mostRecentSnapshotLine,
    [](Chunk const& lhs, long rhs) { return lhs.OrigStart < rhs; });
  if (itToExecFrom == chunks.end() ||
      itToExecFrom->OrigStart != mostRecentSnapshotLine) {
    --itToExecFrom;
  }
  isNotCommon = itToExecFrom->NumAdded != 0 || itToExecFrom->NumRemoved != 0;
  long startFrom = 0;
  if (isNotCommon) {
    startFrom = -1;
  } else {
    auto oldLinesDistance = mostRecentSnapshotLine - itToExecFrom->OrigStart;

    startFrom = itToExecFrom->NewStart + oldLinesDistance;
  }

  return std::make_pair(ctx.first, startFrom);
}

std::pair<cmState::Snapshot, cmListFileFunction>
cmServerProtocol::GetDesiredSnapshot(
  std::vector<std::string> const& editorLines, long startLine,
  cmState::Snapshot snp, long fileLine, bool completionMode)
{
  auto prParseStart = editorLines.begin() + startLine - 1;
  assert((long)editorLines.size() >= fileLine);
  auto prParseEnd = editorLines.begin() + fileLine - 1;
  if (completionMode) {
    ++prParseEnd;
  }

  auto newString = cmJoin(cmMakeRange(prParseStart, prParseEnd), "\n");

  cmGlobalGenerator* gg = this->CMakeInstance->GetGlobalGenerator();

  cmMakefile mf(gg, snp);

  cmListFile listFile;

  if (!listFile.ParseString(newString.c_str(),
                            snp.GetExecutionListFile().c_str(), &mf)) {
    std::cout << "STRING PARSE ERROR" << std::endl;
    return std::make_pair(cmState::Snapshot(), cmListFileFunction());
  }

  auto newToParse = fileLine - startLine + 1;
  assert(newToParse >= 0);
  return mf.ReadCommands(listFile.Functions, newToParse);
}

void cmServerProtocol::writeContent(cmState::Snapshot snp, std::string matcher)
{
  Json::Value obj = Json::objectValue;

  Json::Value& content = obj["content"] = Json::objectValue;

  std::vector<std::string> keys = snp.ClosureKeys();
  for (const auto& p : keys) {
    if (p.find(matcher) == 0)
      content[p] = snp.GetDefinition(p);
  }

  this->Server->WriteResponse(obj);
}

bool cmServerProtocol::IsNotExecuted(std::string filePath, long fileLine)
{
  auto nx = this->CMakeInstance->GetState()->GetNotExecuted(filePath);
  for (auto it = nx.begin(); it != nx.end(); ++it) {
    if (fileLine >= it->first && fileLine < it->second) {
      return true;
    }
  }
  return false;
}

void cmServerProtocol::ProcessContent(std::string filePath, long fileLine,
                                      DifferentialFileContent diff,
                                      std::string matcher)
{
  assert(fileLine > 0);
  if (this->IsNotExecuted(filePath, fileLine)) {
    Json::Value obj = Json::objectValue;

    obj["content_result"] = "unexecuted";
    this->Server->WriteResponse(obj);
    return;
  }

  auto res = this->GetSnapshotAndStartLine(filePath, fileLine, diff);
  if (res.second < 0) {
    Json::Value obj = Json::objectValue;
    obj["content_result"] = "unexecuted";
    this->Server->WriteResponse(obj);
    return;
  }

  auto desired = this->GetDesiredSnapshot(diff.EditorLines, res.second,
                                          res.first, fileLine);
  cmState::Snapshot contentSnp = desired.first;
  if (!contentSnp.IsValid()) {
    Json::Value obj = Json::objectValue;
    obj["content_result"] = "unexecuted";
    this->Server->WriteResponse(obj);
    return;
  }

  this->writeContent(contentSnp, matcher);
}

std::pair<cmState::Snapshot, long> cmServerProtocol::GetSnapshotContext(
  std::string filePath, long fileLine)
{
  cmListFileContext lfc;
  lfc.FilePath = filePath;
  lfc.Line = fileLine;

  auto it = this->Snapshots.lower_bound(lfc);

  // Do some checks before any of this? ie, if the prev, then popped is
  // macro or function, then do it, otherwise don't?
  // Also some logic to know whether to go prev?

  if (it != this->Snapshots.end()) {
    cmState::Snapshot snp = it->second.back();

    if (snp.GetExecutionListFile() == filePath &&
        snp.GetStartingPoint() == fileLine) {
      return std::make_pair(this->CMakeInstance->GetState()->PopArbitrary(snp),
                            fileLine);
    }
  }

  assert(it != this->Snapshots.begin());

  --it;
  cmState::Snapshot snp = it->second.back();

  {
    // Do this conditionally, depending on whether we need the state from
    // inside
    // a macro or included file.
    cmListFileContext lfc2;
    lfc2.FilePath = it->second.front().GetExecutionListFile();
    lfc2.Line = it->second.front().GetStartingPoint();
    //  if (lfc2 == it->first)
    {
      snp = this->CMakeInstance->GetState()->PopArbitrary(snp);
    }
  }

  long startingPoint = it->first.Line;
  return std::make_pair(snp, startingPoint);
}

void getUnreachable(Json::Value& unreachable,
                    DifferentialFileContent const& diff,
                    std::map<long, long> const& nx)
{
  auto& chunks = diff.Chunks;

  auto chunkIt = chunks.begin();

  for (auto it = nx.begin(); it != nx.end(); ++it) {
    chunkIt = std::lower_bound(
      chunkIt, chunks.end(), it->first,
      [](const Chunk& lhs, long rhs) { return lhs.OrigStart < rhs; });
    if (chunkIt == chunks.end() || chunkIt->OrigStart != it->first) {
      --chunkIt;
    }
    auto theLine = chunkIt->NewStart;
    while (chunkIt->NumCommon + chunkIt->NumAdded ==
           0) // Should be NumRemoved?
    {
      ++chunkIt;
    }
    assert(theLine == chunkIt->NewStart);
    if (chunkIt->OrigStart > it->first) {
      continue;
    }

    long offset = chunkIt->NewStart - chunkIt->OrigStart;

    Json::Value elem = Json::objectValue;
    elem["begin"] = (int)(it->first + offset);
    elem["end"] = (int)(it->second + offset);
    unreachable.append(elem);
  }
}

void cmServerProtocol::ProcessParse(std::string file_path,
                                    DifferentialFileContent diff)
{
  Json::Value obj = Json::objectValue;
  Json::Value& root = obj["parsed"] = Json::objectValue;

  cmServerParser p(this->CMakeInstance->GetState(), file_path,
                   cmSystemTools::GetCMakeRoot());
  root["tokens"] = p.Parse(diff);

  auto& unreachable = root["unreachable"] = Json::arrayValue;

  auto nx = this->CMakeInstance->GetState()->GetNotExecuted(file_path);

  getUnreachable(unreachable, diff, nx);

  this->Server->WriteResponse(obj);
}

bool cmServerProtocol::WriteContextualHelp(std::string const& context,
                                           std::string const& help_key)
{
  std::string pdir = cmSystemTools::GetCMakeRoot();
  pdir += "/Help/" + context + "/";

  std::string relevant = cmSystemTools::HelpFileName(help_key);
  std::string helpFile = pdir + relevant + ".rst";
  if (!cmSystemTools::FileExists(helpFile.c_str(), true)) {
    return false;
  }
  Json::Value obj = Json::objectValue;

  Json::Value& contextual_help = obj["contextual_help"] = Json::objectValue;

  contextual_help["context"] = context;
  contextual_help["help_key"] = relevant;

  this->Server->WriteResponse(obj);

  return true;
}

bool cmServerProtocol::EmitTypedIdentifier(
  std::string const& commandName, std::vector<cmListFileArgument> args,
  size_t argIndex)
{
  cmCommand* proto = this->CMakeInstance->GetState()->GetCommand(commandName);
  if (!proto) {
    return false;
  }

  std::vector<std::string> argStrings;
  for (auto arg : args) {
    argStrings.push_back(arg.Value);
  }

  auto contextType = proto->GetContextForParameter(argStrings, argIndex);

  auto value = args[argIndex].Value;

  std::string context;
  switch (contextType) {
    case cmCommand::TargetPropertyParameter:
      context = "prop_tgt";
      break;
    case cmCommand::DirectoryPropertyParameter:
      context = "prop_dir";
      break;
    case cmCommand::VariableIdentifierParameter:
      context = "variable";
      break;
    case cmCommand::PolicyParameter:
      context = "policy";
      break;
    case cmCommand::ModuleNameParameter:
      context = "module";
      break;
    case cmCommand::PackageNameParameter:
      context = "module";
      value = "Find" + value;
      break;
    default:
      break;
  }

  if (context.empty()) {
    return false;
  }

  return this->WriteContextualHelp(context, value);
}

void cmServerProtocol::ProcessContextualHelp(std::string filePath,
                                             long fileLine, long fileColumn,
                                             std::string fileContent)
{
  assert(fileLine > 0);

  std::string content;
  {
    std::stringstream ss(fileContent);

    long desiredLines = fileLine;
    for (std::string line; std::getline(ss, line, '\n') && desiredLines > 0;
         --desiredLines) {
      content += line + "\n";
    }
  }

  cmListFile listFile;

  if (!listFile.ParseString(
        content.c_str(), filePath.c_str(),
        this->CMakeInstance->GetGlobalGenerator()->GetMakefiles()[0])) {
    this->Error("cmListFile::ParseString failed.");
    return;
  }

  const size_t numberFunctions = listFile.Functions.size();
  size_t funcIndex = 0;
  for (; funcIndex < numberFunctions; ++funcIndex) {
    if (listFile.Functions[funcIndex].Line > fileLine) {
      Json::Value obj = Json::objectValue;
      Json::Value& contextual_help = obj["contextual_help"] =
        Json::objectValue;

      contextual_help["nocontext"] = true;

      this->Server->WriteResponse(obj);
      break;
    }

    const long closeParenLine = listFile.Functions[funcIndex].CloseParenLine;

    if (listFile.Functions[funcIndex].Line <= fileLine &&
        closeParenLine >= fileLine) {
      auto args = listFile.Functions[funcIndex].Arguments;
      const size_t numberArgs = args.size();
      size_t argIndex = 0;

      for (; argIndex < numberArgs; ++argIndex) {
        if (args[argIndex].Delim == cmListFileArgument::Bracket) {
          continue;
        }

        const bool lastArg = (argIndex == numberArgs - 1);

        if (lastArg || (argIndex != numberArgs &&
                        (args[argIndex + 1].Line > fileLine ||
                         args[argIndex + 1].Column > fileColumn))) {
          if (args[argIndex].Line > fileLine ||
              args[argIndex].Column > fileColumn) {
            this->WriteContextualHelp("command",
                                      listFile.Functions[funcIndex].Name);
            return;
          }
          if (args[argIndex].Delim == cmListFileArgument::Unquoted) {
            auto endPos = args[argIndex].Column + args[argIndex].Value.size();
            if (args[argIndex].Line == fileLine &&
                args[argIndex].Column <= fileColumn &&
                (long)endPos >= fileColumn) {
              auto inPos = fileColumn - args[argIndex].Column;
              auto closePos = args[argIndex].Value.find('}', inPos);
              auto openPos = args[argIndex].Value.rfind('{', inPos);
              if (openPos != std::string::npos) {
                if (openPos > 0 && args[argIndex].Value[openPos - 1] == '$') {
                  auto endRel = closePos == std::string::npos
                    ? closePos - openPos - 1
                    : inPos - openPos - 1;
                  std::string relevant =
                    args[argIndex].Value.substr(openPos + 1, endRel);
                  if (this->WriteContextualHelp("variable", relevant))
                    return;
                }
              }
              if (this->EmitTypedIdentifier(listFile.Functions[funcIndex].Name,
                                            args, argIndex)) {
                return;
              }
            }
            break;
          }

          long fileLineDiff = fileLine - args[argIndex].Line;

          long fileColumnDiff = fileColumn - args[argIndex].Column;

          bool breakOut = false;

          size_t argPos = 0;
          while (fileLineDiff != 0) {
            argPos = args[argIndex].Value.find('\n', argPos);
            if (argPos == std::string::npos) {
              breakOut = true;
              break;
            }
            ++argPos;
            fileColumnDiff = 0;
            --fileLineDiff;
          }
          if (breakOut) {
            break;
          }

          assert(fileLineDiff == 0);

          size_t sentinal = args[argIndex].Value.find('\n', argPos);
          if (sentinal == std::string::npos) {
            sentinal = args[argIndex].Value.size() - argPos;
            if ((long)sentinal < fileColumn) {
              break;
            }
            if (this->EmitTypedIdentifier(listFile.Functions[funcIndex].Name,
                                          args, argIndex)) {
              return;
            }
          }

          if (sentinal < argPos) {
            // In between args?
            break;
          }

          long inPos = fileColumnDiff;

          std::string relevant =
            args[argIndex].Value.substr(argPos, sentinal - argPos);

          auto closePos = relevant.find('}', inPos);
          auto openPos = relevant.rfind('{', inPos);
          if (openPos != std::string::npos) {
            if (openPos > 0 && relevant[openPos - 1] == '$') {
              auto endRel = closePos == std::string::npos
                ? closePos - openPos - 1
                : inPos - openPos - 1;
              relevant = relevant.substr(openPos + 1, endRel);
              if (this->WriteContextualHelp("variable", relevant))
                return;
              else
                break;
            }
          }
          break;
        }
      }

      this->WriteContextualHelp("command", listFile.Functions[funcIndex].Name);
      return;
    }
  }
}

void cmServerProtocol::ProcessContentDiff(
  std::string filePath1, long fileLine1, std::string filePath2, long fileLine2,
  std::pair<DifferentialFileContent, DifferentialFileContent> diffs)
{
  assert(fileLine1 > 0);
  assert(fileLine2 > 0);

  if (this->IsNotExecuted(filePath1, fileLine1) ||
      this->IsNotExecuted(filePath2, fileLine2)) {
    Json::Value obj = Json::objectValue;

    obj["content_result"] = "unexecuted";
    this->Server->WriteResponse(obj);
    return;
  }

  auto res1 = GetSnapshotAndStartLine(filePath1, fileLine1, diffs.first);
  if (res1.second < 0) {
    Json::Value obj = Json::objectValue;
    obj["content_result"] = "unexecuted";
    this->Server->WriteResponse(obj);
    return;
  }

  auto res2 = GetSnapshotAndStartLine(filePath2, fileLine2, diffs.second);
  if (res2.second < 0) {
    Json::Value obj = Json::objectValue;
    obj["content_result"] = "unexecuted";
    this->Server->WriteResponse(obj);
    return;
  }

  auto desired1 = GetDesiredSnapshot(diffs.first.EditorLines, res1.second,
                                     res1.first, fileLine1);
  cmState::Snapshot contentSnp1 = desired1.first;
  if (!contentSnp1.IsValid()) {
    Json::Value obj = Json::objectValue;
    obj["content_result"] = "unexecuted";
    this->Server->WriteResponse(obj);
    return;
  }

  auto desired2 = GetDesiredSnapshot(diffs.second.EditorLines, res2.second,
                                     res2.first, fileLine2);
  cmState::Snapshot contentSnp2 = desired2.first;
  if (!contentSnp2.IsValid()) {
    Json::Value obj = Json::objectValue;
    obj["content_result"] = "unexecuted";
    this->Server->WriteResponse(obj);
    return;
  }

  Json::Value obj = Json::objectValue;

  Json::Value& content = obj["content_diff"] = Json::objectValue;

  std::vector<std::string> keys1 = contentSnp1.ClosureKeys();
  std::vector<std::string> keys2 = contentSnp2.ClosureKeys();

  auto& addedDefs = content["addedDefs"] = Json::arrayValue;
  auto& removedDefs = content["removedDefs"] = Json::arrayValue;

  for (auto key : keys2) {
    auto d1 = contentSnp1.GetDefinition(key);
    d1 = d1 ? d1 : "";
    auto d2 = contentSnp2.GetDefinition(key);
    d2 = d2 ? d2 : "";
    if (std::find(keys1.begin(), keys1.end(), key) != keys1.end() &&
        !strcmp(d1, d2))
      continue;
    Json::Value def = Json::objectValue;
    def["key"] = key;
    def["value"] = contentSnp2.GetDefinition(key);
    addedDefs.append(def);
  }

  for (auto key : keys1) {
    auto d1 = contentSnp1.GetDefinition(key);
    d1 = d1 ? d1 : "";
    auto d2 = contentSnp2.GetDefinition(key);
    d2 = d2 ? d2 : "";
    if (!strcmp(d1, d2))
      continue;
    Json::Value def = Json::objectValue;
    def["key"] = key;
    def["value"] = contentSnp1.GetDefinition(key);
    removedDefs.append(def);
  }

  this->Server->WriteResponse(obj);
}

void cmServerProtocol::ProcessCodeComplete(std::string filePath, long fileLine,
                                           long fileColumn,
                                           DifferentialFileContent diff)
{
  assert(fileLine > 0);

  auto res = GetSnapshotAndStartLine(filePath, fileLine, diff);
  if (res.second < 0) {
    Json::Value obj = Json::objectValue;
    obj["result"] = "no_completions";
    this->Server->WriteResponse(obj);
    return;
  }

  auto desired = GetDesiredSnapshot(diff.EditorLines, res.second, res.first,
                                    fileLine, true);
  cmState::Snapshot completionSnp = desired.first;
  if (!completionSnp.IsValid()) {
    Json::Value obj = Json::objectValue;
    obj["result"] = "no_completions";
    this->Server->WriteResponse(obj);
    return;
  }

  auto prParseStart = diff.EditorLines.begin() + res.second - 1;
  auto prParseEnd = diff.EditorLines.begin() + fileLine - 1;
  auto newToParse = std::distance(prParseStart, prParseEnd) + 1;

  auto theLine = *prParseEnd;

  std::string completionPrefix;

  auto columnData = theLine.substr(0, fileColumn);
  auto strt = columnData.find_first_not_of(' ');
  if (strt != std::string::npos) {
    completionPrefix = columnData.substr(strt);
  }

  cmServerCompleter completer(this->CMakeInstance, completionSnp);

  auto result = completer.Complete(completionSnp, desired.second,
                                   completionPrefix, newToParse, fileColumn);

  this->Server->WriteResponse(result);
}

void cmServerProtocol::ProcessContextWriters(std::string filePath,
                                             long fileLine, long fileColumn,
                                             DifferentialFileContent diff)
{
  assert(fileLine > 0);

  auto res = GetSnapshotAndStartLine(filePath, fileLine, diff);
  if (res.second < 0) {
    Json::Value obj = Json::objectValue;
    obj["result"] = "no_context";
    this->Server->WriteResponse(obj);
    return;
  }

  auto desired = GetDesiredSnapshot(diff.EditorLines, res.second, res.first,
                                    fileLine, true);
  cmState::Snapshot completionSnp = desired.first;
  if (!completionSnp.IsValid()) {
    Json::Value obj = Json::objectValue;
    obj["result"] = "no_context";
    this->Server->WriteResponse(obj);
    return;
  }

  auto prParseStart = diff.EditorLines.begin() + res.second - 1;
  auto prParseEnd = diff.EditorLines.begin() + fileLine - 1;
  auto newToParse = std::distance(prParseStart, prParseEnd) + 1;

  auto theLine = *prParseEnd;

  std::string completionPrefix;

  auto columnData = theLine.substr(0, fileColumn);

  auto strt = columnData.find_first_not_of(' ');
  if (strt != std::string::npos) {
    completionPrefix = columnData.substr(strt);
  }

  cmServerCompleter completer(this->CMakeInstance, completionSnp, true);

  auto result = completer.Complete(completionSnp, desired.second,
                                   completionPrefix, newToParse, fileColumn);

  if (!result.isMember("context_origin")) {
    Json::Value obj = Json::objectValue;
    obj["result"] = "no_context";
    this->Server->WriteResponse(obj);
    return;
  }

  if (!result["context_origin"].isMember("matcher")) {
    Json::Value obj = Json::objectValue;
    obj["result"] = "no_context";
    this->Server->WriteResponse(obj);
    return;
  }

  auto varName = result["context_origin"]["matcher"].asString();

  auto snps =
    this->CMakeInstance->GetState()->GetWriters(completionSnp, varName);

  if (snps.empty()) {
    Json::Value obj = Json::objectValue;
    obj["result"] = "no_context";
    this->Server->WriteResponse(obj);
    return;
  }

  cmState::Snapshot snp = snps.front();

  cmListFileContext lfc;
  lfc.FilePath = snp.GetExecutionListFile();
  lfc.Line = snp.GetStartingPoint();
  auto it = this->Snapshots.lower_bound(lfc);

  if (it == this->Snapshots.end()) {
    Json::Value obj = Json::objectValue;
    obj["result"] = "no_context";
    this->Server->WriteResponse(obj);
    return;
  }

  if (it->second.empty()) {
    Json::Value obj = Json::objectValue;
    obj["result"] = "no_context";
    this->Server->WriteResponse(obj);
    return;
  }

  ++it;

  Json::Value obj = Json::objectValue;
  obj["def_match"] = varName;
  obj["def_origin"] = (int)it->first.Line - 1;
  this->Server->WriteResponse(obj);
}
