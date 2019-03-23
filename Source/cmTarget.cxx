/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTarget.h"

#include "cmsys/RegularExpression.hxx"
#include <algorithm>
#include <assert.h>
#include <initializer_list>
#include <iterator>
#include <set>
#include <sstream>
#include <string.h>
#include <unordered_set>

#include "cmAlgorithms.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmMessenger.h"
#include "cmProperty.h"
#include "cmPropertyMap.h"
#include "cmRange.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmSourceFileLocationKind.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmSystemTools.h"
#include "cmTargetPropertyComputer.h"
#include "cmake.h"

///! Append all elements from the second container to the first container
template <class C, class R>
static inline void CApp(C& container, R const& range)
{
  container.insert(container.end(), range.begin(), range.end());
}

template <>
const char* cmTargetPropertyComputer::ComputeLocationForBuild<cmTarget>(
  cmTarget const* tgt)
{
  static std::string loc;
  if (tgt->IsImported()) {
    loc = tgt->ImportedGetFullPath("", cmStateEnums::RuntimeBinaryArtifact);
    return loc.c_str();
  }

  cmGlobalGenerator* gg = tgt->GetGlobalGenerator();
  if (!gg->GetConfigureDoneCMP0026()) {
    gg->CreateGenerationObjects();
  }
  cmGeneratorTarget* gt = gg->FindGeneratorTarget(tgt->GetName());
  loc = gt->GetLocationForBuild();
  return loc.c_str();
}

template <>
const char* cmTargetPropertyComputer::ComputeLocation<cmTarget>(
  cmTarget const* tgt, const std::string& config)
{
  static std::string loc;
  if (tgt->IsImported()) {
    loc =
      tgt->ImportedGetFullPath(config, cmStateEnums::RuntimeBinaryArtifact);
    return loc.c_str();
  }

  cmGlobalGenerator* gg = tgt->GetGlobalGenerator();
  if (!gg->GetConfigureDoneCMP0026()) {
    gg->CreateGenerationObjects();
  }
  cmGeneratorTarget* gt = gg->FindGeneratorTarget(tgt->GetName());
  loc = gt->GetFullPath(config, cmStateEnums::RuntimeBinaryArtifact);
  return loc.c_str();
}

template <>
const char* cmTargetPropertyComputer::GetSources<cmTarget>(
  cmTarget const* tgt, cmMessenger* messenger,
  cmListFileBacktrace const& context)
{
  cmStringRange entries = tgt->GetSourceEntries();
  if (entries.empty()) {
    return nullptr;
  }

  std::ostringstream ss;
  const char* sep = "";
  for (std::string const& entry : entries) {
    std::vector<std::string> files;
    cmSystemTools::ExpandListArgument(entry, files);
    for (std::string const& file : files) {
      if (cmHasLiteralPrefix(file, "$<TARGET_OBJECTS:") &&
          file.back() == '>') {
        std::string objLibName = file.substr(17, file.size() - 18);

        if (cmGeneratorExpression::Find(objLibName) != std::string::npos) {
          ss << sep;
          sep = ";";
          ss << file;
          continue;
        }

        bool addContent = false;
        bool noMessage = true;
        std::ostringstream e;
        MessageType messageType = MessageType::AUTHOR_WARNING;
        switch (context.GetBottom().GetPolicy(cmPolicies::CMP0051)) {
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
        if (!noMessage) {
          e << "Target \"" << tgt->GetName()
            << "\" contains $<TARGET_OBJECTS> generator expression in its "
               "sources list.  This content was not previously part of the "
               "SOURCES property when that property was read at configure "
               "time.  Code reading that property needs to be adapted to "
               "ignore the generator expression using the string(GENEX_STRIP) "
               "command.";
          messenger->IssueMessage(messageType, e.str(), context);
        }
        if (addContent) {
          ss << sep;
          sep = ";";
          ss << file;
        }
      } else if (cmGeneratorExpression::Find(file) == std::string::npos) {
        ss << sep;
        sep = ";";
        ss << file;
      } else {
        cmSourceFile* sf = tgt->GetMakefile()->GetOrCreateSource(file);
        // Construct what is known about this source file location.
        cmSourceFileLocation const& location = sf->GetLocation();
        std::string sname = location.GetDirectory();
        if (!sname.empty()) {
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
  static std::string srcs;
  srcs = ss.str();
  return srcs.c_str();
}

class cmTargetInternals
{
public:
  cmStateEnums::TargetType TargetType;
  cmMakefile* Makefile;
  cmPolicies::PolicyMap PolicyMap;
  cmPropertyMap Properties;
  std::set<BT<std::string>> Utilities;
  std::set<std::string> SystemIncludeDirectories;
  cmTarget::LinkLibraryVectorType OriginalLinkLibraries;
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
  std::vector<std::string> LinkOptionsEntries;
  std::vector<cmListFileBacktrace> LinkOptionsBacktraces;
  std::vector<std::string> LinkDirectoriesEntries;
  std::vector<cmListFileBacktrace> LinkDirectoriesBacktraces;
  std::vector<std::string> LinkImplementationPropertyEntries;
  std::vector<cmListFileBacktrace> LinkImplementationPropertyBacktraces;
};

cmTarget::cmTarget(std::string const& name, cmStateEnums::TargetType type,
                   Visibility vis, cmMakefile* mf)
{
  assert(mf);
  impl->TargetType = type;
  impl->Makefile = mf;
  this->IsGeneratorProvided = false;
  this->Name = name;
  this->HaveInstallRule = false;
  this->DLLPlatform = false;
  this->IsAndroid = false;
  this->IsImportedTarget =
    (vis == VisibilityImported || vis == VisibilityImportedGlobally);
  this->ImportedGloballyVisible = vis == VisibilityImportedGlobally;
  this->BuildInterfaceIncludesAppended = false;

  // Check whether this is a DLL platform.
  this->DLLPlatform =
    !impl->Makefile->GetSafeDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX").empty();

  // Check whether we are targeting an Android platform.
  this->IsAndroid =
    (impl->Makefile->GetSafeDefinition("CMAKE_SYSTEM_NAME") == "Android");

  // Setup default property values.
  if (this->GetType() != cmStateEnums::INTERFACE_LIBRARY &&
      this->GetType() != cmStateEnums::UTILITY) {
    this->SetPropertyDefault("ANDROID_API", nullptr);
    this->SetPropertyDefault("ANDROID_API_MIN", nullptr);
    this->SetPropertyDefault("ANDROID_ARCH", nullptr);
    this->SetPropertyDefault("ANDROID_STL_TYPE", nullptr);
    this->SetPropertyDefault("ANDROID_SKIP_ANT_STEP", nullptr);
    this->SetPropertyDefault("ANDROID_PROCESS_MAX", nullptr);
    this->SetPropertyDefault("ANDROID_PROGUARD", nullptr);
    this->SetPropertyDefault("ANDROID_PROGUARD_CONFIG_PATH", nullptr);
    this->SetPropertyDefault("ANDROID_SECURE_PROPS_PATH", nullptr);
    this->SetPropertyDefault("ANDROID_NATIVE_LIB_DIRECTORIES", nullptr);
    this->SetPropertyDefault("ANDROID_NATIVE_LIB_DEPENDENCIES", nullptr);
    this->SetPropertyDefault("ANDROID_JAVA_SOURCE_DIR", nullptr);
    this->SetPropertyDefault("ANDROID_JAR_DIRECTORIES", nullptr);
    this->SetPropertyDefault("ANDROID_JAR_DEPENDENCIES", nullptr);
    this->SetPropertyDefault("ANDROID_ASSETS_DIRECTORIES", nullptr);
    this->SetPropertyDefault("ANDROID_ANT_ADDITIONAL_OPTIONS", nullptr);
    this->SetPropertyDefault("BUILD_RPATH", nullptr);
    this->SetPropertyDefault("BUILD_RPATH_USE_ORIGIN", nullptr);
    this->SetPropertyDefault("INSTALL_NAME_DIR", nullptr);
    this->SetPropertyDefault("INSTALL_RPATH", "");
    this->SetPropertyDefault("INSTALL_RPATH_USE_LINK_PATH", "OFF");
    this->SetPropertyDefault("INTERPROCEDURAL_OPTIMIZATION", nullptr);
    this->SetPropertyDefault("SKIP_BUILD_RPATH", "OFF");
    this->SetPropertyDefault("BUILD_WITH_INSTALL_RPATH", "OFF");
    this->SetPropertyDefault("ARCHIVE_OUTPUT_DIRECTORY", nullptr);
    this->SetPropertyDefault("LIBRARY_OUTPUT_DIRECTORY", nullptr);
    this->SetPropertyDefault("RUNTIME_OUTPUT_DIRECTORY", nullptr);
    this->SetPropertyDefault("PDB_OUTPUT_DIRECTORY", nullptr);
    this->SetPropertyDefault("COMPILE_PDB_OUTPUT_DIRECTORY", nullptr);
    this->SetPropertyDefault("Fortran_FORMAT", nullptr);
    this->SetPropertyDefault("Fortran_MODULE_DIRECTORY", nullptr);
    this->SetPropertyDefault("Fortran_COMPILER_LAUNCHER", nullptr);
    this->SetPropertyDefault("GNUtoMS", nullptr);
    this->SetPropertyDefault("OSX_ARCHITECTURES", nullptr);
    this->SetPropertyDefault("IOS_INSTALL_COMBINED", nullptr);
    this->SetPropertyDefault("AUTOMOC", nullptr);
    this->SetPropertyDefault("AUTOUIC", nullptr);
    this->SetPropertyDefault("AUTORCC", nullptr);
    this->SetPropertyDefault("AUTOGEN_ORIGIN_DEPENDS", nullptr);
    this->SetPropertyDefault("AUTOGEN_PARALLEL", nullptr);
    this->SetPropertyDefault("AUTOMOC_COMPILER_PREDEFINES", nullptr);
    this->SetPropertyDefault("AUTOMOC_DEPEND_FILTERS", nullptr);
    this->SetPropertyDefault("AUTOMOC_MACRO_NAMES", nullptr);
    this->SetPropertyDefault("AUTOMOC_MOC_OPTIONS", nullptr);
    this->SetPropertyDefault("AUTOUIC_OPTIONS", nullptr);
    this->SetPropertyDefault("AUTOUIC_SEARCH_PATHS", nullptr);
    this->SetPropertyDefault("AUTORCC_OPTIONS", nullptr);
    this->SetPropertyDefault("LINK_DEPENDS_NO_SHARED", nullptr);
    this->SetPropertyDefault("LINK_INTERFACE_LIBRARIES", nullptr);
    this->SetPropertyDefault("WIN32_EXECUTABLE", nullptr);
    this->SetPropertyDefault("MACOSX_BUNDLE", nullptr);
    this->SetPropertyDefault("MACOSX_RPATH", nullptr);
    this->SetPropertyDefault("NO_SYSTEM_FROM_IMPORTED", nullptr);
    this->SetPropertyDefault("BUILD_WITH_INSTALL_NAME_DIR", nullptr);
    this->SetPropertyDefault("C_CLANG_TIDY", nullptr);
    this->SetPropertyDefault("C_COMPILER_LAUNCHER", nullptr);
    this->SetPropertyDefault("C_CPPLINT", nullptr);
    this->SetPropertyDefault("C_CPPCHECK", nullptr);
    this->SetPropertyDefault("C_INCLUDE_WHAT_YOU_USE", nullptr);
    this->SetPropertyDefault("LINK_WHAT_YOU_USE", nullptr);
    this->SetPropertyDefault("C_STANDARD", nullptr);
    this->SetPropertyDefault("C_STANDARD_REQUIRED", nullptr);
    this->SetPropertyDefault("C_EXTENSIONS", nullptr);
    this->SetPropertyDefault("CXX_CLANG_TIDY", nullptr);
    this->SetPropertyDefault("CXX_COMPILER_LAUNCHER", nullptr);
    this->SetPropertyDefault("CXX_CPPLINT", nullptr);
    this->SetPropertyDefault("CXX_CPPCHECK", nullptr);
    this->SetPropertyDefault("CXX_INCLUDE_WHAT_YOU_USE", nullptr);
    this->SetPropertyDefault("CXX_STANDARD", nullptr);
    this->SetPropertyDefault("CXX_STANDARD_REQUIRED", nullptr);
    this->SetPropertyDefault("CXX_EXTENSIONS", nullptr);
    this->SetPropertyDefault("CUDA_STANDARD", nullptr);
    this->SetPropertyDefault("CUDA_STANDARD_REQUIRED", nullptr);
    this->SetPropertyDefault("CUDA_EXTENSIONS", nullptr);
    this->SetPropertyDefault("CUDA_COMPILER_LAUNCHER", nullptr);
    this->SetPropertyDefault("CUDA_SEPARABLE_COMPILATION", nullptr);
    this->SetPropertyDefault("LINK_SEARCH_START_STATIC", nullptr);
    this->SetPropertyDefault("LINK_SEARCH_END_STATIC", nullptr);
    this->SetPropertyDefault("FOLDER", nullptr);
#ifdef __APPLE__
    if (this->GetGlobalGenerator()->IsXcode()) {
      this->SetPropertyDefault("XCODE_GENERATE_SCHEME", nullptr);
      this->SetPropertyDefault("XCODE_SCHEME_ADDRESS_SANITIZER", nullptr);
      this->SetPropertyDefault(
        "XCODE_SCHEME_ADDRESS_SANITIZER_USE_AFTER_RETURN", nullptr);
      this->SetPropertyDefault("XCODE_SCHEME_THREAD_SANITIZER", nullptr);
      this->SetPropertyDefault("XCODE_SCHEME_THREAD_SANITIZER_STOP", nullptr);
      this->SetPropertyDefault("XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER",
                               nullptr);
      this->SetPropertyDefault(
        "XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER_STOP", nullptr);
      this->SetPropertyDefault("XCODE_SCHEME_DISABLE_MAIN_THREAD_CHECKER",
                               nullptr);
      this->SetPropertyDefault("XCODE_SCHEME_MAIN_THREAD_CHECKER_STOP",
                               nullptr);
      this->SetPropertyDefault("XCODE_SCHEME_MALLOC_SCRIBBLE", nullptr);
      this->SetPropertyDefault("XCODE_SCHEME_MALLOC_GUARD_EDGES", nullptr);
      this->SetPropertyDefault("XCODE_SCHEME_GUARD_MALLOC", nullptr);
      this->SetPropertyDefault("XCODE_SCHEME_ZOMBIE_OBJECTS", nullptr);
      this->SetPropertyDefault("XCODE_SCHEME_MALLOC_STACK", nullptr);
      this->SetPropertyDefault("XCODE_SCHEME_DYNAMIC_LINKER_API_USAGE",
                               nullptr);
      this->SetPropertyDefault("XCODE_SCHEME_DYNAMIC_LIBRARY_LOADS", nullptr);
    }
#endif
  }

  // Setup per-configuration property default values.
  if (this->GetType() != cmStateEnums::UTILITY) {
    static const auto configProps = {
      /* clang-format needs this comment to break after the opening brace */
      "ARCHIVE_OUTPUT_DIRECTORY_",     "LIBRARY_OUTPUT_DIRECTORY_",
      "RUNTIME_OUTPUT_DIRECTORY_",     "PDB_OUTPUT_DIRECTORY_",
      "COMPILE_PDB_OUTPUT_DIRECTORY_", "MAP_IMPORTED_CONFIG_",
      "INTERPROCEDURAL_OPTIMIZATION_"
    };
    // Collect the set of configuration types.
    std::vector<std::string> configNames;
    mf->GetConfigurations(configNames);
    for (std::string const& configName : configNames) {
      std::string configUpper = cmSystemTools::UpperCase(configName);
      for (auto const& prop : configProps) {
        // Interface libraries have no output locations, so honor only
        // the configuration map.
        if (impl->TargetType == cmStateEnums::INTERFACE_LIBRARY &&
            strcmp(prop, "MAP_IMPORTED_CONFIG_") != 0) {
          continue;
        }
        std::string property = prop;
        property += configUpper;
        this->SetPropertyDefault(property, nullptr);
      }

      // Initialize per-configuration name postfix property from the
      // variable only for non-executable targets.  This preserves
      // compatibility with previous CMake versions in which executables
      // did not support this variable.  Projects may still specify the
      // property directly.
      if (impl->TargetType != cmStateEnums::EXECUTABLE &&
          impl->TargetType != cmStateEnums::INTERFACE_LIBRARY) {
        std::string property = cmSystemTools::UpperCase(configName);
        property += "_POSTFIX";
        this->SetPropertyDefault(property, nullptr);
      }
    }
  }

  // Save the backtrace of target construction.
  this->Backtrace = impl->Makefile->GetBacktrace();

  if (!this->IsImported()) {
    // Initialize the INCLUDE_DIRECTORIES property based on the current value
    // of the same directory property:
    CApp(impl->IncludeDirectoriesEntries,
         impl->Makefile->GetIncludeDirectoriesEntries());
    CApp(impl->IncludeDirectoriesBacktraces,
         impl->Makefile->GetIncludeDirectoriesBacktraces());

    {
      auto const& sysInc = impl->Makefile->GetSystemIncludeDirectories();
      impl->SystemIncludeDirectories.insert(sysInc.begin(), sysInc.end());
    }

    CApp(impl->CompileOptionsEntries,
         impl->Makefile->GetCompileOptionsEntries());
    CApp(impl->CompileOptionsBacktraces,
         impl->Makefile->GetCompileOptionsBacktraces());

    CApp(impl->LinkOptionsEntries, impl->Makefile->GetLinkOptionsEntries());
    CApp(impl->LinkOptionsBacktraces,
         impl->Makefile->GetLinkOptionsBacktraces());

    CApp(impl->LinkDirectoriesEntries,
         impl->Makefile->GetLinkDirectoriesEntries());
    CApp(impl->LinkDirectoriesBacktraces,
         impl->Makefile->GetLinkDirectoriesBacktraces());
  }

  if (this->GetType() != cmStateEnums::INTERFACE_LIBRARY &&
      this->GetType() != cmStateEnums::UTILITY) {
    this->SetPropertyDefault("C_VISIBILITY_PRESET", nullptr);
    this->SetPropertyDefault("CXX_VISIBILITY_PRESET", nullptr);
    this->SetPropertyDefault("CUDA_VISIBILITY_PRESET", nullptr);
    this->SetPropertyDefault("VISIBILITY_INLINES_HIDDEN", nullptr);
  }

  if (impl->TargetType == cmStateEnums::EXECUTABLE) {
    this->SetPropertyDefault("ANDROID_GUI", nullptr);
    this->SetPropertyDefault("CROSSCOMPILING_EMULATOR", nullptr);
    this->SetPropertyDefault("ENABLE_EXPORTS", nullptr);
  }
  if (impl->TargetType == cmStateEnums::SHARED_LIBRARY ||
      impl->TargetType == cmStateEnums::MODULE_LIBRARY) {
    this->SetProperty("POSITION_INDEPENDENT_CODE", "True");
  }
  if (impl->TargetType == cmStateEnums::SHARED_LIBRARY ||
      impl->TargetType == cmStateEnums::EXECUTABLE) {
    this->SetPropertyDefault("WINDOWS_EXPORT_ALL_SYMBOLS", nullptr);
  }

  if (this->GetType() != cmStateEnums::INTERFACE_LIBRARY &&
      this->GetType() != cmStateEnums::UTILITY) {
    this->SetPropertyDefault("POSITION_INDEPENDENT_CODE", nullptr);
  }

  // Record current policies for later use.
  impl->Makefile->RecordPolicies(impl->PolicyMap);

  if (impl->TargetType == cmStateEnums::INTERFACE_LIBRARY) {
    // This policy is checked in a few conditions. The properties relevant
    // to the policy are always ignored for cmStateEnums::INTERFACE_LIBRARY
    // targets,
    // so ensure that the conditions don't lead to nonsense.
    impl->PolicyMap.Set(cmPolicies::CMP0022, cmPolicies::NEW);
  }

  if (this->GetType() != cmStateEnums::INTERFACE_LIBRARY &&
      this->GetType() != cmStateEnums::UTILITY) {
    this->SetPropertyDefault("JOB_POOL_COMPILE", nullptr);
    this->SetPropertyDefault("JOB_POOL_LINK", nullptr);
  }

  if (impl->TargetType <= cmStateEnums::UTILITY) {
    this->SetPropertyDefault("DOTNET_TARGET_FRAMEWORK_VERSION", nullptr);
  }

  if (this->GetType() != cmStateEnums::INTERFACE_LIBRARY &&
      this->GetType() != cmStateEnums::UTILITY) {

    // check for "CMAKE_VS_GLOBALS" variable and set up target properties
    // if any
    const char* globals = mf->GetDefinition("CMAKE_VS_GLOBALS");
    if (globals) {
      const std::string genName = mf->GetGlobalGenerator()->GetName();
      if (cmHasLiteralPrefix(genName, "Visual Studio")) {
        std::vector<std::string> props;
        cmSystemTools::ExpandListArgument(globals, props);
        const std::string vsGlobal = "VS_GLOBAL_";
        for (const std::string& i : props) {
          // split NAME=VALUE
          const std::string::size_type assignment = i.find('=');
          if (assignment != std::string::npos) {
            const std::string propName = vsGlobal + i.substr(0, assignment);
            const std::string propValue = i.substr(assignment + 1);
            this->SetPropertyDefault(propName, propValue.c_str());
          }
        }
      }
    }
  }
}

cmStateEnums::TargetType cmTarget::GetType() const
{
  return impl->TargetType;
}

cmMakefile* cmTarget::GetMakefile() const
{
  return impl->Makefile;
}

cmPolicies::PolicyMap const& cmTarget::GetPolicyMap() const
{
  return impl->PolicyMap;
}

cmPolicies::PolicyStatus cmTarget::GetPolicyStatus(
  cmPolicies::PolicyID policy) const
{
  return impl->PolicyMap.Get(policy);
}

cmGlobalGenerator* cmTarget::GetGlobalGenerator() const
{
  return impl->Makefile->GetGlobalGenerator();
}

void cmTarget::AddUtility(std::string const& name, cmMakefile* mf)
{
  impl->Utilities.insert(
    BT<std::string>(name, mf ? mf->GetBacktrace() : cmListFileBacktrace()));
}

std::set<BT<std::string>> const& cmTarget::GetUtilities() const
{
  return impl->Utilities;
}

cmListFileBacktrace const& cmTarget::GetBacktrace() const
{
  return this->Backtrace;
}

bool cmTarget::IsExecutableWithExports() const
{
  return (this->GetType() == cmStateEnums::EXECUTABLE &&
          this->GetPropertyAsBool("ENABLE_EXPORTS"));
}

bool cmTarget::HasImportLibrary() const
{
  return (this->DLLPlatform &&
          (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
           this->IsExecutableWithExports()));
}

bool cmTarget::IsFrameworkOnApple() const
{
  return ((this->GetType() == cmStateEnums::SHARED_LIBRARY ||
           this->GetType() == cmStateEnums::STATIC_LIBRARY) &&
          impl->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("FRAMEWORK"));
}

bool cmTarget::IsAppBundleOnApple() const
{
  return (this->GetType() == cmStateEnums::EXECUTABLE &&
          impl->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("MACOSX_BUNDLE"));
}

void cmTarget::AddTracedSources(std::vector<std::string> const& srcs)
{
  if (!srcs.empty()) {
    cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
    impl->SourceEntries.push_back(cmJoin(srcs, ";"));
    impl->SourceBacktraces.push_back(lfbt);
  }
}

void cmTarget::AddSources(std::vector<std::string> const& srcs)
{
  std::string srcFiles;
  const char* sep = "";
  for (auto filename : srcs) {
    if (!cmGeneratorExpression::StartsWithGeneratorExpression(filename)) {
      if (!filename.empty()) {
        filename = this->ProcessSourceItemCMP0049(filename);
        if (filename.empty()) {
          return;
        }
      }
      impl->Makefile->GetOrCreateSource(filename);
    }
    srcFiles += sep;
    srcFiles += filename;
    sep = ";";
  }
  if (!srcFiles.empty()) {
    cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
    impl->SourceEntries.push_back(std::move(srcFiles));
    impl->SourceBacktraces.push_back(lfbt);
  }
}

std::string cmTarget::ProcessSourceItemCMP0049(const std::string& s)
{
  std::string src = s;

  // For backwards compatibility replace variables in source names.
  // This should eventually be removed.
  impl->Makefile->ExpandVariablesInString(src);
  if (src != s) {
    std::ostringstream e;
    bool noMessage = false;
    MessageType messageType = MessageType::AUTHOR_WARNING;
    switch (impl->Makefile->GetPolicyStatus(cmPolicies::CMP0049)) {
      case cmPolicies::WARN:
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0049) << "\n";
        break;
      case cmPolicies::OLD:
        noMessage = true;
        break;
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::NEW:
        messageType = MessageType::FATAL_ERROR;
    }
    if (!noMessage) {
      e << "Legacy variable expansion in source file \"" << s
        << "\" expanded to \"" << src << "\" in target \"" << this->GetName()
        << "\".  This behavior will be removed in a "
           "future version of CMake.";
      impl->Makefile->IssueMessage(messageType, e.str());
      if (messageType == MessageType::FATAL_ERROR) {
        return "";
      }
    }
  }
  return src;
}

cmSourceFile* cmTarget::AddSourceCMP0049(const std::string& s)
{
  std::string src = this->ProcessSourceItemCMP0049(s);
  if (!s.empty() && src.empty()) {
    return nullptr;
  }
  return this->AddSource(src);
}

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

struct LocationMatcher
{
  const cmSourceFileLocation& Needle;

  LocationMatcher(const cmSourceFileLocation& needle)
    : Needle(needle)
  {
  }

  bool operator()(cmSourceFileLocation& loc)
  {
    return loc.Matches(this->Needle);
  }
};

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
    std::vector<cmSourceFileLocation> locations;
    locations.reserve(files.size());
    std::transform(files.begin(), files.end(), std::back_inserter(locations),
                   CreateLocation(this->Needle.GetMakefile()));

    return std::find_if(locations.begin(), locations.end(),
                        LocationMatcher(this->Needle)) != locations.end();
  }
};

cmSourceFile* cmTarget::AddSource(const std::string& src, bool before)
{
  cmSourceFileLocation sfl(impl->Makefile, src,
                           cmSourceFileLocationKind::Known);
  if (std::find_if(impl->SourceEntries.begin(), impl->SourceEntries.end(),
                   TargetPropertyEntryFinder(sfl)) ==
      impl->SourceEntries.end()) {
    cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
    impl->SourceEntries.insert(
      before ? impl->SourceEntries.begin() : impl->SourceEntries.end(), src);
    impl->SourceBacktraces.insert(before ? impl->SourceBacktraces.begin()
                                         : impl->SourceBacktraces.end(),
                                  lfbt);
  }
  if (cmGeneratorExpression::Find(src) != std::string::npos) {
    return nullptr;
  }
  return impl->Makefile->GetOrCreateSource(src, false,
                                           cmSourceFileLocationKind::Known);
}

void cmTarget::ClearDependencyInformation(cmMakefile& mf)
{
  std::string depname = this->GetName();
  depname += "_LIB_DEPENDS";
  mf.RemoveCacheDefinition(depname);
}

std::string cmTarget::GetDebugGeneratorExpressions(
  const std::string& value, cmTargetLinkLibraryType llt) const
{
  if (llt == GENERAL_LibraryType) {
    return value;
  }

  // Get the list of configurations considered to be DEBUG.
  std::vector<std::string> debugConfigs =
    impl->Makefile->GetCMakeInstance()->GetDebugConfigs();

  std::string configString = "$<CONFIG:" + debugConfigs[0] + ">";

  if (debugConfigs.size() > 1) {
    for (std::string const& conf : cmMakeRange(debugConfigs).advance(1)) {
      configString += ",$<CONFIG:" + conf + ">";
    }
    configString = "$<OR:" + configString + ">";
  }

  if (llt == OPTIMIZED_LibraryType) {
    configString = "$<NOT:" + configString + ">";
  }
  return "$<" + configString + ":" + value + ">";
}

static std::string targetNameGenex(const std::string& lib)
{
  return "$<TARGET_NAME:" + lib + ">";
}

bool cmTarget::PushTLLCommandTrace(TLLSignature signature,
                                   cmListFileContext const& lfc)
{
  bool ret = true;
  if (!this->TLLCommands.empty()) {
    if (this->TLLCommands.back().first != signature) {
      ret = false;
    }
  }
  if (this->TLLCommands.empty() || this->TLLCommands.back().second != lfc) {
    this->TLLCommands.emplace_back(signature, lfc);
  }
  return ret;
}

void cmTarget::GetTllSignatureTraces(std::ostream& s, TLLSignature sig) const
{
  const char* sigString =
    (sig == cmTarget::KeywordTLLSignature ? "keyword" : "plain");
  s << "The uses of the " << sigString << " signature are here:\n";
  cmStateDirectory cmDir = impl->Makefile->GetStateSnapshot().GetDirectory();
  for (auto const& cmd : this->TLLCommands) {
    if (cmd.first == sig) {
      cmListFileContext lfc = cmd.second;
      lfc.FilePath = cmDir.ConvertToRelPathIfNotContained(
        impl->Makefile->GetState()->GetSourceDirectory(), lfc.FilePath);
      s << " * " << lfc << std::endl;
    }
  }
}

cmTarget::LinkLibraryVectorType const& cmTarget::GetOriginalLinkLibraries()
  const
{
  return impl->OriginalLinkLibraries;
}

void cmTarget::AddLinkLibrary(cmMakefile& mf, const std::string& lib,
                              cmTargetLinkLibraryType llt)
{
  this->AddLinkLibrary(mf, lib, lib, llt);
}

void cmTarget::AddLinkLibrary(cmMakefile& mf, std::string const& lib,
                              std::string const& libRef,
                              cmTargetLinkLibraryType llt)
{
  cmTarget* tgt = mf.FindTargetToUse(lib);
  {
    const bool isNonImportedTarget = tgt && !tgt->IsImported();

    const std::string libName =
      (isNonImportedTarget && llt != GENERAL_LibraryType)
      ? targetNameGenex(libRef)
      : libRef;
    this->AppendProperty(
      "LINK_LIBRARIES",
      this->GetDebugGeneratorExpressions(libName, llt).c_str());
  }

  if (cmGeneratorExpression::Find(lib) != std::string::npos || lib != libRef ||
      (tgt &&
       (tgt->GetType() == cmStateEnums::INTERFACE_LIBRARY ||
        tgt->GetType() == cmStateEnums::OBJECT_LIBRARY)) ||
      (this->Name == lib)) {
    return;
  }

  impl->OriginalLinkLibraries.emplace_back(lib, llt);

  // Add the explicit dependency information for libraries. This is
  // simply a set of libraries separated by ";". There should always
  // be a trailing ";". These library names are not canonical, in that
  // they may be "-framework x", "-ly", "/path/libz.a", etc.
  // We shouldn't remove duplicates here because external libraries
  // may be purposefully duplicated to handle recursive dependencies,
  // and we removing one instance will break the link line. Duplicates
  // will be appropriately eliminated at emit time.
  if (impl->TargetType >= cmStateEnums::STATIC_LIBRARY &&
      impl->TargetType <= cmStateEnums::MODULE_LIBRARY &&
      (this->GetPolicyStatusCMP0073() == cmPolicies::OLD ||
       this->GetPolicyStatusCMP0073() == cmPolicies::WARN)) {
    std::string targetEntry = this->Name;
    targetEntry += "_LIB_DEPENDS";
    std::string dependencies;
    const char* old_val = mf.GetDefinition(targetEntry);
    if (old_val) {
      dependencies += old_val;
    }
    switch (llt) {
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
    mf.AddCacheDefinition(targetEntry, dependencies.c_str(),
                          "Dependencies for the target", cmStateEnums::STATIC);
  }
}

void cmTarget::AddSystemIncludeDirectories(const std::set<std::string>& incs)
{
  impl->SystemIncludeDirectories.insert(incs.begin(), incs.end());
}

std::set<std::string> const& cmTarget::GetSystemIncludeDirectories() const
{
  return impl->SystemIncludeDirectories;
}

cmStringRange cmTarget::GetIncludeDirectoriesEntries() const
{
  return cmMakeRange(impl->IncludeDirectoriesEntries);
}

cmBacktraceRange cmTarget::GetIncludeDirectoriesBacktraces() const
{
  return cmMakeRange(impl->IncludeDirectoriesBacktraces);
}

cmStringRange cmTarget::GetCompileOptionsEntries() const
{
  return cmMakeRange(impl->CompileOptionsEntries);
}

cmBacktraceRange cmTarget::GetCompileOptionsBacktraces() const
{
  return cmMakeRange(impl->CompileOptionsBacktraces);
}

cmStringRange cmTarget::GetCompileFeaturesEntries() const
{
  return cmMakeRange(impl->CompileFeaturesEntries);
}

cmBacktraceRange cmTarget::GetCompileFeaturesBacktraces() const
{
  return cmMakeRange(impl->CompileFeaturesBacktraces);
}

cmStringRange cmTarget::GetCompileDefinitionsEntries() const
{
  return cmMakeRange(impl->CompileDefinitionsEntries);
}

cmBacktraceRange cmTarget::GetCompileDefinitionsBacktraces() const
{
  return cmMakeRange(impl->CompileDefinitionsBacktraces);
}

cmStringRange cmTarget::GetSourceEntries() const
{
  return cmMakeRange(impl->SourceEntries);
}

cmBacktraceRange cmTarget::GetSourceBacktraces() const
{
  return cmMakeRange(impl->SourceBacktraces);
}

cmStringRange cmTarget::GetLinkOptionsEntries() const
{
  return cmMakeRange(impl->LinkOptionsEntries);
}

cmBacktraceRange cmTarget::GetLinkOptionsBacktraces() const
{
  return cmMakeRange(impl->LinkOptionsBacktraces);
}

cmStringRange cmTarget::GetLinkDirectoriesEntries() const
{
  return cmMakeRange(impl->LinkDirectoriesEntries);
}

cmBacktraceRange cmTarget::GetLinkDirectoriesBacktraces() const
{
  return cmMakeRange(impl->LinkDirectoriesBacktraces);
}

cmStringRange cmTarget::GetLinkImplementationEntries() const
{
  return cmMakeRange(impl->LinkImplementationPropertyEntries);
}

cmBacktraceRange cmTarget::GetLinkImplementationBacktraces() const
{
  return cmMakeRange(impl->LinkImplementationPropertyBacktraces);
}

void cmTarget::SetProperty(const std::string& prop, const char* value)
{
  if (!cmTargetPropertyComputer::PassesWhitelist(
        this->GetType(), prop, impl->Makefile->GetMessenger(),
        impl->Makefile->GetBacktrace())) {
    return;
  }
#define MAKE_STATIC_PROP(PROP) static const std::string prop##PROP = #PROP
  MAKE_STATIC_PROP(COMPILE_DEFINITIONS);
  MAKE_STATIC_PROP(COMPILE_FEATURES);
  MAKE_STATIC_PROP(COMPILE_OPTIONS);
  MAKE_STATIC_PROP(CUDA_PTX_COMPILATION);
  MAKE_STATIC_PROP(EXPORT_NAME);
  MAKE_STATIC_PROP(IMPORTED_GLOBAL);
  MAKE_STATIC_PROP(INCLUDE_DIRECTORIES);
  MAKE_STATIC_PROP(LINK_OPTIONS);
  MAKE_STATIC_PROP(LINK_DIRECTORIES);
  MAKE_STATIC_PROP(LINK_LIBRARIES);
  MAKE_STATIC_PROP(MANUALLY_ADDED_DEPENDENCIES);
  MAKE_STATIC_PROP(NAME);
  MAKE_STATIC_PROP(SOURCES);
  MAKE_STATIC_PROP(TYPE);
#undef MAKE_STATIC_PROP
  if (prop == propMANUALLY_ADDED_DEPENDENCIES) {
    std::ostringstream e;
    e << "MANUALLY_ADDED_DEPENDENCIES property is read-only\n";
    impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }
  if (prop == propNAME) {
    std::ostringstream e;
    e << "NAME property is read-only\n";
    impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }
  if (prop == propTYPE) {
    std::ostringstream e;
    e << "TYPE property is read-only\n";
    impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }
  if (prop == propEXPORT_NAME && this->IsImported()) {
    std::ostringstream e;
    e << "EXPORT_NAME property can't be set on imported targets (\""
      << this->Name << "\")\n";
    impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }
  if (prop == propSOURCES && this->IsImported()) {
    std::ostringstream e;
    e << "SOURCES property can't be set on imported targets (\"" << this->Name
      << "\")\n";
    impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }
  if (prop == propIMPORTED_GLOBAL && !this->IsImported()) {
    std::ostringstream e;
    e << "IMPORTED_GLOBAL property can't be set on non-imported targets (\""
      << this->Name << "\")\n";
    impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }

  if (prop == propINCLUDE_DIRECTORIES) {
    impl->IncludeDirectoriesEntries.clear();
    impl->IncludeDirectoriesBacktraces.clear();
    if (value) {
      impl->IncludeDirectoriesEntries.emplace_back(value);
      cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
      impl->IncludeDirectoriesBacktraces.push_back(lfbt);
    }
  } else if (prop == propCOMPILE_OPTIONS) {
    impl->CompileOptionsEntries.clear();
    impl->CompileOptionsBacktraces.clear();
    if (value) {
      impl->CompileOptionsEntries.emplace_back(value);
      cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
      impl->CompileOptionsBacktraces.push_back(lfbt);
    }
  } else if (prop == propCOMPILE_FEATURES) {
    impl->CompileFeaturesEntries.clear();
    impl->CompileFeaturesBacktraces.clear();
    if (value) {
      impl->CompileFeaturesEntries.emplace_back(value);
      cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
      impl->CompileFeaturesBacktraces.push_back(lfbt);
    }
  } else if (prop == propCOMPILE_DEFINITIONS) {
    impl->CompileDefinitionsEntries.clear();
    impl->CompileDefinitionsBacktraces.clear();
    if (value) {
      impl->CompileDefinitionsEntries.emplace_back(value);
      cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
      impl->CompileDefinitionsBacktraces.push_back(lfbt);
    }
  } else if (prop == propLINK_OPTIONS) {
    impl->LinkOptionsEntries.clear();
    impl->LinkOptionsBacktraces.clear();
    if (value) {
      impl->LinkOptionsEntries.emplace_back(value);
      cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
      impl->LinkOptionsBacktraces.push_back(lfbt);
    }
  } else if (prop == propLINK_DIRECTORIES) {
    impl->LinkDirectoriesEntries.clear();
    impl->LinkDirectoriesBacktraces.clear();
    if (value) {
      impl->LinkDirectoriesEntries.emplace_back(value);
      cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
      impl->LinkDirectoriesBacktraces.push_back(lfbt);
    }
  } else if (prop == propLINK_LIBRARIES) {
    impl->LinkImplementationPropertyEntries.clear();
    impl->LinkImplementationPropertyBacktraces.clear();
    if (value) {
      cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
      impl->LinkImplementationPropertyEntries.emplace_back(value);
      impl->LinkImplementationPropertyBacktraces.push_back(lfbt);
    }
  } else if (prop == propSOURCES) {
    impl->SourceEntries.clear();
    impl->SourceBacktraces.clear();
    if (value) {
      cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
      impl->SourceEntries.emplace_back(value);
      impl->SourceBacktraces.push_back(lfbt);
    }
  } else if (prop == propIMPORTED_GLOBAL) {
    if (!cmSystemTools::IsOn(value)) {
      std::ostringstream e;
      e << "IMPORTED_GLOBAL property can't be set to FALSE on targets (\""
        << this->Name << "\")\n";
      impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return;
    }
    /* no need to change anything if value does not change */
    if (!this->ImportedGloballyVisible) {
      this->ImportedGloballyVisible = true;
      this->GetGlobalGenerator()->IndexTarget(this);
    }
  } else if (cmHasLiteralPrefix(prop, "IMPORTED_LIBNAME") &&
             !this->CheckImportedLibName(prop, value ? value : "")) {
    /* error was reported by check method */
  } else if (prop == propCUDA_PTX_COMPILATION &&
             this->GetType() != cmStateEnums::OBJECT_LIBRARY) {
    std::ostringstream e;
    e << "CUDA_PTX_COMPILATION property can only be applied to OBJECT "
         "targets (\""
      << this->Name << "\")\n";
    impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  } else {
    impl->Properties.SetProperty(prop, value);
  }
}

void cmTarget::AppendProperty(const std::string& prop, const char* value,
                              bool asString)
{
  if (!cmTargetPropertyComputer::PassesWhitelist(
        this->GetType(), prop, impl->Makefile->GetMessenger(),
        impl->Makefile->GetBacktrace())) {
    return;
  }
  if (prop == "NAME") {
    std::ostringstream e;
    e << "NAME property is read-only\n";
    impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }
  if (prop == "EXPORT_NAME" && this->IsImported()) {
    std::ostringstream e;
    e << "EXPORT_NAME property can't be set on imported targets (\""
      << this->Name << "\")\n";
    impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }
  if (prop == "SOURCES" && this->IsImported()) {
    std::ostringstream e;
    e << "SOURCES property can't be set on imported targets (\"" << this->Name
      << "\")\n";
    impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }
  if (prop == "IMPORTED_GLOBAL") {
    std::ostringstream e;
    e << "IMPORTED_GLOBAL property can't be appended, only set on imported "
         "targets (\""
      << this->Name << "\")\n";
    impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }
  if (prop == "INCLUDE_DIRECTORIES") {
    if (value && *value) {
      impl->IncludeDirectoriesEntries.emplace_back(value);
      cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
      impl->IncludeDirectoriesBacktraces.push_back(lfbt);
    }
  } else if (prop == "COMPILE_OPTIONS") {
    if (value && *value) {
      impl->CompileOptionsEntries.emplace_back(value);
      cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
      impl->CompileOptionsBacktraces.push_back(lfbt);
    }
  } else if (prop == "COMPILE_FEATURES") {
    if (value && *value) {
      impl->CompileFeaturesEntries.emplace_back(value);
      cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
      impl->CompileFeaturesBacktraces.push_back(lfbt);
    }
  } else if (prop == "COMPILE_DEFINITIONS") {
    if (value && *value) {
      impl->CompileDefinitionsEntries.emplace_back(value);
      cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
      impl->CompileDefinitionsBacktraces.push_back(lfbt);
    }
  } else if (prop == "LINK_OPTIONS") {
    if (value && *value) {
      impl->LinkOptionsEntries.emplace_back(value);
      cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
      impl->LinkOptionsBacktraces.push_back(lfbt);
    }
  } else if (prop == "LINK_DIRECTORIES") {
    if (value && *value) {
      impl->LinkDirectoriesEntries.emplace_back(value);
      cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
      impl->LinkDirectoriesBacktraces.push_back(lfbt);
    }
  } else if (prop == "LINK_LIBRARIES") {
    if (value && *value) {
      cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
      impl->LinkImplementationPropertyEntries.emplace_back(value);
      impl->LinkImplementationPropertyBacktraces.push_back(lfbt);
    }
  } else if (prop == "SOURCES") {
    cmListFileBacktrace lfbt = impl->Makefile->GetBacktrace();
    impl->SourceEntries.emplace_back(value);
    impl->SourceBacktraces.push_back(lfbt);
  } else if (cmHasLiteralPrefix(prop, "IMPORTED_LIBNAME")) {
    impl->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                 prop + " property may not be APPENDed.");
  } else {
    impl->Properties.AppendProperty(prop, value, asString);
  }
}

void cmTarget::AppendBuildInterfaceIncludes()
{
  if (this->GetType() != cmStateEnums::SHARED_LIBRARY &&
      this->GetType() != cmStateEnums::STATIC_LIBRARY &&
      this->GetType() != cmStateEnums::MODULE_LIBRARY &&
      this->GetType() != cmStateEnums::INTERFACE_LIBRARY &&
      !this->IsExecutableWithExports()) {
    return;
  }
  if (this->BuildInterfaceIncludesAppended) {
    return;
  }
  this->BuildInterfaceIncludesAppended = true;

  if (impl->Makefile->IsOn("CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE")) {
    std::string dirs = impl->Makefile->GetCurrentBinaryDirectory();
    if (!dirs.empty()) {
      dirs += ';';
    }
    dirs += impl->Makefile->GetCurrentSourceDirectory();
    if (!dirs.empty()) {
      this->AppendProperty("INTERFACE_INCLUDE_DIRECTORIES",
                           ("$<BUILD_INTERFACE:" + dirs + ">").c_str());
    }
  }
}

void cmTarget::InsertInclude(std::string const& entry,
                             cmListFileBacktrace const& bt, bool before)
{
  std::vector<std::string>::iterator position = before
    ? impl->IncludeDirectoriesEntries.begin()
    : impl->IncludeDirectoriesEntries.end();

  std::vector<cmListFileBacktrace>::iterator btPosition = before
    ? impl->IncludeDirectoriesBacktraces.begin()
    : impl->IncludeDirectoriesBacktraces.end();

  impl->IncludeDirectoriesEntries.insert(position, entry);
  impl->IncludeDirectoriesBacktraces.insert(btPosition, bt);
}

void cmTarget::InsertCompileOption(std::string const& entry,
                                   cmListFileBacktrace const& bt, bool before)
{
  std::vector<std::string>::iterator position = before
    ? impl->CompileOptionsEntries.begin()
    : impl->CompileOptionsEntries.end();

  std::vector<cmListFileBacktrace>::iterator btPosition = before
    ? impl->CompileOptionsBacktraces.begin()
    : impl->CompileOptionsBacktraces.end();

  impl->CompileOptionsEntries.insert(position, entry);
  impl->CompileOptionsBacktraces.insert(btPosition, bt);
}

void cmTarget::InsertCompileDefinition(std::string const& entry,
                                       cmListFileBacktrace const& bt)
{
  impl->CompileDefinitionsEntries.push_back(entry);
  impl->CompileDefinitionsBacktraces.push_back(bt);
}

void cmTarget::InsertLinkOption(std::string const& entry,
                                cmListFileBacktrace const& bt, bool before)
{
  std::vector<std::string>::iterator position =
    before ? impl->LinkOptionsEntries.begin() : impl->LinkOptionsEntries.end();

  std::vector<cmListFileBacktrace>::iterator btPosition = before
    ? impl->LinkOptionsBacktraces.begin()
    : impl->LinkOptionsBacktraces.end();

  impl->LinkOptionsEntries.insert(position, entry);
  impl->LinkOptionsBacktraces.insert(btPosition, bt);
}

void cmTarget::InsertLinkDirectory(std::string const& entry,
                                   cmListFileBacktrace const& bt, bool before)
{
  std::vector<std::string>::iterator position = before
    ? impl->LinkDirectoriesEntries.begin()
    : impl->LinkDirectoriesEntries.end();

  std::vector<cmListFileBacktrace>::iterator btPosition = before
    ? impl->LinkDirectoriesBacktraces.begin()
    : impl->LinkDirectoriesBacktraces.end();

  impl->LinkDirectoriesEntries.insert(position, entry);
  impl->LinkDirectoriesBacktraces.insert(btPosition, bt);
}

static void cmTargetCheckLINK_INTERFACE_LIBRARIES(const std::string& prop,
                                                  const char* value,
                                                  cmMakefile* context,
                                                  bool imported)
{
  // Look for link-type keywords in the value.
  static cmsys::RegularExpression keys("(^|;)(debug|optimized|general)(;|$)");
  if (!keys.find(value)) {
    return;
  }

  // Support imported and non-imported versions of the property.
  const char* base = (imported ? "IMPORTED_LINK_INTERFACE_LIBRARIES"
                               : "LINK_INTERFACE_LIBRARIES");

  // Report an error.
  std::ostringstream e;
  e << "Property " << prop << " may not contain link-type keyword \""
    << keys.match(2) << "\".  "
    << "The " << base << " property has a per-configuration "
    << "version called " << base << "_<CONFIG> which may be "
    << "used to specify per-configuration rules.";
  if (!imported) {
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
  context->IssueMessage(MessageType::FATAL_ERROR, e.str());
}

static void cmTargetCheckINTERFACE_LINK_LIBRARIES(const char* value,
                                                  cmMakefile* context)
{
  // Look for link-type keywords in the value.
  static cmsys::RegularExpression keys("(^|;)(debug|optimized|general)(;|$)");
  if (!keys.find(value)) {
    return;
  }

  // Report an error.
  std::ostringstream e;

  e << "Property INTERFACE_LINK_LIBRARIES may not contain link-type "
       "keyword \""
    << keys.match(2)
    << "\".  The INTERFACE_LINK_LIBRARIES "
       "property may contain configuration-sensitive generator-expressions "
       "which may be used to specify per-configuration rules.";

  context->IssueMessage(MessageType::FATAL_ERROR, e.str());
}

static void cmTargetCheckIMPORTED_GLOBAL(const cmTarget* target,
                                         cmMakefile* context)
{
  std::vector<cmTarget*> targets = context->GetOwnedImportedTargets();
  std::vector<cmTarget*>::const_iterator it =
    std::find(targets.begin(), targets.end(), target);
  if (it == targets.end()) {
    std::ostringstream e;
    e << "Attempt to promote imported target \"" << target->GetName()
      << "\" to global scope (by setting IMPORTED_GLOBAL) "
         "which is not built in this directory.";
    context->IssueMessage(MessageType::FATAL_ERROR, e.str());
  }
}

void cmTarget::CheckProperty(const std::string& prop,
                             cmMakefile* context) const
{
  // Certain properties need checking.
  if (cmHasLiteralPrefix(prop, "LINK_INTERFACE_LIBRARIES")) {
    if (const char* value = this->GetProperty(prop)) {
      cmTargetCheckLINK_INTERFACE_LIBRARIES(prop, value, context, false);
    }
  }
  if (cmHasLiteralPrefix(prop, "IMPORTED_LINK_INTERFACE_LIBRARIES")) {
    if (const char* value = this->GetProperty(prop)) {
      cmTargetCheckLINK_INTERFACE_LIBRARIES(prop, value, context, true);
    }
  }
  if (prop == "INTERFACE_LINK_LIBRARIES") {
    if (const char* value = this->GetProperty(prop)) {
      cmTargetCheckINTERFACE_LINK_LIBRARIES(value, context);
    }
  }
  if (prop == "IMPORTED_GLOBAL") {
    if (this->IsImported()) {
      cmTargetCheckIMPORTED_GLOBAL(this, context);
    }
  }
}

const char* cmTarget::GetComputedProperty(
  const std::string& prop, cmMessenger* messenger,
  cmListFileBacktrace const& context) const
{
  return cmTargetPropertyComputer::GetProperty(this, prop, messenger, context);
}

const char* cmTarget::GetProperty(const std::string& prop) const
{
  static std::unordered_set<std::string> specialProps;
#define MAKE_STATIC_PROP(PROP) static const std::string prop##PROP = #PROP
  MAKE_STATIC_PROP(LINK_LIBRARIES);
  MAKE_STATIC_PROP(TYPE);
  MAKE_STATIC_PROP(INCLUDE_DIRECTORIES);
  MAKE_STATIC_PROP(COMPILE_FEATURES);
  MAKE_STATIC_PROP(COMPILE_OPTIONS);
  MAKE_STATIC_PROP(COMPILE_DEFINITIONS);
  MAKE_STATIC_PROP(LINK_OPTIONS);
  MAKE_STATIC_PROP(LINK_DIRECTORIES);
  MAKE_STATIC_PROP(IMPORTED);
  MAKE_STATIC_PROP(IMPORTED_GLOBAL);
  MAKE_STATIC_PROP(MANUALLY_ADDED_DEPENDENCIES);
  MAKE_STATIC_PROP(NAME);
  MAKE_STATIC_PROP(BINARY_DIR);
  MAKE_STATIC_PROP(SOURCE_DIR);
  MAKE_STATIC_PROP(SOURCES);
#undef MAKE_STATIC_PROP
  if (specialProps.empty()) {
    specialProps.insert(propLINK_LIBRARIES);
    specialProps.insert(propTYPE);
    specialProps.insert(propINCLUDE_DIRECTORIES);
    specialProps.insert(propCOMPILE_FEATURES);
    specialProps.insert(propCOMPILE_OPTIONS);
    specialProps.insert(propCOMPILE_DEFINITIONS);
    specialProps.insert(propLINK_OPTIONS);
    specialProps.insert(propLINK_DIRECTORIES);
    specialProps.insert(propIMPORTED);
    specialProps.insert(propIMPORTED_GLOBAL);
    specialProps.insert(propMANUALLY_ADDED_DEPENDENCIES);
    specialProps.insert(propNAME);
    specialProps.insert(propBINARY_DIR);
    specialProps.insert(propSOURCE_DIR);
    specialProps.insert(propSOURCES);
  }
  if (specialProps.count(prop)) {
    if (prop == propLINK_LIBRARIES) {
      if (impl->LinkImplementationPropertyEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(impl->LinkImplementationPropertyEntries, ";");
      return output.c_str();
    }
    // the type property returns what type the target is
    if (prop == propTYPE) {
      return cmState::GetTargetTypeName(this->GetType());
    }
    if (prop == propINCLUDE_DIRECTORIES) {
      if (impl->IncludeDirectoriesEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(impl->IncludeDirectoriesEntries, ";");
      return output.c_str();
    }
    if (prop == propCOMPILE_FEATURES) {
      if (impl->CompileFeaturesEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(impl->CompileFeaturesEntries, ";");
      return output.c_str();
    }
    if (prop == propCOMPILE_OPTIONS) {
      if (impl->CompileOptionsEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(impl->CompileOptionsEntries, ";");
      return output.c_str();
    }
    if (prop == propCOMPILE_DEFINITIONS) {
      if (impl->CompileDefinitionsEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(impl->CompileDefinitionsEntries, ";");
      return output.c_str();
    }
    if (prop == propLINK_OPTIONS) {
      if (impl->LinkOptionsEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(impl->LinkOptionsEntries, ";");
      return output.c_str();
    }
    if (prop == propLINK_DIRECTORIES) {
      if (impl->LinkDirectoriesEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(impl->LinkDirectoriesEntries, ";");

      return output.c_str();
    }
    if (prop == propMANUALLY_ADDED_DEPENDENCIES) {
      if (impl->Utilities.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(impl->Utilities, ";");
      return output.c_str();
    }
    if (prop == propIMPORTED) {
      return this->IsImported() ? "TRUE" : "FALSE";
    }
    if (prop == propIMPORTED_GLOBAL) {
      return this->IsImportedGloballyVisible() ? "TRUE" : "FALSE";
    }
    if (prop == propNAME) {
      return this->GetName().c_str();
    }
    if (prop == propBINARY_DIR) {
      return impl->Makefile->GetStateSnapshot()
        .GetDirectory()
        .GetCurrentBinary()
        .c_str();
    }
    if (prop == propSOURCE_DIR) {
      return impl->Makefile->GetStateSnapshot()
        .GetDirectory()
        .GetCurrentSource()
        .c_str();
    }
  }

  const char* retVal = impl->Properties.GetPropertyValue(prop);
  if (!retVal) {
    const bool chain =
      impl->Makefile->GetState()->IsPropertyChained(prop, cmProperty::TARGET);
    if (chain) {
      return impl->Makefile->GetStateSnapshot().GetDirectory().GetProperty(
        prop, chain);
    }
  }
  return retVal;
}

const char* cmTarget::GetSafeProperty(const std::string& prop) const
{
  const char* ret = this->GetProperty(prop);
  if (!ret) {
    return "";
  }
  return ret;
}

bool cmTarget::GetPropertyAsBool(const std::string& prop) const
{
  return cmSystemTools::IsOn(this->GetProperty(prop));
}

cmPropertyMap const& cmTarget::GetProperties() const
{
  return impl->Properties;
}

const char* cmTarget::GetSuffixVariableInternal(
  cmStateEnums::ArtifactType artifact) const
{
  switch (this->GetType()) {
    case cmStateEnums::STATIC_LIBRARY:
      return "CMAKE_STATIC_LIBRARY_SUFFIX";
    case cmStateEnums::SHARED_LIBRARY:
      switch (artifact) {
        case cmStateEnums::RuntimeBinaryArtifact:
          return "CMAKE_SHARED_LIBRARY_SUFFIX";
        case cmStateEnums::ImportLibraryArtifact:
          return "CMAKE_IMPORT_LIBRARY_SUFFIX";
      }
      break;
    case cmStateEnums::MODULE_LIBRARY:
      switch (artifact) {
        case cmStateEnums::RuntimeBinaryArtifact:
          return "CMAKE_SHARED_MODULE_SUFFIX";
        case cmStateEnums::ImportLibraryArtifact:
          return "CMAKE_IMPORT_LIBRARY_SUFFIX";
      }
      break;
    case cmStateEnums::EXECUTABLE:
      switch (artifact) {
        case cmStateEnums::RuntimeBinaryArtifact:
          // Android GUI application packages store the native
          // binary as a shared library.
          return (this->IsAndroid && this->GetPropertyAsBool("ANDROID_GUI")
                    ? "CMAKE_SHARED_LIBRARY_SUFFIX"
                    : "CMAKE_EXECUTABLE_SUFFIX");
        case cmStateEnums::ImportLibraryArtifact:
          return "CMAKE_IMPORT_LIBRARY_SUFFIX";
      }
      break;
    default:
      break;
  }
  return "";
}

const char* cmTarget::GetPrefixVariableInternal(
  cmStateEnums::ArtifactType artifact) const
{
  switch (this->GetType()) {
    case cmStateEnums::STATIC_LIBRARY:
      return "CMAKE_STATIC_LIBRARY_PREFIX";
    case cmStateEnums::SHARED_LIBRARY:
      switch (artifact) {
        case cmStateEnums::RuntimeBinaryArtifact:
          return "CMAKE_SHARED_LIBRARY_PREFIX";
        case cmStateEnums::ImportLibraryArtifact:
          return "CMAKE_IMPORT_LIBRARY_PREFIX";
      }
      break;
    case cmStateEnums::MODULE_LIBRARY:
      switch (artifact) {
        case cmStateEnums::RuntimeBinaryArtifact:
          return "CMAKE_SHARED_MODULE_PREFIX";
        case cmStateEnums::ImportLibraryArtifact:
          return "CMAKE_IMPORT_LIBRARY_PREFIX";
      }
      break;
    case cmStateEnums::EXECUTABLE:
      switch (artifact) {
        case cmStateEnums::RuntimeBinaryArtifact:
          // Android GUI application packages store the native
          // binary as a shared library.
          return (this->IsAndroid && this->GetPropertyAsBool("ANDROID_GUI")
                    ? "CMAKE_SHARED_LIBRARY_PREFIX"
                    : "");
        case cmStateEnums::ImportLibraryArtifact:
          return "CMAKE_IMPORT_LIBRARY_PREFIX";
      }
      break;
    default:
      break;
  }
  return "";
}

std::string cmTarget::ImportedGetFullPath(
  const std::string& config, cmStateEnums::ArtifactType artifact) const
{
  assert(this->IsImported());

  // Lookup/compute/cache the import information for this
  // configuration.
  std::string desired_config = config;
  if (config.empty()) {
    desired_config = "NOCONFIG";
  }

  std::string result;

  const char* loc = nullptr;
  const char* imp = nullptr;
  std::string suffix;

  if (this->GetType() != cmStateEnums::INTERFACE_LIBRARY &&
      this->GetMappedConfig(desired_config, &loc, &imp, suffix)) {
    switch (artifact) {
      case cmStateEnums::RuntimeBinaryArtifact:
        if (loc) {
          result = loc;
        } else {
          std::string impProp = "IMPORTED_LOCATION";
          impProp += suffix;
          if (const char* config_location = this->GetProperty(impProp)) {
            result = config_location;
          } else if (const char* location =
                       this->GetProperty("IMPORTED_LOCATION")) {
            result = location;
          }
        }
        break;

      case cmStateEnums::ImportLibraryArtifact:
        if (imp) {
          result = imp;
        } else if (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
                   this->IsExecutableWithExports()) {
          std::string impProp = "IMPORTED_IMPLIB";
          impProp += suffix;
          if (const char* config_implib = this->GetProperty(impProp)) {
            result = config_implib;
          } else if (const char* implib =
                       this->GetProperty("IMPORTED_IMPLIB")) {
            result = implib;
          }
        }
        break;
    }
  }

  if (result.empty()) {
    result = this->GetName();
    result += "-NOTFOUND";
  }
  return result;
}

void cmTarget::SetPropertyDefault(const std::string& property,
                                  const char* default_value)
{
  // Compute the name of the variable holding the default value.
  std::string var = "CMAKE_";
  var += property;

  if (const char* value = impl->Makefile->GetDefinition(var)) {
    this->SetProperty(property, value);
  } else if (default_value) {
    this->SetProperty(property, default_value);
  }
}

bool cmTarget::CheckImportedLibName(std::string const& prop,
                                    std::string const& value) const
{
  if (this->GetType() != cmStateEnums::INTERFACE_LIBRARY ||
      !this->IsImported()) {
    impl->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      prop +
        " property may be set only on imported INTERFACE library targets.");
    return false;
  }
  if (!value.empty()) {
    if (value[0] == '-') {
      impl->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                   prop + " property value\n  " + value +
                                     "\nmay not start with '-'.");
      return false;
    }
    std::string::size_type bad = value.find_first_of(":/\\;");
    if (bad != std::string::npos) {
      impl->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                   prop + " property value\n  " + value +
                                     "\nmay not contain '" +
                                     value.substr(bad, 1) + "'.");
      return false;
    }
  }
  return true;
}

bool cmTarget::GetMappedConfig(std::string const& desired_config,
                               const char** loc, const char** imp,
                               std::string& suffix) const
{
  std::string config_upper;
  if (!desired_config.empty()) {
    config_upper = cmSystemTools::UpperCase(desired_config);
  }

  std::string locPropBase;
  if (this->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    locPropBase = "IMPORTED_LIBNAME";
  } else if (this->GetType() == cmStateEnums::OBJECT_LIBRARY) {
    locPropBase = "IMPORTED_OBJECTS";
  } else {
    locPropBase = "IMPORTED_LOCATION";
  }

  // Track the configuration-specific property suffix.
  suffix = "_";
  suffix += config_upper;

  std::vector<std::string> mappedConfigs;
  {
    std::string mapProp = "MAP_IMPORTED_CONFIG_";
    mapProp += config_upper;
    if (const char* mapValue = this->GetProperty(mapProp)) {
      cmSystemTools::ExpandListArgument(mapValue, mappedConfigs, true);
    }
  }

  // If we needed to find one of the mapped configurations but did not
  // On a DLL platform there may be only IMPORTED_IMPLIB for a shared
  // library or an executable with exports.
  bool allowImp = this->HasImportLibrary();

  // If a mapping was found, check its configurations.
  for (std::vector<std::string>::const_iterator mci = mappedConfigs.begin();
       !*loc && !*imp && mci != mappedConfigs.end(); ++mci) {
    // Look for this configuration.
    if (mci->empty()) {
      // An empty string in the mapping has a special meaning:
      // look up the config-less properties.
      *loc = this->GetProperty(locPropBase);
      if (allowImp) {
        *imp = this->GetProperty("IMPORTED_IMPLIB");
      }
      // If it was found, set the suffix.
      if (*loc || *imp) {
        suffix.clear();
      }
    } else {
      std::string mcUpper = cmSystemTools::UpperCase(*mci);
      std::string locProp = locPropBase + "_";
      locProp += mcUpper;
      *loc = this->GetProperty(locProp);
      if (allowImp) {
        std::string impProp = "IMPORTED_IMPLIB_";
        impProp += mcUpper;
        *imp = this->GetProperty(impProp);
      }

      // If it was found, use it for all properties below.
      if (*loc || *imp) {
        suffix = "_";
        suffix += mcUpper;
      }
    }
  }

  // If we needed to find one of the mapped configurations but did not
  // then the target location is not found.  The project does not want
  // any other configuration.
  if (!mappedConfigs.empty() && !*loc && !*imp) {
    // Interface libraries are always available because their
    // library name is optional so it is okay to leave *loc empty.
    return this->GetType() == cmStateEnums::INTERFACE_LIBRARY;
  }

  // If we have not yet found it then there are no mapped
  // configurations.  Look for an exact-match.
  if (!*loc && !*imp) {
    std::string locProp = locPropBase;
    locProp += suffix;
    *loc = this->GetProperty(locProp);
    if (allowImp) {
      std::string impProp = "IMPORTED_IMPLIB";
      impProp += suffix;
      *imp = this->GetProperty(impProp);
    }
  }

  // If we have not yet found it then there are no mapped
  // configurations and no exact match.
  if (!*loc && !*imp) {
    // The suffix computed above is not useful.
    suffix.clear();

    // Look for a configuration-less location.  This may be set by
    // manually-written code.
    *loc = this->GetProperty(locPropBase);
    if (allowImp) {
      *imp = this->GetProperty("IMPORTED_IMPLIB");
    }
  }

  // If we have not yet found it then the project is willing to try
  // any available configuration.
  if (!*loc && !*imp) {
    std::vector<std::string> availableConfigs;
    if (const char* iconfigs = this->GetProperty("IMPORTED_CONFIGURATIONS")) {
      cmSystemTools::ExpandListArgument(iconfigs, availableConfigs);
    }
    for (std::vector<std::string>::const_iterator aci =
           availableConfigs.begin();
         !*loc && !*imp && aci != availableConfigs.end(); ++aci) {
      suffix = "_";
      suffix += cmSystemTools::UpperCase(*aci);
      std::string locProp = locPropBase;
      locProp += suffix;
      *loc = this->GetProperty(locProp);
      if (allowImp) {
        std::string impProp = "IMPORTED_IMPLIB";
        impProp += suffix;
        *imp = this->GetProperty(impProp);
      }
    }
  }
  // If we have not yet found it then the target location is not available.
  if (!*loc && !*imp) {
    // Interface libraries are always available because their
    // library name is optional so it is okay to leave *loc empty.
    return this->GetType() == cmStateEnums::INTERFACE_LIBRARY;
  }

  return true;
}

cmTargetInternalPointer::cmTargetInternalPointer()
{
  this->Pointer = new cmTargetInternals;
}

cmTargetInternalPointer::cmTargetInternalPointer(
  cmTargetInternalPointer const& r)
{
  // Ideally cmTarget instances should never be copied.  However until
  // we can make a sweep to remove that, this copy constructor avoids
  // allowing the resources (Internals) to be copied.
  this->Pointer = new cmTargetInternals(*r.Pointer);
}

cmTargetInternalPointer::~cmTargetInternalPointer()
{
  delete this->Pointer;
}

cmTargetInternalPointer& cmTargetInternalPointer::operator=(
  cmTargetInternalPointer const& r)
{
  if (this == &r) {
    return *this;
  } // avoid warning on HP about self check
  // Ideally cmTarget instances should never be copied.  However until
  // we can make a sweep to remove that, this copy constructor avoids
  // allowing the resources (Internals) to be copied.
  cmTargetInternals* oldPointer = this->Pointer;
  this->Pointer = new cmTargetInternals(*r.Pointer);
  delete oldPointer;
  return *this;
}
