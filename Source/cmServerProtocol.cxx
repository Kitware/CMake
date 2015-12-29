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

#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmServer.h"
#include "cmServerDiff.h"
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

void cmServerProtocol::processRequest(const std::string& json)
{
  Json::Reader reader;
  Json::Value value;
  reader.parse(json, value);

  if (this->Server->GetState() == cmMetadataServer::Started) {
    if (value["type"] == "handshake") {
      this->ProcessHandshake(value["protocolVersion"].asString());
    }
  }
  if (this->Server->GetState() == cmMetadataServer::ProcessingRequests) {
    if (value["type"] == "version") {
      this->ProcessVersion();
    }
    if (value["type"] == "buildsystem") {
      this->ProcessBuildsystem();
    }
    if (value["type"] == "target_info") {
      const char* language = 0;
      if (value.isMember("language")) {
        language = value["language"].asCString();
      }
      this->ProcessTargetInfo(value["target_name"].asString(),
                              value["config"].asString(), language);
    }
    if (value["type"] == "file_info") {
      this->ProcessFileInfo(value["target_name"].asString(),
                            value["config"].asString(),
                            value["file_path"].asString());
    }
    if (value["type"] == "content") {
      auto diff = cmServerDiff::GetDiff(value);
      this->ProcessContent(value["file_path"].asString(),
                           value["file_line"].asInt(), diff,
                           value["matcher"].asString());
    }
  }
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
    // Error;
    return;
  }

  const char* genName =
    this->CMakeInstance->GetState()->GetInitializedCacheValue(
      "CMAKE_GENERATOR");
  if (!genName) {
    // Error
    return;
  }

  const char* sourceDir =
    this->CMakeInstance->GetState()->GetInitializedCacheValue(
      "CMAKE_HOME_DIRECTORY");
  if (!sourceDir) {
    // Error
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
    // Error
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

void cmServerProtocol::ProcessTargetInfo(std::string tgtName,
                                         std::string config,
                                         const char* language)
{
  Json::Value obj = Json::objectValue;
  Json::Value& root = obj["target_info"] = Json::objectValue;

  auto tgt =
    this->CMakeInstance->GetGlobalGenerator()->FindGeneratorTarget(tgtName);

  if (!tgt) {
    // Error
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
    // Error
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
    // Error
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
