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

//----------------------------------------------------------------------------
class cmTargetInternals
{
public:
  std::vector<std::string> IncludeDirectoriesEntries;
  std::vector<cmListFileBacktrace> IncludeDirectoriesBacktraces;
  std::vector<std::string> CompileOptionsEntries;
  std::vector<cmListFileBacktrace> CompileOptionsBacktraces;
  std::vector<std::string> CompileFeaturesEntries;
  std::vector<cmListFileBacktrace> CompileFeaturesBacktraces;
  std::vector<std::string> CompileDefinitionsEntries;
  std::vector<cmListFileBacktrace> CompileDefinitionsBacktraces;
  std::vector<std::string> SourceEntries;
  std::vector<cmListFileBacktrace> SourceBacktraces;
  std::vector<std::string> LinkImplementationPropertyEntries;
  std::vector<cmListFileBacktrace> LinkImplementationPropertyBacktraces;
};

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
  this->IsImportedTarget = false;
  this->ImportedGloballyVisible = false;
  this->BuildInterfaceIncludesAppended = false;
}

void cmTarget::SetType(cmState::TargetType type, const std::string& name)
{
  this->Name = name;
  // only add dependency information for library targets
  this->TargetTypeValue = type;
  if(this->TargetTypeValue >= cmState::STATIC_LIBRARY
     && this->TargetTypeValue <= cmState::MODULE_LIBRARY)
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

  // Setup default property values.
  if (this->GetType() != cmState::INTERFACE_LIBRARY
      && this->GetType() != cmState::UTILITY)
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
    this->SetPropertyDefault("IOS_INSTALL_COMBINED", 0);
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
  if (this->GetType() != cmState::UTILITY)
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
        if (this->TargetTypeValue == cmState::INTERFACE_LIBRARY
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
      if(this->TargetTypeValue != cmState::EXECUTABLE
          && this->TargetTypeValue != cmState::INTERFACE_LIBRARY)
        {
        std::string property = cmSystemTools::UpperCase(*ci);
        property += "_POSTFIX";
        this->SetPropertyDefault(property, 0);
        }
      }
    }

  // Save the backtrace of target construction.
  this->Backtrace = this->Makefile->GetBacktrace();

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

  if (this->GetType() != cmState::INTERFACE_LIBRARY
      && this->GetType() != cmState::UTILITY)
    {
    this->SetPropertyDefault("C_VISIBILITY_PRESET", 0);
    this->SetPropertyDefault("CXX_VISIBILITY_PRESET", 0);
    this->SetPropertyDefault("VISIBILITY_INLINES_HIDDEN", 0);
    }

  if(this->TargetTypeValue == cmState::EXECUTABLE)
    {
    this->SetPropertyDefault("ANDROID_GUI", 0);
    this->SetPropertyDefault("CROSSCOMPILING_EMULATOR", 0);
    this->SetPropertyDefault("ENABLE_EXPORTS", 0);
    }
  if(this->TargetTypeValue == cmState::SHARED_LIBRARY
      || this->TargetTypeValue == cmState::MODULE_LIBRARY)
    {
    this->SetProperty("POSITION_INDEPENDENT_CODE", "True");
    }
  if(this->TargetTypeValue == cmState::SHARED_LIBRARY)
    {
    this->SetPropertyDefault("WINDOWS_EXPORT_ALL_SYMBOLS", 0);
    }

  if (this->GetType() != cmState::INTERFACE_LIBRARY
      && this->GetType() != cmState::UTILITY)
    {
    this->SetPropertyDefault("POSITION_INDEPENDENT_CODE", 0);
    }

  // Record current policies for later use.
  this->Makefile->RecordPolicies(this->PolicyMap);

  if (this->TargetTypeValue == cmState::INTERFACE_LIBRARY)
    {
    // This policy is checked in a few conditions. The properties relevant
    // to the policy are always ignored for cmState::INTERFACE_LIBRARY targets,
    // so ensure that the conditions don't lead to nonsense.
    this->PolicyMap.Set(cmPolicies::CMP0022, cmPolicies::NEW);
    }

  if (this->GetType() != cmState::INTERFACE_LIBRARY
      && this->GetType() != cmState::UTILITY)
    {
    this->SetPropertyDefault("JOB_POOL_COMPILE", 0);
    this->SetPropertyDefault("JOB_POOL_LINK", 0);
    }
}

//----------------------------------------------------------------------------
void cmTarget::AddUtility(const std::string& u, cmMakefile *makefile)
{
  if(this->Utilities.insert(u).second && makefile)
    {
    this->UtilityBacktraces.insert(
            std::make_pair(u, makefile->GetBacktrace()));
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
cmListFileBacktrace const& cmTarget::GetBacktrace() const
{
  return this->Backtrace;
}

//----------------------------------------------------------------------------
bool cmTarget::IsExecutableWithExports() const
{
  return (this->GetType() == cmState::EXECUTABLE &&
          this->GetPropertyAsBool("ENABLE_EXPORTS"));
}

//----------------------------------------------------------------------------
bool cmTarget::HasImportLibrary() const
{
  return (this->DLLPlatform &&
          (this->GetType() == cmState::SHARED_LIBRARY ||
           this->IsExecutableWithExports()));
}

//----------------------------------------------------------------------------
bool cmTarget::IsFrameworkOnApple() const
{
  return (this->GetType() == cmState::SHARED_LIBRARY &&
          this->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("FRAMEWORK"));
}

//----------------------------------------------------------------------------
bool cmTarget::IsAppBundleOnApple() const
{
  return (this->GetType() == cmState::EXECUTABLE &&
          this->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("MACOSX_BUNDLE"));
}

//----------------------------------------------------------------------------
void cmTarget::AddTracedSources(std::vector<std::string> const& srcs)
{
  if (!srcs.empty())
    {
    cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
    this->Internal->SourceEntries.push_back(cmJoin(srcs, ";"));
    this->Internal->SourceBacktraces.push_back(lfbt);
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
    cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
    this->Internal->SourceEntries.push_back(srcFiles);
    this->Internal->SourceBacktraces.push_back(lfbt);
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

  bool operator()(std::string const& entry)
  {
    std::vector<std::string> files;
    cmSystemTools::ExpandListArgument(entry, files);
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
    cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
    this->Internal->SourceEntries.push_back(src);
    this->Internal->SourceBacktraces.push_back(lfbt);
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
std::string cmTarget::GetDebugGeneratorExpressions(const std::string &value,
                                  cmTargetLinkLibraryType llt) const
{
  if (llt == GENERAL_LibraryType)
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

  if (llt == OPTIMIZED_LibraryType)
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
                              cmTargetLinkLibraryType llt)
{
  cmTarget *tgt = this->Makefile->FindTargetToUse(lib);
  {
  const bool isNonImportedTarget = tgt && !tgt->IsImported();

  const std::string libName =
      (isNonImportedTarget && llt != GENERAL_LibraryType)
      ? targetNameGenex(lib)
      : lib;
  this->AppendProperty("LINK_LIBRARIES",
                       this->GetDebugGeneratorExpressions(libName,
                                                          llt).c_str());
  }

  if (cmGeneratorExpression::Find(lib) != std::string::npos
      || (tgt && tgt->GetType() == cmState::INTERFACE_LIBRARY)
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
      case GENERAL_LibraryType:
        dependencies += "general";
        break;
      case DEBUG_LibraryType:
        dependencies += "debug";
        break;
      case OPTIMIZED_LibraryType:
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

cmStringRange cmTarget::GetSourceEntries() const
{
  return cmMakeRange(this->Internal->SourceEntries);
}

cmBacktraceRange cmTarget::GetSourceBacktraces() const
{
  return cmMakeRange(this->Internal->SourceBacktraces);
}

cmStringRange cmTarget::GetLinkImplementationEntries() const
{
  return cmMakeRange(this->Internal->LinkImplementationPropertyEntries);
}

cmBacktraceRange cmTarget::GetLinkImplementationBacktraces() const
{
  return cmMakeRange(this->Internal->LinkImplementationPropertyBacktraces);
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
    cmTargetLinkLibraryType llt = GENERAL_LibraryType;
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
          llt = DEBUG_LibraryType;
          }
        else if (l == "optimized")
          {
          llt = OPTIMIZED_LibraryType;
          }
        else if (l == "general")
          {
          llt = GENERAL_LibraryType;
          }
        else
          {
          LibraryID lib2(l,llt);
          this->InsertDependencyForVS6( dep_map, lib, lib2);
          this->GatherDependenciesForVS6( mf, lib2, dep_map);
          llt = GENERAL_LibraryType;
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
  if (this->GetType() == cmState::INTERFACE_LIBRARY
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
    this->Internal->LinkImplementationPropertyBacktraces.clear();
    if (value)
      {
      cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
      this->Internal->LinkImplementationPropertyEntries.push_back(value);
      this->Internal->LinkImplementationPropertyBacktraces.push_back(lfbt);
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

    this->Internal->SourceEntries.clear();
    this->Internal->SourceBacktraces.clear();
    if (value)
      {
      cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
      this->Internal->SourceEntries.push_back(value);
      this->Internal->SourceBacktraces.push_back(lfbt);
      }
    }
  else
    {
    this->Properties.SetProperty(prop, value);
    }
}

//----------------------------------------------------------------------------
void cmTarget::AppendProperty(const std::string& prop, const char* value,
                              bool asString)
{
  if (this->GetType() == cmState::INTERFACE_LIBRARY
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
      this->Internal->LinkImplementationPropertyEntries.push_back(value);
      this->Internal->LinkImplementationPropertyBacktraces.push_back(lfbt);
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
    cmListFileBacktrace lfbt = this->Makefile->GetBacktrace();
    this->Internal->SourceEntries.push_back(value);
    this->Internal->SourceBacktraces.push_back(lfbt);
    }
  else
    {
    this->Properties.AppendProperty(prop, value, asString);
    }
}

//----------------------------------------------------------------------------
void cmTarget::AppendBuildInterfaceIncludes()
{
  if(this->GetType() != cmState::SHARED_LIBRARY &&
     this->GetType() != cmState::STATIC_LIBRARY &&
     this->GetType() != cmState::MODULE_LIBRARY &&
     this->GetType() != cmState::INTERFACE_LIBRARY &&
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
void cmTarget::MarkAsImported(bool global)
{
  this->IsImportedTarget = true;
  this->ImportedGloballyVisible = global;
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
  if (this->GetType() == cmState::INTERFACE_LIBRARY
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
  if(this->GetType() == cmState::EXECUTABLE ||
     this->GetType() == cmState::STATIC_LIBRARY ||
     this->GetType() == cmState::SHARED_LIBRARY ||
     this->GetType() == cmState::MODULE_LIBRARY ||
     this->GetType() == cmState::UNKNOWN_LIBRARY)
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
        cmGeneratorTarget* gt = gg->FindGeneratorTarget(this->GetName());
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
        cmGeneratorTarget* gt = gg->FindGeneratorTarget(this->GetName());
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
          cmGeneratorTarget* gt = gg->FindGeneratorTarget(this->GetName());
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
      output = cmJoin(this->Internal->LinkImplementationPropertyEntries, ";");
      return output.c_str();
      }
    // the type property returns what type the target is
    else if (prop == propTYPE)
      {
      return cmState::GetTargetTypeName(this->GetType());
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
      for(std::vector<std::string>::const_iterator
            i = this->Internal->SourceEntries.begin();
          i != this->Internal->SourceEntries.end(); ++i)
        {
        std::string const& entry = *i;

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
    case cmState::STATIC_LIBRARY:
      return "CMAKE_STATIC_LIBRARY_SUFFIX";
    case cmState::SHARED_LIBRARY:
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_SUFFIX"
              : "CMAKE_SHARED_LIBRARY_SUFFIX");
    case cmState::MODULE_LIBRARY:
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_SUFFIX"
              : "CMAKE_SHARED_MODULE_SUFFIX");
    case cmState::EXECUTABLE:
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
    case cmState::STATIC_LIBRARY:
      return "CMAKE_STATIC_LIBRARY_PREFIX";
    case cmState::SHARED_LIBRARY:
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_PREFIX"
              : "CMAKE_SHARED_LIBRARY_PREFIX");
    case cmState::MODULE_LIBRARY:
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_PREFIX"
              : "CMAKE_SHARED_MODULE_PREFIX");
    case cmState::EXECUTABLE:
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
std::string
cmTarget::ImportedGetFullPath(const std::string& config, bool pimplib) const
{
  assert(this->IsImported());

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

  std::string result;

  const char* loc = 0;
  const char* imp = 0;
  std::string suffix;

  if(this->GetType() != cmState::INTERFACE_LIBRARY
     && this->GetMappedConfig(config_upper, &loc, &imp, suffix))
    {
    if (!pimplib)
      {
      if(loc)
        {
        result = loc;
        }
      else
        {
        std::string impProp = "IMPORTED_LOCATION";
        impProp += suffix;
        if(const char* config_location = this->GetProperty(impProp))
          {
          result = config_location;
          }
        else if(const char* location =
                this->GetProperty("IMPORTED_LOCATION"))
          {
          result = location;
          }
        }
      }
    else
      {
      if(imp)
        {
        result = imp;
        }
      else if(this->GetType() == cmState::SHARED_LIBRARY ||
              this->IsExecutableWithExports())
        {
        std::string impProp = "IMPORTED_IMPLIB";
        impProp += suffix;
        if(const char* config_implib = this->GetProperty(impProp))
          {
          result = config_implib;
          }
        else if(const char* implib = this->GetProperty("IMPORTED_IMPLIB"))
          {
          result = implib;
          }
        }
      }
    }

  if(result.empty())
    {
    result = this->GetName();
    result += "-NOTFOUND";
    }
  return result;
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

bool cmTarget::GetMappedConfig(std::string const& desired_config,
                               const char** loc,
                               const char** imp,
                               std::string& suffix) const
{
  if (this->GetType() == cmState::INTERFACE_LIBRARY)
    {
    // This method attempts to find a config-specific LOCATION for the
    // IMPORTED library. In the case of cmState::INTERFACE_LIBRARY, there is no
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
