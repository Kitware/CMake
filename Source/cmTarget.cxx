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
#include "cmOutputConverter.h"
#include "cmGlobalGenerator.h"
#include "cmComputeLinkInformation.h"
#include "cmListFileCache.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmAlgorithms.h"
#include <cmsys/RegularExpression.hxx>
#include <map>
#include <set>
#include <stdlib.h> // required for atof
#include <assert.h>
#include <errno.h>
#if defined(CMAKE_BUILD_WITH_CMAKE)
#include <cmsys/hash_set.hxx>
#define UNORDERED_SET cmsys::hash_set
#else
#define UNORDERED_SET std::set
#endif

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
  bool empty() const
    { return OutDir.empty() && ImpDir.empty() && PdbDir.empty(); }
};

//----------------------------------------------------------------------------
class cmTargetInternals
{
public:
  cmTargetInternals()
    : Backtrace()
    {
    this->UtilityItemsDone = false;
    }
  cmTargetInternals(cmTargetInternals const&)
    : Backtrace()
    {
    this->UtilityItemsDone = false;
    }
  ~cmTargetInternals();

  // The backtrace when the target was created.
  cmListFileBacktrace Backtrace;

  typedef std::map<std::string, cmTarget::OutputInfo> OutputInfoMapType;
  OutputInfoMapType OutputInfoMap;

  typedef std::map<std::string, cmTarget::ImportInfo> ImportInfoMapType;
  ImportInfoMapType ImportInfoMap;

  struct HeadToLinkImplementationMap:
    public std::map<cmTarget const*, cmOptionalLinkImplementation> {};
  typedef std::map<std::string,
                   HeadToLinkImplementationMap> LinkImplMapType;
  LinkImplMapType LinkImplMap;

  typedef std::map<std::string, std::vector<cmSourceFile*> >
                                                       SourceFilesMapType;
  SourceFilesMapType SourceFilesMap;

  std::set<cmLinkItem> UtilityItems;
  bool UtilityItemsDone;

  class TargetPropertyEntry {
    static cmLinkImplItem NoLinkImplItem;
  public:
    TargetPropertyEntry(cmsys::auto_ptr<cmCompiledGeneratorExpression> cge,
                        cmLinkImplItem const& item = NoLinkImplItem)
      : ge(cge), LinkImplItem(item)
    {}
    const cmsys::auto_ptr<cmCompiledGeneratorExpression> ge;
    cmLinkImplItem const& LinkImplItem;
  };
  std::vector<std::string> IncludeDirectoriesEntries;
  std::vector<cmListFileBacktrace> IncludeDirectoriesBacktraces;
  std::vector<std::string> CompileOptionsEntries;
  std::vector<cmListFileBacktrace> CompileOptionsBacktraces;
  std::vector<std::string> CompileFeaturesEntries;
  std::vector<cmListFileBacktrace> CompileFeaturesBacktraces;
  std::vector<std::string> CompileDefinitionsEntries;
  std::vector<cmListFileBacktrace> CompileDefinitionsBacktraces;
  std::vector<TargetPropertyEntry*> SourceEntries;
  std::vector<cmValueWithOrigin> LinkImplementationPropertyEntries;

  void AddInterfaceEntries(
    cmTarget const* thisTarget, std::string const& config,
    std::string const& prop, std::vector<TargetPropertyEntry*>& entries);
};

cmLinkImplItem cmTargetInternals::TargetPropertyEntry::NoLinkImplItem;

//----------------------------------------------------------------------------
cmTargetInternals::~cmTargetInternals()
{
}

//----------------------------------------------------------------------------
cmTarget::cmTarget()
{
  this->Makefile = 0;
#if defined(_WIN32) && !defined(__CYGWIN__)
  this->LinkLibrariesForVS6Analyzed = false;
#endif
  this->HaveInstallRule = false;
  this->DLLPlatform = false;
  this->IsAndroid = false;
  this->IsApple = false;
  this->IsImportedTarget = false;
  this->BuildInterfaceIncludesAppended = false;
  this->DebugSourcesDone = false;
  this->LinkImplementationLanguageIsContextDependent = true;
}

void cmTarget::SetType(TargetType type, const std::string& name)
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

  // Check whether this is a DLL platform.
  this->DLLPlatform = (this->Makefile->IsOn("WIN32") ||
                       this->Makefile->IsOn("CYGWIN") ||
                       this->Makefile->IsOn("MINGW"));

  // Check whether we are targeting an Android platform.
  this->IsAndroid =
    strcmp(this->Makefile->GetSafeDefinition("CMAKE_SYSTEM_NAME"),
           "Android") == 0;

  // Check whether we are targeting an Apple platform.
  this->IsApple = this->Makefile->IsOn("APPLE");

  // Setup default property values.
  if (this->GetType() != INTERFACE_LIBRARY && this->GetType() != UTILITY)
    {
    this->SetPropertyDefault("ANDROID_API", 0);
    this->SetPropertyDefault("ANDROID_API_MIN", 0);
    this->SetPropertyDefault("ANDROID_ARCH", 0);
    this->SetPropertyDefault("ANDROID_STL_TYPE", 0);
    this->SetPropertyDefault("ANDROID_SKIP_ANT_STEP", 0);
    this->SetPropertyDefault("ANDROID_PROCESS_MAX", 0);
    this->SetPropertyDefault("ANDROID_PROGUARD", 0);
    this->SetPropertyDefault("ANDROID_PROGUARD_CONFIG_PATH", 0);
    this->SetPropertyDefault("ANDROID_SECURE_PROPS_PATH", 0);
    this->SetPropertyDefault("ANDROID_NATIVE_LIB_DIRECTORIES", 0);
    this->SetPropertyDefault("ANDROID_NATIVE_LIB_DEPENDENCIES", 0);
    this->SetPropertyDefault("ANDROID_JAVA_SOURCE_DIR", 0);
    this->SetPropertyDefault("ANDROID_JAR_DIRECTORIES", 0);
    this->SetPropertyDefault("ANDROID_JAR_DEPENDENCIES", 0);
    this->SetPropertyDefault("ANDROID_ASSETS_DIRECTORIES", 0);
    this->SetPropertyDefault("ANDROID_ANT_ADDITIONAL_OPTIONS", 0);
    this->SetPropertyDefault("INSTALL_NAME_DIR", 0);
    this->SetPropertyDefault("INSTALL_RPATH", "");
    this->SetPropertyDefault("INSTALL_RPATH_USE_LINK_PATH", "OFF");
    this->SetPropertyDefault("SKIP_BUILD_RPATH", "OFF");
    this->SetPropertyDefault("BUILD_WITH_INSTALL_RPATH", "OFF");
    this->SetPropertyDefault("ARCHIVE_OUTPUT_DIRECTORY", 0);
    this->SetPropertyDefault("LIBRARY_OUTPUT_DIRECTORY", 0);
    this->SetPropertyDefault("RUNTIME_OUTPUT_DIRECTORY", 0);
    this->SetPropertyDefault("PDB_OUTPUT_DIRECTORY", 0);
    this->SetPropertyDefault("COMPILE_PDB_OUTPUT_DIRECTORY", 0);
    this->SetPropertyDefault("Fortran_FORMAT", 0);
    this->SetPropertyDefault("Fortran_MODULE_DIRECTORY", 0);
    this->SetPropertyDefault("GNUtoMS", 0);
    this->SetPropertyDefault("OSX_ARCHITECTURES", 0);
    this->SetPropertyDefault("AUTOMOC", 0);
    this->SetPropertyDefault("AUTOUIC", 0);
    this->SetPropertyDefault("AUTORCC", 0);
    this->SetPropertyDefault("AUTOMOC_MOC_OPTIONS", 0);
    this->SetPropertyDefault("AUTOUIC_OPTIONS", 0);
    this->SetPropertyDefault("AUTORCC_OPTIONS", 0);
    this->SetPropertyDefault("LINK_DEPENDS_NO_SHARED", 0);
    this->SetPropertyDefault("LINK_INTERFACE_LIBRARIES", 0);
    this->SetPropertyDefault("WIN32_EXECUTABLE", 0);
    this->SetPropertyDefault("MACOSX_BUNDLE", 0);
    this->SetPropertyDefault("MACOSX_RPATH", 0);
    this->SetPropertyDefault("NO_SYSTEM_FROM_IMPORTED", 0);
    this->SetPropertyDefault("C_COMPILER_LAUNCHER", 0);
    this->SetPropertyDefault("C_INCLUDE_WHAT_YOU_USE", 0);
    this->SetPropertyDefault("C_STANDARD", 0);
    this->SetPropertyDefault("C_STANDARD_REQUIRED", 0);
    this->SetPropertyDefault("C_EXTENSIONS", 0);
    this->SetPropertyDefault("CXX_COMPILER_LAUNCHER", 0);
    this->SetPropertyDefault("CXX_INCLUDE_WHAT_YOU_USE", 0);
    this->SetPropertyDefault("CXX_STANDARD", 0);
    this->SetPropertyDefault("CXX_STANDARD_REQUIRED", 0);
    this->SetPropertyDefault("CXX_EXTENSIONS", 0);
    this->SetPropertyDefault("LINK_SEARCH_START_STATIC", 0);
    this->SetPropertyDefault("LINK_SEARCH_END_STATIC", 0);
    }

  // Collect the set of configuration types.
  std::vector<std::string> configNames;
  mf->GetConfigurations(configNames);

  // Setup per-configuration property default values.
  if (this->GetType() != UTILITY)
    {
    const char* configProps[] = {
      "ARCHIVE_OUTPUT_DIRECTORY_",
      "LIBRARY_OUTPUT_DIRECTORY_",
      "RUNTIME_OUTPUT_DIRECTORY_",
      "PDB_OUTPUT_DIRECTORY_",
      "COMPILE_PDB_OUTPUT_DIRECTORY_",
      "MAP_IMPORTED_CONFIG_",
      0};
    for(std::vector<std::string>::iterator ci = configNames.begin();
        ci != configNames.end(); ++ci)
      {
      std::string configUpper = cmSystemTools::UpperCase(*ci);
      for(const char** p = configProps; *p; ++p)
        {
        if (this->TargetTypeValue == INTERFACE_LIBRARY
            && strcmp(*p, "MAP_IMPORTED_CONFIG_") != 0)
          {
          continue;
          }
        std::string property = *p;
        property += configUpper;
        this->SetPropertyDefault(property, 0);
        }

      // Initialize per-configuration name postfix property from the
      // variable only for non-executable targets.  This preserves
      // compatibility with previous CMake versions in which executables
      // did not support this variable.  Projects may still specify the
      // property directly.
      if(this->TargetTypeValue != cmTarget::EXECUTABLE
          && this->TargetTypeValue != cmTarget::INTERFACE_LIBRARY)
        {
        std::string property = cmSystemTools::UpperCase(*ci);
        property += "_POSTFIX";
        this->SetPropertyDefault(property, 0);
        }
      }
    }

  // Save the backtrace of target construction.
  this->Internal->Backtrace = this->Makefile->GetBacktrace();

  if (!this->IsImported())
    {
    // Initialize the INCLUDE_DIRECTORIES property based on the current value
    // of the same directory property:
    const cmStringRange parentIncludes =
        this->Makefile->GetIncludeDirectoriesEntries();
    const cmBacktraceRange parentIncludesBts =
        this->Makefile->GetIncludeDirectoriesBacktraces();

    this->Internal->IncludeDirectoriesEntries.insert(
          this->Internal->IncludeDirectoriesEntries.end(),
          parentIncludes.begin(), parentIncludes.end());
    this->Internal->IncludeDirectoriesBacktraces.insert(
          this->Internal->IncludeDirectoriesBacktraces.end(),
          parentIncludesBts.begin(), parentIncludesBts.end());

    const std::set<std::string> parentSystemIncludes =
                                this->Makefile->GetSystemIncludeDirectories();

    this->SystemIncludeDirectories.insert(parentSystemIncludes.begin(),
                                          parentSystemIncludes.end());

    const cmStringRange parentOptions =
                                this->Makefile->GetCompileOptionsEntries();
    const cmBacktraceRange parentOptionsBts =
                                this->Makefile->GetCompileOptionsBacktraces();

    this->Internal->CompileOptionsEntries.insert(
          this->Internal->CompileOptionsEntries.end(),
          parentOptions.begin(), parentOptions.end());
    this->Internal->CompileOptionsBacktraces.insert(
          this->Internal->CompileOptionsBacktraces.end(),
          parentOptionsBts.begin(), parentOptionsBts.end());
    }

  if (this->GetType() != INTERFACE_LIBRARY && this->GetType() != UTILITY)
    {
    this->SetPropertyDefault("C_VISIBILITY_PRESET", 0);
    this->SetPropertyDefault("CXX_VISIBILITY_PRESET", 0);
    this->SetPropertyDefault("VISIBILITY_INLINES_HIDDEN", 0);
    }

  if(this->TargetTypeValue == cmTarget::EXECUTABLE)
    {
    this->SetPropertyDefault("ANDROID_GUI", 0);
    this->SetPropertyDefault("CROSSCOMPILING_EMULATOR", 0);
    this->SetPropertyDefault("ENABLE_EXPORTS", 0);
    }
  if(this->TargetTypeValue == cmTarget::SHARED_LIBRARY
      || this->TargetTypeValue == cmTarget::MODULE_LIBRARY)
    {
    this->SetProperty("POSITION_INDEPENDENT_CODE", "True");
    }
  if(this->TargetTypeValue == cmTarget::SHARED_LIBRARY)
    {
    this->SetPropertyDefault("WINDOWS_EXPORT_ALL_SYMBOLS", 0);
    }

  if (this->GetType() != INTERFACE_LIBRARY && this->GetType() != UTILITY)
    {
    this->SetPropertyDefault("POSITION_INDEPENDENT_CODE", 0);
    }

  // Record current policies for later use.
  this->Makefile->RecordPolicies(this->PolicyMap);

  if (this->TargetTypeValue == INTERFACE_LIBRARY)
    {
    // This policy is checked in a few conditions. The properties relevant
    // to the policy are always ignored for INTERFACE_LIBRARY targets,
    // so ensure that the conditions don't lead to nonsense.
    this->PolicyMap.Set(cmPolicies::CMP0022, cmPolicies::NEW);
    }

  if (this->GetType() != INTERFACE_LIBRARY && this->GetType() != UTILITY)
    {
    this->SetPropertyDefault("JOB_POOL_COMPILE", 0);
    this->SetPropertyDefault("JOB_POOL_LINK", 0);
    }
}

void CreatePropertyGeneratorExpressions(
    std::vector<std::string> const& entries,
    std::vector<cmListFileBacktrace> const& backtraces,
    std::vector<cmTargetInternals::TargetPropertyEntry*>& items)
{
  std::vector<cmListFileBacktrace>::const_iterator btIt = backtraces.begin();
  for (std::vector<std::string>::const_iterator it = entries.begin();
       it != entries.end(); ++it, ++btIt)
    {
    cmGeneratorExpression ge(*btIt);
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(*it);
    items.push_back(new cmTargetInternals::TargetPropertyEntry(cge));
    }
}

//----------------------------------------------------------------------------
void cmTarget::AddUtility(const std::string& u, cmMakefile *makefile)
{
  if(this->Utilities.insert(u).second && makefile)
    {
    UtilityBacktraces.insert(std::make_pair(u, makefile->GetBacktrace()));
    }
}

//----------------------------------------------------------------------------
cmListFileBacktrace const* cmTarget::GetUtilityBacktrace(
    const std::string& u) const
{
  std::map<std::string, cmListFileBacktrace>::const_iterator i =
    this->UtilityBacktraces.find(u);
  if(i == this->UtilityBacktraces.end()) return 0;

  return &i->second;
}

//----------------------------------------------------------------------------
std::set<cmLinkItem> const& cmTarget::GetUtilityItems() const
{
  if(!this->Internal->UtilityItemsDone)
    {
    this->Internal->UtilityItemsDone = true;
    for(std::set<std::string>::const_iterator i = this->Utilities.begin();
        i != this->Utilities.end(); ++i)
      {
      this->Internal->UtilityItems.insert(
        cmLinkItem(*i, this->Makefile->FindTargetToUse(*i)));
      }
    }
  return this->Internal->UtilityItems;
}

//----------------------------------------------------------------------------
void cmTarget::FinishConfigure()
{
  // Erase any cached link information that might have been comptued
  // on-demand during the configuration.  This ensures that build
  // system generation uses up-to-date information even if other cache
  // invalidation code in this source file is buggy.
  this->ClearLinkMaps();

#if defined(_WIN32) && !defined(__CYGWIN__)
  // Do old-style link dependency analysis only for CM_USE_OLD_VS6.
  if(this->Makefile->GetGlobalGenerator()->IsForVS6())
    {
    this->AnalyzeLibDependenciesForVS6(*this->Makefile);
    }
#endif
}

//----------------------------------------------------------------------------
void cmTarget::ClearLinkMaps()
{
  this->LinkImplementationLanguageIsContextDependent = true;
  this->Internal->LinkImplMap.clear();
  this->Internal->SourceFilesMap.clear();
}

//----------------------------------------------------------------------------
cmListFileBacktrace const& cmTarget::GetBacktrace() const
{
  return this->Internal->Backtrace;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetSupportDirectory() const
{
  std::string dir = this->Makefile->GetCurrentBinaryDirectory();
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
bool cmTarget::IsExecutableWithExports() const
{
  return (this->GetType() == cmTarget::EXECUTABLE &&
          this->GetPropertyAsBool("ENABLE_EXPORTS"));
}

//----------------------------------------------------------------------------
bool cmTarget::IsLinkable() const
{
  return (this->GetType() == cmTarget::STATIC_LIBRARY ||
          this->GetType() == cmTarget::SHARED_LIBRARY ||
          this->GetType() == cmTarget::MODULE_LIBRARY ||
          this->GetType() == cmTarget::UNKNOWN_LIBRARY ||
          this->GetType() == cmTarget::INTERFACE_LIBRARY ||
          this->IsExecutableWithExports());
}

//----------------------------------------------------------------------------
bool cmTarget::HasImportLibrary() const
{
  return (this->DLLPlatform &&
          (this->GetType() == cmTarget::SHARED_LIBRARY ||
           this->IsExecutableWithExports()));
}

//----------------------------------------------------------------------------
bool cmTarget::IsFrameworkOnApple() const
{
  return (this->GetType() == cmTarget::SHARED_LIBRARY &&
          this->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("FRAMEWORK"));
}

//----------------------------------------------------------------------------
bool cmTarget::IsAppBundleOnApple() const
{
  return (this->GetType() == cmTarget::EXECUTABLE &&
          this->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("MACOSX_BUNDLE"));
}

//----------------------------------------------------------------------------
bool cmTarget::IsCFBundleOnApple() const
{
  return (this->GetType() == cmTarget::MODULE_LIBRARY &&
          this->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("BUNDLE"));
}

//----------------------------------------------------------------------------
bool cmTarget::IsXCTestOnApple() const
{
  return (this->IsCFBundleOnApple() &&
          this->GetPropertyAsBool("XCTEST"));
}

//----------------------------------------------------------------------------
static bool processSources(cmTarget const* tgt,
      const std::vector<cmTargetInternals::TargetPropertyEntry*> &entries,
      std::vector<std::string> &srcs,
      UNORDERED_SET<std::string> &uniqueSrcs,
      cmGeneratorExpressionDAGChecker *dagChecker,
      std::string const& config, bool debugSources)
{
  cmMakefile *mf = tgt->GetMakefile();

  bool contextDependent = false;

  for (std::vector<cmTargetInternals::TargetPropertyEntry*>::const_iterator
      it = entries.begin(), end = entries.end(); it != end; ++it)
    {
    cmLinkImplItem const& item = (*it)->LinkImplItem;
    std::string const& targetName = item;
    std::vector<std::string> entrySources;
    cmSystemTools::ExpandListArgument((*it)->ge->Evaluate(mf,
                                              config,
                                              false,
                                              tgt,
                                              tgt,
                                              dagChecker),
                                    entrySources);

    if ((*it)->ge->GetHadContextSensitiveCondition())
      {
      contextDependent = true;
      }

    for(std::vector<std::string>::iterator i = entrySources.begin();
        i != entrySources.end(); ++i)
      {
      std::string& src = *i;
      cmSourceFile* sf = mf->GetOrCreateSource(src);
      std::string e;
      std::string fullPath = sf->GetFullPath(&e);
      if(fullPath.empty())
        {
        if(!e.empty())
          {
          cmake* cm = mf->GetCMakeInstance();
          cm->IssueMessage(cmake::FATAL_ERROR, e,
                          tgt->GetBacktrace());
          }
        return contextDependent;
        }

      if (!targetName.empty() && !cmSystemTools::FileIsFullPath(src.c_str()))
        {
        std::ostringstream err;
        if (!targetName.empty())
          {
          err << "Target \"" << targetName << "\" contains relative "
            "path in its INTERFACE_SOURCES:\n"
            "  \"" << src << "\"";
          }
        else
          {
          err << "Found relative path while evaluating sources of "
          "\"" << tgt->GetName() << "\":\n  \"" << src << "\"\n";
          }
        tgt->GetMakefile()->IssueMessage(cmake::FATAL_ERROR, err.str());
        return contextDependent;
        }
      src = fullPath;
      }
    std::string usedSources;
    for(std::vector<std::string>::iterator
          li = entrySources.begin(); li != entrySources.end(); ++li)
      {
      std::string src = *li;

      if(uniqueSrcs.insert(src).second)
        {
        srcs.push_back(src);
        if (debugSources)
          {
          usedSources += " * " + src + "\n";
          }
        }
      }
    if (!usedSources.empty())
      {
      mf->GetCMakeInstance()->IssueMessage(cmake::LOG,
                            std::string("Used sources for target ")
                            + tgt->GetName() + ":\n"
                            + usedSources, (*it)->ge->GetBacktrace());
      }
    }
  return contextDependent;
}

//----------------------------------------------------------------------------
void cmTarget::GetSourceFiles(std::vector<std::string> &files,
                              const std::string& config) const
{
  assert(this->GetType() != INTERFACE_LIBRARY);

  if (!this->GetMakefile()->GetGlobalGenerator()->GetConfigureDoneCMP0026())
    {
    // At configure-time, this method can be called as part of getting the
    // LOCATION property or to export() a file to be include()d.  However
    // there is no cmGeneratorTarget at configure-time, so search the SOURCES
    // for TARGET_OBJECTS instead for backwards compatibility with OLD
    // behavior of CMP0024 and CMP0026 only.

    typedef cmTargetInternals::TargetPropertyEntry
                                TargetPropertyEntry;
    for(std::vector<TargetPropertyEntry*>::const_iterator
          i = this->Internal->SourceEntries.begin();
        i != this->Internal->SourceEntries.end(); ++i)
      {
      std::string entry = (*i)->ge->GetInput();

      std::vector<std::string> items;
      cmSystemTools::ExpandListArgument(entry, items);
      for (std::vector<std::string>::const_iterator
          li = items.begin(); li != items.end(); ++li)
        {
        if(cmHasLiteralPrefix(*li, "$<TARGET_OBJECTS:") &&
            (*li)[li->size() - 1] == '>')
          {
          continue;
          }
        files.push_back(*li);
        }
      }
    return;
    }

  std::vector<std::string> debugProperties;
  const char *debugProp =
              this->Makefile->GetDefinition("CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp)
    {
    cmSystemTools::ExpandListArgument(debugProp, debugProperties);
    }

  bool debugSources = !this->DebugSourcesDone
                    && std::find(debugProperties.begin(),
                                 debugProperties.end(),
                                 "SOURCES")
                        != debugProperties.end();

  if (this->GetMakefile()->GetGlobalGenerator()->GetConfigureDoneCMP0026())
    {
    this->DebugSourcesDone = true;
    }

  cmGeneratorExpressionDAGChecker dagChecker(this->GetName(),
                                             "SOURCES", 0, 0);

  UNORDERED_SET<std::string> uniqueSrcs;
  bool contextDependentDirectSources = processSources(this,
                 this->Internal->SourceEntries,
                 files,
                 uniqueSrcs,
                 &dagChecker,
                 config,
                 debugSources);

  std::vector<cmTargetInternals::TargetPropertyEntry*>
    linkInterfaceSourcesEntries;

  this->Internal->AddInterfaceEntries(
    this, config, "INTERFACE_SOURCES",
    linkInterfaceSourcesEntries);

  std::vector<std::string>::size_type numFilesBefore = files.size();
  bool contextDependentInterfaceSources = processSources(this,
    linkInterfaceSourcesEntries,
                            files,
                            uniqueSrcs,
                            &dagChecker,
                            config,
                            debugSources);

  if (!contextDependentDirectSources
      && !(contextDependentInterfaceSources && numFilesBefore < files.size()))
    {
    this->LinkImplementationLanguageIsContextDependent = false;
    }

  cmDeleteAll(linkInterfaceSourcesEntries);
}

//----------------------------------------------------------------------------
void cmTarget::GetSourceFiles(std::vector<cmSourceFile*> &files,
                              const std::string& config) const
{

  // Lookup any existing link implementation for this configuration.
  std::string key = cmSystemTools::UpperCase(config);

  if(!this->LinkImplementationLanguageIsContextDependent)
    {
    files = this->Internal->SourceFilesMap.begin()->second;
    return;
    }

  cmTargetInternals::SourceFilesMapType::iterator
    it = this->Internal->SourceFilesMap.find(key);
  if(it != this->Internal->SourceFilesMap.end())
    {
    files = it->second;
    }
  else
    {
    std::vector<std::string> srcs;
    this->GetSourceFiles(srcs, config);

    std::set<cmSourceFile*> emitted;

    for(std::vector<std::string>::const_iterator i = srcs.begin();
        i != srcs.end(); ++i)
      {
      cmSourceFile* sf = this->Makefile->GetOrCreateSource(*i);
      if (emitted.insert(sf).second)
        {
        files.push_back(sf);
        }
      }
    this->Internal->SourceFilesMap[key] = files;
    }
}

//----------------------------------------------------------------------------
void cmTarget::AddTracedSources(std::vector<std::string> const& srcs)
{
  std::string srcFiles = cmJoin(srcs, ";");
  if (!srcFiles.empty())
    {
    this->Internal->SourceFilesMap.clear();
    this->LinkImplementationLanguageIsContextDependent = true;
    cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
    cmGeneratorExpression ge(lfbt);
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(srcFiles);
    cge->SetEvaluateForBuildsystem(true);
    this->Internal->SourceEntries.push_back(
                          new cmTargetInternals::TargetPropertyEntry(cge));
    }
}

//----------------------------------------------------------------------------
void cmTarget::AddSources(std::vector<std::string> const& srcs)
{
  std::string srcFiles;
  const char* sep = "";
  for(std::vector<std::string>::const_iterator i = srcs.begin();
      i != srcs.end(); ++i)
    {
    std::string filename = *i;
    const char* src = filename.c_str();

    if(!(src[0] == '$' && src[1] == '<'))
      {
      if(!filename.empty())
        {
        filename = this->ProcessSourceItemCMP0049(filename);
        if(filename.empty())
          {
          return;
          }
        }
      this->Makefile->GetOrCreateSource(filename);
      }
    srcFiles += sep;
    srcFiles += filename;
    sep = ";";
    }
  if (!srcFiles.empty())
    {
    this->Internal->SourceFilesMap.clear();
    this->LinkImplementationLanguageIsContextDependent = true;
    cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
    cmGeneratorExpression ge(lfbt);
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(srcFiles);
    cge->SetEvaluateForBuildsystem(true);
    this->Internal->SourceEntries.push_back(
                          new cmTargetInternals::TargetPropertyEntry(cge));
    }
}

//----------------------------------------------------------------------------
std::string cmTarget::ProcessSourceItemCMP0049(const std::string& s)
{
  std::string src = s;

  // For backwards compatibility replace varibles in source names.
  // This should eventually be removed.
  this->Makefile->ExpandVariablesInString(src);
  if (src != s)
    {
    std::ostringstream e;
    bool noMessage = false;
    cmake::MessageType messageType = cmake::AUTHOR_WARNING;
    switch(this->Makefile->GetPolicyStatus(cmPolicies::CMP0049))
      {
      case cmPolicies::WARN:
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0049) << "\n";
        break;
      case cmPolicies::OLD:
        noMessage = true;
        break;
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::NEW:
        messageType = cmake::FATAL_ERROR;
      }
    if (!noMessage)
      {
      e << "Legacy variable expansion in source file \""
        << s << "\" expanded to \"" << src << "\" in target \""
        << this->GetName() << "\".  This behavior will be removed in a "
        "future version of CMake.";
      this->Makefile->IssueMessage(messageType, e.str());
      if (messageType == cmake::FATAL_ERROR)
        {
        return "";
        }
      }
    }
  return src;
}

//----------------------------------------------------------------------------
cmSourceFile* cmTarget::AddSourceCMP0049(const std::string& s)
{
  std::string src = this->ProcessSourceItemCMP0049(s);
  if(!s.empty() && src.empty())
    {
    return 0;
    }
  return this->AddSource(src);
}

//----------------------------------------------------------------------------
struct CreateLocation
{
  cmMakefile const* Makefile;

  CreateLocation(cmMakefile const* mf)
    : Makefile(mf)
  {

  }

  cmSourceFileLocation operator()(const std::string& filename)
  {
    return cmSourceFileLocation(this->Makefile, filename);
  }
};

//----------------------------------------------------------------------------
struct LocationMatcher
{
  const cmSourceFileLocation& Needle;

  LocationMatcher(const cmSourceFileLocation& needle)
    : Needle(needle)
  {

  }

  bool operator()(cmSourceFileLocation &loc)
  {
    return loc.Matches(this->Needle);
  }
};


//----------------------------------------------------------------------------
struct TargetPropertyEntryFinder
{
private:
  const cmSourceFileLocation& Needle;
public:
  TargetPropertyEntryFinder(const cmSourceFileLocation& needle)
    : Needle(needle)
  {

  }

  bool operator()(cmTargetInternals::TargetPropertyEntry* entry)
  {
    std::vector<std::string> files;
    cmSystemTools::ExpandListArgument(entry->ge->GetInput(), files);
    std::vector<cmSourceFileLocation> locations(files.size());
    std::transform(files.begin(), files.end(), locations.begin(),
                   CreateLocation(this->Needle.GetMakefile()));

    return std::find_if(locations.begin(), locations.end(),
        LocationMatcher(this->Needle)) != locations.end();
  }
};

//----------------------------------------------------------------------------
cmSourceFile* cmTarget::AddSource(const std::string& src)
{
  cmSourceFileLocation sfl(this->Makefile, src);
  if (std::find_if(this->Internal->SourceEntries.begin(),
                   this->Internal->SourceEntries.end(),
                   TargetPropertyEntryFinder(sfl))
                                      == this->Internal->SourceEntries.end())
    {
    this->Internal->SourceFilesMap.clear();
    this->LinkImplementationLanguageIsContextDependent = true;
    cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
    cmGeneratorExpression ge(lfbt);
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(src);
    cge->SetEvaluateForBuildsystem(true);
    this->Internal->SourceEntries.push_back(
                          new cmTargetInternals::TargetPropertyEntry(cge));
    }
  if (cmGeneratorExpression::Find(src) != std::string::npos)
    {
    return 0;
    }
  return this->Makefile->GetOrCreateSource(src);
}

//----------------------------------------------------------------------------
void cmTarget::MergeLinkLibraries( cmMakefile& mf,
                                   const std::string& selfname,
                                   const LinkLibraryVectorType& libs )
{
  // Only add on libraries we haven't added on before.
  // Assumption: the global link libraries could only grow, never shrink
  LinkLibraryVectorType::const_iterator i = libs.begin();
  i += this->PrevLinkedLibraries.size();
  for( ; i != libs.end(); ++i )
    {
    // This is equivalent to the target_link_libraries plain signature.
    this->AddLinkLibrary( mf, selfname, i->first, i->second );
    this->AppendProperty("INTERFACE_LINK_LIBRARIES",
      this->GetDebugGeneratorExpressions(i->first, i->second).c_str());
    }
  this->PrevLinkedLibraries = libs;
}

//----------------------------------------------------------------------------
void cmTarget::AddLinkDirectory(const std::string& d)
{
  // Make sure we don't add unnecessary search directories.
  if(this->LinkDirectoriesEmmitted.insert(d).second)
    {
    this->LinkDirectories.push_back(d);
    }
}

//----------------------------------------------------------------------------
const std::vector<std::string>& cmTarget::GetLinkDirectories() const
{
  return this->LinkDirectories;
}

//----------------------------------------------------------------------------
cmTarget::LinkLibraryType cmTarget::ComputeLinkType(
                                      const std::string& config) const
{
  // No configuration is always optimized.
  if(config.empty())
    {
    return cmTarget::OPTIMIZED;
    }

  // Get the list of configurations considered to be DEBUG.
  std::vector<std::string> debugConfigs =
    this->Makefile->GetCMakeInstance()->GetDebugConfigs();

  // Check if any entry in the list matches this configuration.
  std::string configUpper = cmSystemTools::UpperCase(config);
  if (std::find(debugConfigs.begin(), debugConfigs.end(), configUpper) !=
      debugConfigs.end())
    {
    return cmTarget::DEBUG;
    }
  // The current configuration is not a debug configuration.
  return cmTarget::OPTIMIZED;
}

//----------------------------------------------------------------------------
void cmTarget::ClearDependencyInformation( cmMakefile& mf,
                                           const std::string& target )
{
  // Clear the dependencies. The cache variable must exist iff we are
  // recording dependency information for this target.
  std::string depname = target;
  depname += "_LIB_DEPENDS";
  if (this->RecordDependencies)
    {
    mf.AddCacheDefinition(depname, "",
                          "Dependencies for target", cmState::STATIC);
    }
  else
    {
    if (mf.GetDefinition( depname ))
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
bool cmTarget::NameResolvesToFramework(const std::string& libname) const
{
  return this->Makefile->GetGlobalGenerator()->
    NameResolvesToFramework(libname);
}

//----------------------------------------------------------------------------
std::string cmTarget::GetDebugGeneratorExpressions(const std::string &value,
                                  cmTarget::LinkLibraryType llt) const
{
  if (llt == GENERAL)
    {
    return value;
    }

  // Get the list of configurations considered to be DEBUG.
  std::vector<std::string> debugConfigs =
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
static std::string targetNameGenex(const std::string& lib)
{
  return "$<TARGET_NAME:" + lib + ">";
}

//----------------------------------------------------------------------------
bool cmTarget::PushTLLCommandTrace(TLLSignature signature,
                                   cmListFileContext const& lfc)
{
  bool ret = true;
  if (!this->TLLCommands.empty())
    {
    if (this->TLLCommands.back().first != signature)
      {
      ret = false;
      }
    }
  if (this->TLLCommands.empty() || this->TLLCommands.back().second != lfc)
    {
    this->TLLCommands.push_back(std::make_pair(signature, lfc));
    }
  return ret;
}

//----------------------------------------------------------------------------
void cmTarget::GetTllSignatureTraces(std::ostringstream &s,
                                     TLLSignature sig) const
{
  const char *sigString = (sig == cmTarget::KeywordTLLSignature ? "keyword"
                                                                : "plain");
  s << "The uses of the " << sigString << " signature are here:\n";
  typedef std::vector<std::pair<TLLSignature, cmListFileContext> > Container;
  cmOutputConverter converter(this->GetMakefile()->GetStateSnapshot());
  for(Container::const_iterator it = this->TLLCommands.begin();
      it != this->TLLCommands.end(); ++it)
    {
    if (it->first == sig)
      {
      cmListFileContext lfc = it->second;
      lfc.FilePath = converter.Convert(lfc.FilePath, cmOutputConverter::HOME);
      s << " * " << lfc << std::endl;
      }
    }
}

//----------------------------------------------------------------------------
void cmTarget::AddLinkLibrary(cmMakefile& mf,
                              const std::string& target,
                              const std::string& lib,
                              LinkLibraryType llt)
{
  cmTarget *tgt = this->Makefile->FindTargetToUse(lib);
  {
  const bool isNonImportedTarget = tgt && !tgt->IsImported();

  const std::string libName = (isNonImportedTarget && llt != GENERAL)
                                                        ? targetNameGenex(lib)
                                                        : lib;
  this->AppendProperty("LINK_LIBRARIES",
                       this->GetDebugGeneratorExpressions(libName,
                                                          llt).c_str());
  }

  if (cmGeneratorExpression::Find(lib) != std::string::npos
      || (tgt && tgt->GetType() == INTERFACE_LIBRARY)
      || (target == lib ))
    {
    return;
    }

  cmTarget::LibraryID tmp;
  tmp.first = lib;
  tmp.second = llt;
#if defined(_WIN32) && !defined(__CYGWIN__)
  this->LinkLibrariesForVS6.push_back( tmp );
#endif
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
    const char* old_val = mf.GetDefinition( targetEntry );
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
    mf.AddCacheDefinition( targetEntry, dependencies.c_str(),
                           "Dependencies for the target",
                           cmState::STATIC );
    }

}

//----------------------------------------------------------------------------
void
cmTarget::AddSystemIncludeDirectories(const std::set<std::string> &incs)
{
  this->SystemIncludeDirectories.insert(incs.begin(), incs.end());
}

cmStringRange cmTarget::GetIncludeDirectoriesEntries() const
{
  return cmMakeRange(this->Internal->IncludeDirectoriesEntries);
}

cmBacktraceRange cmTarget::GetIncludeDirectoriesBacktraces() const
{
  return cmMakeRange(this->Internal->IncludeDirectoriesBacktraces);
}

cmStringRange cmTarget::GetCompileOptionsEntries() const
{
  return cmMakeRange(this->Internal->CompileOptionsEntries);
}

cmBacktraceRange cmTarget::GetCompileOptionsBacktraces() const
{
  return cmMakeRange(this->Internal->CompileOptionsBacktraces);
}

cmStringRange cmTarget::GetCompileFeaturesEntries() const
{
  return cmMakeRange(this->Internal->CompileFeaturesEntries);
}

cmBacktraceRange cmTarget::GetCompileFeaturesBacktraces() const
{
  return cmMakeRange(this->Internal->CompileFeaturesBacktraces);
}

cmStringRange cmTarget::GetCompileDefinitionsEntries() const
{
  return cmMakeRange(this->Internal->CompileDefinitionsEntries);
}

cmBacktraceRange cmTarget::GetCompileDefinitionsBacktraces() const
{
  return cmMakeRange(this->Internal->CompileDefinitionsBacktraces);
}

#if defined(_WIN32) && !defined(__CYGWIN__)
//----------------------------------------------------------------------------
void
cmTarget::AnalyzeLibDependenciesForVS6( const cmMakefile& mf )
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
 for(LinkLibraryVectorType::iterator p = this->LinkLibrariesForVS6.begin();
     p != this->LinkLibrariesForVS6.end(); ++p)
   {
   this->Makefile->ExpandVariablesInString(p->first, true, true);
   }
 }

 // The dependency map.
 DependencyMap dep_map;

 // 1. Build the dependency graph
 //
 for(LinkLibraryVectorType::reverse_iterator lib
       = this->LinkLibrariesForVS6.rbegin();
     lib != this->LinkLibrariesForVS6.rend(); ++lib)
   {
   this->GatherDependenciesForVS6( mf, *lib, dep_map);
   }

 // 2. Remove any dependencies that are already satisfied in the original
 // link line.
 //
 for(LinkLibraryVectorType::iterator lib = this->LinkLibrariesForVS6.begin();
     lib != this->LinkLibrariesForVS6.end(); ++lib)
   {
   for( LinkLibraryVectorType::iterator lib2 = lib;
        lib2 != this->LinkLibrariesForVS6.end(); ++lib2)
     {
     this->DeleteDependencyForVS6( dep_map, *lib, *lib2);
     }
   }


 // 3. Create the new link line by simply emitting any dependencies that are
 // missing.  Start from the back and keep adding.
 //
 std::set<DependencyMap::key_type> done, visited;
 std::vector<DependencyMap::key_type> newLinkLibrariesForVS6;
 for(LinkLibraryVectorType::reverse_iterator lib =
       this->LinkLibrariesForVS6.rbegin();
     lib != this->LinkLibrariesForVS6.rend(); ++lib)
   {
   // skip zero size library entries, this may happen
   // if a variable expands to nothing.
   if (!lib->first.empty())
     {
     this->EmitForVS6( *lib, dep_map, done, visited, newLinkLibrariesForVS6 );
     }
   }

 // 4. Add the new libraries to the link line.
 //
 for( std::vector<DependencyMap::key_type>::reverse_iterator k =
        newLinkLibrariesForVS6.rbegin();
      k != newLinkLibrariesForVS6.rend(); ++k )
   {
   // get the llt from the dep_map
   this->LinkLibrariesForVS6.push_back( std::make_pair(k->first,k->second) );
   }
 this->LinkLibrariesForVS6Analyzed = true;
}

//----------------------------------------------------------------------------
void cmTarget::InsertDependencyForVS6( DependencyMap& depMap,
                                       const LibraryID& lib,
                                       const LibraryID& dep)
{
  depMap[lib].push_back(dep);
}

//----------------------------------------------------------------------------
void cmTarget::DeleteDependencyForVS6( DependencyMap& depMap,
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
    DependencyList::iterator begin =
        std::remove(depList.begin(), depList.end(), dep);
    depList.erase(begin, depList.end());
    }
}

//----------------------------------------------------------------------------
void cmTarget::EmitForVS6(const LibraryID lib,
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
            this->EmitForVS6( *i, dep_map, emitted, visited, link_line );
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
void cmTarget::GatherDependenciesForVS6( const cmMakefile& mf,
                                         const LibraryID& lib,
                                         DependencyMap& dep_map)
{
  // If the library is already in the dependency map, then it has
  // already been fully processed.
  if( dep_map.find(lib) != dep_map.end() )
    {
    return;
    }

  const char* deps = mf.GetDefinition( lib.first+"_LIB_DEPENDS" );
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
      if(!l.empty())
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
          this->InsertDependencyForVS6( dep_map, lib, lib2);
          this->GatherDependenciesForVS6( mf, lib2, dep_map);
          llt = cmTarget::GENERAL;
          }
        }
      start = end+1; // skip the ;
      end = depline.find( ";", start );
      }
    // cannot depend on itself
    this->DeleteDependencyForVS6( dep_map, lib, lib);
    }
}
#endif

//----------------------------------------------------------------------------
static bool whiteListedInterfaceProperty(const std::string& prop)
{
  if(cmHasLiteralPrefix(prop, "INTERFACE_"))
    {
    return true;
    }
  static UNORDERED_SET<std::string> builtIns;
  if (builtIns.empty())
    {
    builtIns.insert("COMPATIBLE_INTERFACE_BOOL");
    builtIns.insert("COMPATIBLE_INTERFACE_NUMBER_MAX");
    builtIns.insert("COMPATIBLE_INTERFACE_NUMBER_MIN");
    builtIns.insert("COMPATIBLE_INTERFACE_STRING");
    builtIns.insert("EXPORT_NAME");
    builtIns.insert("IMPORTED");
    builtIns.insert("NAME");
    builtIns.insert("TYPE");
    }

  if (builtIns.count(prop))
    {
    return true;
    }

  if (cmHasLiteralPrefix(prop, "MAP_IMPORTED_CONFIG_"))
    {
    return true;
    }

  return false;
}

//----------------------------------------------------------------------------
void cmTarget::SetProperty(const std::string& prop, const char* value)
{
  if (this->GetType() == INTERFACE_LIBRARY
      && !whiteListedInterfaceProperty(prop))
    {
    std::ostringstream e;
    e << "INTERFACE_LIBRARY targets may only have whitelisted properties.  "
         "The property \"" << prop << "\" is not allowed.";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
    return;
    }
  else if (prop == "NAME")
    {
    std::ostringstream e;
    e << "NAME property is read-only\n";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
    return;
    }
  else if(prop == "INCLUDE_DIRECTORIES")
    {
    this->Internal->IncludeDirectoriesEntries.clear();
    this->Internal->IncludeDirectoriesBacktraces.clear();
    if (value)
      {
      this->Internal->IncludeDirectoriesEntries.push_back(value);
      cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
      this->Internal->IncludeDirectoriesBacktraces.push_back(lfbt);
      }
    }
  else if(prop == "COMPILE_OPTIONS")
    {
    this->Internal->CompileOptionsEntries.clear();
    this->Internal->CompileOptionsBacktraces.clear();
    if (value)
      {
      this->Internal->CompileOptionsEntries.push_back(value);
      cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
      this->Internal->CompileOptionsBacktraces.push_back(lfbt);
      }
    }
  else if(prop == "COMPILE_FEATURES")
    {
    this->Internal->CompileFeaturesEntries.clear();
    this->Internal->CompileFeaturesBacktraces.clear();
    if (value)
      {
      this->Internal->CompileFeaturesEntries.push_back(value);
      cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
      this->Internal->CompileFeaturesBacktraces.push_back(lfbt);
      }
    }
  else if(prop == "COMPILE_DEFINITIONS")
    {
    this->Internal->CompileDefinitionsEntries.clear();
    this->Internal->CompileDefinitionsBacktraces.clear();
    if (value)
      {
      this->Internal->CompileDefinitionsEntries.push_back(value);
      cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
      this->Internal->CompileDefinitionsBacktraces.push_back(lfbt);
      }
    }
  else if(prop == "EXPORT_NAME" && this->IsImported())
    {
    std::ostringstream e;
    e << "EXPORT_NAME property can't be set on imported targets (\""
          << this->Name << "\")\n";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
    }
  else if (prop == "LINK_LIBRARIES")
    {
    this->Internal->LinkImplementationPropertyEntries.clear();
    if (value)
      {
      cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
      cmValueWithOrigin entry(value, lfbt);
      this->Internal->LinkImplementationPropertyEntries.push_back(entry);
      }
    }
  else if (prop == "SOURCES")
    {
    if(this->IsImported())
      {
      std::ostringstream e;
      e << "SOURCES property can't be set on imported targets (\""
            << this->Name << "\")\n";
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
      return;
      }
    this->Internal->SourceFilesMap.clear();
    cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
    cmGeneratorExpression ge(lfbt);
    cmDeleteAll(this->Internal->SourceEntries);
    this->Internal->SourceEntries.clear();
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(value);
    this->Internal->SourceEntries.push_back(
                          new cmTargetInternals::TargetPropertyEntry(cge));
    }
  else
    {
    this->Properties.SetProperty(prop, value);
    this->MaybeInvalidatePropertyCache(prop);
    }
}

//----------------------------------------------------------------------------
void cmTarget::AppendProperty(const std::string& prop, const char* value,
                              bool asString)
{
  if (this->GetType() == INTERFACE_LIBRARY
      && !whiteListedInterfaceProperty(prop))
    {
    std::ostringstream e;
    e << "INTERFACE_LIBRARY targets may only have whitelisted properties.  "
         "The property \"" << prop << "\" is not allowed.";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
    return;
    }
  else if (prop == "NAME")
    {
    std::ostringstream e;
    e << "NAME property is read-only\n";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
    return;
    }
  else if(prop == "INCLUDE_DIRECTORIES")
    {
    if (value && *value)
      {
      this->Internal->IncludeDirectoriesEntries.push_back(value);
      cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
      this->Internal->IncludeDirectoriesBacktraces.push_back(lfbt);
      }
    }
  else if(prop == "COMPILE_OPTIONS")
    {
    if (value && *value)
      {
      this->Internal->CompileOptionsEntries.push_back(value);
      cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
      this->Internal->CompileOptionsBacktraces.push_back(lfbt);
      }
    }
  else if(prop == "COMPILE_FEATURES")
    {
    if (value && *value)
      {
      this->Internal->CompileFeaturesEntries.push_back(value);
      cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
      this->Internal->CompileFeaturesBacktraces.push_back(lfbt);
      }
    }
  else if(prop == "COMPILE_DEFINITIONS")
    {
    if (value && *value)
      {
      this->Internal->CompileDefinitionsEntries.push_back(value);
      cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
      this->Internal->CompileDefinitionsBacktraces.push_back(lfbt);
      }
    }
  else if(prop == "EXPORT_NAME" && this->IsImported())
    {
    std::ostringstream e;
    e << "EXPORT_NAME property can't be set on imported targets (\""
          << this->Name << "\")\n";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
    }
  else if (prop == "LINK_LIBRARIES")
    {
    if (value && *value)
      {
      cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
      cmValueWithOrigin entry(value, lfbt);
      this->Internal->LinkImplementationPropertyEntries.push_back(entry);
      }
    }
  else if (prop == "SOURCES")
    {
    if(this->IsImported())
      {
      std::ostringstream e;
      e << "SOURCES property can't be set on imported targets (\""
            << this->Name << "\")\n";
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
      return;
      }
    this->Internal->SourceFilesMap.clear();
    cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
    cmGeneratorExpression ge(lfbt);
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(value);
    this->Internal->SourceEntries.push_back(
                          new cmTargetInternals::TargetPropertyEntry(cge));
    }
  else
    {
    this->Properties.AppendProperty(prop, value, asString);
    this->MaybeInvalidatePropertyCache(prop);
    }
}

//----------------------------------------------------------------------------
std::string cmTarget::GetExportName() const
{
  const char *exportName = this->GetProperty("EXPORT_NAME");

  if (exportName && *exportName)
    {
    if (!cmGeneratorExpression::IsValidTargetName(exportName))
      {
      std::ostringstream e;
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
     this->GetType() != cmTarget::INTERFACE_LIBRARY &&
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
    const char *binDir = this->Makefile->GetCurrentBinaryDirectory();
    const char *srcDir = this->Makefile->GetCurrentSourceDirectory();
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
void cmTarget::InsertInclude(std::string const& entry,
                             cmListFileBacktrace const& bt,
                             bool before)
{
  std::vector<std::string>::iterator position =
      before ? this->Internal->IncludeDirectoriesEntries.begin()
             : this->Internal->IncludeDirectoriesEntries.end();

  std::vector<cmListFileBacktrace>::iterator btPosition =
      before ? this->Internal->IncludeDirectoriesBacktraces.begin()
             : this->Internal->IncludeDirectoriesBacktraces.end();

  this->Internal->IncludeDirectoriesEntries.insert(position, entry);
  this->Internal->IncludeDirectoriesBacktraces.insert(btPosition, bt);
}

//----------------------------------------------------------------------------
void cmTarget::InsertCompileOption(std::string const& entry,
                                   cmListFileBacktrace const& bt,
                                   bool before)
{
  std::vector<std::string>::iterator position =
      before ? this->Internal->CompileOptionsEntries.begin()
             : this->Internal->CompileOptionsEntries.end();

  std::vector<cmListFileBacktrace>::iterator btPosition =
      before ? this->Internal->CompileOptionsBacktraces.begin()
             : this->Internal->CompileOptionsBacktraces.end();

  this->Internal->CompileOptionsEntries.insert(position, entry);
  this->Internal->CompileOptionsBacktraces.insert(btPosition, bt);
}

//----------------------------------------------------------------------------
void cmTarget::InsertCompileDefinition(std::string const& entry,
                                       cmListFileBacktrace const& bt)
{
  this->Internal->CompileDefinitionsEntries.push_back(entry);
  this->Internal->CompileDefinitionsBacktraces.push_back(bt);
}

//----------------------------------------------------------------------------
void cmTarget::MaybeInvalidatePropertyCache(const std::string& prop)
{
  // Wipe out maps caching information affected by this property.
  if(this->IsImported() && cmHasLiteralPrefix(prop, "IMPORTED"))
    {
    this->Internal->ImportInfoMap.clear();
    }
  if(!this->IsImported() && cmHasLiteralPrefix(prop, "LINK_INTERFACE_"))
    {
    this->ClearLinkMaps();
    }
}

//----------------------------------------------------------------------------
static void cmTargetCheckLINK_INTERFACE_LIBRARIES(
  const std::string& prop, const char* value, cmMakefile* context,
  bool imported)
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
  std::ostringstream e;
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
  std::ostringstream e;

  e << "Property INTERFACE_LINK_LIBRARIES may not contain link-type "
    "keyword \"" << keys.match(2) << "\".  The INTERFACE_LINK_LIBRARIES "
    "property may contain configuration-sensitive generator-expressions "
    "which may be used to specify per-configuration rules.";

  context->IssueMessage(cmake::FATAL_ERROR, e.str());
}

//----------------------------------------------------------------------------
void cmTarget::CheckProperty(const std::string& prop,
                             cmMakefile* context) const
{
  // Certain properties need checking.
  if(cmHasLiteralPrefix(prop, "LINK_INTERFACE_LIBRARIES"))
    {
    if(const char* value = this->GetProperty(prop))
      {
      cmTargetCheckLINK_INTERFACE_LIBRARIES(prop, value, context, false);
      }
    }
  if(cmHasLiteralPrefix(prop, "IMPORTED_LINK_INTERFACE_LIBRARIES"))
    {
    if(const char* value = this->GetProperty(prop))
      {
      cmTargetCheckLINK_INTERFACE_LIBRARIES(prop, value, context, true);
      }
    }
  if(cmHasLiteralPrefix(prop, "INTERFACE_LINK_LIBRARIES"))
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
bool cmTarget::HaveWellDefinedOutputFiles() const
{
  return
    this->GetType() == cmTarget::STATIC_LIBRARY ||
    this->GetType() == cmTarget::SHARED_LIBRARY ||
    this->GetType() == cmTarget::MODULE_LIBRARY ||
    this->GetType() == cmTarget::EXECUTABLE;
}

//----------------------------------------------------------------------------
cmTarget::OutputInfo const* cmTarget::GetOutputInfo(
    const std::string& config) const
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
    return 0;
    }

  // Lookup/compute/cache the output information for this configuration.
  std::string config_upper;
  if(!config.empty())
    {
    config_upper = cmSystemTools::UpperCase(config);
    }
  typedef cmTargetInternals::OutputInfoMapType OutputInfoMapType;
  OutputInfoMapType::iterator i =
    this->Internal->OutputInfoMap.find(config_upper);
  if(i == this->Internal->OutputInfoMap.end())
    {
    // Add empty info in map to detect potential recursion.
    OutputInfo info;
    OutputInfoMapType::value_type entry(config_upper, info);
    i = this->Internal->OutputInfoMap.insert(entry).first;

    // Compute output directories.
    this->ComputeOutputDir(config, false, info.OutDir);
    this->ComputeOutputDir(config, true, info.ImpDir);
    if(!this->ComputePDBOutputDir("PDB", config, info.PdbDir))
      {
      info.PdbDir = info.OutDir;
      }

    // Now update the previously-prepared map entry.
    i->second = info;
    }
  else if(i->second.empty())
    {
    // An empty map entry indicates we have been called recursively
    // from the above block.
    this->Makefile->GetCMakeInstance()->IssueMessage(
      cmake::FATAL_ERROR,
      "Target '" + this->GetName() + "' OUTPUT_DIRECTORY depends on itself.",
      this->GetBacktrace());
    return 0;
    }
  return &i->second;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetDirectory(const std::string& config,
                                   bool implib) const
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
std::string cmTarget::GetPDBDirectory(const std::string& config) const
{
  if(OutputInfo const* info = this->GetOutputInfo(config))
    {
    // Return the directory in which the target will be built.
    return info->PdbDir;
    }
  return "";
}

//----------------------------------------------------------------------------
const char* cmTarget::ImportedGetLocation(const std::string& config) const
{
  static std::string location;
  assert(this->IsImported());
  location = this->ImportedGetFullPath(config, false);
  return location.c_str();
}

//----------------------------------------------------------------------------
void cmTarget::GetTargetVersion(int& major, int& minor) const
{
  int patch;
  this->GetTargetVersion(false, major, minor, patch);
}

//----------------------------------------------------------------------------
void cmTarget::GetTargetVersion(bool soversion,
                                int& major, int& minor, int& patch) const
{
  // Set the default values.
  major = 0;
  minor = 0;
  patch = 0;

  assert(this->GetType() != INTERFACE_LIBRARY);

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
bool cmTarget::HandleLocationPropertyPolicy(cmMakefile* context) const
{
  if (this->IsImported())
    {
    return true;
    }
  std::ostringstream e;
  const char *modal = 0;
  cmake::MessageType messageType = cmake::AUTHOR_WARNING;
  switch (context->GetPolicyStatus(cmPolicies::CMP0026))
    {
    case cmPolicies::WARN:
      e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0026) << "\n";
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
    e << "The LOCATION property " << modal << " not be read from target \""
      << this->GetName() << "\".  Use the target name directly with "
      "add_custom_command, or use the generator expression $<TARGET_FILE>, "
      "as appropriate.\n";
    context->IssueMessage(messageType, e.str());
    }

  return messageType != cmake::FATAL_ERROR;
}

//----------------------------------------------------------------------------
const char *cmTarget::GetProperty(const std::string& prop) const
{
  return this->GetProperty(prop, this->Makefile);
}

//----------------------------------------------------------------------------
const char *cmTarget::GetProperty(const std::string& prop,
                                  cmMakefile* context) const
{
  if (this->GetType() == INTERFACE_LIBRARY
      && !whiteListedInterfaceProperty(prop))
    {
    std::ostringstream e;
    e << "INTERFACE_LIBRARY targets may only have whitelisted properties.  "
         "The property \"" << prop << "\" is not allowed.";
    context->IssueMessage(cmake::FATAL_ERROR, e.str());
    return 0;
    }

  // Watch for special "computed" properties that are dependent on
  // other properties or variables.  Always recompute them.
  if(this->GetType() == cmTarget::EXECUTABLE ||
     this->GetType() == cmTarget::STATIC_LIBRARY ||
     this->GetType() == cmTarget::SHARED_LIBRARY ||
     this->GetType() == cmTarget::MODULE_LIBRARY ||
     this->GetType() == cmTarget::UNKNOWN_LIBRARY)
    {
    static const std::string propLOCATION = "LOCATION";
    if(prop == propLOCATION)
      {
      if (!this->HandleLocationPropertyPolicy(context))
        {
        return 0;
        }

      // Set the LOCATION property of the target.
      //
      // For an imported target this is the location of an arbitrary
      // available configuration.
      //
      if(this->IsImported())
        {
        this->Properties.SetProperty(
                propLOCATION, this->ImportedGetFullPath("", false).c_str());
        }
      else
        {
        // For a non-imported target this is deprecated because it
        // cannot take into account the per-configuration name of the
        // target because the configuration type may not be known at
        // CMake time.
        cmGlobalGenerator* gg = this->Makefile->GetGlobalGenerator();
        gg->CreateGenerationObjects();
        cmGeneratorTarget* gt = gg->GetGeneratorTarget(this);
        this->Properties.SetProperty(propLOCATION,
                                     gt->GetLocationForBuild());
        }

      }

    // Support "LOCATION_<CONFIG>".
    else if(cmHasLiteralPrefix(prop, "LOCATION_"))
      {
      if (!this->HandleLocationPropertyPolicy(context))
        {
        return 0;
        }
      const char* configName = prop.c_str() + 9;

      if (this->IsImported())
        {
        this->Properties.SetProperty(
                prop, this->ImportedGetFullPath(configName, false).c_str());
        }
      else
        {
        cmGlobalGenerator* gg = this->Makefile->GetGlobalGenerator();
        gg->CreateGenerationObjects();
        cmGeneratorTarget* gt = gg->GetGeneratorTarget(this);
        this->Properties.SetProperty(
                prop, gt->GetFullPath(configName, false).c_str());
        }
      }
    // Support "<CONFIG>_LOCATION".
    else if(cmHasLiteralSuffix(prop, "_LOCATION"))
      {
      std::string configName(prop.c_str(), prop.size() - 9);
      if(configName != "IMPORTED")
        {
        if (!this->HandleLocationPropertyPolicy(context))
          {
          return 0;
          }
        if (this->IsImported())
          {
          this->Properties.SetProperty(
                  prop, this->ImportedGetFullPath(configName, false).c_str());
          }
        else
          {
          cmGlobalGenerator* gg = this->Makefile->GetGlobalGenerator();
          gg->CreateGenerationObjects();
          cmGeneratorTarget* gt = gg->GetGeneratorTarget(this);
          this->Properties.SetProperty(
                  prop, gt->GetFullPath(configName, false).c_str());
          }
        }
      }
    }
  static UNORDERED_SET<std::string> specialProps;
#define MAKE_STATIC_PROP(PROP) \
  static const std::string prop##PROP = #PROP
  MAKE_STATIC_PROP(LINK_LIBRARIES);
  MAKE_STATIC_PROP(TYPE);
  MAKE_STATIC_PROP(INCLUDE_DIRECTORIES);
  MAKE_STATIC_PROP(COMPILE_FEATURES);
  MAKE_STATIC_PROP(COMPILE_OPTIONS);
  MAKE_STATIC_PROP(COMPILE_DEFINITIONS);
  MAKE_STATIC_PROP(IMPORTED);
  MAKE_STATIC_PROP(NAME);
  MAKE_STATIC_PROP(BINARY_DIR);
  MAKE_STATIC_PROP(SOURCE_DIR);
  MAKE_STATIC_PROP(SOURCES);
#undef MAKE_STATIC_PROP
  if(specialProps.empty())
    {
    specialProps.insert(propLINK_LIBRARIES);
    specialProps.insert(propTYPE);
    specialProps.insert(propINCLUDE_DIRECTORIES);
    specialProps.insert(propCOMPILE_FEATURES);
    specialProps.insert(propCOMPILE_OPTIONS);
    specialProps.insert(propCOMPILE_DEFINITIONS);
    specialProps.insert(propIMPORTED);
    specialProps.insert(propNAME);
    specialProps.insert(propBINARY_DIR);
    specialProps.insert(propSOURCE_DIR);
    specialProps.insert(propSOURCES);
    }
  if(specialProps.count(prop))
    {
    if(prop == propLINK_LIBRARIES)
      {
      if (this->Internal->LinkImplementationPropertyEntries.empty())
        {
        return 0;
        }

      static std::string output;
      output = "";
      std::string sep;
      for (std::vector<cmValueWithOrigin>::const_iterator
          it = this->Internal->LinkImplementationPropertyEntries.begin(),
          end = this->Internal->LinkImplementationPropertyEntries.end();
          it != end; ++it)
        {
        output += sep;
        output += it->Value;
        sep = ";";
        }
      return output.c_str();
      }
    // the type property returns what type the target is
    else if (prop == propTYPE)
      {
      return cmTarget::GetTargetTypeName(this->GetType());
      }
    else if(prop == propINCLUDE_DIRECTORIES)
      {
      if (this->Internal->IncludeDirectoriesEntries.empty())
        {
        return 0;
        }

      static std::string output;
      output = cmJoin(this->Internal->IncludeDirectoriesEntries, ";");
      return output.c_str();
      }
    else if(prop == propCOMPILE_FEATURES)
      {
      if (this->Internal->CompileFeaturesEntries.empty())
        {
        return 0;
        }

      static std::string output;
      output = cmJoin(this->Internal->CompileFeaturesEntries, ";");
      return output.c_str();
      }
    else if(prop == propCOMPILE_OPTIONS)
      {
      if (this->Internal->CompileOptionsEntries.empty())
        {
        return 0;
        }

      static std::string output;
      output = cmJoin(this->Internal->CompileOptionsEntries, ";");
      return output.c_str();
      }
    else if(prop == propCOMPILE_DEFINITIONS)
      {
      if (this->Internal->CompileDefinitionsEntries.empty())
        {
        return 0;
        }

      static std::string output;
      output = cmJoin(this->Internal->CompileDefinitionsEntries, ";");
      return output.c_str();
      }
    else if (prop == propIMPORTED)
      {
      return this->IsImported()?"TRUE":"FALSE";
      }
    else if (prop == propNAME)
      {
      return this->GetName().c_str();
      }
    else if (prop == propBINARY_DIR)
      {
      return this->GetMakefile()->GetCurrentBinaryDirectory();
      }
    else if (prop == propSOURCE_DIR)
      {
      return this->GetMakefile()->GetCurrentSourceDirectory();
      }
    else if(prop == propSOURCES)
      {
      if (this->Internal->SourceEntries.empty())
        {
        return 0;
        }

      std::ostringstream ss;
      const char* sep = "";
      typedef cmTargetInternals::TargetPropertyEntry
                                  TargetPropertyEntry;
      for(std::vector<TargetPropertyEntry*>::const_iterator
            i = this->Internal->SourceEntries.begin();
          i != this->Internal->SourceEntries.end(); ++i)
        {
        std::string entry = (*i)->ge->GetInput();

        std::vector<std::string> files;
        cmSystemTools::ExpandListArgument(entry, files);
        for (std::vector<std::string>::const_iterator
            li = files.begin(); li != files.end(); ++li)
          {
          if(cmHasLiteralPrefix(*li, "$<TARGET_OBJECTS:") &&
              (*li)[li->size() - 1] == '>')
            {
            std::string objLibName = li->substr(17, li->size()-18);

            if (cmGeneratorExpression::Find(objLibName) != std::string::npos)
              {
              ss << sep;
              sep = ";";
              ss << *li;
              continue;
              }

            bool addContent = false;
            bool noMessage = true;
            std::ostringstream e;
            cmake::MessageType messageType = cmake::AUTHOR_WARNING;
            switch(context->GetPolicyStatus(cmPolicies::CMP0051))
              {
              case cmPolicies::WARN:
                e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0051) << "\n";
                noMessage = false;
              case cmPolicies::OLD:
                break;
              case cmPolicies::REQUIRED_ALWAYS:
              case cmPolicies::REQUIRED_IF_USED:
              case cmPolicies::NEW:
                addContent = true;
              }
            if (!noMessage)
              {
              e << "Target \"" << this->Name << "\" contains "
              "$<TARGET_OBJECTS> generator expression in its sources list.  "
              "This content was not previously part of the SOURCES property "
              "when that property was read at configure time.  Code reading "
              "that property needs to be adapted to ignore the generator "
              "expression using the string(GENEX_STRIP) command.";
              context->IssueMessage(messageType, e.str());
              }
            if (addContent)
              {
              ss << sep;
              sep = ";";
              ss << *li;
              }
            }
          else if (cmGeneratorExpression::Find(*li) == std::string::npos)
            {
            ss << sep;
            sep = ";";
            ss << *li;
            }
          else
            {
            cmSourceFile *sf = this->Makefile->GetOrCreateSource(*li);
            // Construct what is known about this source file location.
            cmSourceFileLocation const& location = sf->GetLocation();
            std::string sname = location.GetDirectory();
            if(!sname.empty())
              {
              sname += "/";
              }
            sname += location.GetName();

            ss << sep;
            sep = ";";
            // Append this list entry.
            ss << sname;
            }
          }
        }
      this->Properties.SetProperty("SOURCES", ss.str().c_str());
      }
    }

  const char *retVal = this->Properties.GetPropertyValue(prop);
  if (!retVal)
    {
    const bool chain = this->GetMakefile()->GetState()->
                      IsPropertyChained(prop, cmProperty::TARGET);
    if (chain)
      {
      return this->Makefile->GetProperty(prop, chain);
      }
    }
  return retVal;
}

//----------------------------------------------------------------------------
bool cmTarget::GetPropertyAsBool(const std::string& prop) const
{
  return cmSystemTools::IsOn(this->GetProperty(prop));
}

//----------------------------------------------------------------------------
const char* cmTarget::GetSuffixVariableInternal(bool implib) const
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
                // Android GUI application packages store the native
                // binary as a shared library.
              : (this->IsAndroid && this->GetPropertyAsBool("ANDROID_GUI")?
                 "CMAKE_SHARED_LIBRARY_SUFFIX" : "CMAKE_EXECUTABLE_SUFFIX"));
    default:
      break;
    }
  return "";
}


//----------------------------------------------------------------------------
const char* cmTarget::GetPrefixVariableInternal(bool implib) const
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
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_PREFIX"
                // Android GUI application packages store the native
                // binary as a shared library.
              : (this->IsAndroid && this->GetPropertyAsBool("ANDROID_GUI")?
                 "CMAKE_SHARED_LIBRARY_PREFIX" : ""));
    default:
      break;
    }
  return "";
}

//----------------------------------------------------------------------------
bool cmTarget::HasMacOSXRpathInstallNameDir(const std::string& config) const
{
  bool install_name_is_rpath = false;
  bool macosx_rpath = false;

  if(!this->IsImportedTarget)
    {
    if(this->GetType() != cmTarget::SHARED_LIBRARY)
      {
      return false;
      }
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
    if(!install_name_is_rpath)
      {
      macosx_rpath = this->MacOSXRpathInstallNameDirDefault();
      }
    }
  else
    {
    // Lookup the imported soname.
    if(cmTarget::ImportInfo const* info = this->GetImportInfo(config))
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
    std::ostringstream w;
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
bool cmTarget::MacOSXRpathInstallNameDirDefault() const
{
  // we can't do rpaths when unsupported
  if(!this->Makefile->IsSet("CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG"))
    {
    return false;
    }

  const char* macosx_rpath_str = this->GetProperty("MACOSX_RPATH");
  if(macosx_rpath_str)
    {
    return this->GetPropertyAsBool("MACOSX_RPATH");
    }

  cmPolicies::PolicyStatus cmp0042 = this->GetPolicyStatusCMP0042();

  if(cmp0042 == cmPolicies::WARN)
    {
    this->Makefile->GetGlobalGenerator()->
      AddCMP0042WarnTarget(this->GetName());
    }

  if(cmp0042 == cmPolicies::NEW)
    {
    return true;
    }

  return false;
}

//----------------------------------------------------------------------------
bool cmTarget::IsImportedSharedLibWithoutSOName(
                                          const std::string& config) const
{
  if(this->IsImported() && this->GetType() == cmTarget::SHARED_LIBRARY)
    {
    if(cmTarget::ImportInfo const* info = this->GetImportInfo(config))
      {
      return info->NoSOName;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
std::string
cmTarget::GetFullNameImported(const std::string& config, bool implib) const
{
  return cmSystemTools::GetFilenameName(
    this->ImportedGetFullPath(config, implib));
}

//----------------------------------------------------------------------------
std::string
cmTarget::ImportedGetFullPath(const std::string& config, bool implib) const
{
  std::string result;
  if(cmTarget::ImportInfo const* info = this->GetImportInfo(config))
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
void cmTarget::ComputeVersionedName(std::string& vName,
                                    std::string const& prefix,
                                    std::string const& base,
                                    std::string const& suffix,
                                    std::string const& name,
                                    const char* version) const
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
bool cmTarget::HasImplibGNUtoMS() const
{
  return this->HasImportLibrary() && this->GetPropertyAsBool("GNUtoMS");
}

//----------------------------------------------------------------------------
bool cmTarget::GetImplibGNUtoMS(std::string const& gnuName,
                                std::string& out, const char* newExt) const
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
void cmTarget::SetPropertyDefault(const std::string& property,
                                  const char* default_value)
{
  // Compute the name of the variable holding the default value.
  std::string var = "CMAKE_";
  var += property;

  if(const char* value = this->Makefile->GetDefinition(var))
    {
    this->SetProperty(property, value);
    }
  else if(default_value)
    {
    this->SetProperty(property, default_value);
    }
}

//----------------------------------------------------------------------------
bool cmTarget::HaveInstallTreeRPATH() const
{
  const char* install_rpath = this->GetProperty("INSTALL_RPATH");
  return (install_rpath && *install_rpath) &&
          !this->Makefile->IsOn("CMAKE_SKIP_INSTALL_RPATH");
}

//----------------------------------------------------------------------------
const char* cmTarget::GetOutputTargetType(bool implib) const
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
bool cmTarget::ComputeOutputDir(const std::string& config,
                                bool implib, std::string& out) const
{
  bool usesDefaultOutputDir = false;
  std::string conf = config;

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
  std::string configUpper = cmSystemTools::UpperCase(conf);
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
    cmGeneratorExpression ge;
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
      ge.Parse(config_outdir);
    out = cge->Evaluate(this->Makefile, config);

    // Skip per-configuration subdirectory.
    conf = "";
    }
  else if(const char* outdir = this->GetProperty(propertyName))
    {
    // Use the user-specified output directory.
    cmGeneratorExpression ge;
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
      ge.Parse(outdir);
    out = cge->Evaluate(this->Makefile, config);

    // Skip per-configuration subdirectory if the value contained a
    // generator expression.
    if (out != outdir)
      {
      conf = "";
      }
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
         (out, this->Makefile->GetCurrentBinaryDirectory()));

  // The generator may add the configuration's subdirectory.
  if(!conf.empty())
    {
    bool iosPlatform = this->Makefile->PlatformIsAppleIos();
    std::string suffix =
      usesDefaultOutputDir && iosPlatform ? "${EFFECTIVE_PLATFORM_NAME}" : "";
    this->Makefile->GetGlobalGenerator()->
      AppendDirectoryForConfig("/", conf, suffix, out);
    }

  return usesDefaultOutputDir;
}

//----------------------------------------------------------------------------
bool cmTarget::ComputePDBOutputDir(const std::string& kind,
                                   const std::string& config,
                                   std::string& out) const
{
  // Look for a target property defining the target output directory
  // based on the target type.
  const char* propertyName = 0;
  std::string propertyNameStr = kind;
  if(!propertyNameStr.empty())
    {
    propertyNameStr += "_OUTPUT_DIRECTORY";
    propertyName = propertyNameStr.c_str();
    }
  std::string conf = config;

  // Check for a per-configuration output directory target property.
  std::string configUpper = cmSystemTools::UpperCase(conf);
  const char* configProp = 0;
  std::string configPropStr = kind;
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
    conf = "";
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
         (out, this->Makefile->GetCurrentBinaryDirectory()));

  // The generator may add the configuration's subdirectory.
  if(!conf.empty())
    {
    this->Makefile->GetGlobalGenerator()->
      AppendDirectoryForConfig("/", conf, "", out);
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmTarget::UsesDefaultOutputDir(const std::string& config,
                                    bool implib) const
{
  std::string dir;
  return this->ComputeOutputDir(config, implib, dir);
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFrameworkVersion() const
{
  assert(this->GetType() != INTERFACE_LIBRARY);

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
const char* cmTarget::GetExportMacro() const
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
      this->ExportMacro = cmSystemTools::MakeCindentifier(in);
      }
    return this->ExportMacro.c_str();
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
bool cmTarget::IsNullImpliedByLinkLibraries(const std::string &p) const
{
  return this->LinkImplicitNullProperties.find(p)
      != this->LinkImplicitNullProperties.end();
}

//----------------------------------------------------------------------------
void
cmTarget::GetObjectLibrariesCMP0026(std::vector<cmTarget*>& objlibs) const
{
  // At configure-time, this method can be called as part of getting the
  // LOCATION property or to export() a file to be include()d.  However
  // there is no cmGeneratorTarget at configure-time, so search the SOURCES
  // for TARGET_OBJECTS instead for backwards compatibility with OLD
  // behavior of CMP0024 and CMP0026 only.
  typedef cmTargetInternals::TargetPropertyEntry
                              TargetPropertyEntry;
  for(std::vector<TargetPropertyEntry*>::const_iterator
        i = this->Internal->SourceEntries.begin();
      i != this->Internal->SourceEntries.end(); ++i)
    {
    std::string entry = (*i)->ge->GetInput();

    std::vector<std::string> files;
    cmSystemTools::ExpandListArgument(entry, files);
    for (std::vector<std::string>::const_iterator
        li = files.begin(); li != files.end(); ++li)
      {
      if(cmHasLiteralPrefix(*li, "$<TARGET_OBJECTS:") &&
          (*li)[li->size() - 1] == '>')
        {
        std::string objLibName = li->substr(17, li->size()-18);

        if (cmGeneratorExpression::Find(objLibName) != std::string::npos)
          {
          continue;
          }
        cmTarget *objLib = this->Makefile->FindTargetToUse(objLibName);
        if(objLib)
          {
          objlibs.push_back(objLib);
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
cmTarget::ImportInfo const*
cmTarget::GetImportInfo(const std::string& config) const
{
  // There is no imported information for non-imported targets.
  if(!this->IsImported())
    {
    return 0;
    }

  // Lookup/compute/cache the import information for this
  // configuration.
  std::string config_upper;
  if(!config.empty())
    {
    config_upper = cmSystemTools::UpperCase(config);
    }
  else
    {
    config_upper = "NOCONFIG";
    }
  typedef cmTargetInternals::ImportInfoMapType ImportInfoMapType;

  ImportInfoMapType::const_iterator i =
    this->Internal->ImportInfoMap.find(config_upper);
  if(i == this->Internal->ImportInfoMap.end())
    {
    ImportInfo info;
    this->ComputeImportInfo(config_upper, info);
    ImportInfoMapType::value_type entry(config_upper, info);
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
                               std::string& suffix) const
{
  if (this->GetType() == INTERFACE_LIBRARY)
    {
    // This method attempts to find a config-specific LOCATION for the
    // IMPORTED library. In the case of INTERFACE_LIBRARY, there is no
    // LOCATION at all, so leaving *loc and *imp unchanged is the appropriate
    // and valid response.
    return true;
    }

  // Track the configuration-specific property suffix.
  suffix = "_";
  suffix += desired_config;

  std::vector<std::string> mappedConfigs;
  {
  std::string mapProp = "MAP_IMPORTED_CONFIG_";
  mapProp += desired_config;
  if(const char* mapValue = this->GetProperty(mapProp))
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
    std::string mcUpper = cmSystemTools::UpperCase(*mci);
    std::string locProp = "IMPORTED_LOCATION_";
    locProp += mcUpper;
    *loc = this->GetProperty(locProp);
    if(allowImp)
      {
      std::string impProp = "IMPORTED_IMPLIB_";
      impProp += mcUpper;
      *imp = this->GetProperty(impProp);
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
    *loc = this->GetProperty(locProp);
    if(allowImp)
      {
      std::string impProp = "IMPORTED_IMPLIB";
      impProp += suffix;
      *imp = this->GetProperty(impProp);
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
      *loc = this->GetProperty(locProp);
      if(allowImp)
        {
        std::string impProp = "IMPORTED_IMPLIB";
        impProp += suffix;
        *imp = this->GetProperty(impProp);
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
                                 ImportInfo& info) const
{
  // This method finds information about an imported target from its
  // properties.  The "IMPORTED_" namespace is reserved for properties
  // defined by the project exporting the target.

  // Initialize members.
  info.NoSOName = false;

  const char* loc = 0;
  const char* imp = 0;
  std::string suffix;
  if (!this->GetMappedConfig(desired_config, &loc, &imp, suffix))
    {
    return;
    }

  // Get the link interface.
  {
  std::string linkProp = "INTERFACE_LINK_LIBRARIES";
  const char *propertyLibs = this->GetProperty(linkProp);

  if (this->GetType() != INTERFACE_LIBRARY)
    {
    if(!propertyLibs)
      {
      linkProp = "IMPORTED_LINK_INTERFACE_LIBRARIES";
      linkProp += suffix;
      propertyLibs = this->GetProperty(linkProp);
      }

    if(!propertyLibs)
      {
      linkProp = "IMPORTED_LINK_INTERFACE_LIBRARIES";
      propertyLibs = this->GetProperty(linkProp);
      }
    }
  if(propertyLibs)
    {
    info.LibrariesProp = linkProp;
    info.Libraries = propertyLibs;
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
    if(const char* config_location = this->GetProperty(impProp))
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
    if(const char* config_soname = this->GetProperty(soProp))
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
    if(const char* config_no_soname = this->GetProperty(soProp))
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
    if(const char* config_implib = this->GetProperty(impProp))
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
  if(const char* config_libs = this->GetProperty(linkProp))
    {
    info.SharedDeps = config_libs;
    }
  else if(const char* libs =
          this->GetProperty("IMPORTED_LINK_DEPENDENT_LIBRARIES"))
    {
    info.SharedDeps = libs;
    }
  }

  // Get the link languages.
  if(this->LinkLanguagePropagatesToDependents())
    {
    std::string linkProp = "IMPORTED_LINK_INTERFACE_LANGUAGES";
    linkProp += suffix;
    if(const char* config_libs = this->GetProperty(linkProp))
      {
      info.Languages = config_libs;
      }
    else if(const char* libs =
            this->GetProperty("IMPORTED_LINK_INTERFACE_LANGUAGES"))
      {
      info.Languages = libs;
      }
    }

  // Get the cyclic repetition count.
  if(this->GetType() == cmTarget::STATIC_LIBRARY)
    {
    std::string linkProp = "IMPORTED_LINK_INTERFACE_MULTIPLICITY";
    linkProp += suffix;
    if(const char* config_reps = this->GetProperty(linkProp))
      {
      sscanf(config_reps, "%u", &info.Multiplicity);
      }
    else if(const char* reps =
            this->GetProperty("IMPORTED_LINK_INTERFACE_MULTIPLICITY"))
      {
      sscanf(reps, "%u", &info.Multiplicity);
      }
    }
}

//----------------------------------------------------------------------------
void cmTargetInternals::AddInterfaceEntries(
  cmTarget const* thisTarget, std::string const& config,
  std::string const& prop, std::vector<TargetPropertyEntry*>& entries)
{
  if(cmLinkImplementationLibraries const* impl =
     thisTarget->GetLinkImplementationLibraries(config))
    {
    for (std::vector<cmLinkImplItem>::const_iterator
           it = impl->Libraries.begin(), end = impl->Libraries.end();
         it != end; ++it)
      {
      if(it->Target)
        {
        std::string genex =
          "$<TARGET_PROPERTY:" + *it + "," + prop + ">";
        cmGeneratorExpression ge(it->Backtrace);
        cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(genex);
        cge->SetEvaluateForBuildsystem(true);
        entries.push_back(
          new cmTargetInternals::TargetPropertyEntry(cge, *it));
        }
      }
    }
}

cmOptionalLinkImplementation&
cmTarget::GetLinkImplMap(std::string const& config) const
{
  // Populate the link implementation for this configuration.
  std::string CONFIG = cmSystemTools::UpperCase(config);
  return Internal->LinkImplMap[CONFIG][this];
}

//----------------------------------------------------------------------------
cmLinkImplementationLibraries const*
cmTarget::GetLinkImplementationLibraries(const std::string& config) const
{
  return this->GetLinkImplementationLibrariesInternal(config, this);
}

//----------------------------------------------------------------------------
cmLinkImplementationLibraries const*
cmTarget::GetLinkImplementationLibrariesInternal(const std::string& config,
                                                 cmTarget const* head) const
{
  // There is no link implementation for imported targets.
  if(this->IsImported())
    {
    return 0;
    }

  // Populate the link implementation libraries for this configuration.
  std::string CONFIG = cmSystemTools::UpperCase(config);
  cmTargetInternals::HeadToLinkImplementationMap& hm =
    this->Internal->LinkImplMap[CONFIG];

  // If the link implementation does not depend on the head target
  // then return the one we computed first.
  if(!hm.empty() && !hm.begin()->second.HadHeadSensitiveCondition)
    {
    return &hm.begin()->second;
    }

  cmOptionalLinkImplementation& impl = hm[head];
  if(!impl.LibrariesDone)
    {
    impl.LibrariesDone = true;
    this->ComputeLinkImplementationLibraries(config, impl, head);
    }
  return &impl;
}

//----------------------------------------------------------------------------
void cmTarget::ComputeLinkImplementationLibraries(
  const std::string& config,
  cmOptionalLinkImplementation& impl,
  cmTarget const* head) const
{
  // Collect libraries directly linked in this configuration.
  for (std::vector<cmValueWithOrigin>::const_iterator
      le = this->Internal->LinkImplementationPropertyEntries.begin(),
      end = this->Internal->LinkImplementationPropertyEntries.end();
      le != end; ++le)
    {
    std::vector<std::string> llibs;
    cmGeneratorExpressionDAGChecker dagChecker(
                                        this->GetName(),
                                        "LINK_LIBRARIES", 0, 0);
    cmGeneratorExpression ge(le->Backtrace);
    cmsys::auto_ptr<cmCompiledGeneratorExpression> const cge =
      ge.Parse(le->Value);
    std::string const evaluated =
      cge->Evaluate(this->Makefile, config, false, head, &dagChecker);
    cmSystemTools::ExpandListArgument(evaluated, llibs);
    if(cge->GetHadHeadSensitiveCondition())
      {
      impl.HadHeadSensitiveCondition = true;
      }

    for(std::vector<std::string>::const_iterator li = llibs.begin();
        li != llibs.end(); ++li)
      {
      // Skip entries that resolve to the target itself or are empty.
      std::string name = this->CheckCMP0004(*li);
      if(name == this->GetName() || name.empty())
        {
        if(name == this->GetName())
          {
          bool noMessage = false;
          cmake::MessageType messageType = cmake::FATAL_ERROR;
          std::ostringstream e;
          switch(this->GetPolicyStatusCMP0038())
            {
            case cmPolicies::WARN:
              {
              e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0038) << "\n";
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

          if(!noMessage)
            {
            e << "Target \"" << this->GetName() << "\" links to itself.";
            this->Makefile->GetCMakeInstance()->IssueMessage(
              messageType, e.str(), this->GetBacktrace());
            if (messageType == cmake::FATAL_ERROR)
              {
              return;
              }
            }
          }
        continue;
        }

      // The entry is meant for this configuration.
      impl.Libraries.push_back(
        cmLinkImplItem(name, this->FindTargetToLink(name),
                       le->Backtrace, evaluated != le->Value));
      }

    std::set<std::string> const& seenProps = cge->GetSeenTargetProperties();
    for (std::set<std::string>::const_iterator it = seenProps.begin();
        it != seenProps.end(); ++it)
      {
      if (!this->GetProperty(*it))
        {
        this->LinkImplicitNullProperties.insert(*it);
        }
      }
    cge->GetMaxLanguageStandard(this, this->MaxLanguageStandards);
    }

  cmTarget::LinkLibraryType linkType = this->ComputeLinkType(config);
  cmTarget::LinkLibraryVectorType const& oldllibs =
    this->GetOriginalLinkLibraries();
  for(cmTarget::LinkLibraryVectorType::const_iterator li = oldllibs.begin();
      li != oldllibs.end(); ++li)
    {
    if(li->second != cmTarget::GENERAL && li->second != linkType)
      {
      std::string name = this->CheckCMP0004(li->first);
      if(name == this->GetName() || name.empty())
        {
        continue;
        }
      // Support OLD behavior for CMP0003.
      impl.WrongConfigLibraries.push_back(
        cmLinkItem(name, this->FindTargetToLink(name)));
      }
    }
}

//----------------------------------------------------------------------------
cmTarget const* cmTarget::FindTargetToLink(std::string const& name) const
{
  cmTarget const* tgt = this->Makefile->FindTargetToUse(name);

  // Skip targets that will not really be linked.  This is probably a
  // name conflict between an external library and an executable
  // within the project.
  if(tgt && tgt->GetType() == cmTarget::EXECUTABLE &&
     !tgt->IsExecutableWithExports())
    {
    tgt = 0;
    }

  if(tgt && tgt->GetType() == cmTarget::OBJECT_LIBRARY)
    {
    std::ostringstream e;
    e << "Target \"" << this->GetName() << "\" links to "
      "OBJECT library \"" << tgt->GetName() << "\" but this is not "
      "allowed.  "
      "One may link only to STATIC or SHARED libraries, or to executables "
      "with the ENABLE_EXPORTS property set.";
    cmake* cm = this->Makefile->GetCMakeInstance();
    cm->IssueMessage(cmake::FATAL_ERROR, e.str(), this->GetBacktrace());
    tgt = 0;
    }

  // Return the target found, if any.
  return tgt;
}

//----------------------------------------------------------------------------
std::string cmTarget::CheckCMP0004(std::string const& item) const
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
    switch(this->GetPolicyStatusCMP0004())
      {
      case cmPolicies::WARN:
        {
        std::ostringstream w;
        w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0004) << "\n"
          << "Target \"" << this->GetName() << "\" links to item \""
          << item << "\" which has leading or trailing whitespace.";
        cm->IssueMessage(cmake::AUTHOR_WARNING, w.str(),
                         this->GetBacktrace());
        }
      case cmPolicies::OLD:
        break;
      case cmPolicies::NEW:
        {
        std::ostringstream e;
        e << "Target \"" << this->GetName() << "\" links to item \""
          << item << "\" which has leading or trailing whitespace.  "
          << "This is now an error according to policy CMP0004.";
        cm->IssueMessage(cmake::FATAL_ERROR, e.str(), this->GetBacktrace());
        }
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        {
        std::ostringstream e;
        e << cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0004) << "\n"
          << "Target \"" << this->GetName() << "\" links to item \""
          << item << "\" which has leading or trailing whitespace.";
        cm->IssueMessage(cmake::FATAL_ERROR, e.str(), this->GetBacktrace());
        }
        break;
      }
    }
  return lib;
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
  cmDeleteAll(this->Pointer->SourceEntries);
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
