/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmTarget.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmComputeLinkInformation.h"
#include "cmListFileCache.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include <cmsys/RegularExpression.hxx>
#include <map>
#include <set>
#include <queue>
#include <stdlib.h> // required for atof
#include <assert.h>

const char* cmTarget::GetTargetTypeName(TargetType targetType)
{
  switch( targetType )
    {
      case cmTarget::STATIC_LIBRARY:
        return "STATIC_LIBRARY";
      case cmTarget::MODULE_LIBRARY:
        return "MODULE_LIBRARY";
      case cmTarget::SHARED_LIBRARY:
        return "SHARED_LIBRARY";
      case cmTarget::OBJECT_LIBRARY:
        return "OBJECT_LIBRARY";
      case cmTarget::EXECUTABLE:
        return "EXECUTABLE";
      case cmTarget::UTILITY:
        return "UTILITY";
      case cmTarget::GLOBAL_TARGET:
        return "GLOBAL_TARGET";
      case cmTarget::INTERFACE_LIBRARY:
        return "INTERFACE_LIBRARY";
      case cmTarget::UNKNOWN_LIBRARY:
        return "UNKNOWN_LIBRARY";
    }
  assert(0 && "Unexpected target type");
  return 0;
}

//----------------------------------------------------------------------------
struct cmTarget::OutputInfo
{
  std::string OutDir;
  std::string ImpDir;
  std::string PdbDir;
};

//----------------------------------------------------------------------------
struct cmTarget::ImportInfo
{
  bool NoSOName;
  std::string Location;
  std::string SOName;
  std::string ImportLibrary;
  cmTarget::LinkInterface LinkInterface;
};

struct TargetConfigPair : public std::pair<cmTarget*, std::string> {
  TargetConfigPair(cmTarget* tgt, const std::string &config)
    : std::pair<cmTarget*, std::string>(tgt, config) {}
};

//----------------------------------------------------------------------------
class cmTargetInternals
{
public:
  cmTargetInternals()
    {
    this->SourceFileFlagsConstructed = false;
    }
  cmTargetInternals(cmTargetInternals const& r)
    {
    this->SourceFileFlagsConstructed = false;
    // Only some of these entries are part of the object state.
    // Others not copied here are result caches.
    this->SourceEntries = r.SourceEntries;
    }
  ~cmTargetInternals();
  typedef cmTarget::SourceFileFlags SourceFileFlags;
  std::map<cmSourceFile const*, SourceFileFlags> SourceFlagsMap;
  bool SourceFileFlagsConstructed;

  // The backtrace when the target was created.
  cmListFileBacktrace Backtrace;

  // Cache link interface computation from each configuration.
  struct OptionalLinkInterface: public cmTarget::LinkInterface
  {
    OptionalLinkInterface(): Exists(false) {}
    bool Exists;
  };
  typedef std::map<TargetConfigPair, OptionalLinkInterface>
                                                          LinkInterfaceMapType;
  LinkInterfaceMapType LinkInterfaceMap;

  typedef std::map<cmStdString, cmTarget::OutputInfo> OutputInfoMapType;
  OutputInfoMapType OutputInfoMap;

  typedef std::map<TargetConfigPair, cmTarget::ImportInfo>
                                                            ImportInfoMapType;
  ImportInfoMapType ImportInfoMap;

  // Cache link implementation computation from each configuration.
  typedef std::map<TargetConfigPair,
                   cmTarget::LinkImplementation> LinkImplMapType;
  LinkImplMapType LinkImplMap;

  typedef std::map<TargetConfigPair, cmTarget::LinkClosure>
                                                          LinkClosureMapType;
  LinkClosureMapType LinkClosureMap;

  struct SourceEntry { std::vector<cmSourceFile*> Depends; };
  typedef std::map<cmSourceFile*, SourceEntry> SourceEntriesType;
  SourceEntriesType SourceEntries;

  struct TargetPropertyEntry {
    TargetPropertyEntry(cmsys::auto_ptr<cmCompiledGeneratorExpression> cge,
      const std::string &targetName = std::string())
      : ge(cge), TargetName(targetName)
    {}
    const cmsys::auto_ptr<cmCompiledGeneratorExpression> ge;
    std::vector<std::string> CachedEntries;
    const std::string TargetName;
  };
  std::vector<TargetPropertyEntry*> IncludeDirectoriesEntries;
  std::vector<TargetPropertyEntry*> CompileOptionsEntries;
  std::vector<TargetPropertyEntry*> CompileDefinitionsEntries;
  std::vector<cmValueWithOrigin> LinkInterfacePropertyEntries;

  std::map<std::string, std::vector<TargetPropertyEntry*> >
                                CachedLinkInterfaceIncludeDirectoriesEntries;
  std::map<std::string, std::vector<TargetPropertyEntry*> >
                                CachedLinkInterfaceCompileOptionsEntries;
  std::map<std::string, std::vector<TargetPropertyEntry*> >
                                CachedLinkInterfaceCompileDefinitionsEntries;

  std::map<std::string, bool> CacheLinkInterfaceIncludeDirectoriesDone;
  std::map<std::string, bool> CacheLinkInterfaceCompileDefinitionsDone;
  std::map<std::string, bool> CacheLinkInterfaceCompileOptionsDone;
};

//----------------------------------------------------------------------------
void deleteAndClear(
      std::vector<cmTargetInternals::TargetPropertyEntry*> &entries)
{
  for (std::vector<cmTargetInternals::TargetPropertyEntry*>::const_iterator
      it = entries.begin(),
      end = entries.end();
      it != end; ++it)
    {
      delete *it;
    }
  entries.clear();
}

//----------------------------------------------------------------------------
void deleteAndClear(
  std::map<std::string,
          std::vector<cmTargetInternals::TargetPropertyEntry*> > &entries)
{
  for (std::map<std::string,
          std::vector<cmTargetInternals::TargetPropertyEntry*> >::iterator
        it = entries.begin(), end = entries.end(); it != end; ++it)
    {
    deleteAndClear(it->second);
    }
}

//----------------------------------------------------------------------------
cmTargetInternals::~cmTargetInternals()
{
  deleteAndClear(this->CachedLinkInterfaceIncludeDirectoriesEntries);
  deleteAndClear(this->CachedLinkInterfaceCompileOptionsEntries);
  deleteAndClear(this->CachedLinkInterfaceCompileDefinitionsEntries);
}

//----------------------------------------------------------------------------
cmTarget::cmTarget()
{
#define INITIALIZE_TARGET_POLICY_MEMBER(POLICY) \
  this->PolicyStatus ## POLICY = cmPolicies::WARN;

  CM_FOR_EACH_TARGET_POLICY(INITIALIZE_TARGET_POLICY_MEMBER)

#undef INITIALIZE_TARGET_POLICY_MEMBER

  this->Makefile = 0;
  this->LinkLibrariesAnalyzed = false;
  this->HaveInstallRule = false;
  this->DLLPlatform = false;
  this->IsApple = false;
  this->IsImportedTarget = false;
  this->BuildInterfaceIncludesAppended = false;
  this->DebugIncludesDone = false;
  this->DebugCompileOptionsDone = false;
  this->DebugCompileDefinitionsDone = false;
}

//----------------------------------------------------------------------------
void cmTarget::DefineProperties(cmake *cm)
{
  cm->DefineProperty
    ("RULE_LAUNCH_COMPILE", cmProperty::TARGET,
     "", "", true);
  cm->DefineProperty
    ("RULE_LAUNCH_LINK", cmProperty::TARGET,
     "", "", true);
  cm->DefineProperty
    ("RULE_LAUNCH_CUSTOM", cmProperty::TARGET,
     "", "", true);
}

void cmTarget::SetType(TargetType type, const char* name)
{
  this->Name = name;
  // only add dependency information for library targets
  this->TargetTypeValue = type;
  if(this->TargetTypeValue >= STATIC_LIBRARY
     && this->TargetTypeValue <= MODULE_LIBRARY)
    {
    this->RecordDependencies = true;
    }
  else
    {
    this->RecordDependencies = false;
    }
}

//----------------------------------------------------------------------------
void cmTarget::SetMakefile(cmMakefile* mf)
{
  // Set our makefile.
  this->Makefile = mf;

  // set the cmake instance of the properties
  this->Properties.SetCMakeInstance(mf->GetCMakeInstance());

  // Check whether this is a DLL platform.
  this->DLLPlatform = (this->Makefile->IsOn("WIN32") ||
                       this->Makefile->IsOn("CYGWIN") ||
                       this->Makefile->IsOn("MINGW"));

  // Check whether we are targeting an Apple platform.
  this->IsApple = this->Makefile->IsOn("APPLE");

  // Setup default property values.
  this->SetPropertyDefault("INSTALL_NAME_DIR", 0);
  this->SetPropertyDefault("INSTALL_RPATH", "");
  this->SetPropertyDefault("INSTALL_RPATH_USE_LINK_PATH", "OFF");
  this->SetPropertyDefault("SKIP_BUILD_RPATH", "OFF");
  this->SetPropertyDefault("BUILD_WITH_INSTALL_RPATH", "OFF");
  this->SetPropertyDefault("ARCHIVE_OUTPUT_DIRECTORY", 0);
  this->SetPropertyDefault("LIBRARY_OUTPUT_DIRECTORY", 0);
  this->SetPropertyDefault("RUNTIME_OUTPUT_DIRECTORY", 0);
  this->SetPropertyDefault("PDB_OUTPUT_DIRECTORY", 0);
  this->SetPropertyDefault("Fortran_FORMAT", 0);
  this->SetPropertyDefault("Fortran_MODULE_DIRECTORY", 0);
  this->SetPropertyDefault("GNUtoMS", 0);
  this->SetPropertyDefault("OSX_ARCHITECTURES", 0);
  this->SetPropertyDefault("AUTOMOC", 0);
  this->SetPropertyDefault("AUTOMOC_MOC_OPTIONS", 0);
  this->SetPropertyDefault("LINK_DEPENDS_NO_SHARED", 0);
  this->SetPropertyDefault("LINK_INTERFACE_LIBRARIES", 0);
  this->SetPropertyDefault("WIN32_EXECUTABLE", 0);
  this->SetPropertyDefault("MACOSX_BUNDLE", 0);
  this->SetPropertyDefault("MACOSX_RPATH", 0);
  this->SetPropertyDefault("NO_SYSTEM_FROM_IMPORTED", 0);


  // Collect the set of configuration types.
  std::vector<std::string> configNames;
  mf->GetConfigurations(configNames);

  // Setup per-configuration property default values.
  const char* configProps[] = {
    "ARCHIVE_OUTPUT_DIRECTORY_",
    "LIBRARY_OUTPUT_DIRECTORY_",
    "RUNTIME_OUTPUT_DIRECTORY_",
    "PDB_OUTPUT_DIRECTORY_",
    "MAP_IMPORTED_CONFIG_",
    0};
  for(std::vector<std::string>::iterator ci = configNames.begin();
      ci != configNames.end(); ++ci)
    {
    std::string configUpper = cmSystemTools::UpperCase(*ci);
    for(const char** p = configProps; *p; ++p)
      {
      std::string property = *p;
      property += configUpper;
      this->SetPropertyDefault(property.c_str(), 0);
      }

    // Initialize per-configuration name postfix property from the
    // variable only for non-executable targets.  This preserves
    // compatibility with previous CMake versions in which executables
    // did not support this variable.  Projects may still specify the
    // property directly.  TODO: Make this depend on backwards
    // compatibility setting.
    if(this->TargetTypeValue != cmTarget::EXECUTABLE)
      {
      std::string property = cmSystemTools::UpperCase(*ci);
      property += "_POSTFIX";
      this->SetPropertyDefault(property.c_str(), 0);
      }
    }

  // Save the backtrace of target construction.
  this->Makefile->GetBacktrace(this->Internal->Backtrace);

  // Initialize the INCLUDE_DIRECTORIES property based on the current value
  // of the same directory property:
  const std::vector<cmValueWithOrigin> parentIncludes =
                              this->Makefile->GetIncludeDirectoriesEntries();

  for (std::vector<cmValueWithOrigin>::const_iterator it
              = parentIncludes.begin(); it != parentIncludes.end(); ++it)
    {
    this->InsertInclude(*it);
    }

  const std::set<cmStdString> parentSystemIncludes =
                              this->Makefile->GetSystemIncludeDirectories();

  for (std::set<cmStdString>::const_iterator it
        = parentSystemIncludes.begin();
        it != parentSystemIncludes.end(); ++it)
    {
    this->SystemIncludeDirectories.insert(*it);
    }

  const std::vector<cmValueWithOrigin> parentOptions =
                              this->Makefile->GetCompileOptionsEntries();

  for (std::vector<cmValueWithOrigin>::const_iterator it
              = parentOptions.begin(); it != parentOptions.end(); ++it)
    {
    this->InsertCompileOption(*it);
    }

  this->SetPropertyDefault("C_VISIBILITY_PRESET", 0);
  this->SetPropertyDefault("CXX_VISIBILITY_PRESET", 0);
  this->SetPropertyDefault("VISIBILITY_INLINES_HIDDEN", 0);

  if(this->TargetTypeValue == cmTarget::SHARED_LIBRARY
      || this->TargetTypeValue == cmTarget::MODULE_LIBRARY)
    {
    this->SetProperty("POSITION_INDEPENDENT_CODE", "True");
    }
  this->SetPropertyDefault("POSITION_INDEPENDENT_CODE", 0);

  // Record current policies for later use.
#define CAPTURE_TARGET_POLICY(POLICY) \
  this->PolicyStatus ## POLICY = \
    this->Makefile->GetPolicyStatus(cmPolicies::POLICY);

  CM_FOR_EACH_TARGET_POLICY(CAPTURE_TARGET_POLICY)

#undef CAPTURE_TARGET_POLICY

  if (this->TargetTypeValue == INTERFACE_LIBRARY)
    {
    // This policy is checked in a few conditions. The properties relevant
    // to the policy are always ignored for INTERFACE_LIBRARY targets,
    // so ensure that the conditions don't lead to nonsense.
    this->PolicyStatusCMP0022 = cmPolicies::NEW;
    }
}

//----------------------------------------------------------------------------
void cmTarget::FinishConfigure()
{
  // Erase any cached link information that might have been comptued
  // on-demand during the configuration.  This ensures that build
  // system generation uses up-to-date information even if other cache
  // invalidation code in this source file is buggy.
  this->ClearLinkMaps();

  // Do old-style link dependency analysis.
  this->AnalyzeLibDependencies(*this->Makefile);
}

//----------------------------------------------------------------------------
void cmTarget::ClearLinkMaps()
{
  this->Internal->LinkImplMap.clear();
  this->Internal->LinkInterfaceMap.clear();
  this->Internal->LinkClosureMap.clear();
  for (cmTargetLinkInformationMap::const_iterator it
      = this->LinkInformation.begin();
      it != this->LinkInformation.end(); ++it)
    {
    delete it->second;
    }
  this->LinkInformation.clear();
}

//----------------------------------------------------------------------------
cmListFileBacktrace const& cmTarget::GetBacktrace() const
{
  return this->Internal->Backtrace;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetSupportDirectory() const
{
  std::string dir = this->Makefile->GetCurrentOutputDirectory();
  dir += cmake::GetCMakeFilesDirectory();
  dir += "/";
  dir += this->Name;
#if defined(__VMS)
  dir += "_dir";
#else
  dir += ".dir";
#endif
  return dir;
}

//----------------------------------------------------------------------------
bool cmTarget::IsExecutableWithExports()
{
  return (this->GetType() == cmTarget::EXECUTABLE &&
          this->GetPropertyAsBool("ENABLE_EXPORTS"));
}

//----------------------------------------------------------------------------
bool cmTarget::IsLinkable()
{
  return (this->GetType() == cmTarget::STATIC_LIBRARY ||
          this->GetType() == cmTarget::SHARED_LIBRARY ||
          this->GetType() == cmTarget::MODULE_LIBRARY ||
          this->GetType() == cmTarget::UNKNOWN_LIBRARY ||
          this->GetType() == cmTarget::INTERFACE_LIBRARY ||
          this->IsExecutableWithExports());
}

//----------------------------------------------------------------------------
bool cmTarget::HasImportLibrary()
{
  return (this->DLLPlatform &&
          (this->GetType() == cmTarget::SHARED_LIBRARY ||
           this->IsExecutableWithExports()));
}

//----------------------------------------------------------------------------
bool cmTarget::IsFrameworkOnApple()
{
  return (this->GetType() == cmTarget::SHARED_LIBRARY &&
          this->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("FRAMEWORK"));
}

//----------------------------------------------------------------------------
bool cmTarget::IsAppBundleOnApple()
{
  return (this->GetType() == cmTarget::EXECUTABLE &&
          this->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("MACOSX_BUNDLE"));
}

//----------------------------------------------------------------------------
bool cmTarget::IsCFBundleOnApple()
{
  return (this->GetType() == cmTarget::MODULE_LIBRARY &&
          this->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("BUNDLE"));
}

//----------------------------------------------------------------------------
bool cmTarget::IsBundleOnApple()
{
  return this->IsFrameworkOnApple() || this->IsAppBundleOnApple() ||
         this->IsCFBundleOnApple();
}

//----------------------------------------------------------------------------
class cmTargetTraceDependencies
{
public:
  cmTargetTraceDependencies(cmTarget* target, cmTargetInternals* internal);
  void Trace();
private:
  cmTarget* Target;
  cmTargetInternals* Internal;
  cmMakefile* Makefile;
  cmGlobalGenerator* GlobalGenerator;
  typedef cmTargetInternals::SourceEntry SourceEntry;
  SourceEntry* CurrentEntry;
  std::queue<cmSourceFile*> SourceQueue;
  std::set<cmSourceFile*> SourcesQueued;
  typedef std::map<cmStdString, cmSourceFile*> NameMapType;
  NameMapType NameMap;

  void QueueSource(cmSourceFile* sf);
  void FollowName(std::string const& name);
  void FollowNames(std::vector<std::string> const& names);
  bool IsUtility(std::string const& dep);
  void CheckCustomCommand(cmCustomCommand const& cc);
  void CheckCustomCommands(const std::vector<cmCustomCommand>& commands);
};

//----------------------------------------------------------------------------
cmTargetTraceDependencies
::cmTargetTraceDependencies(cmTarget* target, cmTargetInternals* internal):
  Target(target), Internal(internal)
{
  // Convenience.
  this->Makefile = this->Target->GetMakefile();
  this->GlobalGenerator =
    this->Makefile->GetLocalGenerator()->GetGlobalGenerator();
  this->CurrentEntry = 0;

  // Queue all the source files already specified for the target.
  std::vector<cmSourceFile*> const& sources = this->Target->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator si = sources.begin();
      si != sources.end(); ++si)
    {
    this->QueueSource(*si);
    }

  // Queue pre-build, pre-link, and post-build rule dependencies.
  this->CheckCustomCommands(this->Target->GetPreBuildCommands());
  this->CheckCustomCommands(this->Target->GetPreLinkCommands());
  this->CheckCustomCommands(this->Target->GetPostBuildCommands());
}

//----------------------------------------------------------------------------
void cmTargetTraceDependencies::Trace()
{
  // Process one dependency at a time until the queue is empty.
  while(!this->SourceQueue.empty())
    {
    // Get the next source from the queue.
    cmSourceFile* sf = this->SourceQueue.front();
    this->SourceQueue.pop();
    this->CurrentEntry = &this->Internal->SourceEntries[sf];

    // Queue dependencies added explicitly by the user.
    if(const char* additionalDeps = sf->GetProperty("OBJECT_DEPENDS"))
      {
      std::vector<std::string> objDeps;
      cmSystemTools::ExpandListArgument(additionalDeps, objDeps);
      this->FollowNames(objDeps);
      }

    // Queue the source needed to generate this file, if any.
    this->FollowName(sf->GetFullPath());

    // Queue dependencies added programatically by commands.
    this->FollowNames(sf->GetDepends());

    // Queue custom command dependencies.
    if(cmCustomCommand const* cc = sf->GetCustomCommand())
      {
      this->CheckCustomCommand(*cc);
      }
    }
  this->CurrentEntry = 0;
}

//----------------------------------------------------------------------------
void cmTargetTraceDependencies::QueueSource(cmSourceFile* sf)
{
  if(this->SourcesQueued.insert(sf).second)
    {
    this->SourceQueue.push(sf);

    // Make sure this file is in the target.
    this->Target->AddSourceFile(sf);
    }
}

//----------------------------------------------------------------------------
void cmTargetTraceDependencies::FollowName(std::string const& name)
{
  NameMapType::iterator i = this->NameMap.find(name);
  if(i == this->NameMap.end())
    {
    // Check if we know how to generate this file.
    cmSourceFile* sf = this->Makefile->GetSourceFileWithOutput(name.c_str());
    NameMapType::value_type entry(name, sf);
    i = this->NameMap.insert(entry).first;
    }
  if(cmSourceFile* sf = i->second)
    {
    // Record the dependency we just followed.
    if(this->CurrentEntry)
      {
      this->CurrentEntry->Depends.push_back(sf);
      }

    this->QueueSource(sf);
    }
}

//----------------------------------------------------------------------------
void
cmTargetTraceDependencies::FollowNames(std::vector<std::string> const& names)
{
  for(std::vector<std::string>::const_iterator i = names.begin();
      i != names.end(); ++i)
    {
    this->FollowName(*i);
    }
}

//----------------------------------------------------------------------------
bool cmTargetTraceDependencies::IsUtility(std::string const& dep)
{
  // Dependencies on targets (utilities) are supposed to be named by
  // just the target name.  However for compatibility we support
  // naming the output file generated by the target (assuming there is
  // no output-name property which old code would not have set).  In
  // that case the target name will be the file basename of the
  // dependency.
  std::string util = cmSystemTools::GetFilenameName(dep);
  if(cmSystemTools::GetFilenameLastExtension(util) == ".exe")
    {
    util = cmSystemTools::GetFilenameWithoutLastExtension(util);
    }

  // Check for a target with this name.
  if(cmTarget* t = this->Makefile->FindTargetToUse(util.c_str()))
    {
    // If we find the target and the dep was given as a full path,
    // then make sure it was not a full path to something else, and
    // the fact that the name matched a target was just a coincidence.
    if(cmSystemTools::FileIsFullPath(dep.c_str()))
      {
      if(t->GetType() >= cmTarget::EXECUTABLE &&
         t->GetType() <= cmTarget::MODULE_LIBRARY)
        {
        // This is really only for compatibility so we do not need to
        // worry about configuration names and output names.
        std::string tLocation = t->GetLocation(0);
        tLocation = cmSystemTools::GetFilenamePath(tLocation);
        std::string depLocation = cmSystemTools::GetFilenamePath(dep);
        depLocation = cmSystemTools::CollapseFullPath(depLocation.c_str());
        tLocation = cmSystemTools::CollapseFullPath(tLocation.c_str());
        if(depLocation == tLocation)
          {
          this->Target->AddUtility(util.c_str());
          return true;
          }
        }
      }
    else
      {
      // The original name of the dependency was not a full path.  It
      // must name a target, so add the target-level dependency.
      this->Target->AddUtility(util.c_str());
      return true;
      }
    }

  // The dependency does not name a target built in this project.
  return false;
}

//----------------------------------------------------------------------------
void
cmTargetTraceDependencies
::CheckCustomCommand(cmCustomCommand const& cc)
{
  // Transform command names that reference targets built in this
  // project to corresponding target-level dependencies.
  cmGeneratorExpression ge(cc.GetBacktrace());

  // Add target-level dependencies referenced by generator expressions.
  std::set<cmTarget*> targets;

  for(cmCustomCommandLines::const_iterator cit = cc.GetCommandLines().begin();
      cit != cc.GetCommandLines().end(); ++cit)
    {
    std::string const& command = *cit->begin();
    // Check for a target with this name.
    if(cmTarget* t = this->Makefile->FindTargetToUse(command.c_str()))
      {
      if(t->GetType() == cmTarget::EXECUTABLE)
        {
        // The command refers to an executable target built in
        // this project.  Add the target-level dependency to make
        // sure the executable is up to date before this custom
        // command possibly runs.
        this->Target->AddUtility(command.c_str());
        }
      }

    // Check for target references in generator expressions.
    for(cmCustomCommandLine::const_iterator cli = cit->begin();
        cli != cit->end(); ++cli)
      {
      const cmsys::auto_ptr<cmCompiledGeneratorExpression> cge
                                                              = ge.Parse(*cli);
      cge->Evaluate(this->Makefile, 0, true);
      std::set<cmTarget*> geTargets = cge->GetTargets();
      for(std::set<cmTarget*>::const_iterator it = geTargets.begin();
          it != geTargets.end(); ++it)
        {
        targets.insert(*it);
        }
      }
    }

  for(std::set<cmTarget*>::iterator ti = targets.begin();
      ti != targets.end(); ++ti)
    {
    this->Target->AddUtility((*ti)->GetName());
    }

  // Queue the custom command dependencies.
  std::vector<std::string> const& depends = cc.GetDepends();
  for(std::vector<std::string>::const_iterator di = depends.begin();
      di != depends.end(); ++di)
    {
    std::string const& dep = *di;
    if(!this->IsUtility(dep))
      {
      // The dependency does not name a target and may be a file we
      // know how to generate.  Queue it.
      this->FollowName(dep);
      }
    }
}

//----------------------------------------------------------------------------
void
cmTargetTraceDependencies
::CheckCustomCommands(const std::vector<cmCustomCommand>& commands)
{
  for(std::vector<cmCustomCommand>::const_iterator cli = commands.begin();
      cli != commands.end(); ++cli)
    {
    this->CheckCustomCommand(*cli);
    }
}

//----------------------------------------------------------------------------
void cmTarget::TraceDependencies()
{
  // CMake-generated targets have no dependencies to trace.  Normally tracing
  // would find nothing anyway, but when building CMake itself the "install"
  // target command ends up referencing the "cmake" target but we do not
  // really want the dependency because "install" depend on "all" anyway.
  if(this->GetType() == cmTarget::GLOBAL_TARGET)
    {
    return;
    }

  // Use a helper object to trace the dependencies.
  cmTargetTraceDependencies tracer(this, this->Internal.Get());
  tracer.Trace();
}

//----------------------------------------------------------------------------
bool cmTarget::FindSourceFiles()
{
  for(std::vector<cmSourceFile*>::const_iterator
        si = this->SourceFiles.begin();
      si != this->SourceFiles.end(); ++si)
    {
    std::string e;
    if((*si)->GetFullPath(&e).empty())
      {
      if(!e.empty())
        {
        cmake* cm = this->Makefile->GetCMakeInstance();
        cm->IssueMessage(cmake::FATAL_ERROR, e,
                         this->GetBacktrace());
        }
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
std::vector<cmSourceFile*> const& cmTarget::GetSourceFiles()
{
  return this->SourceFiles;
}

//----------------------------------------------------------------------------
void cmTarget::AddSourceFile(cmSourceFile* sf)
{
  typedef cmTargetInternals::SourceEntriesType SourceEntriesType;
  SourceEntriesType::iterator i = this->Internal->SourceEntries.find(sf);
  if(i == this->Internal->SourceEntries.end())
    {
    typedef cmTargetInternals::SourceEntry SourceEntry;
    SourceEntriesType::value_type entry(sf, SourceEntry());
    i = this->Internal->SourceEntries.insert(entry).first;
    this->SourceFiles.push_back(sf);
    }
}

//----------------------------------------------------------------------------
std::vector<cmSourceFile*> const*
cmTarget::GetSourceDepends(cmSourceFile* sf)
{
  typedef cmTargetInternals::SourceEntriesType SourceEntriesType;
  SourceEntriesType::iterator i = this->Internal->SourceEntries.find(sf);
  if(i != this->Internal->SourceEntries.end())
    {
    return &i->second.Depends;
    }
  return 0;
}

//----------------------------------------------------------------------------
void cmTarget::AddSources(std::vector<std::string> const& srcs)
{
  for(std::vector<std::string>::const_iterator i = srcs.begin();
      i != srcs.end(); ++i)
    {
    const char* src = i->c_str();
    if(src[0] == '$' && src[1] == '<')
      {
      this->ProcessSourceExpression(*i);
      }
    else
      {
      this->AddSource(src);
      }
    }
}

//----------------------------------------------------------------------------
cmSourceFile* cmTarget::AddSource(const char* s)
{
  std::string src = s;

  // For backwards compatibility replace varibles in source names.
  // This should eventually be removed.
  this->Makefile->ExpandVariablesInString(src);

  cmSourceFile* sf = this->Makefile->GetOrCreateSource(src.c_str());
  this->AddSourceFile(sf);
  return sf;
}

//----------------------------------------------------------------------------
void cmTarget::ProcessSourceExpression(std::string const& expr)
{
  if(strncmp(expr.c_str(), "$<TARGET_OBJECTS:", 17) == 0 &&
     expr[expr.size()-1] == '>')
    {
    std::string objLibName = expr.substr(17, expr.size()-18);
    this->ObjectLibraries.push_back(objLibName);
    }
  else
    {
    cmOStringStream e;
    e << "Unrecognized generator expression:\n"
      << "  " << expr;
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
    }
}

//----------------------------------------------------------------------------
struct cmTarget::SourceFileFlags
cmTarget::GetTargetSourceFileFlags(const cmSourceFile* sf)
{
  struct SourceFileFlags flags;
  this->ConstructSourceFileFlags();
  std::map<cmSourceFile const*, SourceFileFlags>::iterator si =
    this->Internal->SourceFlagsMap.find(sf);
  if(si != this->Internal->SourceFlagsMap.end())
    {
    flags = si->second;
    }
  return flags;
}

//----------------------------------------------------------------------------
void cmTarget::ConstructSourceFileFlags()
{
  if(this->Internal->SourceFileFlagsConstructed)
    {
    return;
    }
  this->Internal->SourceFileFlagsConstructed = true;

  // Process public headers to mark the source files.
  if(const char* files = this->GetProperty("PUBLIC_HEADER"))
    {
    std::vector<std::string> relFiles;
    cmSystemTools::ExpandListArgument(files, relFiles);
    for(std::vector<std::string>::iterator it = relFiles.begin();
        it != relFiles.end(); ++it)
      {
      if(cmSourceFile* sf = this->Makefile->GetSource(it->c_str()))
        {
        SourceFileFlags& flags = this->Internal->SourceFlagsMap[sf];
        flags.MacFolder = "Headers";
        flags.Type = cmTarget::SourceFileTypePublicHeader;
        }
      }
    }

  // Process private headers after public headers so that they take
  // precedence if a file is listed in both.
  if(const char* files = this->GetProperty("PRIVATE_HEADER"))
    {
    std::vector<std::string> relFiles;
    cmSystemTools::ExpandListArgument(files, relFiles);
    for(std::vector<std::string>::iterator it = relFiles.begin();
        it != relFiles.end(); ++it)
      {
      if(cmSourceFile* sf = this->Makefile->GetSource(it->c_str()))
        {
        SourceFileFlags& flags = this->Internal->SourceFlagsMap[sf];
        flags.MacFolder = "PrivateHeaders";
        flags.Type = cmTarget::SourceFileTypePrivateHeader;
        }
      }
    }

  // Mark sources listed as resources.
  if(const char* files = this->GetProperty("RESOURCE"))
    {
    std::vector<std::string> relFiles;
    cmSystemTools::ExpandListArgument(files, relFiles);
    for(std::vector<std::string>::iterator it = relFiles.begin();
        it != relFiles.end(); ++it)
      {
      if(cmSourceFile* sf = this->Makefile->GetSource(it->c_str()))
        {
        SourceFileFlags& flags = this->Internal->SourceFlagsMap[sf];
        flags.MacFolder = "Resources";
        flags.Type = cmTarget::SourceFileTypeResource;
        }
      }
    }

  // Handle the MACOSX_PACKAGE_LOCATION property on source files that
  // were not listed in one of the other lists.
  std::vector<cmSourceFile*> const& sources = this->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator si = sources.begin();
      si != sources.end(); ++si)
    {
    cmSourceFile* sf = *si;
    if(const char* location = sf->GetProperty("MACOSX_PACKAGE_LOCATION"))
      {
      SourceFileFlags& flags = this->Internal->SourceFlagsMap[sf];
      if(flags.Type == cmTarget::SourceFileTypeNormal)
        {
        flags.MacFolder = location;
        if(strcmp(location, "Resources") == 0)
          {
          flags.Type = cmTarget::SourceFileTypeResource;
          }
        else
          {
          flags.Type = cmTarget::SourceFileTypeMacContent;
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmTarget::MergeLinkLibraries( cmMakefile& mf,
                                   const char *selfname,
                                   const LinkLibraryVectorType& libs )
{
  // Only add on libraries we haven't added on before.
  // Assumption: the global link libraries could only grow, never shrink
  LinkLibraryVectorType::const_iterator i = libs.begin();
  i += this->PrevLinkedLibraries.size();
  for( ; i != libs.end(); ++i )
    {
    // We call this so that the dependencies get written to the cache
    this->AddLinkLibrary( mf, selfname, i->first.c_str(), i->second );

    if (this->GetType() == cmTarget::STATIC_LIBRARY)
      {
      this->AppendProperty("INTERFACE_LINK_LIBRARIES",
            ("$<LINK_ONLY:" +
            this->GetDebugGeneratorExpressions(i->first.c_str(), i->second) +
            ">").c_str());
      }
    }
  this->PrevLinkedLibraries = libs;
}

//----------------------------------------------------------------------------
void cmTarget::AddLinkDirectory(const char* d)
{
  // Make sure we don't add unnecessary search directories.
  if(this->LinkDirectoriesEmmitted.insert(d).second)
    {
    this->LinkDirectories.push_back(d);
    }
}

//----------------------------------------------------------------------------
const std::vector<std::string>& cmTarget::GetLinkDirectories()
{
  return this->LinkDirectories;
}

//----------------------------------------------------------------------------
cmTarget::LinkLibraryType cmTarget::ComputeLinkType(const char* config)
{
  // No configuration is always optimized.
  if(!(config && *config))
    {
    return cmTarget::OPTIMIZED;
    }

  // Get the list of configurations considered to be DEBUG.
  std::vector<std::string> const& debugConfigs =
    this->Makefile->GetCMakeInstance()->GetDebugConfigs();

  // Check if any entry in the list matches this configuration.
  std::string configUpper = cmSystemTools::UpperCase(config);
  for(std::vector<std::string>::const_iterator i = debugConfigs.begin();
      i != debugConfigs.end(); ++i)
    {
    if(*i == configUpper)
      {
      return cmTarget::DEBUG;
      }
    }

  // The current configuration is not a debug configuration.
  return cmTarget::OPTIMIZED;
}

//----------------------------------------------------------------------------
void cmTarget::ClearDependencyInformation( cmMakefile& mf,
                                           const char* target )
{
  // Clear the dependencies. The cache variable must exist iff we are
  // recording dependency information for this target.
  std::string depname = target;
  depname += "_LIB_DEPENDS";
  if (this->RecordDependencies)
    {
    mf.AddCacheDefinition(depname.c_str(), "",
                          "Dependencies for target", cmCacheManager::STATIC);
    }
  else
    {
    if (mf.GetDefinition( depname.c_str() ))
      {
      std::string message = "Target ";
      message += target;
      message += " has dependency information when it shouldn't.\n";
      message += "Your cache is probably stale. Please remove the entry\n  ";
      message += depname;
      message += "\nfrom the cache.";
      cmSystemTools::Error( message.c_str() );
      }
    }
}

//----------------------------------------------------------------------------
bool cmTarget::NameResolvesToFramework(const std::string& libname)
{
  return this->GetMakefile()->GetLocalGenerator()->GetGlobalGenerator()->
    NameResolvesToFramework(libname);
}

//----------------------------------------------------------------------------
void cmTarget::GetDirectLinkLibraries(const char *config,
                            std::vector<std::string> &libs, cmTarget *head)
{
  const char *prop = this->GetProperty("LINK_LIBRARIES");
  if (prop)
    {
    cmListFileBacktrace lfbt;
    cmGeneratorExpression ge(lfbt);
    const cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(prop);

    cmGeneratorExpressionDAGChecker dagChecker(lfbt,
                                        this->GetName(),
                                        "LINK_LIBRARIES", 0, 0);
    cmSystemTools::ExpandListArgument(cge->Evaluate(this->Makefile,
                                        config,
                                        false,
                                        head,
                                        &dagChecker),
                                      libs);

    std::set<cmStdString> seenProps = cge->GetSeenTargetProperties();
    for (std::set<cmStdString>::const_iterator it = seenProps.begin();
        it != seenProps.end(); ++it)
      {
      if (!this->GetProperty(it->c_str()))
        {
        this->LinkImplicitNullProperties.insert(*it);
        }
      }
    }
}

//----------------------------------------------------------------------------
std::string cmTarget::GetDebugGeneratorExpressions(const std::string &value,
                                  cmTarget::LinkLibraryType llt)
{
  if (llt == GENERAL)
    {
    return value;
    }

  // Get the list of configurations considered to be DEBUG.
  std::vector<std::string> const& debugConfigs =
                      this->Makefile->GetCMakeInstance()->GetDebugConfigs();

  std::string configString = "$<CONFIG:" + debugConfigs[0] + ">";

  if (debugConfigs.size() > 1)
    {
    for(std::vector<std::string>::const_iterator
          li = debugConfigs.begin() + 1; li != debugConfigs.end(); ++li)
      {
      configString += ",$<CONFIG:" + *li + ">";
      }
    configString = "$<OR:" + configString + ">";
    }

  if (llt == OPTIMIZED)
    {
    configString = "$<NOT:" + configString + ">";
    }
  return "$<" + configString + ":" + value + ">";
}

//----------------------------------------------------------------------------
static std::string targetNameGenex(const char *lib)
{
  return std::string("$<TARGET_NAME:") + lib + ">";
}

//----------------------------------------------------------------------------
bool cmTarget::PushTLLCommandTrace(TLLSignature signature)
{
  bool ret = true;
  if (!this->TLLCommands.empty())
    {
    if (this->TLLCommands.back().first != signature)
      {
      ret = false;
      }
    }
  cmListFileBacktrace lfbt;
  this->Makefile->GetBacktrace(lfbt);
  this->TLLCommands.push_back(std::make_pair(signature, lfbt));
  return ret;
}

//----------------------------------------------------------------------------
void cmTarget::GetTllSignatureTraces(cmOStringStream &s,
                                     TLLSignature sig) const
{
  std::vector<cmListFileBacktrace> sigs;
  typedef std::vector<std::pair<TLLSignature, cmListFileBacktrace> > Container;
  for(Container::const_iterator it = this->TLLCommands.begin();
      it != this->TLLCommands.end(); ++it)
    {
    if (it->first == sig)
      {
      sigs.push_back(it->second);
      }
    }
  if (!sigs.empty())
    {
    const char *sigString
                        = (sig == cmTarget::KeywordTLLSignature ? "keyword"
                                                                : "plain");
    s << "The uses of the " << sigString << " signature are here:\n";
    std::set<cmStdString> emitted;
    for(std::vector<cmListFileBacktrace>::const_iterator it = sigs.begin();
        it != sigs.end(); ++it)
      {
      cmListFileBacktrace::const_iterator i = it->begin();
      if(i != it->end())
        {
        cmListFileContext const& lfc = *i;
        cmOStringStream line;
        line << " * " << (lfc.Line? "": " in ") << lfc << std::endl;
        if (emitted.insert(line.str()).second)
          {
          s << line.str();
          }
        ++i;
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmTarget::AddLinkLibrary(cmMakefile& mf,
                              const char *target, const char* lib,
                              LinkLibraryType llt)
{
  // Never add a self dependency, even if the user asks for it.
  if(strcmp( target, lib ) == 0)
    {
    return;
    }

  cmTarget *tgt = this->Makefile->FindTargetToUse(lib);
  {
  const bool isNonImportedTarget = tgt && !tgt->IsImported();

  const std::string libName = (isNonImportedTarget && llt != GENERAL)
                                                        ? targetNameGenex(lib)
                                                        : std::string(lib);
  this->AppendProperty("LINK_LIBRARIES",
                       this->GetDebugGeneratorExpressions(libName,
                                                          llt).c_str());
  }

  if (cmGeneratorExpression::Find(lib) != std::string::npos
      || (tgt && tgt->GetType() == INTERFACE_LIBRARY))
    {
    return;
    }

  cmTarget::LibraryID tmp;
  tmp.first = lib;
  tmp.second = llt;
  this->LinkLibraries.push_back( tmp );
  this->OriginalLinkLibraries.push_back(tmp);
  this->ClearLinkMaps();

  // Add the explicit dependency information for this target. This is
  // simply a set of libraries separated by ";". There should always
  // be a trailing ";". These library names are not canonical, in that
  // they may be "-framework x", "-ly", "/path/libz.a", etc.
  // We shouldn't remove duplicates here because external libraries
  // may be purposefully duplicated to handle recursive dependencies,
  // and we removing one instance will break the link line. Duplicates
  // will be appropriately eliminated at emit time.
  if(this->RecordDependencies)
    {
    std::string targetEntry = target;
    targetEntry += "_LIB_DEPENDS";
    std::string dependencies;
    const char* old_val = mf.GetDefinition( targetEntry.c_str() );
    if( old_val )
      {
      dependencies += old_val;
      }
    switch (llt)
      {
      case cmTarget::GENERAL:
        dependencies += "general";
        break;
      case cmTarget::DEBUG:
        dependencies += "debug";
        break;
      case cmTarget::OPTIMIZED:
        dependencies += "optimized";
        break;
      }
    dependencies += ";";
    dependencies += lib;
    dependencies += ";";
    mf.AddCacheDefinition( targetEntry.c_str(), dependencies.c_str(),
                           "Dependencies for the target",
                           cmCacheManager::STATIC );
    }

}

//----------------------------------------------------------------------------
void
cmTarget::AddSystemIncludeDirectories(const std::set<cmStdString> &incs)
{
  for(std::set<cmStdString>::const_iterator li = incs.begin();
      li != incs.end(); ++li)
    {
    this->SystemIncludeDirectories.insert(*li);
    }
}

//----------------------------------------------------------------------------
void
cmTarget::AddSystemIncludeDirectories(const std::vector<std::string> &incs)
{
  for(std::vector<std::string>::const_iterator li = incs.begin();
      li != incs.end(); ++li)
    {
    this->SystemIncludeDirectories.insert(*li);
    }
}

//----------------------------------------------------------------------------
void cmTarget::FinalizeSystemIncludeDirectories()
{
  for (std::vector<cmValueWithOrigin>::const_iterator
      it = this->Internal->LinkInterfacePropertyEntries.begin(),
      end = this->Internal->LinkInterfacePropertyEntries.end();
      it != end; ++it)
    {
    if (!cmGeneratorExpression::IsValidTargetName(it->Value)
        && cmGeneratorExpression::Find(it->Value) == std::string::npos)
      {
      continue;
      }
    {
    cmListFileBacktrace lfbt;
    cmGeneratorExpression ge(lfbt);
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
                                                      ge.Parse(it->Value);
    std::string targetName = cge->Evaluate(this->Makefile, 0,
                                      false, this, 0, 0);
    cmTarget *tgt = this->Makefile->FindTargetToUse(targetName.c_str());
    if (!tgt)
      {
      continue;
      }
    if (tgt->IsImported()
        && tgt->GetProperty("INTERFACE_INCLUDE_DIRECTORIES")
        && !this->GetPropertyAsBool("NO_SYSTEM_FROM_IMPORTED"))
      {
      std::string includeGenex = "$<TARGET_PROPERTY:" +
                                it->Value + ",INTERFACE_INCLUDE_DIRECTORIES>";
      if (cmGeneratorExpression::Find(it->Value) != std::string::npos)
        {
        // Because it->Value is a generator expression, ensure that it
        // evaluates to the non-empty string before being used in the
        // TARGET_PROPERTY expression.
        includeGenex = "$<$<BOOL:" + it->Value + ">:" + includeGenex + ">";
        }
      this->SystemIncludeDirectories.insert(includeGenex);
      return; // The INTERFACE_SYSTEM_INCLUDE_DIRECTORIES are a subset
              // of the INTERFACE_INCLUDE_DIRECTORIES
      }
    }
    std::string includeGenex = "$<TARGET_PROPERTY:" +
                        it->Value + ",INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>";
    if (cmGeneratorExpression::Find(it->Value) != std::string::npos)
      {
      // Because it->Value is a generator expression, ensure that it
      // evaluates to the non-empty string before being used in the
      // TARGET_PROPERTY expression.
      includeGenex = "$<$<BOOL:" + it->Value + ">:" + includeGenex + ">";
      }
    this->SystemIncludeDirectories.insert(includeGenex);
    }
}

//----------------------------------------------------------------------------
void
cmTarget::AnalyzeLibDependencies( const cmMakefile& mf )
{
  // There are two key parts of the dependency analysis: (1)
  // determining the libraries in the link line, and (2) constructing
  // the dependency graph for those libraries.
  //
  // The latter is done using the cache entries that record the
  // dependencies of each library.
  //
  // The former is a more thorny issue, since it is not clear how to
  // determine if two libraries listed on the link line refer to the a
  // single library or not. For example, consider the link "libraries"
  //    /usr/lib/libtiff.so -ltiff
  // Is this one library or two? The solution implemented here is the
  // simplest (and probably the only practical) one: two libraries are
  // the same if their "link strings" are identical. Thus, the two
  // libraries above are considered distinct. This also means that for
  // dependency analysis to be effective, the CMake user must specify
  // libraries build by his project without using any linker flags or
  // file extensions. That is,
  //    LINK_LIBRARIES( One Two )
  // instead of
  //    LINK_LIBRARIES( -lOne ${binarypath}/libTwo.a )
  // The former is probably what most users would do, but it never
  // hurts to document the assumptions. :-) Therefore, in the analysis
  // code, the "canonical name" of a library is simply its name as
  // given to a LINK_LIBRARIES command.
  //
  // Also, we will leave the original link line intact; we will just add any
  // dependencies that were missing.
  //
  // There is a problem with recursive external libraries
  // (i.e. libraries with no dependency information that are
  // recursively dependent). We must make sure that the we emit one of
  // the libraries twice to satisfy the recursion, but we shouldn't
  // emit it more times than necessary. In particular, we must make
  // sure that handling this improbable case doesn't cost us when
  // dealing with the common case of non-recursive libraries. The
  // solution is to assume that the recursion is satisfied at one node
  // of the dependency tree. To illustrate, assume libA and libB are
  // extrenal and mutually dependent. Suppose libX depends on
  // libA, and libY on libA and libX. Then
  //   TARGET_LINK_LIBRARIES( Y X A B A )
  //   TARGET_LINK_LIBRARIES( X A B A )
  //   TARGET_LINK_LIBRARIES( Exec Y )
  // would result in "-lY -lX -lA -lB -lA". This is the correct way to
  // specify the dependencies, since the mutual dependency of A and B
  // is resolved *every time libA is specified*.
  //
  // Something like
  //   TARGET_LINK_LIBRARIES( Y X A B A )
  //   TARGET_LINK_LIBRARIES( X A B )
  //   TARGET_LINK_LIBRARIES( Exec Y )
  // would result in "-lY -lX -lA -lB", and the mutual dependency
  // information is lost. This is because in some case (Y), the mutual
  // dependency of A and B is listed, while in another other case (X),
  // it is not. Depending on which line actually emits A, the mutual
  // dependency may or may not be on the final link line.  We can't
  // handle this pathalogical case cleanly without emitting extra
  // libraries for the normal cases. Besides, the dependency
  // information for X is wrong anyway: if we build an executable
  // depending on X alone, we would not have the mutual dependency on
  // A and B resolved.
  //
  // IMPROVEMENTS:
  // -- The current algorithm will not always pick the "optimal" link line
  //    when recursive dependencies are present. It will instead break the
  //    cycles at an aribtrary point. The majority of projects won't have
  //    cyclic dependencies, so this is probably not a big deal. Note that
  //    the link line is always correct, just not necessary optimal.

 {
 // Expand variables in link library names.  This is for backwards
 // compatibility with very early CMake versions and should
 // eventually be removed.  This code was moved here from the end of
 // old source list processing code which was called just before this
 // method.
 for(LinkLibraryVectorType::iterator p = this->LinkLibraries.begin();
     p != this->LinkLibraries.end(); ++p)
   {
   this->Makefile->ExpandVariablesInString(p->first, true, true);
   }
 }

 // The dependency map.
 DependencyMap dep_map;

 // 1. Build the dependency graph
 //
 for(LinkLibraryVectorType::reverse_iterator lib
       = this->LinkLibraries.rbegin();
     lib != this->LinkLibraries.rend(); ++lib)
   {
   this->GatherDependencies( mf, *lib, dep_map);
   }

 // 2. Remove any dependencies that are already satisfied in the original
 // link line.
 //
 for(LinkLibraryVectorType::iterator lib = this->LinkLibraries.begin();
     lib != this->LinkLibraries.end(); ++lib)
   {
   for( LinkLibraryVectorType::iterator lib2 = lib;
        lib2 != this->LinkLibraries.end(); ++lib2)
     {
     this->DeleteDependency( dep_map, *lib, *lib2);
     }
   }


 // 3. Create the new link line by simply emitting any dependencies that are
 // missing.  Start from the back and keep adding.
 //
 std::set<DependencyMap::key_type> done, visited;
 std::vector<DependencyMap::key_type> newLinkLibraries;
 for(LinkLibraryVectorType::reverse_iterator lib =
       this->LinkLibraries.rbegin();
     lib != this->LinkLibraries.rend(); ++lib)
   {
   // skip zero size library entries, this may happen
   // if a variable expands to nothing.
   if (lib->first.size() != 0)
     {
     this->Emit( *lib, dep_map, done, visited, newLinkLibraries );
     }
   }

 // 4. Add the new libraries to the link line.
 //
 for( std::vector<DependencyMap::key_type>::reverse_iterator k =
        newLinkLibraries.rbegin();
      k != newLinkLibraries.rend(); ++k )
   {
   // get the llt from the dep_map
   this->LinkLibraries.push_back( std::make_pair(k->first,k->second) );
   }
 this->LinkLibrariesAnalyzed = true;
}

//----------------------------------------------------------------------------
void cmTarget::InsertDependency( DependencyMap& depMap,
                                 const LibraryID& lib,
                                 const LibraryID& dep)
{
  depMap[lib].push_back(dep);
}

//----------------------------------------------------------------------------
void cmTarget::DeleteDependency( DependencyMap& depMap,
                                 const LibraryID& lib,
                                 const LibraryID& dep)
{
  // Make sure there is an entry in the map for lib. If so, delete all
  // dependencies to dep. There may be repeated entries because of
  // external libraries that are specified multiple times.
  DependencyMap::iterator map_itr = depMap.find( lib );
  if( map_itr != depMap.end() )
    {
    DependencyList& depList = map_itr->second;
    DependencyList::iterator itr;
    while( (itr = std::find(depList.begin(), depList.end(), dep)) !=
           depList.end() )
      {
      depList.erase( itr );
      }
    }
}

//----------------------------------------------------------------------------
void cmTarget::Emit(const LibraryID lib,
                    const DependencyMap& dep_map,
                    std::set<LibraryID>& emitted,
                    std::set<LibraryID>& visited,
                    DependencyList& link_line )
{
  // It's already been emitted
  if( emitted.find(lib) != emitted.end() )
    {
    return;
    }

  // Emit the dependencies only if this library node hasn't been
  // visited before. If it has, then we have a cycle. The recursion
  // that got us here should take care of everything.

  if( visited.insert(lib).second )
    {
    if( dep_map.find(lib) != dep_map.end() ) // does it have dependencies?
      {
      const DependencyList& dep_on = dep_map.find( lib )->second;
      DependencyList::const_reverse_iterator i;

      // To cater for recursive external libraries, we must emit
      // duplicates on this link line *unless* they were emitted by
      // some other node, in which case we assume that the recursion
      // was resolved then. We making the simplifying assumption that
      // any duplicates on a single link line are on purpose, and must
      // be preserved.

      // This variable will keep track of the libraries that were
      // emitted directly from the current node, and not from a
      // recursive call. This way, if we come across a library that
      // has already been emitted, we repeat it iff it has been
      // emitted here.
      std::set<DependencyMap::key_type> emitted_here;
      for( i = dep_on.rbegin(); i != dep_on.rend(); ++i )
        {
        if( emitted_here.find(*i) != emitted_here.end() )
          {
          // a repeat. Must emit.
          emitted.insert(*i);
          link_line.push_back( *i );
          }
        else
          {
          // Emit only if no-one else has
          if( emitted.find(*i) == emitted.end() )
            {
            // emit dependencies
            Emit( *i, dep_map, emitted, visited, link_line );
            // emit self
            emitted.insert(*i);
            emitted_here.insert(*i);
            link_line.push_back( *i );
            }
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmTarget::GatherDependencies( const cmMakefile& mf,
                                   const LibraryID& lib,
                                   DependencyMap& dep_map)
{
  // If the library is already in the dependency map, then it has
  // already been fully processed.
  if( dep_map.find(lib) != dep_map.end() )
    {
    return;
    }

  const char* deps = mf.GetDefinition( (lib.first+"_LIB_DEPENDS").c_str() );
  if( deps && strcmp(deps,"") != 0 )
    {
    // Make sure this library is in the map, even if it has an empty
    // set of dependencies. This distinguishes the case of explicitly
    // no dependencies with that of unspecified dependencies.
    dep_map[lib];

    // Parse the dependency information, which is a set of
    // type, library pairs separated by ";". There is always a trailing ";".
    cmTarget::LinkLibraryType llt = cmTarget::GENERAL;
    std::string depline = deps;
    std::string::size_type start = 0;
    std::string::size_type end;
    end = depline.find( ";", start );
    while( end != std::string::npos )
      {
      std::string l = depline.substr( start, end-start );
      if( l.size() != 0 )
        {
        if (l == "debug")
          {
          llt = cmTarget::DEBUG;
          }
        else if (l == "optimized")
          {
          llt = cmTarget::OPTIMIZED;
          }
        else if (l == "general")
          {
          llt = cmTarget::GENERAL;
          }
        else
          {
          LibraryID lib2(l,llt);
          this->InsertDependency( dep_map, lib, lib2);
          this->GatherDependencies( mf, lib2, dep_map);
          llt = cmTarget::GENERAL;
          }
        }
      start = end+1; // skip the ;
      end = depline.find( ";", start );
      }
    // cannot depend on itself
    this->DeleteDependency( dep_map, lib, lib);
    }
}

//----------------------------------------------------------------------------
void cmTarget::SetProperty(const char* prop, const char* value)
{
  if (!prop)
    {
    return;
    }
  if (strcmp(prop, "NAME") == 0)
    {
    cmOStringStream e;
    e << "NAME property is read-only\n";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str().c_str());
    return;
    }
  if(strcmp(prop,"INCLUDE_DIRECTORIES") == 0)
    {
    cmListFileBacktrace lfbt;
    this->Makefile->GetBacktrace(lfbt);
    cmGeneratorExpression ge(lfbt);
    deleteAndClear(this->Internal->IncludeDirectoriesEntries);
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(value);
    this->Internal->IncludeDirectoriesEntries.push_back(
                          new cmTargetInternals::TargetPropertyEntry(cge));
    return;
    }
  if(strcmp(prop,"COMPILE_OPTIONS") == 0)
    {
    cmListFileBacktrace lfbt;
    this->Makefile->GetBacktrace(lfbt);
    cmGeneratorExpression ge(lfbt);
    deleteAndClear(this->Internal->CompileOptionsEntries);
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(value);
    this->Internal->CompileOptionsEntries.push_back(
                          new cmTargetInternals::TargetPropertyEntry(cge));
    return;
    }
  if(strcmp(prop,"COMPILE_DEFINITIONS") == 0)
    {
    cmListFileBacktrace lfbt;
    this->Makefile->GetBacktrace(lfbt);
    cmGeneratorExpression ge(lfbt);
    deleteAndClear(this->Internal->CompileDefinitionsEntries);
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(value);
    this->Internal->CompileDefinitionsEntries.push_back(
                          new cmTargetInternals::TargetPropertyEntry(cge));
    return;
    }
  if(strcmp(prop,"EXPORT_NAME") == 0 && this->IsImported())
    {
    cmOStringStream e;
    e << "EXPORT_NAME property can't be set on imported targets (\""
          << this->Name << "\")\n";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str().c_str());
    return;
    }
  if (strcmp(prop, "LINK_LIBRARIES") == 0)
    {
    this->Internal->LinkInterfacePropertyEntries.clear();
    cmListFileBacktrace lfbt;
    this->Makefile->GetBacktrace(lfbt);
    cmValueWithOrigin entry(value, lfbt);
    this->Internal->LinkInterfacePropertyEntries.push_back(entry);
    return;
    }
  this->Properties.SetProperty(prop, value, cmProperty::TARGET);
  this->MaybeInvalidatePropertyCache(prop);
}

//----------------------------------------------------------------------------
void cmTarget::AppendProperty(const char* prop, const char* value,
                              bool asString)
{
  if (!prop)
    {
    return;
    }
  if (strcmp(prop, "NAME") == 0)
    {
    cmOStringStream e;
    e << "NAME property is read-only\n";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str().c_str());
    return;
    }
  if(strcmp(prop,"INCLUDE_DIRECTORIES") == 0)
    {
    cmListFileBacktrace lfbt;
    this->Makefile->GetBacktrace(lfbt);
    cmGeneratorExpression ge(lfbt);
    this->Internal->IncludeDirectoriesEntries.push_back(
              new cmTargetInternals::TargetPropertyEntry(ge.Parse(value)));
    return;
    }
  if(strcmp(prop,"COMPILE_OPTIONS") == 0)
    {
    cmListFileBacktrace lfbt;
    this->Makefile->GetBacktrace(lfbt);
    cmGeneratorExpression ge(lfbt);
    this->Internal->CompileOptionsEntries.push_back(
              new cmTargetInternals::TargetPropertyEntry(ge.Parse(value)));
    return;
    }
  if(strcmp(prop,"COMPILE_DEFINITIONS") == 0)
    {
    cmListFileBacktrace lfbt;
    this->Makefile->GetBacktrace(lfbt);
    cmGeneratorExpression ge(lfbt);
    this->Internal->CompileDefinitionsEntries.push_back(
              new cmTargetInternals::TargetPropertyEntry(ge.Parse(value)));
    return;
    }
  if(strcmp(prop,"EXPORT_NAME") == 0 && this->IsImported())
    {
    cmOStringStream e;
    e << "EXPORT_NAME property can't be set on imported targets (\""
          << this->Name << "\")\n";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str().c_str());
    return;
    }
  if (strcmp(prop, "LINK_LIBRARIES") == 0)
    {
    cmListFileBacktrace lfbt;
    this->Makefile->GetBacktrace(lfbt);
    cmValueWithOrigin entry(value, lfbt);
    this->Internal->LinkInterfacePropertyEntries.push_back(entry);
    return;
    }
  this->Properties.AppendProperty(prop, value, cmProperty::TARGET, asString);
  this->MaybeInvalidatePropertyCache(prop);
}

//----------------------------------------------------------------------------
const char* cmTarget::GetExportName()
{
  const char *exportName = this->GetProperty("EXPORT_NAME");

  if (exportName && *exportName)
    {
    if (!cmGeneratorExpression::IsValidTargetName(exportName))
      {
      cmOStringStream e;
      e << "EXPORT_NAME property \"" << exportName << "\" for \""
        << this->GetName() << "\": is not valid.";
      cmSystemTools::Error(e.str().c_str());
      return "";
      }
    return exportName;
    }
  return this->GetName();
}

//----------------------------------------------------------------------------
void cmTarget::AppendBuildInterfaceIncludes()
{
  if(this->GetType() != cmTarget::SHARED_LIBRARY &&
     this->GetType() != cmTarget::STATIC_LIBRARY &&
     this->GetType() != cmTarget::MODULE_LIBRARY &&
     !this->IsExecutableWithExports())
    {
    return;
    }
  if (this->BuildInterfaceIncludesAppended)
    {
    return;
    }
  this->BuildInterfaceIncludesAppended = true;

  if (this->Makefile->IsOn("CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE"))
    {
    const char *binDir = this->Makefile->GetStartOutputDirectory();
    const char *srcDir = this->Makefile->GetStartDirectory();
    const std::string dirs = std::string(binDir ? binDir : "")
                            + std::string(binDir ? ";" : "")
                            + std::string(srcDir ? srcDir : "");
    if (!dirs.empty())
      {
      this->AppendProperty("INTERFACE_INCLUDE_DIRECTORIES",
                            ("$<BUILD_INTERFACE:" + dirs + ">").c_str());
      }
    }
}

//----------------------------------------------------------------------------
void cmTarget::InsertInclude(const cmValueWithOrigin &entry,
                     bool before)
{
  cmGeneratorExpression ge(entry.Backtrace);

  std::vector<cmTargetInternals::TargetPropertyEntry*>::iterator position
                = before ? this->Internal->IncludeDirectoriesEntries.begin()
                         : this->Internal->IncludeDirectoriesEntries.end();

  this->Internal->IncludeDirectoriesEntries.insert(position,
      new cmTargetInternals::TargetPropertyEntry(ge.Parse(entry.Value)));
}

//----------------------------------------------------------------------------
void cmTarget::InsertCompileOption(const cmValueWithOrigin &entry,
                     bool before)
{
  cmGeneratorExpression ge(entry.Backtrace);

  std::vector<cmTargetInternals::TargetPropertyEntry*>::iterator position
                = before ? this->Internal->CompileOptionsEntries.begin()
                         : this->Internal->CompileOptionsEntries.end();

  this->Internal->CompileOptionsEntries.insert(position,
      new cmTargetInternals::TargetPropertyEntry(ge.Parse(entry.Value)));
}

//----------------------------------------------------------------------------
void cmTarget::InsertCompileDefinition(const cmValueWithOrigin &entry,
                     bool before)
{
  cmGeneratorExpression ge(entry.Backtrace);

  std::vector<cmTargetInternals::TargetPropertyEntry*>::iterator position
                = before ? this->Internal->CompileDefinitionsEntries.begin()
                         : this->Internal->CompileDefinitionsEntries.end();

  this->Internal->CompileDefinitionsEntries.insert(position,
      new cmTargetInternals::TargetPropertyEntry(ge.Parse(entry.Value)));
}

//----------------------------------------------------------------------------
static void processIncludeDirectories(cmTarget *tgt,
      const std::vector<cmTargetInternals::TargetPropertyEntry*> &entries,
      std::vector<std::string> &includes,
      std::set<std::string> &uniqueIncludes,
      cmGeneratorExpressionDAGChecker *dagChecker,
      const char *config, bool debugIncludes)
{
  cmMakefile *mf = tgt->GetMakefile();

  for (std::vector<cmTargetInternals::TargetPropertyEntry*>::const_iterator
      it = entries.begin(), end = entries.end(); it != end; ++it)
    {
    bool testIsOff = true;
    bool cacheIncludes = false;
    std::vector<std::string> entryIncludes = (*it)->CachedEntries;
    if(!entryIncludes.empty())
      {
      testIsOff = false;
      }
    else
      {
      cmSystemTools::ExpandListArgument((*it)->ge->Evaluate(mf,
                                                config,
                                                false,
                                                tgt,
                                                dagChecker),
                                      entryIncludes);
      if (mf->IsGeneratingBuildSystem()
          && !(*it)->ge->GetHadContextSensitiveCondition())
        {
        cacheIncludes = true;
        }
      }
    std::string usedIncludes;
    cmListFileBacktrace lfbt;
    for(std::vector<std::string>::iterator
          li = entryIncludes.begin(); li != entryIncludes.end(); ++li)
      {
      std::string targetName = (*it)->TargetName;
      std::string evaluatedTargetName;
      {
      cmGeneratorExpression ge(lfbt);
      cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
                                                        ge.Parse(targetName);
      evaluatedTargetName = cge->Evaluate(mf, config, false, tgt, 0, 0);
      }

      cmTarget *dependentTarget = mf->FindTargetToUse(targetName.c_str());

      const bool fromImported = dependentTarget
                             && dependentTarget->IsImported();

      cmTarget *evaluatedDependentTarget =
        (targetName != evaluatedTargetName)
          ? mf->FindTargetToUse(evaluatedTargetName.c_str())
          : 0;

      targetName = evaluatedTargetName;

      const bool fromEvaluatedImported = evaluatedDependentTarget
                             && evaluatedDependentTarget->IsImported();

      if ((fromImported || fromEvaluatedImported)
          && !cmSystemTools::FileExists(li->c_str()))
        {
        cmOStringStream e;
        cmake::MessageType messageType = cmake::FATAL_ERROR;
        if (fromEvaluatedImported)
          {
          switch(mf->GetPolicyStatus(cmPolicies::CMP0027))
            {
            case cmPolicies::WARN:
              e << (mf->GetPolicies()
                    ->GetPolicyWarning(cmPolicies::CMP0027)) << "\n";
            case cmPolicies::OLD:
              messageType = cmake::AUTHOR_WARNING;
              break;
            case cmPolicies::REQUIRED_ALWAYS:
            case cmPolicies::REQUIRED_IF_USED:
            case cmPolicies::NEW:
              break;
            }
          }
        e << "Imported target \"" << targetName << "\" includes "
             "non-existent path\n  \"" << *li << "\"\nin its "
             "INTERFACE_INCLUDE_DIRECTORIES. Possible reasons include:\n"
             "* The path was deleted, renamed, or moved to another "
             "location.\n"
             "* An install or uninstall procedure did not complete "
             "successfully.\n"
             "* The installation package was faulty and references files it "
             "does not provide.\n";
        tgt->GetMakefile()->IssueMessage(messageType, e.str().c_str());
        return;
        }

      if (!cmSystemTools::FileIsFullPath(li->c_str()))
        {
        cmOStringStream e;
        bool noMessage = false;
        cmake::MessageType messageType = cmake::FATAL_ERROR;
        if (!targetName.empty())
          {
          e << "Target \"" << targetName << "\" contains relative "
            "path in its INTERFACE_INCLUDE_DIRECTORIES:\n"
            "  \"" << *li << "\"";
          }
        else
          {
          switch(tgt->GetPolicyStatusCMP0021())
            {
            case cmPolicies::WARN:
              {
              e << (mf->GetPolicies()
                    ->GetPolicyWarning(cmPolicies::CMP0021)) << "\n";
              messageType = cmake::AUTHOR_WARNING;
              }
              break;
            case cmPolicies::OLD:
              noMessage = true;
            case cmPolicies::REQUIRED_IF_USED:
            case cmPolicies::REQUIRED_ALWAYS:
            case cmPolicies::NEW:
              // Issue the fatal message.
              break;
            }
          e << "Found relative path while evaluating include directories of "
          "\"" << tgt->GetName() << "\":\n  \"" << *li << "\"\n";
          }
        if (!noMessage)
          {
          tgt->GetMakefile()->IssueMessage(messageType, e.str().c_str());
          if (messageType == cmake::FATAL_ERROR)
            {
            return;
            }
          }
        }

      if (testIsOff && !cmSystemTools::IsOff(li->c_str()))
        {
        cmSystemTools::ConvertToUnixSlashes(*li);
        }
      std::string inc = *li;

      if(uniqueIncludes.insert(inc).second)
        {
        includes.push_back(inc);
        if (debugIncludes)
          {
          usedIncludes += " * " + inc + "\n";
          }
        }
      }
    if (cacheIncludes)
      {
      (*it)->CachedEntries = entryIncludes;
      }
    if (!usedIncludes.empty())
      {
      mf->GetCMakeInstance()->IssueMessage(cmake::LOG,
                            std::string("Used includes for target ")
                            + tgt->GetName() + ":\n"
                            + usedIncludes, (*it)->ge->GetBacktrace());
      }
    }
}

//----------------------------------------------------------------------------
std::vector<std::string> cmTarget::GetIncludeDirectories(const char *config)
{
  std::vector<std::string> includes;
  std::set<std::string> uniqueIncludes;
  cmListFileBacktrace lfbt;

  cmGeneratorExpressionDAGChecker dagChecker(lfbt,
                                             this->GetName(),
                                             "INCLUDE_DIRECTORIES", 0, 0);

  this->AppendBuildInterfaceIncludes();

  std::vector<std::string> debugProperties;
  const char *debugProp =
              this->Makefile->GetDefinition("CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp)
    {
    cmSystemTools::ExpandListArgument(debugProp, debugProperties);
    }

  bool debugIncludes = !this->DebugIncludesDone
                    && std::find(debugProperties.begin(),
                                 debugProperties.end(),
                                 "INCLUDE_DIRECTORIES")
                        != debugProperties.end();

  if (this->Makefile->IsGeneratingBuildSystem())
    {
    this->DebugIncludesDone = true;
    }

  processIncludeDirectories(this,
                            this->Internal->IncludeDirectoriesEntries,
                            includes,
                            uniqueIncludes,
                            &dagChecker,
                            config,
                            debugIncludes);

  std::string configString = config ? config : "";
  if (!this->Internal->CacheLinkInterfaceIncludeDirectoriesDone[configString])
    {
    for (std::vector<cmValueWithOrigin>::const_iterator
        it = this->Internal->LinkInterfacePropertyEntries.begin(),
        end = this->Internal->LinkInterfacePropertyEntries.end();
        it != end; ++it)
      {
      if (!cmGeneratorExpression::IsValidTargetName(it->Value)
          && cmGeneratorExpression::Find(it->Value) == std::string::npos)
        {
        continue;
        }
      {
      cmGeneratorExpression ge(lfbt);
      cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
                                                        ge.Parse(it->Value);
      std::string result = cge->Evaluate(this->Makefile, config,
                                        false, this, 0, 0);
      if (!this->Makefile->FindTargetToUse(result.c_str()))
        {
        continue;
        }
      }
      std::string includeGenex = "$<TARGET_PROPERTY:" +
                              it->Value + ",INTERFACE_INCLUDE_DIRECTORIES>";
      if (cmGeneratorExpression::Find(it->Value) != std::string::npos)
        {
        // Because it->Value is a generator expression, ensure that it
        // evaluates to the non-empty string before being used in the
        // TARGET_PROPERTY expression.
        includeGenex = "$<$<BOOL:" + it->Value + ">:" + includeGenex + ">";
        }
      cmGeneratorExpression ge(it->Backtrace);
      cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(
                                                              includeGenex);

      this->Internal
        ->CachedLinkInterfaceIncludeDirectoriesEntries[configString].push_back(
                        new cmTargetInternals::TargetPropertyEntry(cge,
                                                              it->Value));
      }

    if(this->Makefile->IsOn("APPLE"))
      {
      LinkImplementation const* impl = this->GetLinkImplementation(config,
                                                                   this);
      for(std::vector<std::string>::const_iterator
          it = impl->Libraries.begin();
          it != impl->Libraries.end(); ++it)
        {
        std::string libDir = cmSystemTools::CollapseFullPath(it->c_str());

        static cmsys::RegularExpression
          frameworkCheck("(.*\\.framework)(/Versions/[^/]+)?/[^/]+$");
        if(!frameworkCheck.find(libDir))
          {
          continue;
          }

        libDir = frameworkCheck.match(1);

        cmGeneratorExpression ge(lfbt);
        cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
                  ge.Parse(libDir.c_str());
        this->Internal
                ->CachedLinkInterfaceIncludeDirectoriesEntries[configString]
                .push_back(new cmTargetInternals::TargetPropertyEntry(cge));
        }
      }
    }

  processIncludeDirectories(this,
    this->Internal->CachedLinkInterfaceIncludeDirectoriesEntries[configString],
                            includes,
                            uniqueIncludes,
                            &dagChecker,
                            config,
                            debugIncludes);

  if (!this->Makefile->IsGeneratingBuildSystem())
    {
    deleteAndClear(
                this->Internal->CachedLinkInterfaceIncludeDirectoriesEntries);
    }
  else
    {
    this->Internal->CacheLinkInterfaceIncludeDirectoriesDone[configString]
                                                                      = true;
    }

  return includes;
}

//----------------------------------------------------------------------------
static void processCompileOptionsInternal(cmTarget *tgt,
      const std::vector<cmTargetInternals::TargetPropertyEntry*> &entries,
      std::vector<std::string> &options,
      std::set<std::string> &uniqueOptions,
      cmGeneratorExpressionDAGChecker *dagChecker,
      const char *config, bool debugOptions, const char *logName)
{
  cmMakefile *mf = tgt->GetMakefile();

  for (std::vector<cmTargetInternals::TargetPropertyEntry*>::const_iterator
      it = entries.begin(), end = entries.end(); it != end; ++it)
    {
    bool cacheOptions = false;
    std::vector<std::string> entryOptions = (*it)->CachedEntries;
    if(entryOptions.empty())
      {
      cmSystemTools::ExpandListArgument((*it)->ge->Evaluate(mf,
                                                config,
                                                false,
                                                tgt,
                                                dagChecker),
                                      entryOptions);
      if (mf->IsGeneratingBuildSystem()
          && !(*it)->ge->GetHadContextSensitiveCondition())
        {
        cacheOptions = true;
        }
      }
    std::string usedOptions;
    for(std::vector<std::string>::iterator
          li = entryOptions.begin(); li != entryOptions.end(); ++li)
      {
      std::string opt = *li;

      if(uniqueOptions.insert(opt).second)
        {
        options.push_back(opt);
        if (debugOptions)
          {
          usedOptions += " * " + opt + "\n";
          }
        }
      }
    if (cacheOptions)
      {
      (*it)->CachedEntries = entryOptions;
      }
    if (!usedOptions.empty())
      {
      mf->GetCMakeInstance()->IssueMessage(cmake::LOG,
                            std::string("Used compile ") + logName
                            + std::string(" for target ")
                            + tgt->GetName() + ":\n"
                            + usedOptions, (*it)->ge->GetBacktrace());
      }
    }
}

//----------------------------------------------------------------------------
static void processCompileOptions(cmTarget *tgt,
      const std::vector<cmTargetInternals::TargetPropertyEntry*> &entries,
      std::vector<std::string> &options,
      std::set<std::string> &uniqueOptions,
      cmGeneratorExpressionDAGChecker *dagChecker,
      const char *config, bool debugOptions)
{
  processCompileOptionsInternal(tgt, entries, options, uniqueOptions,
                                dagChecker, config, debugOptions, "options");
}

//----------------------------------------------------------------------------
void cmTarget::GetCompileOptions(std::vector<std::string> &result,
                                 const char *config)
{
  std::set<std::string> uniqueOptions;
  cmListFileBacktrace lfbt;

  cmGeneratorExpressionDAGChecker dagChecker(lfbt,
                                              this->GetName(),
                                              "COMPILE_OPTIONS", 0, 0);

  std::vector<std::string> debugProperties;
  const char *debugProp =
              this->Makefile->GetDefinition("CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp)
    {
    cmSystemTools::ExpandListArgument(debugProp, debugProperties);
    }

  bool debugOptions = !this->DebugCompileOptionsDone
                    && std::find(debugProperties.begin(),
                                 debugProperties.end(),
                                 "COMPILE_OPTIONS")
                        != debugProperties.end();

  if (this->Makefile->IsGeneratingBuildSystem())
    {
    this->DebugCompileOptionsDone = true;
    }

  processCompileOptions(this,
                            this->Internal->CompileOptionsEntries,
                            result,
                            uniqueOptions,
                            &dagChecker,
                            config,
                            debugOptions);

  std::string configString = config ? config : "";
  if (!this->Internal->CacheLinkInterfaceCompileOptionsDone[configString])
    {
    for (std::vector<cmValueWithOrigin>::const_iterator
        it = this->Internal->LinkInterfacePropertyEntries.begin(),
        end = this->Internal->LinkInterfacePropertyEntries.end();
        it != end; ++it)
      {
      if (!cmGeneratorExpression::IsValidTargetName(it->Value)
          && cmGeneratorExpression::Find(it->Value) == std::string::npos)
        {
        continue;
        }
      {
      cmGeneratorExpression ge(lfbt);
      cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
                                                        ge.Parse(it->Value);
      std::string targetResult = cge->Evaluate(this->Makefile, config,
                                        false, this, 0, 0);
      if (!this->Makefile->FindTargetToUse(targetResult.c_str()))
        {
        continue;
        }
      }
      std::string optionGenex = "$<TARGET_PROPERTY:" +
                              it->Value + ",INTERFACE_COMPILE_OPTIONS>";
      if (cmGeneratorExpression::Find(it->Value) != std::string::npos)
        {
        // Because it->Value is a generator expression, ensure that it
        // evaluates to the non-empty string before being used in the
        // TARGET_PROPERTY expression.
        optionGenex = "$<$<BOOL:" + it->Value + ">:" + optionGenex + ">";
        }
      cmGeneratorExpression ge(it->Backtrace);
      cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(
                                                                optionGenex);

      this->Internal
        ->CachedLinkInterfaceCompileOptionsEntries[configString].push_back(
                        new cmTargetInternals::TargetPropertyEntry(cge,
                                                              it->Value));
      }
    }

  processCompileOptions(this,
    this->Internal->CachedLinkInterfaceCompileOptionsEntries[configString],
                            result,
                            uniqueOptions,
                            &dagChecker,
                            config,
                            debugOptions);

  if (!this->Makefile->IsGeneratingBuildSystem())
    {
    deleteAndClear(this->Internal->CachedLinkInterfaceCompileOptionsEntries);
    }
  else
    {
    this->Internal->CacheLinkInterfaceCompileOptionsDone[configString] = true;
    }
}

//----------------------------------------------------------------------------
static void processCompileDefinitions(cmTarget *tgt,
      const std::vector<cmTargetInternals::TargetPropertyEntry*> &entries,
      std::vector<std::string> &options,
      std::set<std::string> &uniqueOptions,
      cmGeneratorExpressionDAGChecker *dagChecker,
      const char *config, bool debugOptions)
{
  processCompileOptionsInternal(tgt, entries, options, uniqueOptions,
                                dagChecker, config, debugOptions,
                                "definitions");
}

//----------------------------------------------------------------------------
void cmTarget::GetCompileDefinitions(std::vector<std::string> &list,
                                            const char *config)
{
  std::set<std::string> uniqueOptions;
  cmListFileBacktrace lfbt;

  cmGeneratorExpressionDAGChecker dagChecker(lfbt,
                                              this->GetName(),
                                              "COMPILE_DEFINITIONS", 0, 0);

  std::vector<std::string> debugProperties;
  const char *debugProp =
              this->Makefile->GetDefinition("CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp)
    {
    cmSystemTools::ExpandListArgument(debugProp, debugProperties);
    }

  bool debugDefines = !this->DebugCompileDefinitionsDone
                          && std::find(debugProperties.begin(),
                                debugProperties.end(),
                                "COMPILE_DEFINITIONS")
                        != debugProperties.end();

  if (this->Makefile->IsGeneratingBuildSystem())
    {
    this->DebugCompileDefinitionsDone = true;
    }

  processCompileDefinitions(this,
                            this->Internal->CompileDefinitionsEntries,
                            list,
                            uniqueOptions,
                            &dagChecker,
                            config,
                            debugDefines);

  std::string configString = config ? config : "";
  if (!this->Internal->CacheLinkInterfaceCompileDefinitionsDone[configString])
    {
    for (std::vector<cmValueWithOrigin>::const_iterator
        it = this->Internal->LinkInterfacePropertyEntries.begin(),
        end = this->Internal->LinkInterfacePropertyEntries.end();
        it != end; ++it)
      {
      if (!cmGeneratorExpression::IsValidTargetName(it->Value)
          && cmGeneratorExpression::Find(it->Value) == std::string::npos)
        {
        continue;
        }
      {
      cmGeneratorExpression ge(lfbt);
      cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
                                                        ge.Parse(it->Value);
      std::string targetResult = cge->Evaluate(this->Makefile, config,
                                        false, this, 0, 0);
      if (!this->Makefile->FindTargetToUse(targetResult.c_str()))
        {
        continue;
        }
      }
      std::string defsGenex = "$<TARGET_PROPERTY:" +
                              it->Value + ",INTERFACE_COMPILE_DEFINITIONS>";
      if (cmGeneratorExpression::Find(it->Value) != std::string::npos)
        {
        // Because it->Value is a generator expression, ensure that it
        // evaluates to the non-empty string before being used in the
        // TARGET_PROPERTY expression.
        defsGenex = "$<$<BOOL:" + it->Value + ">:" + defsGenex + ">";
        }
      cmGeneratorExpression ge(it->Backtrace);
      cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(
                                                                defsGenex);

      this->Internal
        ->CachedLinkInterfaceCompileDefinitionsEntries[configString].push_back(
                        new cmTargetInternals::TargetPropertyEntry(cge,
                                                              it->Value));
      }
    if (config)
      {
      std::string configPropName = "COMPILE_DEFINITIONS_"
                                          + cmSystemTools::UpperCase(config);
      const char *configProp = this->GetProperty(configPropName.c_str());
      std::string defsString = (configProp ? configProp : "");

      cmGeneratorExpression ge(lfbt);
      cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
                                                        ge.Parse(defsString);
      this->Internal
        ->CachedLinkInterfaceCompileDefinitionsEntries[configString].push_back(
                        new cmTargetInternals::TargetPropertyEntry(cge));
      }

    }

  processCompileDefinitions(this,
    this->Internal->CachedLinkInterfaceCompileDefinitionsEntries[configString],
                            list,
                            uniqueOptions,
                            &dagChecker,
                            config,
                            debugDefines);

  if (!this->Makefile->IsGeneratingBuildSystem())
    {
    deleteAndClear(this->Internal
                              ->CachedLinkInterfaceCompileDefinitionsEntries);
    }
  else
    {
    this->Internal->CacheLinkInterfaceCompileDefinitionsDone[configString]
                                                                      = true;
    }
}

//----------------------------------------------------------------------------
void cmTarget::MaybeInvalidatePropertyCache(const char* prop)
{
  // Wipe out maps caching information affected by this property.
  if(this->IsImported() && strncmp(prop, "IMPORTED", 8) == 0)
    {
    this->Internal->ImportInfoMap.clear();
    }
  if(!this->IsImported() && strncmp(prop, "LINK_INTERFACE_", 15) == 0)
    {
    this->ClearLinkMaps();
    }
}

//----------------------------------------------------------------------------
static void cmTargetCheckLINK_INTERFACE_LIBRARIES(
  const char* prop, const char* value, cmMakefile* context, bool imported
  )
{
  // Look for link-type keywords in the value.
  static cmsys::RegularExpression
    keys("(^|;)(debug|optimized|general)(;|$)");
  if(!keys.find(value))
    {
    return;
    }

  // Support imported and non-imported versions of the property.
  const char* base = (imported?
                      "IMPORTED_LINK_INTERFACE_LIBRARIES" :
                      "LINK_INTERFACE_LIBRARIES");

  // Report an error.
  cmOStringStream e;
  e << "Property " << prop << " may not contain link-type keyword \""
    << keys.match(2) << "\".  "
    << "The " << base << " property has a per-configuration "
    << "version called " << base << "_<CONFIG> which may be "
    << "used to specify per-configuration rules.";
  if(!imported)
    {
    e << "  "
      << "Alternatively, an IMPORTED library may be created, configured "
      << "with a per-configuration location, and then named in the "
      << "property value.  "
      << "See the add_library command's IMPORTED mode for details."
      << "\n"
      << "If you have a list of libraries that already contains the "
      << "keyword, use the target_link_libraries command with its "
      << "LINK_INTERFACE_LIBRARIES mode to set the property.  "
      << "The command automatically recognizes link-type keywords and sets "
      << "the LINK_INTERFACE_LIBRARIES and LINK_INTERFACE_LIBRARIES_DEBUG "
      << "properties accordingly.";
    }
  context->IssueMessage(cmake::FATAL_ERROR, e.str());
}

//----------------------------------------------------------------------------
static void cmTargetCheckINTERFACE_LINK_LIBRARIES(const char* value,
                                                  cmMakefile* context)
{
  // Look for link-type keywords in the value.
  static cmsys::RegularExpression
    keys("(^|;)(debug|optimized|general)(;|$)");
  if(!keys.find(value))
    {
    return;
    }

  // Report an error.
  cmOStringStream e;

  e << "Property INTERFACE_LINK_LIBRARIES may not contain link-type "
    "keyword \"" << keys.match(2) << "\".  The INTERFACE_LINK_LIBRARIES "
    "property may contain configuration-sensitive generator-expressions "
    "which may be used to specify per-configuration rules.";

  context->IssueMessage(cmake::FATAL_ERROR, e.str());
}

//----------------------------------------------------------------------------
void cmTarget::CheckProperty(const char* prop, cmMakefile* context)
{
  // Certain properties need checking.
  if(strncmp(prop, "LINK_INTERFACE_LIBRARIES", 24) == 0)
    {
    if(const char* value = this->GetProperty(prop))
      {
      cmTargetCheckLINK_INTERFACE_LIBRARIES(prop, value, context, false);
      }
    }
  if(strncmp(prop, "IMPORTED_LINK_INTERFACE_LIBRARIES", 33) == 0)
    {
    if(const char* value = this->GetProperty(prop))
      {
      cmTargetCheckLINK_INTERFACE_LIBRARIES(prop, value, context, true);
      }
    }
  if(strncmp(prop, "INTERFACE_LINK_LIBRARIES", 24) == 0)
    {
    if(const char* value = this->GetProperty(prop))
      {
      cmTargetCheckINTERFACE_LINK_LIBRARIES(value, context);
      }
    }
}

//----------------------------------------------------------------------------
void cmTarget::MarkAsImported()
{
  this->IsImportedTarget = true;
}

//----------------------------------------------------------------------------
bool cmTarget::HaveWellDefinedOutputFiles()
{
  return
    this->GetType() == cmTarget::STATIC_LIBRARY ||
    this->GetType() == cmTarget::SHARED_LIBRARY ||
    this->GetType() == cmTarget::MODULE_LIBRARY ||
    this->GetType() == cmTarget::EXECUTABLE;
}

//----------------------------------------------------------------------------
cmTarget::OutputInfo const* cmTarget::GetOutputInfo(const char* config)
{
  // There is no output information for imported targets.
  if(this->IsImported())
    {
    return 0;
    }

  // Only libraries and executables have well-defined output files.
  if(!this->HaveWellDefinedOutputFiles())
    {
    std::string msg = "cmTarget::GetOutputInfo called for ";
    msg += this->GetName();
    msg += " which has type ";
    msg += cmTarget::GetTargetTypeName(this->GetType());
    this->GetMakefile()->IssueMessage(cmake::INTERNAL_ERROR, msg);
    abort();
    return 0;
    }

  // Lookup/compute/cache the output information for this configuration.
  std::string config_upper;
  if(config && *config)
    {
    config_upper = cmSystemTools::UpperCase(config);
    }
  typedef cmTargetInternals::OutputInfoMapType OutputInfoMapType;
  OutputInfoMapType::const_iterator i =
    this->Internal->OutputInfoMap.find(config_upper);
  if(i == this->Internal->OutputInfoMap.end())
    {
    OutputInfo info;
    this->ComputeOutputDir(config, false, info.OutDir);
    this->ComputeOutputDir(config, true, info.ImpDir);
    if(!this->ComputePDBOutputDir(config, info.PdbDir))
      {
      info.PdbDir = info.OutDir;
      }
    OutputInfoMapType::value_type entry(config_upper, info);
    i = this->Internal->OutputInfoMap.insert(entry).first;
    }
  return &i->second;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetDirectory(const char* config, bool implib)
{
  if (this->IsImported())
    {
    // Return the directory from which the target is imported.
    return
      cmSystemTools::GetFilenamePath(
      this->ImportedGetFullPath(config, implib));
    }
  else if(OutputInfo const* info = this->GetOutputInfo(config))
    {
    // Return the directory in which the target will be built.
    return implib? info->ImpDir : info->OutDir;
    }
  return "";
}

//----------------------------------------------------------------------------
std::string cmTarget::GetPDBDirectory(const char* config)
{
  if(OutputInfo const* info = this->GetOutputInfo(config))
    {
    // Return the directory in which the target will be built.
    return info->PdbDir;
    }
  return "";
}

//----------------------------------------------------------------------------
const char* cmTarget::GetLocation(const char* config)
{
  if (this->IsImported())
    {
    return this->ImportedGetLocation(config);
    }
  else
    {
    return this->NormalGetLocation(config);
    }
}

//----------------------------------------------------------------------------
const char* cmTarget::ImportedGetLocation(const char* config)
{
  this->Location = this->ImportedGetFullPath(config, false);
  return this->Location.c_str();
}

//----------------------------------------------------------------------------
const char* cmTarget::NormalGetLocation(const char* config)
{
  // Handle the configuration-specific case first.
  if(config)
    {
    this->Location = this->GetFullPath(config, false);
    return this->Location.c_str();
    }

  // Now handle the deprecated build-time configuration location.
  this->Location = this->GetDirectory();
  const char* cfgid = this->Makefile->GetDefinition("CMAKE_CFG_INTDIR");
  if(cfgid && strcmp(cfgid, ".") != 0)
    {
    this->Location += "/";
    this->Location += cfgid;
    }

  if(this->IsAppBundleOnApple())
    {
    std::string macdir = this->BuildMacContentDirectory("", config, false);
    if(!macdir.empty())
      {
      this->Location += "/";
      this->Location += macdir;
      }
    }
  this->Location += "/";
  this->Location += this->GetFullName(config, false);
  return this->Location.c_str();
}

//----------------------------------------------------------------------------
void cmTarget::GetTargetVersion(int& major, int& minor)
{
  int patch;
  this->GetTargetVersion(false, major, minor, patch);
}

//----------------------------------------------------------------------------
void cmTarget::GetTargetVersion(bool soversion,
                                int& major, int& minor, int& patch)
{
  // Set the default values.
  major = 0;
  minor = 0;
  patch = 0;

  // Look for a VERSION or SOVERSION property.
  const char* prop = soversion? "SOVERSION" : "VERSION";
  if(const char* version = this->GetProperty(prop))
    {
    // Try to parse the version number and store the results that were
    // successfully parsed.
    int parsed_major;
    int parsed_minor;
    int parsed_patch;
    switch(sscanf(version, "%d.%d.%d",
                  &parsed_major, &parsed_minor, &parsed_patch))
      {
      case 3: patch = parsed_patch; // no break!
      case 2: minor = parsed_minor; // no break!
      case 1: major = parsed_major; // no break!
      default: break;
      }
    }
}

//----------------------------------------------------------------------------
const char* cmTarget::GetFeature(const char* feature, const char* config)
{
  if(config && *config)
    {
    std::string featureConfig = feature;
    featureConfig += "_";
    featureConfig += cmSystemTools::UpperCase(config);
    if(const char* value = this->GetProperty(featureConfig.c_str()))
      {
      return value;
      }
    }
  if(const char* value = this->GetProperty(feature))
    {
    return value;
    }
  return this->Makefile->GetFeature(feature, config);
}

//----------------------------------------------------------------------------
const char *cmTarget::GetProperty(const char* prop)
{
  return this->GetProperty(prop, cmProperty::TARGET);
}

//----------------------------------------------------------------------------
bool cmTarget::HandleLocationPropertyPolicy()
{
  if (this->IsImported())
    {
    return true;
    }
  const char *modal = 0;
  cmake::MessageType messageType = cmake::AUTHOR_WARNING;
  switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0026))
    {
    case cmPolicies::WARN:
      modal = "should";
    case cmPolicies::OLD:
      break;
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::NEW:
      modal = "may";
      messageType = cmake::FATAL_ERROR;
    }

  if (modal)
    {
    cmOStringStream e;
    e << (this->Makefile->GetPolicies()
          ->GetPolicyWarning(cmPolicies::CMP0026)) << "\n";
    e << "The LOCATION property " << modal << " not be read from target \""
      << this->GetName() << "\".  Use the target name directly with "
      "add_custom_command, or use the generator expression $<TARGET_FILE>, "
      "as appropriate.\n";
    this->Makefile->IssueMessage(messageType, e.str().c_str());
    }

  return messageType != cmake::FATAL_ERROR;
}

//----------------------------------------------------------------------------
const char *cmTarget::GetProperty(const char* prop,
                                  cmProperty::ScopeType scope)
{
  if(!prop)
    {
    return 0;
    }

  if (strcmp(prop, "NAME") == 0)
    {
    return this->GetName();
    }

  // Watch for special "computed" properties that are dependent on
  // other properties or variables.  Always recompute them.
  if(this->GetType() == cmTarget::EXECUTABLE ||
     this->GetType() == cmTarget::STATIC_LIBRARY ||
     this->GetType() == cmTarget::SHARED_LIBRARY ||
     this->GetType() == cmTarget::MODULE_LIBRARY ||
     this->GetType() == cmTarget::INTERFACE_LIBRARY ||
     this->GetType() == cmTarget::UNKNOWN_LIBRARY)
    {
    if(strcmp(prop,"LOCATION") == 0)
      {
      if (!this->HandleLocationPropertyPolicy())
        {
        return 0;
        }

      // Set the LOCATION property of the target.
      //
      // For an imported target this is the location of an arbitrary
      // available configuration.
      //
      // For a non-imported target this is deprecated because it
      // cannot take into account the per-configuration name of the
      // target because the configuration type may not be known at
      // CMake time.
      this->SetProperty("LOCATION", this->GetLocation(0));
      }

    // Support "LOCATION_<CONFIG>".
    if(strncmp(prop, "LOCATION_", 9) == 0)
      {
      if (!this->HandleLocationPropertyPolicy())
        {
        return 0;
        }
      std::string configName = prop+9;
      this->SetProperty(prop, this->GetLocation(configName.c_str()));
      }
    else
      {
      // Support "<CONFIG>_LOCATION" for compatibility.
      int len = static_cast<int>(strlen(prop));
      if(len > 9 && strcmp(prop+len-9, "_LOCATION") == 0)
        {
        std::string configName(prop, len-9);
        if(configName != "IMPORTED")
          {
          if (!this->HandleLocationPropertyPolicy())
            {
            return 0;
            }
          this->SetProperty(prop, this->GetLocation(configName.c_str()));
          }
        }
      }
    }
  if(strcmp(prop,"INCLUDE_DIRECTORIES") == 0)
    {
    static std::string output;
    output = "";
    std::string sep;
    typedef cmTargetInternals::TargetPropertyEntry
                                TargetPropertyEntry;
    for (std::vector<TargetPropertyEntry*>::const_iterator
        it = this->Internal->IncludeDirectoriesEntries.begin(),
        end = this->Internal->IncludeDirectoriesEntries.end();
        it != end; ++it)
      {
      output += sep;
      output += (*it)->ge->GetInput();
      sep = ";";
      }
    return output.c_str();
    }
  if(strcmp(prop,"COMPILE_OPTIONS") == 0)
    {
    static std::string output;
    output = "";
    std::string sep;
    typedef cmTargetInternals::TargetPropertyEntry
                                TargetPropertyEntry;
    for (std::vector<TargetPropertyEntry*>::const_iterator
        it = this->Internal->CompileOptionsEntries.begin(),
        end = this->Internal->CompileOptionsEntries.end();
        it != end; ++it)
      {
      output += sep;
      output += (*it)->ge->GetInput();
      sep = ";";
      }
    return output.c_str();
    }
  if(strcmp(prop,"COMPILE_DEFINITIONS") == 0)
    {
    static std::string output;
    output = "";
    std::string sep;
    typedef cmTargetInternals::TargetPropertyEntry
                                TargetPropertyEntry;
    for (std::vector<TargetPropertyEntry*>::const_iterator
        it = this->Internal->CompileDefinitionsEntries.begin(),
        end = this->Internal->CompileDefinitionsEntries.end();
        it != end; ++it)
      {
      output += sep;
      output += (*it)->ge->GetInput();
      sep = ";";
      }
    return output.c_str();
    }
  if(strcmp(prop,"LINK_LIBRARIES") == 0)
    {
    static std::string output;
    output = "";
    std::string sep;
    for (std::vector<cmValueWithOrigin>::const_iterator
        it = this->Internal->LinkInterfacePropertyEntries.begin(),
        end = this->Internal->LinkInterfacePropertyEntries.end();
        it != end; ++it)
      {
      output += sep;
      output += it->Value;
      sep = ";";
      }
    return output.c_str();
    }

  if (strcmp(prop,"IMPORTED") == 0)
    {
    return this->IsImported()?"TRUE":"FALSE";
    }

  if(!strcmp(prop,"SOURCES"))
    {
    cmOStringStream ss;
    const char* sep = "";
    for(std::vector<cmSourceFile*>::const_iterator
          i = this->SourceFiles.begin();
        i != this->SourceFiles.end(); ++i)
      {
      // Separate from the previous list entries.
      ss << sep;
      sep = ";";

      // Construct what is known about this source file location.
      cmSourceFileLocation const& location = (*i)->GetLocation();
      std::string sname = location.GetDirectory();
      if(!sname.empty())
        {
        sname += "/";
        }
      sname += location.GetName();

      // Append this list entry.
      ss << sname;
      }
    this->SetProperty("SOURCES", ss.str().c_str());
    }

  // the type property returns what type the target is
  if (!strcmp(prop,"TYPE"))
    {
    return cmTarget::GetTargetTypeName(this->GetType());
    }
  bool chain = false;
  const char *retVal =
    this->Properties.GetPropertyValue(prop, scope, chain);
  if (chain)
    {
    return this->Makefile->GetProperty(prop,scope);
    }
  return retVal;
}

//----------------------------------------------------------------------------
bool cmTarget::GetPropertyAsBool(const char* prop)
{
  return cmSystemTools::IsOn(this->GetProperty(prop));
}

//----------------------------------------------------------------------------
class cmTargetCollectLinkLanguages
{
public:
  cmTargetCollectLinkLanguages(cmTarget* target, const char* config,
                               std::set<cmStdString>& languages,
                               cmTarget* head):
    Config(config), Languages(languages), HeadTarget(head)
  { this->Visited.insert(target); }

  void Visit(cmTarget* target)
    {
    if(!target || !this->Visited.insert(target).second)
      {
      return;
      }

    cmTarget::LinkInterface const* iface =
      target->GetLinkInterface(this->Config, this->HeadTarget);
    if(!iface) { return; }

    for(std::vector<std::string>::const_iterator
          li = iface->Languages.begin(); li != iface->Languages.end(); ++li)
      {
      this->Languages.insert(*li);
      }

    cmMakefile* mf = target->GetMakefile();
    for(std::vector<std::string>::const_iterator
          li = iface->Libraries.begin(); li != iface->Libraries.end(); ++li)
      {
      this->Visit(mf->FindTargetToUse(li->c_str()));
      }
    }
private:
  const char* Config;
  std::set<cmStdString>& Languages;
  cmTarget* HeadTarget;
  std::set<cmTarget*> Visited;
};

//----------------------------------------------------------------------------
const char* cmTarget::GetLinkerLanguage(const char* config, cmTarget *head)
{
  cmTarget *headTarget = head ? head : this;
  const char* lang = this->GetLinkClosure(config, headTarget)
                                                    ->LinkerLanguage.c_str();
  return *lang? lang : 0;
}

//----------------------------------------------------------------------------
cmTarget::LinkClosure const* cmTarget::GetLinkClosure(const char* config,
                                                      cmTarget *head)
{
  TargetConfigPair key(head, cmSystemTools::UpperCase(config ? config : ""));
  cmTargetInternals::LinkClosureMapType::iterator
    i = this->Internal->LinkClosureMap.find(key);
  if(i == this->Internal->LinkClosureMap.end())
    {
    LinkClosure lc;
    this->ComputeLinkClosure(config, lc, head);
    cmTargetInternals::LinkClosureMapType::value_type entry(key, lc);
    i = this->Internal->LinkClosureMap.insert(entry).first;
    }
  return &i->second;
}

//----------------------------------------------------------------------------
class cmTargetSelectLinker
{
  int Preference;
  cmTarget* Target;
  cmMakefile* Makefile;
  cmGlobalGenerator* GG;
  std::set<cmStdString> Preferred;
public:
  cmTargetSelectLinker(cmTarget* target): Preference(0), Target(target)
    {
    this->Makefile = this->Target->GetMakefile();
    this->GG = this->Makefile->GetLocalGenerator()->GetGlobalGenerator();
    }
  void Consider(const char* lang)
    {
    int preference = this->GG->GetLinkerPreference(lang);
    if(preference > this->Preference)
      {
      this->Preference = preference;
      this->Preferred.clear();
      }
    if(preference == this->Preference)
      {
      this->Preferred.insert(lang);
      }
    }
  std::string Choose()
    {
    if(this->Preferred.empty())
      {
      return "";
      }
    else if(this->Preferred.size() > 1)
      {
      cmOStringStream e;
      e << "Target " << this->Target->GetName()
        << " contains multiple languages with the highest linker preference"
        << " (" << this->Preference << "):\n";
      for(std::set<cmStdString>::const_iterator
            li = this->Preferred.begin(); li != this->Preferred.end(); ++li)
        {
        e << "  " << *li << "\n";
        }
      e << "Set the LINKER_LANGUAGE property for this target.";
      cmake* cm = this->Makefile->GetCMakeInstance();
      cm->IssueMessage(cmake::FATAL_ERROR, e.str(),
                       this->Target->GetBacktrace());
      }
    return *this->Preferred.begin();
    }
};

//----------------------------------------------------------------------------
void cmTarget::ComputeLinkClosure(const char* config, LinkClosure& lc,
                                  cmTarget *head)
{
  // Get languages built in this target.
  std::set<cmStdString> languages;
  LinkImplementation const* impl = this->GetLinkImplementation(config, head);
  for(std::vector<std::string>::const_iterator li = impl->Languages.begin();
      li != impl->Languages.end(); ++li)
    {
    languages.insert(*li);
    }

  // Add interface languages from linked targets.
  cmTargetCollectLinkLanguages cll(this, config, languages, head);
  for(std::vector<std::string>::const_iterator li = impl->Libraries.begin();
      li != impl->Libraries.end(); ++li)
    {
    cll.Visit(this->Makefile->FindTargetToUse(li->c_str()));
    }

  // Store the transitive closure of languages.
  for(std::set<cmStdString>::const_iterator li = languages.begin();
      li != languages.end(); ++li)
    {
    lc.Languages.push_back(*li);
    }

  // Choose the language whose linker should be used.
  if(this->GetProperty("HAS_CXX"))
    {
    lc.LinkerLanguage = "CXX";
    }
  else if(const char* linkerLang = this->GetProperty("LINKER_LANGUAGE"))
    {
    lc.LinkerLanguage = linkerLang;
    }
  else
    {
    // Find the language with the highest preference value.
    cmTargetSelectLinker tsl(this);

    // First select from the languages compiled directly in this target.
    for(std::vector<std::string>::const_iterator li = impl->Languages.begin();
        li != impl->Languages.end(); ++li)
      {
      tsl.Consider(li->c_str());
      }

    // Now consider languages that propagate from linked targets.
    for(std::set<cmStdString>::const_iterator sit = languages.begin();
        sit != languages.end(); ++sit)
      {
      std::string propagates = "CMAKE_"+*sit+"_LINKER_PREFERENCE_PROPAGATES";
      if(this->Makefile->IsOn(propagates.c_str()))
        {
        tsl.Consider(sit->c_str());
        }
      }

    lc.LinkerLanguage = tsl.Choose();
    }
}

//----------------------------------------------------------------------------
const char* cmTarget::GetSuffixVariableInternal(bool implib)
{
  switch(this->GetType())
    {
    case cmTarget::STATIC_LIBRARY:
      return "CMAKE_STATIC_LIBRARY_SUFFIX";
    case cmTarget::SHARED_LIBRARY:
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_SUFFIX"
              : "CMAKE_SHARED_LIBRARY_SUFFIX");
    case cmTarget::MODULE_LIBRARY:
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_SUFFIX"
              : "CMAKE_SHARED_MODULE_SUFFIX");
    case cmTarget::EXECUTABLE:
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_SUFFIX"
              : "CMAKE_EXECUTABLE_SUFFIX");
    default:
      break;
    }
  return "";
}


//----------------------------------------------------------------------------
const char* cmTarget::GetPrefixVariableInternal(bool implib)
{
  switch(this->GetType())
    {
    case cmTarget::STATIC_LIBRARY:
      return "CMAKE_STATIC_LIBRARY_PREFIX";
    case cmTarget::SHARED_LIBRARY:
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_PREFIX"
              : "CMAKE_SHARED_LIBRARY_PREFIX");
    case cmTarget::MODULE_LIBRARY:
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_PREFIX"
              : "CMAKE_SHARED_MODULE_PREFIX");
    case cmTarget::EXECUTABLE:
      return (implib? "CMAKE_IMPORT_LIBRARY_PREFIX" : "");
    default:
      break;
    }
  return "";
}

//----------------------------------------------------------------------------
std::string cmTarget::GetPDBName(const char* config)
{
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, false, prefix, base, suffix);

  std::vector<std::string> props;
  std::string configUpper =
    cmSystemTools::UpperCase(config? config : "");
  if(!configUpper.empty())
    {
    // PDB_NAME_<CONFIG>
    props.push_back("PDB_NAME_" + configUpper);
    }

  // PDB_NAME
  props.push_back("PDB_NAME");

  for(std::vector<std::string>::const_iterator i = props.begin();
      i != props.end(); ++i)
    {
    if(const char* outName = this->GetProperty(i->c_str()))
      {
      base = outName;
      break;
      }
    }
  return prefix+base+".pdb";
}

//----------------------------------------------------------------------------
bool cmTarget::HasSOName(const char* config)
{
  // soname is supported only for shared libraries and modules,
  // and then only when the platform supports an soname flag.
  return ((this->GetType() == cmTarget::SHARED_LIBRARY ||
           this->GetType() == cmTarget::MODULE_LIBRARY) &&
          !this->GetPropertyAsBool("NO_SONAME") &&
          this->Makefile->GetSONameFlag(this->GetLinkerLanguage(config,
                                                                this)));
}

//----------------------------------------------------------------------------
std::string cmTarget::GetSOName(const char* config)
{
  if(this->IsImported())
    {
    // Lookup the imported soname.
    if(cmTarget::ImportInfo const* info = this->GetImportInfo(config, this))
      {
      if(info->NoSOName)
        {
        // The imported library has no builtin soname so the name
        // searched at runtime will be just the filename.
        return cmSystemTools::GetFilenameName(info->Location);
        }
      else
        {
        // Use the soname given if any.
        if(info->SOName.find("@rpath/") == 0)
          {
          return info->SOName.substr(6);
          }
        return info->SOName;
        }
      }
    else
      {
      return "";
      }
    }
  else
    {
    // Compute the soname that will be built.
    std::string name;
    std::string soName;
    std::string realName;
    std::string impName;
    std::string pdbName;
    this->GetLibraryNames(name, soName, realName, impName, pdbName, config);
    return soName;
    }
}

//----------------------------------------------------------------------------
bool cmTarget::HasMacOSXRpath(const char* config)
{
  bool install_name_is_rpath = false;
  bool macosx_rpath = this->GetPropertyAsBool("MACOSX_RPATH");

  if(!this->IsImportedTarget)
    {
    const char* install_name = this->GetProperty("INSTALL_NAME_DIR");
    bool use_install_name =
      this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH");
    if(install_name && use_install_name &&
       std::string(install_name) == "@rpath")
      {
      install_name_is_rpath = true;
      }
    else if(install_name && use_install_name)
      {
      return false;
      }
    }
  else
    {
    // Lookup the imported soname.
    if(cmTarget::ImportInfo const* info = this->GetImportInfo(config, this))
      {
      if(!info->NoSOName && !info->SOName.empty())
        {
        if(info->SOName.find("@rpath/") == 0)
          {
          install_name_is_rpath = true;
          }
        }
      else
        {
        std::string install_name;
        cmSystemTools::GuessLibraryInstallName(info->Location, install_name);
        if(install_name.find("@rpath") != std::string::npos)
          {
          install_name_is_rpath = true;
          }
        }
      }
    }

  if(!install_name_is_rpath && !macosx_rpath)
    {
    return false;
    }

  if(!this->Makefile->IsSet("CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG"))
    {
    cmOStringStream w;
    w << "Attempting to use";
    if(macosx_rpath)
      {
      w << " MACOSX_RPATH";
      }
    else
      {
      w << " @rpath";
      }
    w << " without CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG being set.";
    w << "  This could be because you are using a Mac OS X version";
    w << " less than 10.5 or because CMake's platform configuration is";
    w << " corrupt.";
    cmake* cm = this->Makefile->GetCMakeInstance();
    cm->IssueMessage(cmake::FATAL_ERROR, w.str(), this->GetBacktrace());
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmTarget::IsImportedSharedLibWithoutSOName(const char* config)
{
  if(this->IsImported() && this->GetType() == cmTarget::SHARED_LIBRARY)
    {
    if(cmTarget::ImportInfo const* info = this->GetImportInfo(config, this))
      {
      return info->NoSOName;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
std::string cmTarget::NormalGetRealName(const char* config)
{
  // This should not be called for imported targets.
  // TODO: Split cmTarget into a class hierarchy to get compile-time
  // enforcement of the limited imported target API.
  if(this->IsImported())
    {
    std::string msg =  "NormalGetRealName called on imported target: ";
    msg += this->GetName();
    this->GetMakefile()->
      IssueMessage(cmake::INTERNAL_ERROR,
                   msg.c_str());
    }

  if(this->GetType() == cmTarget::EXECUTABLE)
    {
    // Compute the real name that will be built.
    std::string name;
    std::string realName;
    std::string impName;
    std::string pdbName;
    this->GetExecutableNames(name, realName, impName, pdbName, config);
    return realName;
    }
  else
    {
    // Compute the real name that will be built.
    std::string name;
    std::string soName;
    std::string realName;
    std::string impName;
    std::string pdbName;
    this->GetLibraryNames(name, soName, realName, impName, pdbName, config);
    return realName;
    }
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullName(const char* config, bool implib)
{
  if(this->IsImported())
    {
    return this->GetFullNameImported(config, implib);
    }
  else
    {
    return this->GetFullNameInternal(config, implib);
    }
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullNameImported(const char* config, bool implib)
{
  return cmSystemTools::GetFilenameName(
    this->ImportedGetFullPath(config, implib));
}

//----------------------------------------------------------------------------
void cmTarget::GetFullNameComponents(std::string& prefix, std::string& base,
                                     std::string& suffix, const char* config,
                                     bool implib)
{
  this->GetFullNameInternal(config, implib, prefix, base, suffix);
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullPath(const char* config, bool implib,
                                  bool realname)
{
  if(this->IsImported())
    {
    return this->ImportedGetFullPath(config, implib);
    }
  else
    {
    return this->NormalGetFullPath(config, implib, realname);
    }
}

//----------------------------------------------------------------------------
std::string cmTarget::NormalGetFullPath(const char* config, bool implib,
                                        bool realname)
{
  std::string fpath = this->GetDirectory(config, implib);
  fpath += "/";
  if(this->IsAppBundleOnApple())
    {
    fpath = this->BuildMacContentDirectory(fpath, config, false);
    fpath += "/";
    }

  // Add the full name of the target.
  if(implib)
    {
    fpath += this->GetFullName(config, true);
    }
  else if(realname)
    {
    fpath += this->NormalGetRealName(config);
    }
  else
    {
    fpath += this->GetFullName(config, false);
    }
  return fpath;
}

//----------------------------------------------------------------------------
std::string cmTarget::ImportedGetFullPath(const char* config, bool implib)
{
  std::string result;
  if(cmTarget::ImportInfo const* info = this->GetImportInfo(config, this))
    {
    result = implib? info->ImportLibrary : info->Location;
    }
  if(result.empty())
    {
    result = this->GetName();
    result += "-NOTFOUND";
    }
  return result;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullNameInternal(const char* config, bool implib)
{
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, implib, prefix, base, suffix);
  return prefix+base+suffix;
}

//----------------------------------------------------------------------------
void cmTarget::GetFullNameInternal(const char* config,
                                   bool implib,
                                   std::string& outPrefix,
                                   std::string& outBase,
                                   std::string& outSuffix)
{
  // Use just the target name for non-main target types.
  if(this->GetType() != cmTarget::STATIC_LIBRARY &&
     this->GetType() != cmTarget::SHARED_LIBRARY &&
     this->GetType() != cmTarget::MODULE_LIBRARY &&
     this->GetType() != cmTarget::EXECUTABLE)
    {
    outPrefix = "";
    outBase = this->GetName();
    outSuffix = "";
    return;
    }

  // Return an empty name for the import library if this platform
  // does not support import libraries.
  if(implib &&
     !this->Makefile->GetDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX"))
    {
    outPrefix = "";
    outBase = "";
    outSuffix = "";
    return;
    }

  // The implib option is only allowed for shared libraries, module
  // libraries, and executables.
  if(this->GetType() != cmTarget::SHARED_LIBRARY &&
     this->GetType() != cmTarget::MODULE_LIBRARY &&
     this->GetType() != cmTarget::EXECUTABLE)
    {
    implib = false;
    }

  // Compute the full name for main target types.
  const char* targetPrefix = (implib
                              ? this->GetProperty("IMPORT_PREFIX")
                              : this->GetProperty("PREFIX"));
  const char* targetSuffix = (implib
                              ? this->GetProperty("IMPORT_SUFFIX")
                              : this->GetProperty("SUFFIX"));
  const char* configPostfix = 0;
  if(config && *config)
    {
    std::string configProp = cmSystemTools::UpperCase(config);
    configProp += "_POSTFIX";
    configPostfix = this->GetProperty(configProp.c_str());
    // Mac application bundles and frameworks have no postfix.
    if(configPostfix &&
       (this->IsAppBundleOnApple() || this->IsFrameworkOnApple()))
      {
      configPostfix = 0;
      }
    }
  const char* prefixVar = this->GetPrefixVariableInternal(implib);
  const char* suffixVar = this->GetSuffixVariableInternal(implib);

  // Check for language-specific default prefix and suffix.
  if(const char* ll = this->GetLinkerLanguage(config, this))
    {
    if(!targetSuffix && suffixVar && *suffixVar)
      {
      std::string langSuff = suffixVar + std::string("_") + ll;
      targetSuffix = this->Makefile->GetDefinition(langSuff.c_str());
      }
    if(!targetPrefix && prefixVar && *prefixVar)
      {
      std::string langPrefix = prefixVar + std::string("_") + ll;
      targetPrefix = this->Makefile->GetDefinition(langPrefix.c_str());
      }
    }

  // if there is no prefix on the target use the cmake definition
  if(!targetPrefix && prefixVar)
    {
    targetPrefix = this->Makefile->GetSafeDefinition(prefixVar);
    }
  // if there is no suffix on the target use the cmake definition
  if(!targetSuffix && suffixVar)
    {
    targetSuffix = this->Makefile->GetSafeDefinition(suffixVar);
    }

  // frameworks have directory prefix but no suffix
  std::string fw_prefix;
  if(this->IsFrameworkOnApple())
    {
    fw_prefix = this->GetOutputName(config, false);
    fw_prefix += ".framework/";
    targetPrefix = fw_prefix.c_str();
    targetSuffix = 0;
    }

  if(this->IsCFBundleOnApple())
    {
    fw_prefix = this->GetOutputName(config, false);
    fw_prefix += ".";
    const char *ext = this->GetProperty("BUNDLE_EXTENSION");
    if (!ext)
      {
      ext = "bundle";
      }
    fw_prefix += ext;
    fw_prefix += "/Contents/MacOS/";
    targetPrefix = fw_prefix.c_str();
    targetSuffix = 0;
    }

  // Begin the final name with the prefix.
  outPrefix = targetPrefix?targetPrefix:"";

  // Append the target name or property-specified name.
  outBase += this->GetOutputName(config, implib);

  // Append the per-configuration postfix.
  outBase += configPostfix?configPostfix:"";

  // Name shared libraries with their version number on some platforms.
  if(const char* soversion = this->GetProperty("SOVERSION"))
    {
    if(this->GetType() == cmTarget::SHARED_LIBRARY && !implib &&
       this->Makefile->IsOn("CMAKE_SHARED_LIBRARY_NAME_WITH_VERSION"))
      {
      outBase += "-";
      outBase += soversion;
      }
    }

  // Append the suffix.
  outSuffix = targetSuffix?targetSuffix:"";
}

//----------------------------------------------------------------------------
void cmTarget::GetLibraryNames(std::string& name,
                               std::string& soName,
                               std::string& realName,
                               std::string& impName,
                               std::string& pdbName,
                               const char* config)
{
  // This should not be called for imported targets.
  // TODO: Split cmTarget into a class hierarchy to get compile-time
  // enforcement of the limited imported target API.
  if(this->IsImported())
    {
    std::string msg =  "GetLibraryNames called on imported target: ";
    msg += this->GetName();
    this->Makefile->IssueMessage(cmake::INTERNAL_ERROR,
                                 msg.c_str());
    return;
    }

  // Check for library version properties.
  const char* version = this->GetProperty("VERSION");
  const char* soversion = this->GetProperty("SOVERSION");
  if(!this->HasSOName(config) ||
     this->IsFrameworkOnApple())
    {
    // Versioning is supported only for shared libraries and modules,
    // and then only when the platform supports an soname flag.
    version = 0;
    soversion = 0;
    }
  if(version && !soversion)
    {
    // The soversion must be set if the library version is set.  Use
    // the library version as the soversion.
    soversion = version;
    }
  if(!version && soversion)
    {
    // Use the soversion as the library version.
    version = soversion;
    }

  // Get the components of the library name.
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, false, prefix, base, suffix);

  // The library name.
  name = prefix+base+suffix;

  if(this->IsFrameworkOnApple())
    {
    realName = prefix;
    realName += "Versions/";
    realName += this->GetFrameworkVersion();
    realName += "/";
    realName += base;
    soName = realName;
    }
  else
    {
    // The library's soname.
    this->ComputeVersionedName(soName, prefix, base, suffix,
                               name, soversion);
    // The library's real name on disk.
    this->ComputeVersionedName(realName, prefix, base, suffix,
                               name, version);
    }

  // The import library name.
  if(this->GetType() == cmTarget::SHARED_LIBRARY ||
     this->GetType() == cmTarget::MODULE_LIBRARY)
    {
    impName = this->GetFullNameInternal(config, true);
    }
  else
    {
    impName = "";
    }

  // The program database file name.
  pdbName = this->GetPDBName(config);
}

//----------------------------------------------------------------------------
void cmTarget::ComputeVersionedName(std::string& vName,
                                    std::string const& prefix,
                                    std::string const& base,
                                    std::string const& suffix,
                                    std::string const& name,
                                    const char* version)
{
  vName = this->IsApple? (prefix+base) : name;
  if(version)
    {
    vName += ".";
    vName += version;
    }
  vName += this->IsApple? suffix : std::string();
}

//----------------------------------------------------------------------------
void cmTarget::GetExecutableNames(std::string& name,
                                  std::string& realName,
                                  std::string& impName,
                                  std::string& pdbName,
                                  const char* config)
{
  // This should not be called for imported targets.
  // TODO: Split cmTarget into a class hierarchy to get compile-time
  // enforcement of the limited imported target API.
  if(this->IsImported())
    {
    std::string msg =
      "GetExecutableNames called on imported target: ";
    msg += this->GetName();
    this->GetMakefile()->IssueMessage(cmake::INTERNAL_ERROR, msg.c_str());
    }

  // This versioning is supported only for executables and then only
  // when the platform supports symbolic links.
#if defined(_WIN32) && !defined(__CYGWIN__)
  const char* version = 0;
#else
  // Check for executable version properties.
  const char* version = this->GetProperty("VERSION");
  if(this->GetType() != cmTarget::EXECUTABLE || this->Makefile->IsOn("XCODE"))
    {
    version = 0;
    }
#endif

  // Get the components of the executable name.
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, false, prefix, base, suffix);

  // The executable name.
  name = prefix+base+suffix;

  // The executable's real name on disk.
#if defined(__CYGWIN__)
  realName = prefix+base;
#else
  realName = name;
#endif
  if(version)
    {
    realName += "-";
    realName += version;
    }
#if defined(__CYGWIN__)
  realName += suffix;
#endif

  // The import library name.
  impName = this->GetFullNameInternal(config, true);

  // The program database file name.
  pdbName = this->GetPDBName(config);
}

//----------------------------------------------------------------------------
bool cmTarget::HasImplibGNUtoMS()
{
  return this->HasImportLibrary() && this->GetPropertyAsBool("GNUtoMS");
}

//----------------------------------------------------------------------------
bool cmTarget::GetImplibGNUtoMS(std::string const& gnuName,
                                std::string& out, const char* newExt)
{
  if(this->HasImplibGNUtoMS() &&
     gnuName.size() > 6 && gnuName.substr(gnuName.size()-6) == ".dll.a")
    {
    out = gnuName.substr(0, gnuName.size()-6);
    out += newExt? newExt : ".lib";
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void cmTarget::GenerateTargetManifest(const char* config)
{
  cmMakefile* mf = this->Makefile;
  cmLocalGenerator* lg = mf->GetLocalGenerator();
  cmGlobalGenerator* gg = lg->GetGlobalGenerator();

  // Get the names.
  std::string name;
  std::string soName;
  std::string realName;
  std::string impName;
  std::string pdbName;
  if(this->GetType() == cmTarget::EXECUTABLE)
    {
    this->GetExecutableNames(name, realName, impName, pdbName, config);
    }
  else if(this->GetType() == cmTarget::STATIC_LIBRARY ||
          this->GetType() == cmTarget::SHARED_LIBRARY ||
          this->GetType() == cmTarget::MODULE_LIBRARY)
    {
    this->GetLibraryNames(name, soName, realName, impName, pdbName, config);
    }
  else
    {
    return;
    }

  // Get the directory.
  std::string dir = this->GetDirectory(config, false);

  // Add each name.
  std::string f;
  if(!name.empty())
    {
    f = dir;
    f += "/";
    f += name;
    gg->AddToManifest(config? config:"", f);
    }
  if(!soName.empty())
    {
    f = dir;
    f += "/";
    f += soName;
    gg->AddToManifest(config? config:"", f);
    }
  if(!realName.empty())
    {
    f = dir;
    f += "/";
    f += realName;
    gg->AddToManifest(config? config:"", f);
    }
  if(!pdbName.empty())
    {
    f = this->GetPDBDirectory(config);
    f += "/";
    f += pdbName;
    gg->AddToManifest(config? config:"", f);
    }
  if(!impName.empty())
    {
    f = this->GetDirectory(config, true);
    f += "/";
    f += impName;
    gg->AddToManifest(config? config:"", f);
    }
}

//----------------------------------------------------------------------------
void cmTarget::SetPropertyDefault(const char* property,
                                  const char* default_value)
{
  // Compute the name of the variable holding the default value.
  std::string var = "CMAKE_";
  var += property;

  if(const char* value = this->Makefile->GetDefinition(var.c_str()))
    {
    this->SetProperty(property, value);
    }
  else if(default_value)
    {
    this->SetProperty(property, default_value);
    }
}

//----------------------------------------------------------------------------
bool cmTarget::HaveBuildTreeRPATH(const char *config)
{
  if (this->GetPropertyAsBool("SKIP_BUILD_RPATH"))
    {
    return false;
    }
  std::vector<std::string> libs;
  this->GetDirectLinkLibraries(config, libs, this);
  return !libs.empty();
}

//----------------------------------------------------------------------------
bool cmTarget::HaveInstallTreeRPATH()
{
  const char* install_rpath = this->GetProperty("INSTALL_RPATH");
  return (install_rpath && *install_rpath) &&
          !this->Makefile->IsOn("CMAKE_SKIP_INSTALL_RPATH");
}

//----------------------------------------------------------------------------
bool cmTarget::NeedRelinkBeforeInstall(const char* config)
{
  // Only executables and shared libraries can have an rpath and may
  // need relinking.
  if(this->TargetTypeValue != cmTarget::EXECUTABLE &&
     this->TargetTypeValue != cmTarget::SHARED_LIBRARY &&
     this->TargetTypeValue != cmTarget::MODULE_LIBRARY)
    {
    return false;
    }

  // If there is no install location this target will not be installed
  // and therefore does not need relinking.
  if(!this->GetHaveInstallRule())
    {
    return false;
    }

  // If skipping all rpaths completely then no relinking is needed.
  if(this->Makefile->IsOn("CMAKE_SKIP_RPATH"))
    {
    return false;
    }

  // If building with the install-tree rpath no relinking is needed.
  if(this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH"))
    {
    return false;
    }

  // If chrpath is going to be used no relinking is needed.
  if(this->IsChrpathUsed(config))
    {
    return false;
    }

  // Check for rpath support on this platform.
  if(const char* ll = this->GetLinkerLanguage(config, this))
    {
    std::string flagVar = "CMAKE_SHARED_LIBRARY_RUNTIME_";
    flagVar += ll;
    flagVar += "_FLAG";
    if(!this->Makefile->IsSet(flagVar.c_str()))
      {
      // There is no rpath support on this platform so nothing needs
      // relinking.
      return false;
      }
    }
  else
    {
    // No linker language is known.  This error will be reported by
    // other code.
    return false;
    }

  // If either a build or install tree rpath is set then the rpath
  // will likely change between the build tree and install tree and
  // this target must be relinked.
  return this->HaveBuildTreeRPATH(config) || this->HaveInstallTreeRPATH();
}

//----------------------------------------------------------------------------
std::string cmTarget::GetInstallNameDirForBuildTree(const char* config)
{
  // If building directly for installation then the build tree install_name
  // is the same as the install tree.
  if(this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH"))
    {
    return GetInstallNameDirForInstallTree();
    }

  // Use the build tree directory for the target.
  if(this->Makefile->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME") &&
     !this->Makefile->IsOn("CMAKE_SKIP_RPATH") &&
     !this->GetPropertyAsBool("SKIP_BUILD_RPATH"))
    {
    std::string dir;
    if(this->GetPropertyAsBool("MACOSX_RPATH"))
      {
      dir = "@rpath";
      }
    else
      {
      dir = this->GetDirectory(config);
      }
    dir += "/";
    return dir;
    }
  else
    {
    return "";
    }
}

//----------------------------------------------------------------------------
std::string cmTarget::GetInstallNameDirForInstallTree()
{
  if(this->Makefile->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME"))
    {
    std::string dir;
    const char* install_name_dir = this->GetProperty("INSTALL_NAME_DIR");

    if(!this->Makefile->IsOn("CMAKE_SKIP_RPATH") &&
       !this->Makefile->IsOn("CMAKE_SKIP_INSTALL_RPATH"))
      {
      if(install_name_dir && *install_name_dir)
        {
        dir = install_name_dir;
        dir += "/";
        }
      }
    if(!install_name_dir && this->GetPropertyAsBool("MACOSX_RPATH"))
      {
      dir = "@rpath/";
      }
    return dir;
    }
  else
    {
    return "";
    }
}

//----------------------------------------------------------------------------
const char* cmTarget::GetOutputTargetType(bool implib)
{
  switch(this->GetType())
    {
    case cmTarget::SHARED_LIBRARY:
      if(this->DLLPlatform)
        {
        if(implib)
          {
          // A DLL import library is treated as an archive target.
          return "ARCHIVE";
          }
        else
          {
          // A DLL shared library is treated as a runtime target.
          return "RUNTIME";
          }
        }
      else
        {
        // For non-DLL platforms shared libraries are treated as
        // library targets.
        return "LIBRARY";
        }
    case cmTarget::STATIC_LIBRARY:
      // Static libraries are always treated as archive targets.
      return "ARCHIVE";
    case cmTarget::MODULE_LIBRARY:
      if(implib)
        {
        // Module libraries are always treated as library targets.
        return "ARCHIVE";
        }
      else
        {
        // Module import libraries are treated as archive targets.
        return "LIBRARY";
        }
    case cmTarget::EXECUTABLE:
      if(implib)
        {
        // Executable import libraries are treated as archive targets.
        return "ARCHIVE";
        }
      else
        {
        // Executables are always treated as runtime targets.
        return "RUNTIME";
        }
    default:
      break;
    }
  return "";
}

//----------------------------------------------------------------------------
bool cmTarget::ComputeOutputDir(const char* config,
                                bool implib, std::string& out)
{
  bool usesDefaultOutputDir = false;

  // Look for a target property defining the target output directory
  // based on the target type.
  std::string targetTypeName = this->GetOutputTargetType(implib);
  const char* propertyName = 0;
  std::string propertyNameStr = targetTypeName;
  if(!propertyNameStr.empty())
    {
    propertyNameStr += "_OUTPUT_DIRECTORY";
    propertyName = propertyNameStr.c_str();
    }

  // Check for a per-configuration output directory target property.
  std::string configUpper = cmSystemTools::UpperCase(config? config : "");
  const char* configProp = 0;
  std::string configPropStr = targetTypeName;
  if(!configPropStr.empty())
    {
    configPropStr += "_OUTPUT_DIRECTORY_";
    configPropStr += configUpper;
    configProp = configPropStr.c_str();
    }

  // Select an output directory.
  if(const char* config_outdir = this->GetProperty(configProp))
    {
    // Use the user-specified per-configuration output directory.
    out = config_outdir;

    // Skip per-configuration subdirectory.
    config = 0;
    }
  else if(const char* outdir = this->GetProperty(propertyName))
    {
    // Use the user-specified output directory.
    out = outdir;
    }
  else if(this->GetType() == cmTarget::EXECUTABLE)
    {
    // Lookup the output path for executables.
    out = this->Makefile->GetSafeDefinition("EXECUTABLE_OUTPUT_PATH");
    }
  else if(this->GetType() == cmTarget::STATIC_LIBRARY ||
          this->GetType() == cmTarget::SHARED_LIBRARY ||
          this->GetType() == cmTarget::MODULE_LIBRARY)
    {
    // Lookup the output path for libraries.
    out = this->Makefile->GetSafeDefinition("LIBRARY_OUTPUT_PATH");
    }
  if(out.empty())
    {
    // Default to the current output directory.
    usesDefaultOutputDir = true;
    out = ".";
    }

  // Convert the output path to a full path in case it is
  // specified as a relative path.  Treat a relative path as
  // relative to the current output directory for this makefile.
  out = (cmSystemTools::CollapseFullPath
         (out.c_str(), this->Makefile->GetStartOutputDirectory()));

  // The generator may add the configuration's subdirectory.
  if(config && *config)
    {
    const char *platforms = this->Makefile->GetDefinition(
      "CMAKE_XCODE_EFFECTIVE_PLATFORMS");
    std::string suffix =
      usesDefaultOutputDir && platforms ? "$(EFFECTIVE_PLATFORM_NAME)" : "";
    this->Makefile->GetLocalGenerator()->GetGlobalGenerator()->
      AppendDirectoryForConfig("/", config, suffix.c_str(), out);
    }

  return usesDefaultOutputDir;
}

//----------------------------------------------------------------------------
bool cmTarget::ComputePDBOutputDir(const char* config, std::string& out)
{
  // Look for a target property defining the target output directory
  // based on the target type.
  std::string targetTypeName = "PDB";
  const char* propertyName = 0;
  std::string propertyNameStr = targetTypeName;
  if(!propertyNameStr.empty())
    {
    propertyNameStr += "_OUTPUT_DIRECTORY";
    propertyName = propertyNameStr.c_str();
    }

  // Check for a per-configuration output directory target property.
  std::string configUpper = cmSystemTools::UpperCase(config? config : "");
  const char* configProp = 0;
  std::string configPropStr = targetTypeName;
  if(!configPropStr.empty())
    {
    configPropStr += "_OUTPUT_DIRECTORY_";
    configPropStr += configUpper;
    configProp = configPropStr.c_str();
    }

  // Select an output directory.
  if(const char* config_outdir = this->GetProperty(configProp))
    {
    // Use the user-specified per-configuration output directory.
    out = config_outdir;

    // Skip per-configuration subdirectory.
    config = 0;
    }
  else if(const char* outdir = this->GetProperty(propertyName))
    {
    // Use the user-specified output directory.
    out = outdir;
    }
  if(out.empty())
    {
    return false;
    }

  // Convert the output path to a full path in case it is
  // specified as a relative path.  Treat a relative path as
  // relative to the current output directory for this makefile.
  out = (cmSystemTools::CollapseFullPath
         (out.c_str(), this->Makefile->GetStartOutputDirectory()));

  // The generator may add the configuration's subdirectory.
  if(config && *config)
    {
    this->Makefile->GetLocalGenerator()->GetGlobalGenerator()->
      AppendDirectoryForConfig("/", config, "", out);
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmTarget::UsesDefaultOutputDir(const char* config, bool implib)
{
  std::string dir;
  return this->ComputeOutputDir(config, implib, dir);
}

//----------------------------------------------------------------------------
std::string cmTarget::GetOutputName(const char* config, bool implib)
{
  std::vector<std::string> props;
  std::string type = this->GetOutputTargetType(implib);
  std::string configUpper = cmSystemTools::UpperCase(config? config : "");
  if(!type.empty() && !configUpper.empty())
    {
    // <ARCHIVE|LIBRARY|RUNTIME>_OUTPUT_NAME_<CONFIG>
    props.push_back(type + "_OUTPUT_NAME_" + configUpper);
    }
  if(!type.empty())
    {
    // <ARCHIVE|LIBRARY|RUNTIME>_OUTPUT_NAME
    props.push_back(type + "_OUTPUT_NAME");
    }
  if(!configUpper.empty())
    {
    // OUTPUT_NAME_<CONFIG>
    props.push_back("OUTPUT_NAME_" + configUpper);
    // <CONFIG>_OUTPUT_NAME
    props.push_back(configUpper + "_OUTPUT_NAME");
    }
  // OUTPUT_NAME
  props.push_back("OUTPUT_NAME");

  for(std::vector<std::string>::const_iterator i = props.begin();
      i != props.end(); ++i)
    {
    if(const char* outName = this->GetProperty(i->c_str()))
      {
      return outName;
      }
    }
  return this->GetName();
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFrameworkVersion()
{
  if(const char* fversion = this->GetProperty("FRAMEWORK_VERSION"))
    {
    return fversion;
    }
  else if(const char* tversion = this->GetProperty("VERSION"))
    {
    return tversion;
    }
  else
    {
    return "A";
    }
}

//----------------------------------------------------------------------------
const char* cmTarget::GetExportMacro()
{
  // Define the symbol for targets that export symbols.
  if(this->GetType() == cmTarget::SHARED_LIBRARY ||
     this->GetType() == cmTarget::MODULE_LIBRARY ||
     this->IsExecutableWithExports())
    {
    if(const char* custom_export_name = this->GetProperty("DEFINE_SYMBOL"))
      {
      this->ExportMacro = custom_export_name;
      }
    else
      {
      std::string in = this->GetName();
      in += "_EXPORTS";
      this->ExportMacro = cmSystemTools::MakeCindentifier(in.c_str());
      }
    return this->ExportMacro.c_str();
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
bool cmTarget::IsNullImpliedByLinkLibraries(const std::string &p)
{
  return this->LinkImplicitNullProperties.find(p)
      != this->LinkImplicitNullProperties.end();
}

//----------------------------------------------------------------------------
template<typename PropertyType>
PropertyType getTypedProperty(cmTarget *tgt, const char *prop,
                              PropertyType *);

//----------------------------------------------------------------------------
template<>
bool getTypedProperty<bool>(cmTarget *tgt, const char *prop, bool *)
{
  return tgt->GetPropertyAsBool(prop);
}

//----------------------------------------------------------------------------
template<>
const char *getTypedProperty<const char *>(cmTarget *tgt, const char *prop,
                                          const char **)
{
  return tgt->GetProperty(prop);
}

//----------------------------------------------------------------------------
template<typename PropertyType>
bool consistentProperty(PropertyType lhs, PropertyType rhs);

//----------------------------------------------------------------------------
template<>
bool consistentProperty(bool lhs, bool rhs)
{
  return lhs == rhs;
}

//----------------------------------------------------------------------------
template<>
bool consistentProperty(const char *lhs, const char *rhs)
{
  if (!lhs && !rhs)
    return true;
  if (!lhs || !rhs)
    return false;
  return strcmp(lhs, rhs) == 0;
}

//----------------------------------------------------------------------------
template<typename PropertyType>
PropertyType checkInterfacePropertyCompatibility(cmTarget *tgt,
                                          const std::string &p,
                                          const char *config,
                                          const char *defaultValue,
                                          PropertyType *)
{
  PropertyType propContent = getTypedProperty<PropertyType>(tgt, p.c_str(),
                                                            0);
  const bool explicitlySet = tgt->GetProperties()
                                  .find(p.c_str())
                                  != tgt->GetProperties().end();
  const bool impliedByUse =
          tgt->IsNullImpliedByLinkLibraries(p);
  assert((impliedByUse ^ explicitlySet)
      || (!impliedByUse && !explicitlySet));

  cmComputeLinkInformation *info = tgt->GetLinkInformation(config);
  if(!info)
    {
    return propContent;
    }
  const cmComputeLinkInformation::ItemVector &deps = info->GetItems();
  bool propInitialized = explicitlySet;

  for(cmComputeLinkInformation::ItemVector::const_iterator li =
      deps.begin();
      li != deps.end(); ++li)
    {
    // An error should be reported if one dependency
    // has INTERFACE_POSITION_INDEPENDENT_CODE ON and the other
    // has INTERFACE_POSITION_INDEPENDENT_CODE OFF, or if the
    // target itself has a POSITION_INDEPENDENT_CODE which disagrees
    // with a dependency.

    if (!li->Target)
      {
      continue;
      }

    const bool ifaceIsSet = li->Target->GetProperties()
                            .find("INTERFACE_" + p)
                            != li->Target->GetProperties().end();
    PropertyType ifacePropContent =
                    getTypedProperty<PropertyType>(li->Target,
                              ("INTERFACE_" + p).c_str(), 0);
    if (explicitlySet)
      {
      if (ifaceIsSet)
        {
        if (!consistentProperty(propContent, ifacePropContent))
          {
          cmOStringStream e;
          e << "Property " << p << " on target \""
            << tgt->GetName() << "\" does\nnot match the "
            "INTERFACE_" << p << " property requirement\nof "
            "dependency \"" << li->Target->GetName() << "\".\n";
          cmSystemTools::Error(e.str().c_str());
          break;
          }
        else
          {
          // Agree
          continue;
          }
        }
      else
        {
        // Explicitly set on target and not set in iface. Can't disagree.
        continue;
        }
      }
    else if (impliedByUse)
      {
      if (ifaceIsSet)
        {
        if (!consistentProperty(propContent, ifacePropContent))
          {
          cmOStringStream e;
          e << "Property " << p << " on target \""
            << tgt->GetName() << "\" is\nimplied to be " << defaultValue
            << " because it was used to determine the link libraries\n"
               "already. The INTERFACE_" << p << " property on\ndependency \""
            << li->Target->GetName() << "\" is in conflict.\n";
          cmSystemTools::Error(e.str().c_str());
          break;
          }
        else
          {
          // Agree
          continue;
          }
        }
      else
        {
        // Implicitly set on target and not set in iface. Can't disagree.
        continue;
        }
      }
    else
      {
      if (ifaceIsSet)
        {
        if (propInitialized)
          {
          if (!consistentProperty(propContent, ifacePropContent))
            {
            cmOStringStream e;
            e << "The INTERFACE_" << p << " property of \""
              << li->Target->GetName() << "\" does\nnot agree with the value "
                "of " << p << " already determined\nfor \""
              << tgt->GetName() << "\".\n";
            cmSystemTools::Error(e.str().c_str());
            break;
            }
          else
            {
            // Agree.
            continue;
            }
          }
        else
          {
          propContent = ifacePropContent;
          propInitialized = true;
          }
        }
      else
        {
        // Not set. Nothing to agree on.
        continue;
        }
      }
    }
  return propContent;
}

//----------------------------------------------------------------------------
bool cmTarget::GetLinkInterfaceDependentBoolProperty(const std::string &p,
                                                     const char *config)
{
  return checkInterfacePropertyCompatibility<bool>(this, p, config, "FALSE",
                                                   0);
}

//----------------------------------------------------------------------------
const char * cmTarget::GetLinkInterfaceDependentStringProperty(
                                                      const std::string &p,
                                                      const char *config)
{
  return checkInterfacePropertyCompatibility<const char *>(this,
                                                           p,
                                                           config,
                                                           "empty", 0);
}

//----------------------------------------------------------------------------
bool isLinkDependentProperty(cmTarget *tgt, const std::string &p,
                             const char *interfaceProperty,
                             const char *config)
{
  cmComputeLinkInformation *info = tgt->GetLinkInformation(config);
  if(!info)
    {
    return false;
    }

  const cmComputeLinkInformation::ItemVector &deps = info->GetItems();

  for(cmComputeLinkInformation::ItemVector::const_iterator li =
      deps.begin();
      li != deps.end(); ++li)
    {
    if (!li->Target)
      {
      continue;
      }
    const char *prop = li->Target->GetProperty(interfaceProperty);
    if (!prop)
      {
      continue;
      }

    std::vector<std::string> props;
    cmSystemTools::ExpandListArgument(prop, props);

    for(std::vector<std::string>::iterator pi = props.begin();
        pi != props.end(); ++pi)
      {
      if (*pi == p)
        {
        return true;
        }
      }
    }

  return false;
}

//----------------------------------------------------------------------------
bool cmTarget::IsLinkInterfaceDependentBoolProperty(const std::string &p,
                                           const char *config)
{
  if (this->TargetTypeValue == OBJECT_LIBRARY)
    {
    return false;
    }
  return (p == "POSITION_INDEPENDENT_CODE") ||
    isLinkDependentProperty(this, p, "COMPATIBLE_INTERFACE_BOOL",
                                 config);
}

//----------------------------------------------------------------------------
bool cmTarget::IsLinkInterfaceDependentStringProperty(const std::string &p,
                                    const char *config)
{
  if (this->TargetTypeValue == OBJECT_LIBRARY)
    {
    return false;
    }
  return isLinkDependentProperty(this, p, "COMPATIBLE_INTERFACE_STRING",
                                 config);
}

//----------------------------------------------------------------------------
void cmTarget::GetLanguages(std::set<cmStdString>& languages) const
{
  for(std::vector<cmSourceFile*>::const_iterator
        i = this->SourceFiles.begin(); i != this->SourceFiles.end(); ++i)
    {
    if(const char* lang = (*i)->GetLanguage())
      {
      languages.insert(lang);
      }
    }
}

//----------------------------------------------------------------------------
bool cmTarget::IsChrpathUsed(const char* config)
{
  // Only certain target types have an rpath.
  if(!(this->GetType() == cmTarget::SHARED_LIBRARY ||
       this->GetType() == cmTarget::MODULE_LIBRARY ||
       this->GetType() == cmTarget::EXECUTABLE))
    {
    return false;
    }

  // If the target will not be installed we do not need to change its
  // rpath.
  if(!this->GetHaveInstallRule())
    {
    return false;
    }

  // Skip chrpath if skipping rpath altogether.
  if(this->Makefile->IsOn("CMAKE_SKIP_RPATH"))
    {
    return false;
    }

  // Skip chrpath if it does not need to be changed at install time.
  if(this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH"))
    {
    return false;
    }

  // Allow the user to disable builtin chrpath explicitly.
  if(this->Makefile->IsOn("CMAKE_NO_BUILTIN_CHRPATH"))
    {
    return false;
    }

  if(this->Makefile->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME"))
    {
    return true;
    }

#if defined(CMAKE_USE_ELF_PARSER)
  // Enable if the rpath flag uses a separator and the target uses ELF
  // binaries.
  if(const char* ll = this->GetLinkerLanguage(config, this))
    {
    std::string sepVar = "CMAKE_SHARED_LIBRARY_RUNTIME_";
    sepVar += ll;
    sepVar += "_FLAG_SEP";
    const char* sep = this->Makefile->GetDefinition(sepVar.c_str());
    if(sep && *sep)
      {
      // TODO: Add ELF check to ABI detection and get rid of
      // CMAKE_EXECUTABLE_FORMAT.
      if(const char* fmt =
         this->Makefile->GetDefinition("CMAKE_EXECUTABLE_FORMAT"))
        {
        return strcmp(fmt, "ELF") == 0;
        }
      }
    }
#endif
  static_cast<void>(config);
  return false;
}

//----------------------------------------------------------------------------
cmTarget::ImportInfo const*
cmTarget::GetImportInfo(const char* config, cmTarget *headTarget)
{
  // There is no imported information for non-imported targets.
  if(!this->IsImported())
    {
    return 0;
    }

  // Lookup/compute/cache the import information for this
  // configuration.
  std::string config_upper;
  if(config && *config)
    {
    config_upper = cmSystemTools::UpperCase(config);
    }
  else
    {
    config_upper = "NOCONFIG";
    }
  TargetConfigPair key(headTarget, config_upper);
  typedef cmTargetInternals::ImportInfoMapType ImportInfoMapType;

  ImportInfoMapType::const_iterator i =
    this->Internal->ImportInfoMap.find(key);
  if(i == this->Internal->ImportInfoMap.end())
    {
    ImportInfo info;
    this->ComputeImportInfo(config_upper, info, headTarget);
    ImportInfoMapType::value_type entry(key, info);
    i = this->Internal->ImportInfoMap.insert(entry).first;
    }

  if(this->GetType() == INTERFACE_LIBRARY)
    {
    return &i->second;
    }
  // If the location is empty then the target is not available for
  // this configuration.
  if(i->second.Location.empty() && i->second.ImportLibrary.empty())
    {
    return 0;
    }

  // Return the import information.
  return &i->second;
}

bool cmTarget::GetMappedConfig(std::string const& desired_config,
                               const char** loc,
                               const char** imp,
                               std::string& suffix)
{
  // Track the configuration-specific property suffix.
  suffix = "_";
  suffix += desired_config;

  std::vector<std::string> mappedConfigs;
  {
  std::string mapProp = "MAP_IMPORTED_CONFIG_";
  mapProp += desired_config;
  if(const char* mapValue = this->GetProperty(mapProp.c_str()))
    {
    cmSystemTools::ExpandListArgument(mapValue, mappedConfigs);
    }
  }

  // If we needed to find one of the mapped configurations but did not
  // On a DLL platform there may be only IMPORTED_IMPLIB for a shared
  // library or an executable with exports.
  bool allowImp = this->HasImportLibrary();

  // If a mapping was found, check its configurations.
  for(std::vector<std::string>::const_iterator mci = mappedConfigs.begin();
      !*loc && !*imp && mci != mappedConfigs.end(); ++mci)
    {
    // Look for this configuration.
    std::string mcUpper = cmSystemTools::UpperCase(mci->c_str());
    std::string locProp = "IMPORTED_LOCATION_";
    locProp += mcUpper;
    *loc = this->GetProperty(locProp.c_str());
    if(allowImp)
      {
      std::string impProp = "IMPORTED_IMPLIB_";
      impProp += mcUpper;
      *imp = this->GetProperty(impProp.c_str());
      }

    // If it was found, use it for all properties below.
    if(*loc || *imp)
      {
      suffix = "_";
      suffix += mcUpper;
      }
    }

  // If we needed to find one of the mapped configurations but did not
  // then the target is not found.  The project does not want any
  // other configuration.
  if(!mappedConfigs.empty() && !*loc && !*imp)
    {
    return false;
    }

  // If we have not yet found it then there are no mapped
  // configurations.  Look for an exact-match.
  if(!*loc && !*imp)
    {
    std::string locProp = "IMPORTED_LOCATION";
    locProp += suffix;
    *loc = this->GetProperty(locProp.c_str());
    if(allowImp)
      {
      std::string impProp = "IMPORTED_IMPLIB";
      impProp += suffix;
      *imp = this->GetProperty(impProp.c_str());
      }
    }

  // If we have not yet found it then there are no mapped
  // configurations and no exact match.
  if(!*loc && !*imp)
    {
    // The suffix computed above is not useful.
    suffix = "";

    // Look for a configuration-less location.  This may be set by
    // manually-written code.
    *loc = this->GetProperty("IMPORTED_LOCATION");
    if(allowImp)
      {
      *imp = this->GetProperty("IMPORTED_IMPLIB");
      }
    }

  // If we have not yet found it then the project is willing to try
  // any available configuration.
  if(!*loc && !*imp)
    {
    std::vector<std::string> availableConfigs;
    if(const char* iconfigs = this->GetProperty("IMPORTED_CONFIGURATIONS"))
      {
      cmSystemTools::ExpandListArgument(iconfigs, availableConfigs);
      }
    for(std::vector<std::string>::const_iterator
          aci = availableConfigs.begin();
        !*loc && !*imp && aci != availableConfigs.end(); ++aci)
      {
      suffix = "_";
      suffix += cmSystemTools::UpperCase(*aci);
      std::string locProp = "IMPORTED_LOCATION";
      locProp += suffix;
      *loc = this->GetProperty(locProp.c_str());
      if(allowImp)
        {
        std::string impProp = "IMPORTED_IMPLIB";
        impProp += suffix;
        *imp = this->GetProperty(impProp.c_str());
        }
      }
    }
  // If we have not yet found it then the target is not available.
  if(!*loc && !*imp)
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
void cmTarget::ComputeImportInfo(std::string const& desired_config,
                                 ImportInfo& info,
                                 cmTarget *headTarget)
{
  // This method finds information about an imported target from its
  // properties.  The "IMPORTED_" namespace is reserved for properties
  // defined by the project exporting the target.

  // Initialize members.
  info.NoSOName = false;

  const char* loc = 0;
  const char* imp = 0;
  std::string suffix;
  if (this->GetType() != INTERFACE_LIBRARY &&
      !this->GetMappedConfig(desired_config, &loc, &imp, suffix))
    {
    return;
    }

  // Get the link interface.
  {
  std::string linkProp = "INTERFACE_LINK_LIBRARIES";
  const char *propertyLibs = this->GetProperty(linkProp.c_str());

  if (this->GetType() != INTERFACE_LIBRARY)
    {
    if(!propertyLibs)
      {
      linkProp = "IMPORTED_LINK_INTERFACE_LIBRARIES";
      linkProp += suffix;
      propertyLibs = this->GetProperty(linkProp.c_str());
      }

    if(!propertyLibs)
      {
      linkProp = "IMPORTED_LINK_INTERFACE_LIBRARIES";
      propertyLibs = this->GetProperty(linkProp.c_str());
      }
    }
  if(propertyLibs)
    {
    cmListFileBacktrace lfbt;
    cmGeneratorExpression ge(lfbt);

    cmGeneratorExpressionDAGChecker dagChecker(lfbt,
                                        this->GetName(),
                                        linkProp, 0, 0);
    cmSystemTools::ExpandListArgument(ge.Parse(propertyLibs)
                                       ->Evaluate(this->Makefile,
                                                  desired_config.c_str(),
                                                  false,
                                                  headTarget,
                                                  this,
                                                  &dagChecker),
                                    info.LinkInterface.Libraries);
    }
  }
  if(this->GetType() == INTERFACE_LIBRARY)
    {
    return;
    }

  // A provided configuration has been chosen.  Load the
  // configuration's properties.

  // Get the location.
  if(loc)
    {
    info.Location = loc;
    }
  else
    {
    std::string impProp = "IMPORTED_LOCATION";
    impProp += suffix;
    if(const char* config_location = this->GetProperty(impProp.c_str()))
      {
      info.Location = config_location;
      }
    else if(const char* location = this->GetProperty("IMPORTED_LOCATION"))
      {
      info.Location = location;
      }
    }

  // Get the soname.
  if(this->GetType() == cmTarget::SHARED_LIBRARY)
    {
    std::string soProp = "IMPORTED_SONAME";
    soProp += suffix;
    if(const char* config_soname = this->GetProperty(soProp.c_str()))
      {
      info.SOName = config_soname;
      }
    else if(const char* soname = this->GetProperty("IMPORTED_SONAME"))
      {
      info.SOName = soname;
      }
    }

  // Get the "no-soname" mark.
  if(this->GetType() == cmTarget::SHARED_LIBRARY)
    {
    std::string soProp = "IMPORTED_NO_SONAME";
    soProp += suffix;
    if(const char* config_no_soname = this->GetProperty(soProp.c_str()))
      {
      info.NoSOName = cmSystemTools::IsOn(config_no_soname);
      }
    else if(const char* no_soname = this->GetProperty("IMPORTED_NO_SONAME"))
      {
      info.NoSOName = cmSystemTools::IsOn(no_soname);
      }
    }

  // Get the import library.
  if(imp)
    {
    info.ImportLibrary = imp;
    }
  else if(this->GetType() == cmTarget::SHARED_LIBRARY ||
          this->IsExecutableWithExports())
    {
    std::string impProp = "IMPORTED_IMPLIB";
    impProp += suffix;
    if(const char* config_implib = this->GetProperty(impProp.c_str()))
      {
      info.ImportLibrary = config_implib;
      }
    else if(const char* implib = this->GetProperty("IMPORTED_IMPLIB"))
      {
      info.ImportLibrary = implib;
      }
    }

  // Get the link dependencies.
  {
  std::string linkProp = "IMPORTED_LINK_DEPENDENT_LIBRARIES";
  linkProp += suffix;
  if(const char* config_libs = this->GetProperty(linkProp.c_str()))
    {
    cmSystemTools::ExpandListArgument(config_libs,
                                      info.LinkInterface.SharedDeps);
    }
  else if(const char* libs =
          this->GetProperty("IMPORTED_LINK_DEPENDENT_LIBRARIES"))
    {
    cmSystemTools::ExpandListArgument(libs, info.LinkInterface.SharedDeps);
    }
  }

  // Get the link languages.
  if(this->LinkLanguagePropagatesToDependents())
    {
    std::string linkProp = "IMPORTED_LINK_INTERFACE_LANGUAGES";
    linkProp += suffix;
    if(const char* config_libs = this->GetProperty(linkProp.c_str()))
      {
      cmSystemTools::ExpandListArgument(config_libs,
                                        info.LinkInterface.Languages);
      }
    else if(const char* libs =
            this->GetProperty("IMPORTED_LINK_INTERFACE_LANGUAGES"))
      {
      cmSystemTools::ExpandListArgument(libs,
                                        info.LinkInterface.Languages);
      }
    }

  // Get the cyclic repetition count.
  if(this->GetType() == cmTarget::STATIC_LIBRARY)
    {
    std::string linkProp = "IMPORTED_LINK_INTERFACE_MULTIPLICITY";
    linkProp += suffix;
    if(const char* config_reps = this->GetProperty(linkProp.c_str()))
      {
      sscanf(config_reps, "%u", &info.LinkInterface.Multiplicity);
      }
    else if(const char* reps =
            this->GetProperty("IMPORTED_LINK_INTERFACE_MULTIPLICITY"))
      {
      sscanf(reps, "%u", &info.LinkInterface.Multiplicity);
      }
    }
}

//----------------------------------------------------------------------------
cmTarget::LinkInterface const* cmTarget::GetLinkInterface(const char* config,
                                                      cmTarget *head)
{
  // Imported targets have their own link interface.
  if(this->IsImported())
    {
    if(cmTarget::ImportInfo const* info = this->GetImportInfo(config, head))
      {
      return &info->LinkInterface;
      }
    return 0;
    }

  // Link interfaces are not supported for executables that do not
  // export symbols.
  if(this->GetType() == cmTarget::EXECUTABLE &&
     !this->IsExecutableWithExports())
    {
    return 0;
    }

  // Lookup any existing link interface for this configuration.
  TargetConfigPair key(head, cmSystemTools::UpperCase(config? config : ""));

  cmTargetInternals::LinkInterfaceMapType::iterator
    i = this->Internal->LinkInterfaceMap.find(key);
  if(i == this->Internal->LinkInterfaceMap.end())
    {
    // Compute the link interface for this configuration.
    cmTargetInternals::OptionalLinkInterface iface;
    iface.Exists = this->ComputeLinkInterface(config, iface, head);

    // Store the information for this configuration.
    cmTargetInternals::LinkInterfaceMapType::value_type entry(key, iface);
    i = this->Internal->LinkInterfaceMap.insert(entry).first;
    }

  return i->second.Exists? &i->second : 0;
}

//----------------------------------------------------------------------------
void cmTarget::GetTransitivePropertyLinkLibraries(
                                      const char* config,
                                      cmTarget *headTarget,
                                      std::vector<std::string> &libs)
{
  cmTarget::LinkInterface const* iface = this->GetLinkInterface(config,
                                                                headTarget);
  if (!iface)
    {
    return;
    }
  if(this->GetType() != STATIC_LIBRARY
      || this->GetPolicyStatusCMP0022() == cmPolicies::WARN
      || this->GetPolicyStatusCMP0022() == cmPolicies::OLD)
    {
    libs = iface->Libraries;
    return;
    }

  const char* linkIfaceProp = "INTERFACE_LINK_LIBRARIES";
  const char* interfaceLibs = this->GetProperty(linkIfaceProp);

  if (!interfaceLibs)
    {
    return;
    }

  // The interface libraries have been explicitly set.
  cmListFileBacktrace lfbt;
  cmGeneratorExpression ge(lfbt);
  cmGeneratorExpressionDAGChecker dagChecker(lfbt, this->GetName(),
                                              linkIfaceProp, 0, 0);
  dagChecker.SetTransitivePropertiesOnly();
  cmSystemTools::ExpandListArgument(ge.Parse(interfaceLibs)->Evaluate(
                                      this->Makefile,
                                      config,
                                      false,
                                      headTarget,
                                      this, &dagChecker), libs);
}

//----------------------------------------------------------------------------
bool cmTarget::ComputeLinkInterface(const char* config, LinkInterface& iface,
                                    cmTarget *headTarget)
{
  // Construct the property name suffix for this configuration.
  std::string suffix = "_";
  if(config && *config)
    {
    suffix += cmSystemTools::UpperCase(config);
    }
  else
    {
    suffix += "NOCONFIG";
    }

  // An explicit list of interface libraries may be set for shared
  // libraries and executables that export symbols.
  const char* explicitLibraries = 0;
  const char* newExplicitLibraries =
                                this->GetProperty("INTERFACE_LINK_LIBRARIES");
  std::string linkIfaceProp;
  if(this->GetType() == cmTarget::SHARED_LIBRARY ||
     this->IsExecutableWithExports())
    {
    // Lookup the per-configuration property.
    linkIfaceProp = "LINK_INTERFACE_LIBRARIES";
    linkIfaceProp += suffix;
    explicitLibraries = this->GetProperty(linkIfaceProp.c_str());

    // If not set, try the generic property.
    if(!explicitLibraries)
      {
      linkIfaceProp = "LINK_INTERFACE_LIBRARIES";
      explicitLibraries = this->GetProperty(linkIfaceProp.c_str());
      }
    if (newExplicitLibraries
        && (!explicitLibraries ||
            (explicitLibraries
              && strcmp(newExplicitLibraries, explicitLibraries) != 0)))
      {
      switch(this->GetPolicyStatusCMP0022())
        {
        case cmPolicies::WARN:
          {
          cmOStringStream w;
          w << (this->Makefile->GetPolicies()
                ->GetPolicyWarning(cmPolicies::CMP0022)) << "\n"
            << "Target \"" << this->GetName() << "\" has a "
              "INTERFACE_LINK_LIBRARIES property which differs from its "
            << linkIfaceProp << " properties."
              "\n"
              "INTERFACE_LINK_LIBRARIES:\n  "
            << newExplicitLibraries
            << "\n"
            << linkIfaceProp << ":\n  "
            << (explicitLibraries ? explicitLibraries : "(empty)") << "\n";
          this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, w.str());
          }
          // Fall through
        case cmPolicies::OLD:
          break;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
        case cmPolicies::NEW:
          explicitLibraries = newExplicitLibraries;
          linkIfaceProp = "INTERFACE_LINK_LIBRARIES";
          break;
        }
      }
    }
  else if(this->GetType() == cmTarget::STATIC_LIBRARY)
    {
    if (newExplicitLibraries)
      {
      cmListFileBacktrace lfbt;
      cmGeneratorExpression ge(lfbt);
      cmGeneratorExpressionDAGChecker dagChecker(lfbt, this->GetName(),
                                            "INTERFACE_LINK_LIBRARIES", 0, 0);
      std::vector<std::string> ifaceLibs;
      cmSystemTools::ExpandListArgument(
          ge.Parse(newExplicitLibraries)->Evaluate(
                                          this->Makefile,
                                          config,
                                          false,
                                          headTarget,
                                          this, &dagChecker), ifaceLibs);
      LinkImplementation const* impl = this->GetLinkImplementation(config,
                                                                headTarget);
      if (ifaceLibs != impl->Libraries)
        {
        switch(this->GetPolicyStatusCMP0022())
          {
          case cmPolicies::WARN:
            {
            cmOStringStream w;
            w << (this->Makefile->GetPolicies()
                  ->GetPolicyWarning(cmPolicies::CMP0022)) << "\n"
              << "Static library target \"" << this->GetName() << "\" has a "
                "INTERFACE_LINK_LIBRARIES property.  This should be preferred "
                "as the source of the link interface for this library.  "
                "Ignoring the property and using the link implementation "
                "as the link interface instead.";
            this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, w.str());
            }
            // Fall through
          case cmPolicies::OLD:
            break;
          case cmPolicies::REQUIRED_IF_USED:
          case cmPolicies::REQUIRED_ALWAYS:
          case cmPolicies::NEW:
            explicitLibraries = newExplicitLibraries;
            linkIfaceProp = "INTERFACE_LINK_LIBRARIES";
            break;
          }
        }
      else
        {
        iface.Libraries = impl->Libraries;
        if(this->LinkLanguagePropagatesToDependents())
          {
          // Targets using this archive need its language runtime libraries.
          iface.Languages = impl->Languages;
          }
        }
      }
    }
  else if (this->GetType() == cmTarget::INTERFACE_LIBRARY)
    {
    explicitLibraries = newExplicitLibraries;
    linkIfaceProp = "INTERFACE_LINK_LIBRARIES";
    }

  // There is no implicit link interface for executables or modules
  // so if none was explicitly set then there is no link interface.
  // Note that CMake versions 2.2 and below allowed linking to modules.
  bool canLinkModules = this->Makefile->NeedBackwardsCompatibility(2,2);
  if(!explicitLibraries &&
     (this->GetType() == cmTarget::EXECUTABLE ||
      (this->GetType() == cmTarget::MODULE_LIBRARY && !canLinkModules)))
    {
    return false;
    }

  if(explicitLibraries)
    {
    // The interface libraries have been explicitly set.
    cmListFileBacktrace lfbt;
    cmGeneratorExpression ge(lfbt);
    cmGeneratorExpressionDAGChecker dagChecker(lfbt, this->GetName(),
                                               linkIfaceProp, 0, 0);
    cmSystemTools::ExpandListArgument(ge.Parse(explicitLibraries)->Evaluate(
                                        this->Makefile,
                                        config,
                                        false,
                                        headTarget,
                                        this, &dagChecker), iface.Libraries);

    if(this->GetType() == cmTarget::SHARED_LIBRARY
        || this->GetType() == cmTarget::STATIC_LIBRARY
        || this->GetType() == cmTarget::INTERFACE_LIBRARY)
      {
      // Shared libraries may have runtime implementation dependencies
      // on other shared libraries that are not in the interface.
      std::set<cmStdString> emitted;
      for(std::vector<std::string>::const_iterator
            li = iface.Libraries.begin(); li != iface.Libraries.end(); ++li)
        {
        emitted.insert(*li);
        }
      LinkImplementation const* impl = this->GetLinkImplementation(config,
                                                                headTarget);
      for(std::vector<std::string>::const_iterator
            li = impl->Libraries.begin(); li != impl->Libraries.end(); ++li)
        {
        if(emitted.insert(*li).second)
          {
          if(cmTarget* tgt = this->Makefile->FindTargetToUse(li->c_str()))
            {
            // This is a runtime dependency on another shared library.
            if(tgt->GetType() == cmTarget::SHARED_LIBRARY)
              {
              iface.SharedDeps.push_back(*li);
              }
            }
          else
            {
            // TODO: Recognize shared library file names.  Perhaps this
            // should be moved to cmComputeLinkInformation, but that creates
            // a chicken-and-egg problem since this list is needed for its
            // construction.
            }
          }
        }
      if(this->LinkLanguagePropagatesToDependents())
        {
        // Targets using this archive need its language runtime libraries.
        iface.Languages = impl->Languages;
        }
      }
    }
  else if (this->GetPolicyStatusCMP0022() == cmPolicies::WARN
        || this->GetPolicyStatusCMP0022() == cmPolicies::OLD)
    // The implementation shouldn't be the interface if CMP0022 is NEW. That
    // way, the LINK_LIBRARIES property can be set directly without having to
    // empty the INTERFACE_LINK_LIBRARIES
    {
    // The link implementation is the default link interface.
    LinkImplementation const* impl = this->GetLinkImplementation(config,
                                                              headTarget);
    iface.ImplementationIsInterface = true;
    iface.Libraries = impl->Libraries;
    iface.WrongConfigLibraries = impl->WrongConfigLibraries;
    if(this->LinkLanguagePropagatesToDependents())
      {
      // Targets using this archive need its language runtime libraries.
      iface.Languages = impl->Languages;
      }
    }

  if(this->GetType() == cmTarget::STATIC_LIBRARY)
    {
    // How many repetitions are needed if this library has cyclic
    // dependencies?
    std::string propName = "LINK_INTERFACE_MULTIPLICITY";
    propName += suffix;
    if(const char* config_reps = this->GetProperty(propName.c_str()))
      {
      sscanf(config_reps, "%u", &iface.Multiplicity);
      }
    else if(const char* reps =
            this->GetProperty("LINK_INTERFACE_MULTIPLICITY"))
      {
      sscanf(reps, "%u", &iface.Multiplicity);
      }
    }

  return true;
}

//----------------------------------------------------------------------------
cmTarget::LinkImplementation const*
cmTarget::GetLinkImplementation(const char* config, cmTarget *head)
{
  // There is no link implementation for imported targets.
  if(this->IsImported())
    {
    return 0;
    }

  // Lookup any existing link implementation for this configuration.
  TargetConfigPair key(head, cmSystemTools::UpperCase(config? config : ""));

  cmTargetInternals::LinkImplMapType::iterator
    i = this->Internal->LinkImplMap.find(key);
  if(i == this->Internal->LinkImplMap.end())
    {
    // Compute the link implementation for this configuration.
    LinkImplementation impl;
    this->ComputeLinkImplementation(config, impl, head);

    // Store the information for this configuration.
    cmTargetInternals::LinkImplMapType::value_type entry(key, impl);
    i = this->Internal->LinkImplMap.insert(entry).first;
    }

  return &i->second;
}

//----------------------------------------------------------------------------
void cmTarget::ComputeLinkImplementation(const char* config,
                                         LinkImplementation& impl,
                                         cmTarget *head)
{
  // Compute which library configuration to link.
  cmTarget::LinkLibraryType linkType = this->ComputeLinkType(config);

  // Collect libraries directly linked in this configuration.
  std::vector<std::string> llibs;
  this->GetDirectLinkLibraries(config, llibs, head);
  for(std::vector<std::string>::const_iterator li = llibs.begin();
      li != llibs.end(); ++li)
    {
    // Skip entries that resolve to the target itself or are empty.
    std::string item = this->CheckCMP0004(*li);
    if(item == this->GetName() || item.empty())
      {
      continue;
      }
    // The entry is meant for this configuration.
    impl.Libraries.push_back(item);
    }

  LinkLibraryVectorType const& oldllibs = this->GetOriginalLinkLibraries();
  for(cmTarget::LinkLibraryVectorType::const_iterator li = oldllibs.begin();
      li != oldllibs.end(); ++li)
    {
    if(li->second != cmTarget::GENERAL && li->second != linkType)
      {
      std::string item = this->CheckCMP0004(li->first);
      if(item == this->GetName() || item.empty())
        {
        continue;
        }
      // Support OLD behavior for CMP0003.
      impl.WrongConfigLibraries.push_back(item);
      }
    }

  // This target needs runtime libraries for its source languages.
  std::set<cmStdString> languages;
  // Get languages used in our source files.
  this->GetLanguages(languages);
  // Get languages used in object library sources.
  for(std::vector<std::string>::iterator i = this->ObjectLibraries.begin();
      i != this->ObjectLibraries.end(); ++i)
    {
    if(cmTarget* objLib = this->Makefile->FindTargetToUse(i->c_str()))
      {
      if(objLib->GetType() == cmTarget::OBJECT_LIBRARY)
        {
        objLib->GetLanguages(languages);
        }
      }
    }
  // Copy the set of langauges to the link implementation.
  for(std::set<cmStdString>::iterator li = languages.begin();
      li != languages.end(); ++li)
    {
    impl.Languages.push_back(*li);
    }
}

//----------------------------------------------------------------------------
std::string cmTarget::CheckCMP0004(std::string const& item)
{
  // Strip whitespace off the library names because we used to do this
  // in case variables were expanded at generate time.  We no longer
  // do the expansion but users link to libraries like " ${VAR} ".
  std::string lib = item;
  std::string::size_type pos = lib.find_first_not_of(" \t\r\n");
  if(pos != lib.npos)
    {
    lib = lib.substr(pos, lib.npos);
    }
  pos = lib.find_last_not_of(" \t\r\n");
  if(pos != lib.npos)
    {
    lib = lib.substr(0, pos+1);
    }
  if(lib != item)
    {
    cmake* cm = this->Makefile->GetCMakeInstance();
    switch(this->PolicyStatusCMP0004)
      {
      case cmPolicies::WARN:
        {
        cmOStringStream w;
        w << (this->Makefile->GetPolicies()
              ->GetPolicyWarning(cmPolicies::CMP0004)) << "\n"
          << "Target \"" << this->GetName() << "\" links to item \""
          << item << "\" which has leading or trailing whitespace.";
        cm->IssueMessage(cmake::AUTHOR_WARNING, w.str(),
                         this->GetBacktrace());
        }
      case cmPolicies::OLD:
        break;
      case cmPolicies::NEW:
        {
        cmOStringStream e;
        e << "Target \"" << this->GetName() << "\" links to item \""
          << item << "\" which has leading or trailing whitespace.  "
          << "This is now an error according to policy CMP0004.";
        cm->IssueMessage(cmake::FATAL_ERROR, e.str(), this->GetBacktrace());
        }
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        {
        cmOStringStream e;
        e << (this->Makefile->GetPolicies()
              ->GetRequiredPolicyError(cmPolicies::CMP0004)) << "\n"
          << "Target \"" << this->GetName() << "\" links to item \""
          << item << "\" which has leading or trailing whitespace.";
        cm->IssueMessage(cmake::FATAL_ERROR, e.str(), this->GetBacktrace());
        }
        break;
      }
    }
  return lib;
}

template<typename PropertyType>
PropertyType getLinkInterfaceDependentProperty(cmTarget *tgt,
                                               const std::string prop,
                                               const char *config,
                                               PropertyType *);

template<>
bool getLinkInterfaceDependentProperty(cmTarget *tgt,
                                         const std::string prop,
                                         const char *config, bool *)
{
  return tgt->GetLinkInterfaceDependentBoolProperty(prop, config);
}

template<>
const char * getLinkInterfaceDependentProperty(cmTarget *tgt,
                                                 const std::string prop,
                                                 const char *config,
                                                 const char **)
{
  return tgt->GetLinkInterfaceDependentStringProperty(prop, config);
}

//----------------------------------------------------------------------------
template<typename PropertyType>
void checkPropertyConsistency(cmTarget *depender, cmTarget *dependee,
                              const char *propName,
                              std::set<cmStdString> &emitted,
                              const char *config,
                              PropertyType *)
{
  const char *prop = dependee->GetProperty(propName);
  if (!prop)
    {
    return;
    }

  std::vector<std::string> props;
  cmSystemTools::ExpandListArgument(prop, props);
  std::string pdir =
    dependee->GetMakefile()->GetRequiredDefinition("CMAKE_ROOT");
  pdir += "/Help/prop_tgt/";

  for(std::vector<std::string>::iterator pi = props.begin();
      pi != props.end(); ++pi)
    {
    std::string pname = cmSystemTools::HelpFileName(*pi);
    std::string pfile = pdir + pname + ".rst";
    if(cmSystemTools::FileExists(pfile.c_str(), true))
      {
      cmOStringStream e;
      e << "Target \"" << dependee->GetName() << "\" has property \""
        << *pi << "\" listed in its " << propName << " property.  "
          "This is not allowed.  Only user-defined properties may appear "
          "listed in the " << propName << " property.";
      depender->GetMakefile()->IssueMessage(cmake::FATAL_ERROR, e.str());
      return;
      }
    if(emitted.insert(*pi).second)
      {
      getLinkInterfaceDependentProperty<PropertyType>(depender, *pi, config,
                                                      0);
      if (cmSystemTools::GetErrorOccuredFlag())
        {
        return;
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmTarget::CheckPropertyCompatibility(cmComputeLinkInformation *info,
                                          const char* config)
{
  const cmComputeLinkInformation::ItemVector &deps = info->GetItems();

  std::set<cmStdString> emittedBools;
  std::set<cmStdString> emittedStrings;

  for(cmComputeLinkInformation::ItemVector::const_iterator li =
      deps.begin();
      li != deps.end(); ++li)
    {
    if (!li->Target)
      {
      continue;
      }

    checkPropertyConsistency<bool>(this, li->Target,
                                   "COMPATIBLE_INTERFACE_BOOL",
                                   emittedBools, config, 0);
    if (cmSystemTools::GetErrorOccuredFlag())
      {
      return;
      }
    checkPropertyConsistency<const char *>(this, li->Target,
                                           "COMPATIBLE_INTERFACE_STRING",
                                           emittedStrings, config, 0);
    if (cmSystemTools::GetErrorOccuredFlag())
      {
      return;
      }
    }

  for(std::set<cmStdString>::const_iterator li = emittedBools.begin();
      li != emittedBools.end(); ++li)
    {
    const std::set<cmStdString>::const_iterator si = emittedStrings.find(*li);
    if (si != emittedStrings.end())
      {
      cmOStringStream e;
      e << "Property \"" << *li << "\" appears in both the "
      "COMPATIBLE_INTERFACE_BOOL and the COMPATIBLE_INTERFACE_STRING "
      "property in the dependencies of target \"" << this->GetName() <<
      "\".  This is not allowed. A property may only require compatibility "
      "in a boolean interpretation or a string interpretation, but not both.";
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
      break;
      }
    }
}

//----------------------------------------------------------------------------
cmComputeLinkInformation*
cmTarget::GetLinkInformation(const char* config, cmTarget *head)
{
  cmTarget *headTarget = head ? head : this;
  // Lookup any existing information for this configuration.
  TargetConfigPair key(headTarget,
                                  cmSystemTools::UpperCase(config?config:""));
  cmTargetLinkInformationMap::iterator
    i = this->LinkInformation.find(key);
  if(i == this->LinkInformation.end())
    {
    // Compute information for this configuration.
    cmComputeLinkInformation* info =
      new cmComputeLinkInformation(this, config, headTarget);
    if(!info || !info->Compute())
      {
      delete info;
      info = 0;
      }

    // Store the information for this configuration.
    cmTargetLinkInformationMap::value_type entry(key, info);
    i = this->LinkInformation.insert(entry).first;

    if (info)
      {
      this->CheckPropertyCompatibility(info, config);
      }
    }
  return i->second;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFrameworkDirectory(const char* config,
                                            bool rootDir)
{
  std::string fpath;
  fpath += this->GetOutputName(config, false);
  fpath += ".framework";
  if(!rootDir)
    {
    fpath += "/Versions/";
    fpath += this->GetFrameworkVersion();
    }
  return fpath;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetCFBundleDirectory(const char* config,
                                           bool contentOnly)
{
  std::string fpath;
  fpath += this->GetOutputName(config, false);
  fpath += ".";
  const char *ext = this->GetProperty("BUNDLE_EXTENSION");
  if (!ext)
    {
    ext = "bundle";
    }
  fpath += ext;
  fpath += "/Contents";
  if(!contentOnly)
    fpath += "/MacOS";
  return fpath;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetAppBundleDirectory(const char* config,
                                            bool contentOnly)
{
  std::string fpath = this->GetFullName(config, false);
  fpath += ".app/Contents";
  if(!contentOnly)
    fpath += "/MacOS";
  return fpath;
}

//----------------------------------------------------------------------------
std::string cmTarget::BuildMacContentDirectory(const std::string& base,
                                               const char* config,
                                               bool contentOnly)
{
  std::string fpath = base;
  if(this->IsAppBundleOnApple())
    {
    fpath += this->GetAppBundleDirectory(config, contentOnly);
    }
  if(this->IsFrameworkOnApple())
    {
    fpath += this->GetFrameworkDirectory(config, contentOnly);
    }
  if(this->IsCFBundleOnApple())
    {
    fpath += this->GetCFBundleDirectory(config, contentOnly);
    }
  return fpath;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetMacContentDirectory(const char* config,
                                             bool implib)
{
  // Start with the output directory for the target.
  std::string fpath = this->GetDirectory(config, implib);
  fpath += "/";
  bool contentOnly = true;
  if(this->IsFrameworkOnApple())
    {
    // additional files with a framework go into the version specific
    // directory
    contentOnly = false;
    }
  fpath = this->BuildMacContentDirectory(fpath, config, contentOnly);
  return fpath;
}

//----------------------------------------------------------------------------
cmTargetLinkInformationMap
::cmTargetLinkInformationMap(cmTargetLinkInformationMap const& r): derived()
{
  // Ideally cmTarget instances should never be copied.  However until
  // we can make a sweep to remove that, this copy constructor avoids
  // allowing the resources (LinkInformation) from getting copied.  In
  // the worst case this will lead to extra cmComputeLinkInformation
  // instances.  We also enforce in debug mode that the map be emptied
  // when copied.
  static_cast<void>(r);
  assert(r.empty());
}

//----------------------------------------------------------------------------
cmTargetLinkInformationMap::~cmTargetLinkInformationMap()
{
  for(derived::iterator i = this->begin(); i != this->end(); ++i)
    {
    delete i->second;
    }
}

//----------------------------------------------------------------------------
cmTargetInternalPointer::cmTargetInternalPointer()
{
  this->Pointer = new cmTargetInternals;
}

//----------------------------------------------------------------------------
cmTargetInternalPointer
::cmTargetInternalPointer(cmTargetInternalPointer const& r)
{
  // Ideally cmTarget instances should never be copied.  However until
  // we can make a sweep to remove that, this copy constructor avoids
  // allowing the resources (Internals) to be copied.
  this->Pointer = new cmTargetInternals(*r.Pointer);
}

//----------------------------------------------------------------------------
cmTargetInternalPointer::~cmTargetInternalPointer()
{
  deleteAndClear(this->Pointer->IncludeDirectoriesEntries);
  deleteAndClear(this->Pointer->CompileOptionsEntries);
  deleteAndClear(this->Pointer->CompileDefinitionsEntries);
  delete this->Pointer;
}

//----------------------------------------------------------------------------
cmTargetInternalPointer&
cmTargetInternalPointer::operator=(cmTargetInternalPointer const& r)
{
  if(this == &r) { return *this; } // avoid warning on HP about self check
  // Ideally cmTarget instances should never be copied.  However until
  // we can make a sweep to remove that, this copy constructor avoids
  // allowing the resources (Internals) to be copied.
  cmTargetInternals* oldPointer = this->Pointer;
  this->Pointer = new cmTargetInternals(*r.Pointer);
  delete oldPointer;
  return *this;
}
