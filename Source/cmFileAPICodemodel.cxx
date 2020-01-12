/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmFileAPICodemodel.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cmext/algorithm>

#include "cm_jsoncpp_value.h"

#include "cmCryptoHash.h"
#include "cmFileAPI.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmInstallGenerator.h"
#include "cmInstallSubdirectoryGenerator.h"
#include "cmInstallTargetGenerator.h"
#include "cmLinkLineComputer.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmSourceGroup.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetDepend.h"
#include "cmake.h"

namespace {

class Codemodel
{
  cmFileAPI& FileAPI;
  unsigned long Version;

  Json::Value DumpPaths();
  Json::Value DumpConfigurations();
  Json::Value DumpConfiguration(std::string const& config);

public:
  Codemodel(cmFileAPI& fileAPI, unsigned long version);
  Json::Value Dump();
};

class CodemodelConfig
{
  cmFileAPI& FileAPI;
  unsigned long Version;
  std::string const& Config;
  std::string TopSource;
  std::string TopBuild;

  struct Directory
  {
    cmStateSnapshot Snapshot;
    cmLocalGenerator const* LocalGenerator = nullptr;
    Json::Value TargetIndexes = Json::arrayValue;
    Json::ArrayIndex ProjectIndex;
    bool HasInstallRule = false;
  };
  std::map<cmStateSnapshot, Json::ArrayIndex, cmStateSnapshot::StrictWeakOrder>
    DirectoryMap;
  std::vector<Directory> Directories;

  struct Project
  {
    cmStateSnapshot Snapshot;
    static const Json::ArrayIndex NoParentIndex =
      static_cast<Json::ArrayIndex>(-1);
    Json::ArrayIndex ParentIndex = NoParentIndex;
    Json::Value ChildIndexes = Json::arrayValue;
    Json::Value DirectoryIndexes = Json::arrayValue;
    Json::Value TargetIndexes = Json::arrayValue;
  };
  std::map<cmStateSnapshot, Json::ArrayIndex, cmStateSnapshot::StrictWeakOrder>
    ProjectMap;
  std::vector<Project> Projects;

  void ProcessDirectories();

  Json::ArrayIndex GetDirectoryIndex(cmLocalGenerator const* lg);
  Json::ArrayIndex GetDirectoryIndex(cmStateSnapshot s);

  Json::ArrayIndex AddProject(cmStateSnapshot s);

  Json::Value DumpTargets();
  Json::Value DumpTarget(cmGeneratorTarget* gt, Json::ArrayIndex ti);

  Json::Value DumpDirectories();
  Json::Value DumpDirectory(Directory& d);

  Json::Value DumpProjects();
  Json::Value DumpProject(Project& p);

  Json::Value DumpMinimumCMakeVersion(cmStateSnapshot s);

public:
  CodemodelConfig(cmFileAPI& fileAPI, unsigned long version,
                  std::string const& config);
  Json::Value Dump();
};

std::string RelativeIfUnder(std::string const& top, std::string const& in)
{
  std::string out;
  if (in == top) {
    out = ".";
  } else if (cmSystemTools::IsSubDirectory(in, top)) {
    out = in.substr(top.size() + 1);
  } else {
    out = in;
  }
  return out;
}

std::string TargetId(cmGeneratorTarget const* gt, std::string const& topBuild)
{
  cmCryptoHash hasher(cmCryptoHash::AlgoSHA3_256);
  std::string path = RelativeIfUnder(
    topBuild, gt->GetLocalGenerator()->GetCurrentBinaryDirectory());
  std::string hash = hasher.HashString(path);
  hash.resize(20, '0');
  return gt->GetName() + CMAKE_DIRECTORY_ID_SEP + hash;
}

class JBTIndex
{
public:
  JBTIndex() = default;
  explicit operator bool() const { return Index != None; }
  Json::ArrayIndex Index = None;
  static Json::ArrayIndex const None = static_cast<Json::ArrayIndex>(-1);
};

template <typename T>
class JBT
{
public:
  JBT(T v = T(), JBTIndex bt = JBTIndex())
    : Value(std::move(v))
    , Backtrace(bt)
  {
  }
  T Value;
  JBTIndex Backtrace;
  friend bool operator==(JBT<T> const& l, JBT<T> const& r)
  {
    return l.Value == r.Value && l.Backtrace.Index == r.Backtrace.Index;
  }
  static bool ValueEq(JBT<T> const& l, JBT<T> const& r)
  {
    return l.Value == r.Value;
  }
  static bool ValueLess(JBT<T> const& l, JBT<T> const& r)
  {
    return l.Value < r.Value;
  }
};

class BacktraceData
{
  std::string TopSource;
  std::unordered_map<std::string, Json::ArrayIndex> CommandMap;
  std::unordered_map<std::string, Json::ArrayIndex> FileMap;
  std::unordered_map<cmListFileContext const*, Json::ArrayIndex> NodeMap;
  Json::Value Commands = Json::arrayValue;
  Json::Value Files = Json::arrayValue;
  Json::Value Nodes = Json::arrayValue;

  Json::ArrayIndex AddCommand(std::string const& command)
  {
    auto i = this->CommandMap.find(command);
    if (i == this->CommandMap.end()) {
      auto cmdIndex = static_cast<Json::ArrayIndex>(this->Commands.size());
      i = this->CommandMap.emplace(command, cmdIndex).first;
      this->Commands.append(command);
    }
    return i->second;
  }

  Json::ArrayIndex AddFile(std::string const& file)
  {
    auto i = this->FileMap.find(file);
    if (i == this->FileMap.end()) {
      auto fileIndex = static_cast<Json::ArrayIndex>(this->Files.size());
      i = this->FileMap.emplace(file, fileIndex).first;
      this->Files.append(RelativeIfUnder(this->TopSource, file));
    }
    return i->second;
  }

public:
  BacktraceData(std::string topSource);
  JBTIndex Add(cmListFileBacktrace const& bt);
  Json::Value Dump();
};

BacktraceData::BacktraceData(std::string topSource)
  : TopSource(std::move(topSource))
{
}

JBTIndex BacktraceData::Add(cmListFileBacktrace const& bt)
{
  JBTIndex index;
  if (bt.Empty()) {
    return index;
  }
  cmListFileContext const* top = &bt.Top();
  auto found = this->NodeMap.find(top);
  if (found != this->NodeMap.end()) {
    index.Index = found->second;
    return index;
  }
  Json::Value entry = Json::objectValue;
  entry["file"] = this->AddFile(top->FilePath);
  if (top->Line) {
    entry["line"] = static_cast<int>(top->Line);
  }
  if (!top->Name.empty()) {
    entry["command"] = this->AddCommand(top->Name);
  }
  if (JBTIndex parent = this->Add(bt.Pop())) {
    entry["parent"] = parent.Index;
  }
  index.Index = this->NodeMap[top] = this->Nodes.size();
  this->Nodes.append(std::move(entry)); // NOLINT(*)
  return index;
}

Json::Value BacktraceData::Dump()
{
  Json::Value backtraceGraph;
  this->CommandMap.clear();
  this->FileMap.clear();
  this->NodeMap.clear();
  backtraceGraph["commands"] = std::move(this->Commands);
  backtraceGraph["files"] = std::move(this->Files);
  backtraceGraph["nodes"] = std::move(this->Nodes);
  return backtraceGraph;
}

struct CompileData
{
  struct IncludeEntry
  {
    JBT<std::string> Path;
    bool IsSystem = false;
    IncludeEntry(JBT<std::string> path, bool isSystem)
      : Path(std::move(path))
      , IsSystem(isSystem)
    {
    }
    friend bool operator==(IncludeEntry const& l, IncludeEntry const& r)
    {
      return l.Path == r.Path && l.IsSystem == r.IsSystem;
    }
  };

  std::string Language;
  std::string Sysroot;
  std::vector<JBT<std::string>> Flags;
  std::vector<JBT<std::string>> Defines;
  std::vector<IncludeEntry> Includes;

  friend bool operator==(CompileData const& l, CompileData const& r)
  {
    return (l.Language == r.Language && l.Sysroot == r.Sysroot &&
            l.Flags == r.Flags && l.Defines == r.Defines &&
            l.Includes == r.Includes);
  }
};
}

namespace std {

template <>
struct hash<CompileData>
{
  std::size_t operator()(CompileData const& in) const
  {
    using std::hash;
    size_t result =
      hash<std::string>()(in.Language) ^ hash<std::string>()(in.Sysroot);
    for (auto const& i : in.Includes) {
      result = result ^
        (hash<std::string>()(i.Path.Value) ^
         hash<Json::ArrayIndex>()(i.Path.Backtrace.Index) ^
         (i.IsSystem ? std::numeric_limits<size_t>::max() : 0));
    }
    for (auto const& i : in.Flags) {
      result = result ^ hash<std::string>()(i.Value) ^
        hash<Json::ArrayIndex>()(i.Backtrace.Index);
    }
    for (auto const& i : in.Defines) {
      result = result ^ hash<std::string>()(i.Value) ^
        hash<Json::ArrayIndex>()(i.Backtrace.Index);
    }
    return result;
  }
};

} // namespace std

namespace {
class Target
{
  cmGeneratorTarget* GT;
  std::string const& Config;
  std::string TopSource;
  std::string TopBuild;
  std::vector<cmSourceGroup> SourceGroupsLocal;
  BacktraceData Backtraces;

  std::map<std::string, CompileData> CompileDataMap;

  std::unordered_map<cmSourceFile const*, Json::ArrayIndex> SourceMap;
  Json::Value Sources = Json::arrayValue;

  struct SourceGroup
  {
    std::string Name;
    Json::Value SourceIndexes = Json::arrayValue;
  };
  std::unordered_map<cmSourceGroup const*, Json::ArrayIndex> SourceGroupsMap;
  std::vector<SourceGroup> SourceGroups;

  struct CompileGroup
  {
    std::unordered_map<CompileData, Json::ArrayIndex>::iterator Entry;
    Json::Value SourceIndexes = Json::arrayValue;
  };
  std::unordered_map<CompileData, Json::ArrayIndex> CompileGroupMap;
  std::vector<CompileGroup> CompileGroups;

  template <typename T>
  JBT<T> ToJBT(BT<T> const& bt)
  {
    return JBT<T>(bt.Value, this->Backtraces.Add(bt.Backtrace));
  }

  void ProcessLanguages();
  void ProcessLanguage(std::string const& lang);

  Json::ArrayIndex AddSourceGroup(cmSourceGroup* sg, Json::ArrayIndex si);
  CompileData BuildCompileData(cmSourceFile* sf);
  CompileData MergeCompileData(CompileData const& fd);
  Json::ArrayIndex AddSourceCompileGroup(cmSourceFile* sf,
                                         Json::ArrayIndex si);
  void AddBacktrace(Json::Value& object, cmListFileBacktrace const& bt);
  void AddBacktrace(Json::Value& object, JBTIndex bt);
  Json::Value DumpPaths();
  Json::Value DumpCompileData(CompileData const& cd);
  Json::Value DumpInclude(CompileData::IncludeEntry const& inc);
  Json::Value DumpDefine(JBT<std::string> const& def);
  Json::Value DumpSources();
  Json::Value DumpSource(cmGeneratorTarget::SourceAndKind const& sk,
                         Json::ArrayIndex si);
  Json::Value DumpSourceGroups();
  Json::Value DumpSourceGroup(SourceGroup& sg);
  Json::Value DumpCompileGroups();
  Json::Value DumpCompileGroup(CompileGroup& cg);
  Json::Value DumpSysroot(std::string const& path);
  Json::Value DumpInstall();
  Json::Value DumpInstallPrefix();
  Json::Value DumpInstallDestinations();
  Json::Value DumpInstallDestination(cmInstallTargetGenerator* itGen);
  Json::Value DumpArtifacts();
  Json::Value DumpLink();
  Json::Value DumpArchive();
  Json::Value DumpLinkCommandFragments();
  Json::Value DumpCommandFragments(std::vector<JBT<std::string>> const& frags);
  Json::Value DumpCommandFragment(JBT<std::string> const& frag,
                                  std::string const& role = std::string());
  Json::Value DumpDependencies();
  Json::Value DumpDependency(cmTargetDepend const& td);
  Json::Value DumpFolder();

public:
  Target(cmGeneratorTarget* gt, std::string const& config);
  Json::Value Dump();
};

Codemodel::Codemodel(cmFileAPI& fileAPI, unsigned long version)
  : FileAPI(fileAPI)
  , Version(version)
{
}

Json::Value Codemodel::Dump()
{
  Json::Value codemodel = Json::objectValue;

  codemodel["paths"] = this->DumpPaths();
  codemodel["configurations"] = this->DumpConfigurations();

  return codemodel;
}

Json::Value Codemodel::DumpPaths()
{
  Json::Value paths = Json::objectValue;
  paths["source"] = this->FileAPI.GetCMakeInstance()->GetHomeDirectory();
  paths["build"] = this->FileAPI.GetCMakeInstance()->GetHomeOutputDirectory();
  return paths;
}

Json::Value Codemodel::DumpConfigurations()
{
  Json::Value configurations = Json::arrayValue;
  cmGlobalGenerator* gg =
    this->FileAPI.GetCMakeInstance()->GetGlobalGenerator();
  const auto& makefiles = gg->GetMakefiles();
  if (!makefiles.empty()) {
    std::vector<std::string> const& configs =
      makefiles[0]->GetGeneratorConfigs();
    for (std::string const& config : configs) {
      configurations.append(this->DumpConfiguration(config));
    }
  }
  return configurations;
}

Json::Value Codemodel::DumpConfiguration(std::string const& config)
{
  CodemodelConfig configuration(this->FileAPI, this->Version, config);
  return configuration.Dump();
}

CodemodelConfig::CodemodelConfig(cmFileAPI& fileAPI, unsigned long version,
                                 std::string const& config)
  : FileAPI(fileAPI)
  , Version(version)
  , Config(config)
  , TopSource(this->FileAPI.GetCMakeInstance()->GetHomeDirectory())
  , TopBuild(this->FileAPI.GetCMakeInstance()->GetHomeOutputDirectory())
{
  static_cast<void>(this->Version);
}

Json::Value CodemodelConfig::Dump()
{
  Json::Value configuration = Json::objectValue;
  configuration["name"] = this->Config;
  this->ProcessDirectories();
  configuration["targets"] = this->DumpTargets();
  configuration["directories"] = this->DumpDirectories();
  configuration["projects"] = this->DumpProjects();
  return configuration;
}

void CodemodelConfig::ProcessDirectories()
{
  cmGlobalGenerator* gg =
    this->FileAPI.GetCMakeInstance()->GetGlobalGenerator();
  auto const& localGens = gg->GetLocalGenerators();

  // Add directories in forward order to process parents before children.
  this->Directories.reserve(localGens.size());
  for (const auto& lg : localGens) {
    auto directoryIndex =
      static_cast<Json::ArrayIndex>(this->Directories.size());
    this->Directories.emplace_back();
    Directory& d = this->Directories[directoryIndex];
    d.Snapshot = lg->GetStateSnapshot().GetBuildsystemDirectory();
    d.LocalGenerator = lg.get();
    this->DirectoryMap[d.Snapshot] = directoryIndex;

    d.ProjectIndex = this->AddProject(d.Snapshot);
    this->Projects[d.ProjectIndex].DirectoryIndexes.append(directoryIndex);
  }

  // Update directories in reverse order to process children before parents.
  for (auto di = this->Directories.rbegin(); di != this->Directories.rend();
       ++di) {
    Directory& d = *di;

    // Accumulate the presence of install rules on the way up.
    for (const auto& gen :
         d.LocalGenerator->GetMakefile()->GetInstallGenerators()) {
      if (!dynamic_cast<cmInstallSubdirectoryGenerator*>(gen.get())) {
        d.HasInstallRule = true;
        break;
      }
    }
    if (!d.HasInstallRule) {
      for (cmStateSnapshot const& child : d.Snapshot.GetChildren()) {
        cmStateSnapshot childDir = child.GetBuildsystemDirectory();
        Json::ArrayIndex const childIndex = this->GetDirectoryIndex(childDir);
        if (this->Directories[childIndex].HasInstallRule) {
          d.HasInstallRule = true;
          break;
        }
      }
    }
  }
}

Json::ArrayIndex CodemodelConfig::GetDirectoryIndex(cmLocalGenerator const* lg)
{
  return this->GetDirectoryIndex(
    lg->GetStateSnapshot().GetBuildsystemDirectory());
}

Json::ArrayIndex CodemodelConfig::GetDirectoryIndex(cmStateSnapshot s)
{
  auto i = this->DirectoryMap.find(s);
  assert(i != this->DirectoryMap.end());
  return i->second;
}

Json::ArrayIndex CodemodelConfig::AddProject(cmStateSnapshot s)
{
  cmStateSnapshot ps = s.GetBuildsystemDirectoryParent();
  if (ps.IsValid() && ps.GetProjectName() == s.GetProjectName()) {
    // This directory is part of its parent directory project.
    Json::ArrayIndex const parentDirIndex = this->GetDirectoryIndex(ps);
    return this->Directories[parentDirIndex].ProjectIndex;
  }

  // This directory starts a new project.
  auto projectIndex = static_cast<Json::ArrayIndex>(this->Projects.size());
  this->Projects.emplace_back();
  Project& p = this->Projects[projectIndex];
  p.Snapshot = s;
  this->ProjectMap[s] = projectIndex;
  if (ps.IsValid()) {
    Json::ArrayIndex const parentDirIndex = this->GetDirectoryIndex(ps);
    p.ParentIndex = this->Directories[parentDirIndex].ProjectIndex;
    this->Projects[p.ParentIndex].ChildIndexes.append(projectIndex);
  }
  return projectIndex;
}

Json::Value CodemodelConfig::DumpTargets()
{
  Json::Value targets = Json::arrayValue;

  std::vector<cmGeneratorTarget*> targetList;
  cmGlobalGenerator* gg =
    this->FileAPI.GetCMakeInstance()->GetGlobalGenerator();
  for (const auto& lg : gg->GetLocalGenerators()) {
    cm::append(targetList, lg->GetGeneratorTargets());
  }
  std::sort(targetList.begin(), targetList.end(),
            [](cmGeneratorTarget* l, cmGeneratorTarget* r) {
              return l->GetName() < r->GetName();
            });

  for (cmGeneratorTarget* gt : targetList) {
    if (gt->GetType() == cmStateEnums::GLOBAL_TARGET ||
        gt->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
      continue;
    }

    targets.append(this->DumpTarget(gt, targets.size()));
  }

  return targets;
}

Json::Value CodemodelConfig::DumpTarget(cmGeneratorTarget* gt,
                                        Json::ArrayIndex ti)
{
  Target t(gt, this->Config);
  std::string prefix = "target-" + gt->GetName();
  for (char& c : prefix) {
    // CMP0037 OLD behavior allows slashes in target names.  Remove them.
    if (c == '/' || c == '\\') {
      c = '_';
    }
  }
  if (!this->Config.empty()) {
    prefix += "-" + this->Config;
  }
  Json::Value target = this->FileAPI.MaybeJsonFile(t.Dump(), prefix);
  target["name"] = gt->GetName();
  target["id"] = TargetId(gt, this->TopBuild);

  // Cross-reference directory containing target.
  Json::ArrayIndex di = this->GetDirectoryIndex(gt->GetLocalGenerator());
  target["directoryIndex"] = di;
  this->Directories[di].TargetIndexes.append(ti);

  // Cross-reference project containing target.
  Json::ArrayIndex pi = this->Directories[di].ProjectIndex;
  target["projectIndex"] = pi;
  this->Projects[pi].TargetIndexes.append(ti);

  return target;
}

Json::Value CodemodelConfig::DumpDirectories()
{
  Json::Value directories = Json::arrayValue;
  for (Directory& d : this->Directories) {
    directories.append(this->DumpDirectory(d));
  }
  return directories;
}

Json::Value CodemodelConfig::DumpDirectory(Directory& d)
{
  Json::Value directory = Json::objectValue;

  std::string sourceDir = d.Snapshot.GetDirectory().GetCurrentSource();
  directory["source"] = RelativeIfUnder(this->TopSource, sourceDir);

  std::string buildDir = d.Snapshot.GetDirectory().GetCurrentBinary();
  directory["build"] = RelativeIfUnder(this->TopBuild, buildDir);

  cmStateSnapshot parentDir = d.Snapshot.GetBuildsystemDirectoryParent();
  if (parentDir.IsValid()) {
    directory["parentIndex"] = this->GetDirectoryIndex(parentDir);
  }

  Json::Value childIndexes = Json::arrayValue;
  for (cmStateSnapshot const& child : d.Snapshot.GetChildren()) {
    childIndexes.append(
      this->GetDirectoryIndex(child.GetBuildsystemDirectory()));
  }
  if (!childIndexes.empty()) {
    directory["childIndexes"] = std::move(childIndexes);
  }

  directory["projectIndex"] = d.ProjectIndex;

  if (!d.TargetIndexes.empty()) {
    directory["targetIndexes"] = std::move(d.TargetIndexes);
  }

  Json::Value minimumCMakeVersion = this->DumpMinimumCMakeVersion(d.Snapshot);
  if (!minimumCMakeVersion.isNull()) {
    directory["minimumCMakeVersion"] = std::move(minimumCMakeVersion);
  }

  if (d.HasInstallRule) {
    directory["hasInstallRule"] = true;
  }

  return directory;
}

Json::Value CodemodelConfig::DumpProjects()
{
  Json::Value projects = Json::arrayValue;
  for (Project& p : this->Projects) {
    projects.append(this->DumpProject(p));
  }
  return projects;
}

Json::Value CodemodelConfig::DumpProject(Project& p)
{
  Json::Value project = Json::objectValue;

  project["name"] = p.Snapshot.GetProjectName();

  if (p.ParentIndex != Project::NoParentIndex) {
    project["parentIndex"] = p.ParentIndex;
  }

  if (!p.ChildIndexes.empty()) {
    project["childIndexes"] = std::move(p.ChildIndexes);
  }

  project["directoryIndexes"] = std::move(p.DirectoryIndexes);

  if (!p.TargetIndexes.empty()) {
    project["targetIndexes"] = std::move(p.TargetIndexes);
  }

  return project;
}

Json::Value CodemodelConfig::DumpMinimumCMakeVersion(cmStateSnapshot s)
{
  Json::Value minimumCMakeVersion;
  if (std::string const* def =
        s.GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION")) {
    minimumCMakeVersion = Json::objectValue;
    minimumCMakeVersion["string"] = *def;
  }
  return minimumCMakeVersion;
}

Target::Target(cmGeneratorTarget* gt, std::string const& config)
  : GT(gt)
  , Config(config)
  , TopSource(gt->GetGlobalGenerator()->GetCMakeInstance()->GetHomeDirectory())
  , TopBuild(
      gt->GetGlobalGenerator()->GetCMakeInstance()->GetHomeOutputDirectory())
  , SourceGroupsLocal(this->GT->Makefile->GetSourceGroups())
  , Backtraces(this->TopSource)
{
}

Json::Value Target::Dump()
{
  Json::Value target = Json::objectValue;

  cmStateEnums::TargetType const type = this->GT->GetType();

  target["name"] = this->GT->GetName();
  target["type"] = cmState::GetTargetTypeName(type);
  target["id"] = TargetId(this->GT, this->TopBuild);
  target["paths"] = this->DumpPaths();
  if (this->GT->Target->GetIsGeneratorProvided()) {
    target["isGeneratorProvided"] = true;
  }

  this->AddBacktrace(target, this->GT->GetBacktrace());

  if (this->GT->Target->GetHaveInstallRule()) {
    target["install"] = this->DumpInstall();
  }

  if (this->GT->HaveWellDefinedOutputFiles()) {
    Json::Value artifacts = this->DumpArtifacts();
    if (!artifacts.empty()) {
      target["artifacts"] = std::move(artifacts);
    }
  }

  if (type == cmStateEnums::EXECUTABLE ||
      type == cmStateEnums::SHARED_LIBRARY ||
      type == cmStateEnums::MODULE_LIBRARY) {
    target["nameOnDisk"] = this->GT->GetFullName(this->Config);
    target["link"] = this->DumpLink();
  } else if (type == cmStateEnums::STATIC_LIBRARY) {
    target["nameOnDisk"] = this->GT->GetFullName(this->Config);
    target["archive"] = this->DumpArchive();
  }

  Json::Value dependencies = this->DumpDependencies();
  if (!dependencies.empty()) {
    target["dependencies"] = dependencies;
  }

  {
    this->ProcessLanguages();

    target["sources"] = this->DumpSources();

    Json::Value folder = this->DumpFolder();
    if (!folder.isNull()) {
      target["folder"] = std::move(folder);
    }

    Json::Value sourceGroups = this->DumpSourceGroups();
    if (!sourceGroups.empty()) {
      target["sourceGroups"] = std::move(sourceGroups);
    }

    Json::Value compileGroups = this->DumpCompileGroups();
    if (!compileGroups.empty()) {
      target["compileGroups"] = std::move(compileGroups);
    }
  }

  target["backtraceGraph"] = this->Backtraces.Dump();

  return target;
}

void Target::ProcessLanguages()
{
  std::set<std::string> languages;
  this->GT->GetLanguages(languages, this->Config);
  for (std::string const& lang : languages) {
    this->ProcessLanguage(lang);
  }
}

void Target::ProcessLanguage(std::string const& lang)
{
  CompileData& cd = this->CompileDataMap[lang];
  cd.Language = lang;
  if (const char* sysrootCompile =
        this->GT->Makefile->GetDefinition("CMAKE_SYSROOT_COMPILE")) {
    cd.Sysroot = sysrootCompile;
  } else if (const char* sysroot =
               this->GT->Makefile->GetDefinition("CMAKE_SYSROOT")) {
    cd.Sysroot = sysroot;
  }
  cmLocalGenerator* lg = this->GT->GetLocalGenerator();
  {
    // FIXME: Add flags from end section of ExpandRuleVariable,
    // which may need to be factored out.
    std::vector<BT<std::string>> flags =
      lg->GetTargetCompileFlags(this->GT, this->Config, lang);

    cd.Flags.reserve(flags.size());
    for (const BT<std::string>& f : flags) {
      cd.Flags.emplace_back(this->ToJBT(f));
    }
  }
  std::set<BT<std::string>> defines =
    lg->GetTargetDefines(this->GT, this->Config, lang);
  cd.Defines.reserve(defines.size());
  for (BT<std::string> const& d : defines) {
    cd.Defines.emplace_back(this->ToJBT(d));
  }
  std::vector<BT<std::string>> includePathList =
    lg->GetIncludeDirectories(this->GT, lang, this->Config);
  for (BT<std::string> const& i : includePathList) {
    cd.Includes.emplace_back(
      this->ToJBT(i),
      this->GT->IsSystemIncludeDirectory(i.Value, this->Config, lang));
  }
}

Json::ArrayIndex Target::AddSourceGroup(cmSourceGroup* sg, Json::ArrayIndex si)
{
  auto i = this->SourceGroupsMap.find(sg);
  if (i == this->SourceGroupsMap.end()) {
    auto sgIndex = static_cast<Json::ArrayIndex>(this->SourceGroups.size());
    i = this->SourceGroupsMap.emplace(sg, sgIndex).first;
    SourceGroup g;
    g.Name = sg->GetFullName();
    this->SourceGroups.push_back(std::move(g));
  }
  this->SourceGroups[i->second].SourceIndexes.append(si);
  return i->second;
}

CompileData Target::BuildCompileData(cmSourceFile* sf)
{
  CompileData fd;

  fd.Language = sf->GetOrDetermineLanguage();
  if (fd.Language.empty()) {
    return fd;
  }

  cmLocalGenerator* lg = this->GT->GetLocalGenerator();
  cmGeneratorExpressionInterpreter genexInterpreter(lg, this->Config, this->GT,
                                                    fd.Language);

  const std::string COMPILE_FLAGS("COMPILE_FLAGS");
  if (const char* cflags = sf->GetProperty(COMPILE_FLAGS)) {
    std::string flags = genexInterpreter.Evaluate(cflags, COMPILE_FLAGS);
    fd.Flags.emplace_back(std::move(flags), JBTIndex());
  }
  const std::string COMPILE_OPTIONS("COMPILE_OPTIONS");
  for (BT<std::string> tmpOpt : sf->GetCompileOptions()) {
    tmpOpt.Value = genexInterpreter.Evaluate(tmpOpt.Value, COMPILE_OPTIONS);
    // After generator evaluation we need to use the AppendCompileOptions
    // method so we handle situations where backtrace entries have lists
    // and properly escape flags.
    std::string tmp;
    lg->AppendCompileOptions(tmp, tmpOpt.Value);
    BT<std::string> opt(tmp, tmpOpt.Backtrace);
    fd.Flags.emplace_back(this->ToJBT(opt));
  }

  // Add precompile headers compile options.
  const std::string pchSource =
    this->GT->GetPchSource(this->Config, fd.Language);

  if (!pchSource.empty() && !sf->GetProperty("SKIP_PRECOMPILE_HEADERS")) {
    std::string pchOptions;
    if (sf->ResolveFullPath() == pchSource) {
      pchOptions =
        this->GT->GetPchCreateCompileOptions(this->Config, fd.Language);
    } else {
      pchOptions =
        this->GT->GetPchUseCompileOptions(this->Config, fd.Language);
    }

    BT<std::string> tmpOpt(pchOptions);
    tmpOpt.Value = genexInterpreter.Evaluate(tmpOpt.Value, COMPILE_OPTIONS);

    // After generator evaluation we need to use the AppendCompileOptions
    // method so we handle situations where backtrace entries have lists
    // and properly escape flags.
    std::string tmp;
    lg->AppendCompileOptions(tmp, tmpOpt.Value);
    BT<std::string> opt(tmp, tmpOpt.Backtrace);
    fd.Flags.emplace_back(this->ToJBT(opt));
  }

  // Add include directories from source file properties.
  {
    const std::string INCLUDE_DIRECTORIES("INCLUDE_DIRECTORIES");
    for (BT<std::string> tmpInclude : sf->GetIncludeDirectories()) {
      tmpInclude.Value =
        genexInterpreter.Evaluate(tmpInclude.Value, INCLUDE_DIRECTORIES);

      // After generator evaluation we need to use the AppendIncludeDirectories
      // method so we handle situations where backtrace entries have lists.
      std::vector<std::string> tmp;
      lg->AppendIncludeDirectories(tmp, tmpInclude.Value, *sf);
      for (std::string& i : tmp) {
        bool const isSystemInclude =
          this->GT->IsSystemIncludeDirectory(i, this->Config, fd.Language);
        BT<std::string> include(i, tmpInclude.Backtrace);
        fd.Includes.emplace_back(this->ToJBT(include), isSystemInclude);
      }
    }
  }

  const std::string COMPILE_DEFINITIONS("COMPILE_DEFINITIONS");
  std::set<BT<std::string>> fileDefines;
  for (BT<std::string> tmpDef : sf->GetCompileDefinitions()) {
    tmpDef.Value =
      genexInterpreter.Evaluate(tmpDef.Value, COMPILE_DEFINITIONS);

    // After generator evaluation we need to use the AppendDefines method
    // so we handle situations where backtrace entries have lists.
    std::set<std::string> tmp;
    lg->AppendDefines(tmp, tmpDef.Value);
    for (const std::string& i : tmp) {
      BT<std::string> def(i, tmpDef.Backtrace);
      fileDefines.insert(def);
    }
  }

  std::set<std::string> configFileDefines;
  const std::string defPropName =
    "COMPILE_DEFINITIONS_" + cmSystemTools::UpperCase(this->Config);
  if (const char* config_defs = sf->GetProperty(defPropName)) {
    lg->AppendDefines(
      configFileDefines,
      genexInterpreter.Evaluate(config_defs, COMPILE_DEFINITIONS));
  }

  fd.Defines.reserve(fileDefines.size() + configFileDefines.size());

  for (BT<std::string> const& def : fileDefines) {
    fd.Defines.emplace_back(this->ToJBT(def));
  }

  for (std::string const& d : configFileDefines) {
    fd.Defines.emplace_back(d, JBTIndex());
  }

  return fd;
}

CompileData Target::MergeCompileData(CompileData const& fd)
{
  CompileData cd;
  cd.Language = fd.Language;
  if (cd.Language.empty()) {
    return cd;
  }
  CompileData const& td = this->CompileDataMap.at(cd.Language);

  // All compile groups share the sysroot of the target.
  cd.Sysroot = td.Sysroot;

  // Use target-wide flags followed by source-specific flags.
  cd.Flags.reserve(td.Flags.size() + fd.Flags.size());
  cd.Flags.insert(cd.Flags.end(), td.Flags.begin(), td.Flags.end());
  cd.Flags.insert(cd.Flags.end(), fd.Flags.begin(), fd.Flags.end());

  // Use source-specific includes followed by target-wide includes.
  cd.Includes.reserve(fd.Includes.size() + td.Includes.size());
  cd.Includes.insert(cd.Includes.end(), fd.Includes.begin(),
                     fd.Includes.end());
  cd.Includes.insert(cd.Includes.end(), td.Includes.begin(),
                     td.Includes.end());

  // Use target-wide defines followed by source-specific defines.
  cd.Defines.reserve(td.Defines.size() + fd.Defines.size());
  cd.Defines.insert(cd.Defines.end(), td.Defines.begin(), td.Defines.end());
  cd.Defines.insert(cd.Defines.end(), fd.Defines.begin(), fd.Defines.end());

  // De-duplicate defines.
  std::stable_sort(cd.Defines.begin(), cd.Defines.end(),
                   JBT<std::string>::ValueLess);
  auto end = std::unique(cd.Defines.begin(), cd.Defines.end(),
                         JBT<std::string>::ValueEq);
  cd.Defines.erase(end, cd.Defines.end());

  return cd;
}

Json::ArrayIndex Target::AddSourceCompileGroup(cmSourceFile* sf,
                                               Json::ArrayIndex si)
{
  CompileData compileData = this->BuildCompileData(sf);
  auto i = this->CompileGroupMap.find(compileData);
  if (i == this->CompileGroupMap.end()) {
    Json::ArrayIndex cgIndex =
      static_cast<Json::ArrayIndex>(this->CompileGroups.size());
    i = this->CompileGroupMap.emplace(std::move(compileData), cgIndex).first;
    CompileGroup g;
    g.Entry = i;
    this->CompileGroups.push_back(std::move(g));
  }
  this->CompileGroups[i->second].SourceIndexes.append(si);
  return i->second;
}

void Target::AddBacktrace(Json::Value& object, cmListFileBacktrace const& bt)
{
  if (JBTIndex backtrace = this->Backtraces.Add(bt)) {
    object["backtrace"] = backtrace.Index;
  }
}

void Target::AddBacktrace(Json::Value& object, JBTIndex bt)
{
  if (bt) {
    object["backtrace"] = bt.Index;
  }
}

Json::Value Target::DumpPaths()
{
  Json::Value paths = Json::objectValue;
  cmLocalGenerator* lg = this->GT->GetLocalGenerator();

  std::string const& sourceDir = lg->GetCurrentSourceDirectory();
  paths["source"] = RelativeIfUnder(this->TopSource, sourceDir);

  std::string const& buildDir = lg->GetCurrentBinaryDirectory();
  paths["build"] = RelativeIfUnder(this->TopBuild, buildDir);

  return paths;
}

Json::Value Target::DumpSources()
{
  Json::Value sources = Json::arrayValue;
  cmGeneratorTarget::KindedSources const& kinded =
    this->GT->GetKindedSources(this->Config);
  for (cmGeneratorTarget::SourceAndKind const& sk : kinded.Sources) {
    sources.append(this->DumpSource(sk, sources.size()));
  }
  return sources;
}

Json::Value Target::DumpSource(cmGeneratorTarget::SourceAndKind const& sk,
                               Json::ArrayIndex si)
{
  Json::Value source = Json::objectValue;

  std::string const path = sk.Source.Value->ResolveFullPath();
  source["path"] = RelativeIfUnder(this->TopSource, path);
  if (sk.Source.Value->GetIsGenerated()) {
    source["isGenerated"] = true;
  }
  this->AddBacktrace(source, sk.Source.Backtrace);

  if (cmSourceGroup* sg =
        this->GT->Makefile->FindSourceGroup(path, this->SourceGroupsLocal)) {
    source["sourceGroupIndex"] = this->AddSourceGroup(sg, si);
  }

  switch (sk.Kind) {
    case cmGeneratorTarget::SourceKindObjectSource: {
      source["compileGroupIndex"] =
        this->AddSourceCompileGroup(sk.Source.Value, si);
    } break;
    case cmGeneratorTarget::SourceKindAppManifest:
    case cmGeneratorTarget::SourceKindCertificate:
    case cmGeneratorTarget::SourceKindCustomCommand:
    case cmGeneratorTarget::SourceKindExternalObject:
    case cmGeneratorTarget::SourceKindExtra:
    case cmGeneratorTarget::SourceKindHeader:
    case cmGeneratorTarget::SourceKindIDL:
    case cmGeneratorTarget::SourceKindManifest:
    case cmGeneratorTarget::SourceKindModuleDefinition:
    case cmGeneratorTarget::SourceKindResx:
    case cmGeneratorTarget::SourceKindXaml:
    case cmGeneratorTarget::SourceKindUnityBatched:
      break;
  }

  return source;
}

Json::Value Target::DumpCompileData(CompileData const& cd)
{
  Json::Value result = Json::objectValue;

  if (!cd.Language.empty()) {
    result["language"] = cd.Language;
  }
  if (!cd.Sysroot.empty()) {
    result["sysroot"] = this->DumpSysroot(cd.Sysroot);
  }
  if (!cd.Flags.empty()) {
    result["compileCommandFragments"] = this->DumpCommandFragments(cd.Flags);
  }
  if (!cd.Includes.empty()) {
    Json::Value includes = Json::arrayValue;
    for (auto const& i : cd.Includes) {
      includes.append(this->DumpInclude(i));
    }
    result["includes"] = includes;
  }
  if (!cd.Defines.empty()) {
    Json::Value defines = Json::arrayValue;
    for (JBT<std::string> const& d : cd.Defines) {
      defines.append(this->DumpDefine(d));
    }
    result["defines"] = std::move(defines);
  }

  return result;
}

Json::Value Target::DumpInclude(CompileData::IncludeEntry const& inc)
{
  Json::Value include = Json::objectValue;
  include["path"] = inc.Path.Value;
  if (inc.IsSystem) {
    include["isSystem"] = true;
  }
  this->AddBacktrace(include, inc.Path.Backtrace);
  return include;
}

Json::Value Target::DumpDefine(JBT<std::string> const& def)
{
  Json::Value define = Json::objectValue;
  define["define"] = def.Value;
  this->AddBacktrace(define, def.Backtrace);
  return define;
}

Json::Value Target::DumpSourceGroups()
{
  Json::Value sourceGroups = Json::arrayValue;
  for (auto& sg : this->SourceGroups) {
    sourceGroups.append(this->DumpSourceGroup(sg));
  }
  return sourceGroups;
}

Json::Value Target::DumpSourceGroup(SourceGroup& sg)
{
  Json::Value group = Json::objectValue;
  group["name"] = sg.Name;
  group["sourceIndexes"] = std::move(sg.SourceIndexes);
  return group;
}

Json::Value Target::DumpCompileGroups()
{
  Json::Value compileGroups = Json::arrayValue;
  for (auto& cg : this->CompileGroups) {
    compileGroups.append(this->DumpCompileGroup(cg));
  }
  return compileGroups;
}

Json::Value Target::DumpCompileGroup(CompileGroup& cg)
{
  Json::Value group =
    this->DumpCompileData(this->MergeCompileData(cg.Entry->first));
  group["sourceIndexes"] = std::move(cg.SourceIndexes);
  return group;
}

Json::Value Target::DumpSysroot(std::string const& path)
{
  Json::Value sysroot = Json::objectValue;
  sysroot["path"] = path;
  return sysroot;
}

Json::Value Target::DumpInstall()
{
  Json::Value install = Json::objectValue;
  install["prefix"] = this->DumpInstallPrefix();
  install["destinations"] = this->DumpInstallDestinations();
  return install;
}

Json::Value Target::DumpInstallPrefix()
{
  Json::Value prefix = Json::objectValue;
  std::string p =
    this->GT->Makefile->GetSafeDefinition("CMAKE_INSTALL_PREFIX");
  cmSystemTools::ConvertToUnixSlashes(p);
  prefix["path"] = p;
  return prefix;
}

Json::Value Target::DumpInstallDestinations()
{
  Json::Value destinations = Json::arrayValue;
  auto installGens = this->GT->Target->GetInstallGenerators();
  for (auto itGen : installGens) {
    destinations.append(this->DumpInstallDestination(itGen));
  }
  return destinations;
}

Json::Value Target::DumpInstallDestination(cmInstallTargetGenerator* itGen)
{
  Json::Value destination = Json::objectValue;
  destination["path"] = itGen->GetDestination(this->Config);
  this->AddBacktrace(destination, itGen->GetBacktrace());
  return destination;
}

Json::Value Target::DumpArtifacts()
{
  Json::Value artifacts = Json::arrayValue;

  // Object libraries have only object files as artifacts.
  if (this->GT->GetType() == cmStateEnums::OBJECT_LIBRARY) {
    if (!this->GT->GetGlobalGenerator()->HasKnownObjectFileLocation(nullptr)) {
      return artifacts;
    }
    std::vector<cmSourceFile const*> objectSources;
    this->GT->GetObjectSources(objectSources, this->Config);
    std::string const obj_dir = this->GT->GetObjectDirectory(this->Config);
    for (cmSourceFile const* sf : objectSources) {
      const std::string& obj = this->GT->GetObjectName(sf);
      Json::Value artifact = Json::objectValue;
      artifact["path"] = RelativeIfUnder(this->TopBuild, obj_dir + obj);
      artifacts.append(std::move(artifact)); // NOLINT(*)
    }
    return artifacts;
  }

  // Other target types always have a "main" artifact.
  {
    Json::Value artifact = Json::objectValue;
    artifact["path"] =
      RelativeIfUnder(this->TopBuild,
                      this->GT->GetFullPath(
                        this->Config, cmStateEnums::RuntimeBinaryArtifact));
    artifacts.append(std::move(artifact)); // NOLINT(*)
  }

  // Add Windows-specific artifacts produced by the linker.
  if (this->GT->HasImportLibrary(this->Config)) {
    Json::Value artifact = Json::objectValue;
    artifact["path"] =
      RelativeIfUnder(this->TopBuild,
                      this->GT->GetFullPath(
                        this->Config, cmStateEnums::ImportLibraryArtifact));
    artifacts.append(std::move(artifact)); // NOLINT(*)
  }
  if (this->GT->IsDLLPlatform() &&
      this->GT->GetType() != cmStateEnums::STATIC_LIBRARY) {
    cmGeneratorTarget::OutputInfo const* output =
      this->GT->GetOutputInfo(this->Config);
    if (output && !output->PdbDir.empty()) {
      Json::Value artifact = Json::objectValue;
      artifact["path"] = RelativeIfUnder(this->TopBuild,
                                         output->PdbDir + '/' +
                                           this->GT->GetPDBName(this->Config));
      artifacts.append(std::move(artifact)); // NOLINT(*)
    }
  }
  return artifacts;
}

Json::Value Target::DumpLink()
{
  Json::Value link = Json::objectValue;
  std::string lang = this->GT->GetLinkerLanguage(this->Config);
  link["language"] = lang;
  {
    Json::Value commandFragments = this->DumpLinkCommandFragments();
    if (!commandFragments.empty()) {
      link["commandFragments"] = std::move(commandFragments);
    }
  }
  if (const char* sysrootLink =
        this->GT->Makefile->GetDefinition("CMAKE_SYSROOT_LINK")) {
    link["sysroot"] = this->DumpSysroot(sysrootLink);
  } else if (const char* sysroot =
               this->GT->Makefile->GetDefinition("CMAKE_SYSROOT")) {
    link["sysroot"] = this->DumpSysroot(sysroot);
  }
  if (this->GT->IsIPOEnabled(lang, this->Config)) {
    link["lto"] = true;
  }
  return link;
}

Json::Value Target::DumpArchive()
{
  Json::Value archive = Json::objectValue;
  {
    // The "link" fragments not relevant to static libraries are empty.
    Json::Value commandFragments = this->DumpLinkCommandFragments();
    if (!commandFragments.empty()) {
      archive["commandFragments"] = std::move(commandFragments);
    }
  }
  std::string lang = this->GT->GetLinkerLanguage(this->Config);
  if (this->GT->IsIPOEnabled(lang, this->Config)) {
    archive["lto"] = true;
  }
  return archive;
}

Json::Value Target::DumpLinkCommandFragments()
{
  Json::Value linkFragments = Json::arrayValue;

  std::string linkLanguageFlags;
  std::vector<BT<std::string>> linkFlags;
  std::string frameworkPath;
  std::vector<BT<std::string>> linkPath;
  std::vector<BT<std::string>> linkLibs;
  cmLocalGenerator* lg = this->GT->GetLocalGenerator();
  cmLinkLineComputer linkLineComputer(lg,
                                      lg->GetStateSnapshot().GetDirectory());
  lg->GetTargetFlags(&linkLineComputer, this->Config, linkLibs,
                     linkLanguageFlags, linkFlags, frameworkPath, linkPath,
                     this->GT);
  linkLanguageFlags = cmTrimWhitespace(linkLanguageFlags);
  frameworkPath = cmTrimWhitespace(frameworkPath);

  if (!linkLanguageFlags.empty()) {
    linkFragments.append(
      this->DumpCommandFragment(std::move(linkLanguageFlags), "flags"));
  }

  if (!linkFlags.empty()) {
    for (BT<std::string> frag : linkFlags) {
      frag.Value = cmTrimWhitespace(frag.Value);
      linkFragments.append(
        this->DumpCommandFragment(this->ToJBT(frag), "flags"));
    }
  }

  if (!frameworkPath.empty()) {
    linkFragments.append(
      this->DumpCommandFragment(std::move(frameworkPath), "frameworkPath"));
  }

  if (!linkPath.empty()) {
    for (BT<std::string> frag : linkPath) {
      frag.Value = cmTrimWhitespace(frag.Value);
      linkFragments.append(
        this->DumpCommandFragment(this->ToJBT(frag), "libraryPath"));
    }
  }

  if (!linkLibs.empty()) {
    for (BT<std::string> frag : linkLibs) {
      frag.Value = cmTrimWhitespace(frag.Value);
      linkFragments.append(
        this->DumpCommandFragment(this->ToJBT(frag), "libraries"));
    }
  }

  return linkFragments;
}

Json::Value Target::DumpCommandFragments(
  std::vector<JBT<std::string>> const& frags)
{
  Json::Value commandFragments = Json::arrayValue;
  for (JBT<std::string> const& f : frags) {
    commandFragments.append(this->DumpCommandFragment(f));
  }
  return commandFragments;
}

Json::Value Target::DumpCommandFragment(JBT<std::string> const& frag,
                                        std::string const& role)
{
  Json::Value fragment = Json::objectValue;
  fragment["fragment"] = frag.Value;
  if (!role.empty()) {
    fragment["role"] = role;
  }
  this->AddBacktrace(fragment, frag.Backtrace);
  return fragment;
}

Json::Value Target::DumpDependencies()
{
  Json::Value dependencies = Json::arrayValue;
  cmGlobalGenerator* gg = this->GT->GetGlobalGenerator();
  for (cmTargetDepend const& td : gg->GetTargetDirectDepends(this->GT)) {
    dependencies.append(this->DumpDependency(td));
  }
  return dependencies;
}

Json::Value Target::DumpDependency(cmTargetDepend const& td)
{
  Json::Value dependency = Json::objectValue;
  dependency["id"] = TargetId(td, this->TopBuild);
  this->AddBacktrace(dependency, td.GetBacktrace());
  return dependency;
}

Json::Value Target::DumpFolder()
{
  Json::Value folder;
  if (const char* f = this->GT->GetProperty("FOLDER")) {
    folder = Json::objectValue;
    folder["name"] = f;
  }
  return folder;
}
}

Json::Value cmFileAPICodemodelDump(cmFileAPI& fileAPI, unsigned long version)
{
  Codemodel codemodel(fileAPI, version);
  return codemodel.Dump();
}
