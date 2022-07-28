/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTarget.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <unordered_set>

#include <cm/memory>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmAlgorithms.h"
#include "cmCustomCommand.h"
#include "cmFileSet.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmProperty.h"
#include "cmPropertyDefinition.h"
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
#include "cmValue.h"
#include "cmake.h"

template <>
const std::string& cmTargetPropertyComputer::ComputeLocationForBuild<cmTarget>(
  cmTarget const* tgt)
{
  static std::string loc;
  if (tgt->IsImported()) {
    loc = tgt->ImportedGetFullPath("", cmStateEnums::RuntimeBinaryArtifact);
    return loc;
  }

  cmGlobalGenerator* gg = tgt->GetGlobalGenerator();
  if (!gg->GetConfigureDoneCMP0026()) {
    gg->CreateGenerationObjects();
  }
  cmGeneratorTarget* gt = gg->FindGeneratorTarget(tgt->GetName());
  loc = gt->GetLocationForBuild();
  return loc;
}

template <>
const std::string& cmTargetPropertyComputer::ComputeLocation<cmTarget>(
  cmTarget const* tgt, const std::string& config)
{
  static std::string loc;
  if (tgt->IsImported()) {
    loc =
      tgt->ImportedGetFullPath(config, cmStateEnums::RuntimeBinaryArtifact);
    return loc;
  }

  cmGlobalGenerator* gg = tgt->GetGlobalGenerator();
  if (!gg->GetConfigureDoneCMP0026()) {
    gg->CreateGenerationObjects();
  }
  cmGeneratorTarget* gt = gg->FindGeneratorTarget(tgt->GetName());
  loc = gt->GetFullPath(config, cmStateEnums::RuntimeBinaryArtifact);
  return loc;
}

template <>
cmValue cmTargetPropertyComputer::GetSources<cmTarget>(cmTarget const* tgt,
                                                       cmMakefile const& mf)
{
  cmBTStringRange entries = tgt->GetSourceEntries();
  if (entries.empty()) {
    return nullptr;
  }

  std::ostringstream ss;
  const char* sep = "";
  for (auto const& entry : entries) {
    std::vector<std::string> files = cmExpandedList(entry.Value);
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
        switch (mf.GetPolicyStatus(cmPolicies::CMP0051)) {
          case cmPolicies::WARN:
            e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0051) << "\n";
            noMessage = false;
            CM_FALLTHROUGH;
          case cmPolicies::OLD:
            break;
          case cmPolicies::REQUIRED_ALWAYS:
          case cmPolicies::REQUIRED_IF_USED:
          case cmPolicies::NEW:
            addContent = true;
            break;
        }
        if (!noMessage) {
          e << "Target \"" << tgt->GetName()
            << "\" contains $<TARGET_OBJECTS> generator expression in its "
               "sources list.  This content was not previously part of the "
               "SOURCES property when that property was read at configure "
               "time.  Code reading that property needs to be adapted to "
               "ignore the generator expression using the string(GENEX_STRIP) "
               "command.";
          mf.IssueMessage(messageType, e.str());
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
  return cmValue(srcs);
}

namespace {
struct FileSetEntries
{
  FileSetEntries(cm::static_string_view propertyName)
    : PropertyName(propertyName)
  {
  }

  cm::static_string_view const PropertyName;
  std::vector<BT<std::string>> Entries;
};

struct FileSetType
{
  FileSetType(cm::static_string_view typeName,
              cm::static_string_view defaultDirectoryProperty,
              cm::static_string_view defaultPathProperty,
              cm::static_string_view directoryPrefix,
              cm::static_string_view pathPrefix,
              cm::static_string_view typeDescription,
              cm::static_string_view defaultDescription,
              cm::static_string_view arbitraryDescription,
              FileSetEntries selfEntries, FileSetEntries interfaceEntries)
    : TypeName(typeName)
    , DefaultDirectoryProperty(defaultDirectoryProperty)
    , DefaultPathProperty(defaultPathProperty)
    , DirectoryPrefix(directoryPrefix)
    , PathPrefix(pathPrefix)
    , TypeDescription(typeDescription)
    , DefaultDescription(defaultDescription)
    , ArbitraryDescription(arbitraryDescription)
    , SelfEntries(std::move(selfEntries))
    , InterfaceEntries(std::move(interfaceEntries))
  {
  }

  cm::static_string_view const TypeName;
  cm::static_string_view const DefaultDirectoryProperty;
  cm::static_string_view const DefaultPathProperty;
  cm::static_string_view const DirectoryPrefix;
  cm::static_string_view const PathPrefix;
  cm::static_string_view const TypeDescription;
  cm::static_string_view const DefaultDescription;
  cm::static_string_view const ArbitraryDescription;

  FileSetEntries SelfEntries;
  FileSetEntries InterfaceEntries;

  template <typename ValueType>
  bool WriteProperties(cmTarget* tgt, cmTargetInternals* impl,
                       const std::string& prop, ValueType value, bool clear);
  std::pair<bool, cmValue> ReadProperties(cmTarget const* tgt,
                                          cmTargetInternals const* impl,
                                          const std::string& prop) const;

  void AddFileSet(const std::string& name, cmFileSetVisibility vis,
                  cmListFileBacktrace bt);
};
}

class cmTargetInternals
{
public:
  cmStateEnums::TargetType TargetType;
  cmMakefile* Makefile;
  cmPolicies::PolicyMap PolicyMap;
  std::string Name;
  std::string InstallPath;
  std::string RuntimeInstallPath;
  cmPropertyMap Properties;
  bool IsGeneratorProvided;
  bool HaveInstallRule;
  bool IsDLLPlatform;
  bool IsAIX;
  bool IsAndroid;
  bool IsImportedTarget;
  bool ImportedGloballyVisible;
  bool BuildInterfaceIncludesAppended;
  bool PerConfig;
  std::set<BT<std::pair<std::string, bool>>> Utilities;
  std::vector<cmCustomCommand> PreBuildCommands;
  std::vector<cmCustomCommand> PreLinkCommands;
  std::vector<cmCustomCommand> PostBuildCommands;
  std::vector<cmInstallTargetGenerator*> InstallGenerators;
  std::set<std::string> SystemIncludeDirectories;
  cmTarget::LinkLibraryVectorType OriginalLinkLibraries;
  std::map<std::string, BTs<std::string>> LanguageStandardProperties;
  std::vector<BT<std::string>> IncludeDirectoriesEntries;
  std::map<cmTargetExport const*, std::vector<std::string>>
    InstallIncludeDirectoriesEntries;
  std::vector<BT<std::string>> CompileOptionsEntries;
  std::vector<BT<std::string>> CompileFeaturesEntries;
  std::vector<BT<std::string>> CompileDefinitionsEntries;
  std::vector<BT<std::string>> PrecompileHeadersEntries;
  std::vector<BT<std::string>> SourceEntries;
  std::vector<BT<std::string>> LinkOptionsEntries;
  std::vector<BT<std::string>> LinkDirectoriesEntries;
  std::vector<BT<std::string>> LinkImplementationPropertyEntries;
  std::vector<BT<std::string>> LinkInterfacePropertyEntries;
  std::vector<BT<std::string>> LinkInterfaceDirectPropertyEntries;
  std::vector<BT<std::string>> LinkInterfaceDirectExcludePropertyEntries;
  std::vector<std::pair<cmTarget::TLLSignature, cmListFileContext>>
    TLLCommands;
  std::map<std::string, cmFileSet> FileSets;
  cmListFileBacktrace Backtrace;

  FileSetType HeadersFileSets;

  cmTargetInternals();

  bool CheckImportedLibName(std::string const& prop,
                            std::string const& value) const;

  std::string ProcessSourceItemCMP0049(const std::string& s) const;

  template <typename ValueType>
  void AddDirectoryToFileSet(cmTarget* self, std::string const& fileSetName,
                             ValueType value, cm::string_view fileSetType,
                             cm::string_view description, bool clear);
  template <typename ValueType>
  void AddPathToFileSet(cmTarget* self, std::string const& fileSetName,
                        ValueType value, cm::string_view fileSetType,
                        cm::string_view description, bool clear);
  cmValue GetFileSetDirectories(cmTarget const* self,
                                std::string const& fileSetName,
                                cm::string_view fileSetType) const;
  cmValue GetFileSetPaths(cmTarget const* self, std::string const& fileSetName,
                          cm::string_view fileSetType) const;
};

cmTargetInternals::cmTargetInternals()
  : HeadersFileSets("HEADERS"_s, "HEADER_DIRS"_s, "HEADER_SET"_s,
                    "HEADER_DIRS_"_s, "HEADER_SET_"_s, "Header"_s,
                    "The default header set"_s, "Header set"_s,
                    FileSetEntries("HEADER_SETS"_s),
                    FileSetEntries("INTERFACE_HEADER_SETS"_s))
{
}

template <typename ValueType>
bool FileSetType::WriteProperties(cmTarget* tgt, cmTargetInternals* impl,
                                  const std::string& prop, ValueType value,
                                  bool clear)
{
  if (prop == this->DefaultDirectoryProperty) {
    impl->AddDirectoryToFileSet(tgt, std::string(this->TypeName), value,
                                this->TypeName, this->DefaultDescription,
                                clear);
    return true;
  }
  if (prop == this->DefaultPathProperty) {
    impl->AddPathToFileSet(tgt, std::string(this->TypeName), value,
                           this->TypeName, this->DefaultDescription, clear);
    return true;
  }
  if (cmHasPrefix(prop, this->DirectoryPrefix)) {
    auto fileSetName = prop.substr(this->DirectoryPrefix.size());
    if (fileSetName.empty()) {
      impl->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat(this->ArbitraryDescription, " name cannot be empty."));
    } else {
      impl->AddDirectoryToFileSet(
        tgt, fileSetName, value, this->TypeName,
        cmStrCat(this->ArbitraryDescription, " \"", fileSetName, "\""), clear);
    }
    return true;
  }
  if (cmHasPrefix(prop, this->PathPrefix)) {
    auto fileSetName = prop.substr(this->PathPrefix.size());
    if (fileSetName.empty()) {
      impl->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat(this->ArbitraryDescription, " name cannot be empty."));
    } else {
      impl->AddPathToFileSet(
        tgt, fileSetName, value, this->TypeName,
        cmStrCat(this->ArbitraryDescription, " \"", fileSetName, "\""), clear);
    }
    return true;
  }
  if (prop == this->SelfEntries.PropertyName) {
    impl->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat(this->SelfEntries.PropertyName, " property is read-only\n"));
    return true;
  }
  if (prop == this->InterfaceEntries.PropertyName) {
    impl->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                 cmStrCat(this->InterfaceEntries.PropertyName,
                                          " property is read-only\n"));
    return true;
  }
  return false;
}

std::pair<bool, cmValue> FileSetType::ReadProperties(
  cmTarget const* tgt, cmTargetInternals const* impl,
  const std::string& prop) const
{
  bool did_read = false;
  cmValue value = nullptr;
  if (prop == this->DefaultDirectoryProperty) {
    value = impl->GetFileSetDirectories(tgt, std::string(this->TypeName),
                                        this->TypeName);
    did_read = true;
  } else if (prop == this->DefaultPathProperty) {
    value =
      impl->GetFileSetPaths(tgt, std::string(this->TypeName), this->TypeName);
    did_read = true;
  } else if (prop == this->SelfEntries.PropertyName) {
    static std::string output;
    output = cmJoin(this->SelfEntries.Entries, ";"_s);
    value = cmValue(output);
    did_read = true;
  } else if (prop == this->InterfaceEntries.PropertyName) {
    static std::string output;
    output = cmJoin(this->InterfaceEntries.Entries, ";"_s);
    value = cmValue(output);
    did_read = true;
  } else if (cmHasPrefix(prop, this->DirectoryPrefix)) {
    std::string fileSetName = prop.substr(this->DirectoryPrefix.size());
    if (!fileSetName.empty()) {
      value = impl->GetFileSetDirectories(tgt, fileSetName, this->TypeName);
    }
    did_read = true;
  } else if (cmHasPrefix(prop, this->PathPrefix)) {
    std::string fileSetName = prop.substr(this->PathPrefix.size());
    if (!fileSetName.empty()) {
      value = impl->GetFileSetPaths(tgt, fileSetName, this->TypeName);
    }
    did_read = true;
  }
  return { did_read, value };
}

void FileSetType::AddFileSet(const std::string& name, cmFileSetVisibility vis,
                             cmListFileBacktrace bt)
{
  if (cmFileSetVisibilityIsForSelf(vis)) {
    this->SelfEntries.Entries.emplace_back(name, bt);
  }
  if (cmFileSetVisibilityIsForInterface(vis)) {
    this->InterfaceEntries.Entries.emplace_back(name, std::move(bt));
  }
}

namespace {
#define SETUP_COMMON_LANGUAGE_PROPERTIES(lang)                                \
  initProp(#lang "_COMPILER_LAUNCHER");                                       \
  initProp(#lang "_STANDARD");                                                \
  initProp(#lang "_STANDARD_REQUIRED");                                       \
  initProp(#lang "_EXTENSIONS");                                              \
  initProp(#lang "_VISIBILITY_PRESET")
}

cmTarget::cmTarget(std::string const& name, cmStateEnums::TargetType type,
                   Visibility vis, cmMakefile* mf, PerConfig perConfig)
  : impl(cm::make_unique<cmTargetInternals>())
{
  assert(mf);
  this->impl->TargetType = type;
  this->impl->Makefile = mf;
  this->impl->Name = name;
  this->impl->IsGeneratorProvided = false;
  this->impl->HaveInstallRule = false;
  this->impl->IsDLLPlatform = false;
  this->impl->IsAIX = false;
  this->impl->IsAndroid = false;
  this->impl->IsImportedTarget =
    (vis == VisibilityImported || vis == VisibilityImportedGlobally);
  this->impl->ImportedGloballyVisible = vis == VisibilityImportedGlobally;
  this->impl->BuildInterfaceIncludesAppended = false;
  this->impl->PerConfig = (perConfig == PerConfig::Yes);

  // Check whether this is a DLL platform.
  this->impl->IsDLLPlatform =
    !this->impl->Makefile->GetSafeDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX")
       .empty();

  // Check whether we are targeting AIX.
  {
    std::string const& systemName =
      this->impl->Makefile->GetSafeDefinition("CMAKE_SYSTEM_NAME");
    this->impl->IsAIX = (systemName == "AIX" || systemName == "OS400");
  }

  // Check whether we are targeting an Android platform.
  this->impl->IsAndroid = (this->impl->Makefile->GetSafeDefinition(
                             "CMAKE_SYSTEM_NAME") == "Android");

  std::string defKey;
  defKey.reserve(128);
  defKey += "CMAKE_";
  auto initProp = [this, mf, &defKey](const std::string& property) {
    // Replace everything after "CMAKE_"
    defKey.replace(defKey.begin() + 6, defKey.end(), property);
    if (cmValue value = mf->GetDefinition(defKey)) {
      this->SetProperty(property, value);
    }
  };
  auto initPropValue = [this, mf, &defKey](const std::string& property,
                                           const char* default_value) {
    // Replace everything after "CMAKE_"
    defKey.replace(defKey.begin() + 6, defKey.end(), property);
    if (cmValue value = mf->GetDefinition(defKey)) {
      this->SetProperty(property, value);
    } else if (default_value) {
      this->SetProperty(property, default_value);
    }
  };

  // Setup default property values.
  if (this->CanCompileSources()) {

    SETUP_COMMON_LANGUAGE_PROPERTIES(C);
    SETUP_COMMON_LANGUAGE_PROPERTIES(OBJC);
    SETUP_COMMON_LANGUAGE_PROPERTIES(CXX);
    SETUP_COMMON_LANGUAGE_PROPERTIES(OBJCXX);
    SETUP_COMMON_LANGUAGE_PROPERTIES(CUDA);
    SETUP_COMMON_LANGUAGE_PROPERTIES(HIP);

    initProp("ANDROID_API");
    initProp("ANDROID_API_MIN");
    initProp("ANDROID_ARCH");
    initProp("ANDROID_STL_TYPE");
    initProp("ANDROID_SKIP_ANT_STEP");
    initProp("ANDROID_PROCESS_MAX");
    initProp("ANDROID_PROGUARD");
    initProp("ANDROID_PROGUARD_CONFIG_PATH");
    initProp("ANDROID_SECURE_PROPS_PATH");
    initProp("ANDROID_NATIVE_LIB_DIRECTORIES");
    initProp("ANDROID_NATIVE_LIB_DEPENDENCIES");
    initProp("ANDROID_JAVA_SOURCE_DIR");
    initProp("ANDROID_JAR_DIRECTORIES");
    initProp("ANDROID_JAR_DEPENDENCIES");
    initProp("ANDROID_ASSETS_DIRECTORIES");
    initProp("ANDROID_ANT_ADDITIONAL_OPTIONS");
    initProp("BUILD_RPATH");
    initProp("BUILD_RPATH_USE_ORIGIN");
    initProp("INSTALL_NAME_DIR");
    initProp("INSTALL_REMOVE_ENVIRONMENT_RPATH");
    initPropValue("INSTALL_RPATH", "");
    initPropValue("INSTALL_RPATH_USE_LINK_PATH", "OFF");
    initProp("INTERPROCEDURAL_OPTIMIZATION");
    initPropValue("SKIP_BUILD_RPATH", "OFF");
    initPropValue("BUILD_WITH_INSTALL_RPATH", "OFF");
    initProp("ARCHIVE_OUTPUT_DIRECTORY");
    initProp("LIBRARY_OUTPUT_DIRECTORY");
    initProp("RUNTIME_OUTPUT_DIRECTORY");
    initProp("PDB_OUTPUT_DIRECTORY");
    initProp("COMPILE_PDB_OUTPUT_DIRECTORY");
    initProp("FRAMEWORK");
    initProp("FRAMEWORK_MULTI_CONFIG_POSTFIX");
    initProp("Fortran_FORMAT");
    initProp("Fortran_MODULE_DIRECTORY");
    initProp("Fortran_COMPILER_LAUNCHER");
    initProp("Fortran_PREPROCESS");
    initProp("Fortran_VISIBILITY_PRESET");
    initProp("GNUtoMS");
    initProp("OSX_ARCHITECTURES");
    initProp("IOS_INSTALL_COMBINED");
    initProp("AUTOMOC");
    initProp("AUTOUIC");
    initProp("AUTORCC");
    initProp("AUTOGEN_ORIGIN_DEPENDS");
    initProp("AUTOGEN_PARALLEL");
    initProp("AUTOMOC_COMPILER_PREDEFINES");
    initProp("AUTOMOC_DEPEND_FILTERS");
    initProp("AUTOMOC_MACRO_NAMES");
    initProp("AUTOMOC_MOC_OPTIONS");
    initProp("AUTOUIC_OPTIONS");
    initProp("AUTOMOC_PATH_PREFIX");
    initProp("AUTOUIC_SEARCH_PATHS");
    initProp("AUTORCC_OPTIONS");
    initProp("LINK_DEPENDS_NO_SHARED");
    initProp("LINK_INTERFACE_LIBRARIES");
    initProp("MSVC_RUNTIME_LIBRARY");
    initProp("WATCOM_RUNTIME_LIBRARY");
    initProp("WIN32_EXECUTABLE");
    initProp("MACOSX_BUNDLE");
    initProp("MACOSX_RPATH");
    initProp("NO_SYSTEM_FROM_IMPORTED");
    initProp("BUILD_WITH_INSTALL_NAME_DIR");
    initProp("C_CLANG_TIDY");
    initProp("C_CPPLINT");
    initProp("C_CPPCHECK");
    initProp("C_INCLUDE_WHAT_YOU_USE");
    initProp("C_LINKER_LAUNCHER");
    initProp("LINK_WHAT_YOU_USE");
    initProp("CXX_CLANG_TIDY");
    initProp("CXX_CPPLINT");
    initProp("CXX_CPPCHECK");
    initProp("CXX_INCLUDE_WHAT_YOU_USE");
    initProp("CXX_LINKER_LAUNCHER");
    initProp("CUDA_SEPARABLE_COMPILATION");
    initProp("CUDA_RESOLVE_DEVICE_SYMBOLS");
    initProp("CUDA_RUNTIME_LIBRARY");
    initProp("CUDA_ARCHITECTURES");
    initProp("HIP_RUNTIME_LIBRARY");
    initProp("HIP_ARCHITECTURES");
    initProp("VISIBILITY_INLINES_HIDDEN");
    initProp("JOB_POOL_COMPILE");
    initProp("JOB_POOL_LINK");
    initProp("JOB_POOL_PRECOMPILE_HEADER");
    initProp("ISPC_COMPILER_LAUNCHER");
    initProp("ISPC_HEADER_DIRECTORY");
    initPropValue("ISPC_HEADER_SUFFIX", "_ispc.h");
    initProp("ISPC_INSTRUCTION_SETS");
    initProp("LINK_SEARCH_START_STATIC");
    initProp("LINK_SEARCH_END_STATIC");
    initProp("OBJC_CLANG_TIDY");
    initProp("OBJC_LINKER_LAUNCHER");
    initProp("OBJCXX_CLANG_TIDY");
    initProp("OBJCXX_LINKER_LAUNCHER");
    initProp("Swift_LANGUAGE_VERSION");
    initProp("Swift_MODULE_DIRECTORY");
    initProp("VS_JUST_MY_CODE_DEBUGGING");
    initProp("VS_NO_COMPILE_BATCHING");
    initProp("DISABLE_PRECOMPILE_HEADERS");
    initProp("UNITY_BUILD");
    initProp("UNITY_BUILD_UNIQUE_ID");
    initProp("OPTIMIZE_DEPENDENCIES");
    initProp("EXPORT_COMPILE_COMMANDS");
    initProp("COMPILE_WARNING_AS_ERROR");
    initPropValue("UNITY_BUILD_BATCH_SIZE", "8");
    initPropValue("UNITY_BUILD_MODE", "BATCH");
    initPropValue("PCH_WARN_INVALID", "ON");
    initPropValue("PCH_INSTANTIATE_TEMPLATES", "ON");

#ifdef __APPLE__
    if (this->GetGlobalGenerator()->IsXcode()) {
      initProp("XCODE_SCHEME_ADDRESS_SANITIZER");
      initProp("XCODE_SCHEME_ADDRESS_SANITIZER_USE_AFTER_RETURN");
      initProp("XCODE_SCHEME_DEBUG_DOCUMENT_VERSIONING");
      initProp("XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE");
      initProp("XCODE_SCHEME_THREAD_SANITIZER");
      initProp("XCODE_SCHEME_THREAD_SANITIZER_STOP");
      initProp("XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER");
      initProp("XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER_STOP");
      initProp("XCODE_SCHEME_WORKING_DIRECTORY");
      initProp("XCODE_SCHEME_DISABLE_MAIN_THREAD_CHECKER");
      initProp("XCODE_SCHEME_MAIN_THREAD_CHECKER_STOP");
      initProp("XCODE_SCHEME_MALLOC_SCRIBBLE");
      initProp("XCODE_SCHEME_MALLOC_GUARD_EDGES");
      initProp("XCODE_SCHEME_GUARD_MALLOC");
      initProp("XCODE_SCHEME_ZOMBIE_OBJECTS");
      initProp("XCODE_SCHEME_MALLOC_STACK");
      initProp("XCODE_SCHEME_DYNAMIC_LINKER_API_USAGE");
      initProp("XCODE_SCHEME_DYNAMIC_LIBRARY_LOADS");
      initProp("XCODE_SCHEME_ENVIRONMENT");
      initPropValue("XCODE_LINK_BUILD_PHASE_MODE", "NONE");
    }
#endif
  }

  initProp("FOLDER");
  initProp("VERIFY_INTERFACE_HEADER_SETS");

  if (this->GetGlobalGenerator()->IsXcode()) {
    initProp("XCODE_GENERATE_SCHEME");
  }

  // Setup per-configuration property default values.
  if (this->GetType() != cmStateEnums::UTILITY &&
      this->GetType() != cmStateEnums::GLOBAL_TARGET) {
    static const auto configProps = {
      /* clang-format needs this comment to break after the opening brace */
      "ARCHIVE_OUTPUT_DIRECTORY_",     "LIBRARY_OUTPUT_DIRECTORY_",
      "RUNTIME_OUTPUT_DIRECTORY_",     "PDB_OUTPUT_DIRECTORY_",
      "COMPILE_PDB_OUTPUT_DIRECTORY_", "MAP_IMPORTED_CONFIG_",
      "INTERPROCEDURAL_OPTIMIZATION_"
    };
    // Collect the set of configuration types.
    std::vector<std::string> configNames =
      mf->GetGeneratorConfigs(cmMakefile::ExcludeEmptyConfig);
    for (std::string const& configName : configNames) {
      std::string configUpper = cmSystemTools::UpperCase(configName);
      for (auto const& prop : configProps) {
        // Interface libraries have no output locations, so honor only
        // the configuration map.
        if (this->impl->TargetType == cmStateEnums::INTERFACE_LIBRARY &&
            strcmp(prop, "MAP_IMPORTED_CONFIG_") != 0) {
          continue;
        }
        std::string property = cmStrCat(prop, configUpper);
        initProp(property);
      }

      // Initialize per-configuration name postfix property from the
      // variable only for non-executable targets.  This preserves
      // compatibility with previous CMake versions in which executables
      // did not support this variable.  Projects may still specify the
      // property directly.
      if (this->impl->TargetType != cmStateEnums::EXECUTABLE &&
          this->impl->TargetType != cmStateEnums::INTERFACE_LIBRARY) {
        std::string property =
          cmStrCat(cmSystemTools::UpperCase(configName), "_POSTFIX");
        initProp(property);
      }

      if (this->impl->TargetType == cmStateEnums::SHARED_LIBRARY ||
          this->impl->TargetType == cmStateEnums::STATIC_LIBRARY) {
        std::string property = cmStrCat("FRAMEWORK_MULTI_CONFIG_POSTFIX_",
                                        cmSystemTools::UpperCase(configName));
        initProp(property);
      }
    }
    if (!this->IsImported()) {
      initProp("LINK_LIBRARIES_ONLY_TARGETS");
    }
  }

  // Save the backtrace of target construction.
  this->impl->Backtrace = this->impl->Makefile->GetBacktrace();

  if (!this->IsImported()) {
    // Initialize the INCLUDE_DIRECTORIES property based on the current value
    // of the same directory property:
    cm::append(this->impl->IncludeDirectoriesEntries,
               this->impl->Makefile->GetIncludeDirectoriesEntries());

    {
      auto const& sysInc = this->impl->Makefile->GetSystemIncludeDirectories();
      this->impl->SystemIncludeDirectories.insert(sysInc.begin(),
                                                  sysInc.end());
    }

    cm::append(this->impl->CompileOptionsEntries,
               this->impl->Makefile->GetCompileOptionsEntries());

    cm::append(this->impl->LinkOptionsEntries,
               this->impl->Makefile->GetLinkOptionsEntries());

    cm::append(this->impl->LinkDirectoriesEntries,
               this->impl->Makefile->GetLinkDirectoriesEntries());
  }

  if (this->impl->TargetType == cmStateEnums::EXECUTABLE) {
    initProp("ANDROID_GUI");
    initProp("CROSSCOMPILING_EMULATOR");
    initProp("ENABLE_EXPORTS");
  }
  if (this->impl->TargetType == cmStateEnums::SHARED_LIBRARY ||
      this->impl->TargetType == cmStateEnums::MODULE_LIBRARY) {
    this->SetProperty("POSITION_INDEPENDENT_CODE", "True");
  } else if (this->CanCompileSources()) {
    initProp("POSITION_INDEPENDENT_CODE");
  }
  if (this->impl->TargetType == cmStateEnums::SHARED_LIBRARY ||
      this->impl->TargetType == cmStateEnums::EXECUTABLE) {
    initProp("AIX_EXPORT_ALL_SYMBOLS");
    initProp("WINDOWS_EXPORT_ALL_SYMBOLS");
  }

  // Record current policies for later use.
  this->impl->Makefile->RecordPolicies(this->impl->PolicyMap);

  if (this->impl->TargetType == cmStateEnums::INTERFACE_LIBRARY) {
    // This policy is checked in a few conditions. The properties relevant
    // to the policy are always ignored for cmStateEnums::INTERFACE_LIBRARY
    // targets,
    // so ensure that the conditions don't lead to nonsense.
    this->impl->PolicyMap.Set(cmPolicies::CMP0022, cmPolicies::NEW);
  }

  if (!this->IsImported()) {
    initProp("DOTNET_SDK");
  }

  if (this->impl->TargetType <= cmStateEnums::GLOBAL_TARGET) {
    initProp("DOTNET_TARGET_FRAMEWORK");
    initProp("DOTNET_TARGET_FRAMEWORK_VERSION");
  }

  // check for "CMAKE_VS_GLOBALS" variable and set up target properties
  // if any
  cmValue globals = mf->GetDefinition("CMAKE_VS_GLOBALS");
  if (globals) {
    const std::string genName = mf->GetGlobalGenerator()->GetName();
    if (cmHasLiteralPrefix(genName, "Visual Studio")) {
      std::vector<std::string> props = cmExpandedList(*globals);
      const std::string vsGlobal = "VS_GLOBAL_";
      for (const std::string& i : props) {
        // split NAME=VALUE
        const std::string::size_type assignment = i.find('=');
        if (assignment != std::string::npos) {
          const std::string propName = vsGlobal + i.substr(0, assignment);
          const std::string propValue = i.substr(assignment + 1);
          initPropValue(propName, propValue.c_str());
        }
      }
    }
  }

  for (auto const& prop : mf->GetState()->GetPropertyDefinitions().GetMap()) {
    if (prop.first.second == cmProperty::TARGET &&
        !prop.second.GetInitializeFromVariable().empty()) {
      if (auto value =
            mf->GetDefinition(prop.second.GetInitializeFromVariable())) {
        this->SetProperty(prop.first.first, value);
      }
    }
  }
}

cmTarget::cmTarget(cmTarget&&) noexcept = default;
cmTarget::~cmTarget() = default;

cmTarget& cmTarget::operator=(cmTarget&&) noexcept = default;

cmStateEnums::TargetType cmTarget::GetType() const
{
  return this->impl->TargetType;
}

cmMakefile* cmTarget::GetMakefile() const
{
  return this->impl->Makefile;
}

cmPolicies::PolicyMap const& cmTarget::GetPolicyMap() const
{
  return this->impl->PolicyMap;
}

const std::string& cmTarget::GetName() const
{
  return this->impl->Name;
}

cmPolicies::PolicyStatus cmTarget::GetPolicyStatus(
  cmPolicies::PolicyID policy) const
{
  return this->impl->PolicyMap.Get(policy);
}

cmGlobalGenerator* cmTarget::GetGlobalGenerator() const
{
  return this->impl->Makefile->GetGlobalGenerator();
}

BTs<std::string> const* cmTarget::GetLanguageStandardProperty(
  const std::string& propertyName) const
{
  auto entry = this->impl->LanguageStandardProperties.find(propertyName);
  if (entry != this->impl->LanguageStandardProperties.end()) {
    return &entry->second;
  }

  return nullptr;
}

void cmTarget::SetLanguageStandardProperty(std::string const& lang,
                                           std::string const& value,
                                           const std::string& feature)
{
  cmListFileBacktrace featureBacktrace;
  for (auto const& entry : this->impl->CompileFeaturesEntries) {
    if (entry.Value == feature) {
      featureBacktrace = entry.Backtrace;
      break;
    }
  }

  BTs<std::string>& languageStandardProperty =
    this->impl->LanguageStandardProperties[cmStrCat(lang, "_STANDARD")];
  if (languageStandardProperty.Value != value) {
    languageStandardProperty.Value = value;
    languageStandardProperty.Backtraces.clear();
  }
  languageStandardProperty.Backtraces.emplace_back(featureBacktrace);
}

void cmTarget::AddUtility(std::string const& name, bool cross, cmMakefile* mf)
{
  this->impl->Utilities.insert(BT<std::pair<std::string, bool>>(
    { name, cross }, mf ? mf->GetBacktrace() : cmListFileBacktrace()));
}

void cmTarget::AddUtility(BT<std::pair<std::string, bool>> util)
{
  this->impl->Utilities.emplace(std::move(util));
}

std::set<BT<std::pair<std::string, bool>>> const& cmTarget::GetUtilities()
  const
{
  return this->impl->Utilities;
}

cmListFileBacktrace const& cmTarget::GetBacktrace() const
{
  return this->impl->Backtrace;
}

bool cmTarget::IsExecutableWithExports() const
{
  return (this->GetType() == cmStateEnums::EXECUTABLE &&
          this->GetPropertyAsBool("ENABLE_EXPORTS"));
}

bool cmTarget::IsFrameworkOnApple() const
{
  return ((this->GetType() == cmStateEnums::SHARED_LIBRARY ||
           this->GetType() == cmStateEnums::STATIC_LIBRARY) &&
          this->impl->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("FRAMEWORK"));
}

bool cmTarget::IsAppBundleOnApple() const
{
  return (this->GetType() == cmStateEnums::EXECUTABLE &&
          this->impl->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("MACOSX_BUNDLE"));
}

bool cmTarget::IsAndroidGuiExecutable() const
{
  return (this->GetType() == cmStateEnums::EXECUTABLE &&
          this->impl->IsAndroid && this->GetPropertyAsBool("ANDROID_GUI"));
}

bool cmTarget::HasKnownObjectFileLocation(std::string* reason) const
{
  return this->GetGlobalGenerator()->HasKnownObjectFileLocation(*this, reason);
}

std::vector<cmCustomCommand> const& cmTarget::GetPreBuildCommands() const
{
  return this->impl->PreBuildCommands;
}

void cmTarget::AddPreBuildCommand(cmCustomCommand const& cmd)
{
  this->impl->PreBuildCommands.push_back(cmd);
}

void cmTarget::AddPreBuildCommand(cmCustomCommand&& cmd)
{
  this->impl->PreBuildCommands.push_back(std::move(cmd));
}

std::vector<cmCustomCommand> const& cmTarget::GetPreLinkCommands() const
{
  return this->impl->PreLinkCommands;
}

void cmTarget::AddPreLinkCommand(cmCustomCommand const& cmd)
{
  this->impl->PreLinkCommands.push_back(cmd);
}

void cmTarget::AddPreLinkCommand(cmCustomCommand&& cmd)
{
  this->impl->PreLinkCommands.push_back(std::move(cmd));
}

std::vector<cmCustomCommand> const& cmTarget::GetPostBuildCommands() const
{
  return this->impl->PostBuildCommands;
}

void cmTarget::AddPostBuildCommand(cmCustomCommand const& cmd)
{
  this->impl->PostBuildCommands.push_back(cmd);
}

void cmTarget::AddPostBuildCommand(cmCustomCommand&& cmd)
{
  this->impl->PostBuildCommands.push_back(std::move(cmd));
}

void cmTarget::AddTracedSources(std::vector<std::string> const& srcs)
{
  if (!srcs.empty()) {
    cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
    this->impl->SourceEntries.emplace_back(cmJoin(srcs, ";"), lfbt);
  }
}

void cmTarget::AddSources(std::vector<std::string> const& srcs)
{
  std::string srcFiles;
  const char* sep = "";
  for (auto filename : srcs) {
    if (!cmGeneratorExpression::StartsWithGeneratorExpression(filename)) {
      if (!filename.empty()) {
        filename = this->impl->ProcessSourceItemCMP0049(filename);
        if (filename.empty()) {
          return;
        }
      }
      this->impl->Makefile->GetOrCreateSource(filename);
    }
    srcFiles += sep;
    srcFiles += filename;
    sep = ";";
  }
  if (!srcFiles.empty()) {
    cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
    this->impl->SourceEntries.emplace_back(std::move(srcFiles), lfbt);
  }
}

std::string cmTargetInternals::ProcessSourceItemCMP0049(
  const std::string& s) const
{
  std::string src = s;

  // For backwards compatibility replace variables in source names.
  // This should eventually be removed.
  this->Makefile->ExpandVariablesInString(src);
  if (src != s) {
    std::ostringstream e;
    bool noMessage = false;
    MessageType messageType = MessageType::AUTHOR_WARNING;
    switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0049)) {
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
        << "\" expanded to \"" << src << "\" in target \"" << this->Name
        << "\".  This behavior will be removed in a "
           "future version of CMake.";
      this->Makefile->IssueMessage(messageType, e.str());
      if (messageType == MessageType::FATAL_ERROR) {
        return "";
      }
    }
  }
  return src;
}

std::string cmTarget::GetSourceCMP0049(const std::string& s)
{
  return this->impl->ProcessSourceItemCMP0049(s);
}

struct CreateLocation
{
  cmMakefile const* Makefile;

  CreateLocation(cmMakefile const* mf)
    : Makefile(mf)
  {
  }

  cmSourceFileLocation operator()(const std::string& filename) const
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

  bool operator()(BT<std::string> const& entry)
  {
    std::vector<std::string> files = cmExpandedList(entry.Value);
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
  cmSourceFileLocation sfl(this->impl->Makefile, src,
                           cmSourceFileLocationKind::Known);
  if (std::find_if(
        this->impl->SourceEntries.begin(), this->impl->SourceEntries.end(),
        TargetPropertyEntryFinder(sfl)) == this->impl->SourceEntries.end()) {
    cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
    this->impl->SourceEntries.insert(before ? this->impl->SourceEntries.begin()
                                            : this->impl->SourceEntries.end(),
                                     BT<std::string>(src, lfbt));
  }
  if (cmGeneratorExpression::Find(src) != std::string::npos) {
    return nullptr;
  }
  return this->impl->Makefile->GetOrCreateSource(
    src, false, cmSourceFileLocationKind::Known);
}

void cmTarget::ClearDependencyInformation(cmMakefile& mf) const
{
  std::string depname = cmStrCat(this->GetName(), "_LIB_DEPENDS");
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
    this->impl->Makefile->GetCMakeInstance()->GetDebugConfigs();

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
  if (!this->impl->TLLCommands.empty()) {
    if (this->impl->TLLCommands.back().first != signature) {
      ret = false;
    }
  }
  if (this->impl->TLLCommands.empty() ||
      this->impl->TLLCommands.back().second != lfc) {
    this->impl->TLLCommands.emplace_back(signature, lfc);
  }
  return ret;
}

void cmTarget::GetTllSignatureTraces(std::ostream& s, TLLSignature sig) const
{
  const char* sigString =
    (sig == cmTarget::KeywordTLLSignature ? "keyword" : "plain");
  s << "The uses of the " << sigString << " signature are here:\n";
  for (auto const& cmd : this->impl->TLLCommands) {
    if (cmd.first == sig) {
      cmListFileContext lfc = cmd.second;
      lfc.FilePath = cmSystemTools::RelativeIfUnder(
        this->impl->Makefile->GetState()->GetSourceDirectory(), lfc.FilePath);
      s << " * " << lfc << '\n';
    }
  }
}

std::string const& cmTarget::GetInstallPath() const
{
  return this->impl->InstallPath;
}

void cmTarget::SetInstallPath(std::string const& name)
{
  this->impl->InstallPath = name;
}

std::string const& cmTarget::GetRuntimeInstallPath() const
{
  return this->impl->RuntimeInstallPath;
}

void cmTarget::SetRuntimeInstallPath(std::string const& name)
{
  this->impl->RuntimeInstallPath = name;
}

bool cmTarget::GetHaveInstallRule() const
{
  return this->impl->HaveInstallRule;
}

void cmTarget::SetHaveInstallRule(bool hir)
{
  this->impl->HaveInstallRule = hir;
}

void cmTarget::AddInstallGenerator(cmInstallTargetGenerator* g)
{
  this->impl->InstallGenerators.emplace_back(g);
}

std::vector<cmInstallTargetGenerator*> const& cmTarget::GetInstallGenerators()
  const
{
  return this->impl->InstallGenerators;
}

bool cmTarget::GetIsGeneratorProvided() const
{
  return this->impl->IsGeneratorProvided;
}

void cmTarget::SetIsGeneratorProvided(bool igp)
{
  this->impl->IsGeneratorProvided = igp;
}

cmTarget::LinkLibraryVectorType const& cmTarget::GetOriginalLinkLibraries()
  const
{
  return this->impl->OriginalLinkLibraries;
}

void cmTarget::AddLinkLibrary(cmMakefile& mf, std::string const& lib,
                              cmTargetLinkLibraryType llt)
{
  cmTarget* tgt = mf.FindTargetToUse(lib);
  {
    const bool isNonImportedTarget = tgt && !tgt->IsImported();

    const std::string libName =
      (isNonImportedTarget && llt != GENERAL_LibraryType)
      ? targetNameGenex(lib)
      : lib;
    this->AppendProperty("LINK_LIBRARIES",
                         this->GetDebugGeneratorExpressions(libName, llt));
  }

  if (cmGeneratorExpression::Find(lib) != std::string::npos ||
      (tgt &&
       (tgt->GetType() == cmStateEnums::INTERFACE_LIBRARY ||
        tgt->GetType() == cmStateEnums::OBJECT_LIBRARY)) ||
      (this->impl->Name == lib)) {
    return;
  }

  this->impl->OriginalLinkLibraries.emplace_back(lib, llt);

  // Add the explicit dependency information for libraries. This is
  // simply a set of libraries separated by ";". There should always
  // be a trailing ";". These library names are not canonical, in that
  // they may be "-framework x", "-ly", "/path/libz.a", etc.
  // We shouldn't remove duplicates here because external libraries
  // may be purposefully duplicated to handle recursive dependencies,
  // and we removing one instance will break the link line. Duplicates
  // will be appropriately eliminated at emit time.
  if (this->impl->TargetType >= cmStateEnums::STATIC_LIBRARY &&
      this->impl->TargetType <= cmStateEnums::MODULE_LIBRARY &&
      (this->GetPolicyStatusCMP0073() == cmPolicies::OLD ||
       this->GetPolicyStatusCMP0073() == cmPolicies::WARN)) {
    std::string targetEntry = cmStrCat(this->impl->Name, "_LIB_DEPENDS");
    std::string dependencies;
    cmValue old_val = mf.GetDefinition(targetEntry);
    if (old_val) {
      dependencies += *old_val;
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
    mf.AddCacheDefinition(targetEntry, dependencies,
                          "Dependencies for the target", cmStateEnums::STATIC);
  }
}

void cmTarget::AddSystemIncludeDirectories(const std::set<std::string>& incs)
{
  this->impl->SystemIncludeDirectories.insert(incs.begin(), incs.end());
}

std::set<std::string> const& cmTarget::GetSystemIncludeDirectories() const
{
  return this->impl->SystemIncludeDirectories;
}

void cmTarget::AddInstallIncludeDirectories(cmTargetExport const& te,
                                            cmStringRange const& incs)
{
  std::copy(
    incs.begin(), incs.end(),
    std::back_inserter(this->impl->InstallIncludeDirectoriesEntries[&te]));
}

cmStringRange cmTarget::GetInstallIncludeDirectoriesEntries(
  cmTargetExport const& te) const
{
  auto i = this->impl->InstallIncludeDirectoriesEntries.find(&te);
  if (i == this->impl->InstallIncludeDirectoriesEntries.end()) {
    decltype(i->second) empty;
    return cmMakeRange(empty);
  }
  return cmMakeRange(i->second);
}

cmBTStringRange cmTarget::GetIncludeDirectoriesEntries() const
{
  return cmMakeRange(this->impl->IncludeDirectoriesEntries);
}

cmBTStringRange cmTarget::GetCompileOptionsEntries() const
{
  return cmMakeRange(this->impl->CompileOptionsEntries);
}

cmBTStringRange cmTarget::GetCompileFeaturesEntries() const
{
  return cmMakeRange(this->impl->CompileFeaturesEntries);
}

cmBTStringRange cmTarget::GetCompileDefinitionsEntries() const
{
  return cmMakeRange(this->impl->CompileDefinitionsEntries);
}

cmBTStringRange cmTarget::GetPrecompileHeadersEntries() const
{
  return cmMakeRange(this->impl->PrecompileHeadersEntries);
}

cmBTStringRange cmTarget::GetSourceEntries() const
{
  return cmMakeRange(this->impl->SourceEntries);
}

cmBTStringRange cmTarget::GetLinkOptionsEntries() const
{
  return cmMakeRange(this->impl->LinkOptionsEntries);
}

cmBTStringRange cmTarget::GetLinkDirectoriesEntries() const
{
  return cmMakeRange(this->impl->LinkDirectoriesEntries);
}

cmBTStringRange cmTarget::GetLinkImplementationEntries() const
{
  return cmMakeRange(this->impl->LinkImplementationPropertyEntries);
}

cmBTStringRange cmTarget::GetLinkInterfaceEntries() const
{
  return cmMakeRange(this->impl->LinkInterfacePropertyEntries);
}

cmBTStringRange cmTarget::GetLinkInterfaceDirectEntries() const
{
  return cmMakeRange(this->impl->LinkInterfaceDirectPropertyEntries);
}

cmBTStringRange cmTarget::GetLinkInterfaceDirectExcludeEntries() const
{
  return cmMakeRange(this->impl->LinkInterfaceDirectExcludePropertyEntries);
}

cmBTStringRange cmTarget::GetHeaderSetsEntries() const
{
  return cmMakeRange(this->impl->HeadersFileSets.SelfEntries.Entries);
}

cmBTStringRange cmTarget::GetInterfaceHeaderSetsEntries() const
{
  return cmMakeRange(this->impl->HeadersFileSets.InterfaceEntries.Entries);
}

namespace {
#define MAKE_PROP(PROP) const std::string prop##PROP = #PROP
MAKE_PROP(C_STANDARD);
MAKE_PROP(CXX_STANDARD);
MAKE_PROP(CUDA_STANDARD);
MAKE_PROP(HIP_STANDARD);
MAKE_PROP(OBJC_STANDARD);
MAKE_PROP(OBJCXX_STANDARD);
MAKE_PROP(COMPILE_DEFINITIONS);
MAKE_PROP(COMPILE_FEATURES);
MAKE_PROP(COMPILE_OPTIONS);
MAKE_PROP(PRECOMPILE_HEADERS);
MAKE_PROP(PRECOMPILE_HEADERS_REUSE_FROM);
MAKE_PROP(CUDA_PTX_COMPILATION);
MAKE_PROP(EXPORT_NAME);
MAKE_PROP(IMPORTED);
MAKE_PROP(IMPORTED_GLOBAL);
MAKE_PROP(INCLUDE_DIRECTORIES);
MAKE_PROP(LINK_OPTIONS);
MAKE_PROP(LINK_DIRECTORIES);
MAKE_PROP(LINK_LIBRARIES);
MAKE_PROP(MANUALLY_ADDED_DEPENDENCIES);
MAKE_PROP(NAME);
MAKE_PROP(SOURCES);
MAKE_PROP(TYPE);
MAKE_PROP(BINARY_DIR);
MAKE_PROP(SOURCE_DIR);
MAKE_PROP(FALSE);
MAKE_PROP(TRUE);
MAKE_PROP(INTERFACE_LINK_LIBRARIES);
MAKE_PROP(INTERFACE_LINK_LIBRARIES_DIRECT);
MAKE_PROP(INTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE);
#undef MAKE_PROP
}

namespace {
// to workaround bug on GCC/AIX
// Define a template to force conversion to std::string
template <typename ValueType>
std::string ConvertToString(ValueType value);

template <>
std::string ConvertToString<const char*>(const char* value)
{
  return std::string(value);
}
template <>
std::string ConvertToString<cmValue>(cmValue value)
{
  return std::string(*value);
}

template <typename ValueType>
bool StringIsEmpty(ValueType const& value);

template <>
bool StringIsEmpty<const char*>(const char* const& value)
{
  return cmValue::IsEmpty(value);
}

template <>
bool StringIsEmpty<cmValue>(cmValue const& value)
{
  return value.IsEmpty();
}

template <>
bool StringIsEmpty<std::string>(std::string const& value)
{
  return value.empty();
}
}

template <typename ValueType>
void cmTarget::StoreProperty(const std::string& prop, ValueType value)
{
  if (prop == propMANUALLY_ADDED_DEPENDENCIES) {
    this->impl->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "MANUALLY_ADDED_DEPENDENCIES property is read-only\n");
    return;
  }
  if (prop == propNAME) {
    this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                       "NAME property is read-only\n");
    return;
  }
  if (prop == propTYPE) {
    this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                       "TYPE property is read-only\n");
    return;
  }
  if (prop == propEXPORT_NAME && this->IsImported()) {
    std::ostringstream e;
    e << "EXPORT_NAME property can't be set on imported targets (\""
      << this->impl->Name << "\")\n";
    this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }
  if (prop == propSOURCES && this->IsImported()) {
    std::ostringstream e;
    e << "SOURCES property can't be set on imported targets (\""
      << this->impl->Name << "\")\n";
    this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }
  if (prop == propIMPORTED_GLOBAL && !this->IsImported()) {
    std::ostringstream e;
    e << "IMPORTED_GLOBAL property can't be set on non-imported targets (\""
      << this->impl->Name << "\")\n";
    this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }

  if (prop == propINCLUDE_DIRECTORIES) {
    this->impl->IncludeDirectoriesEntries.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->IncludeDirectoriesEntries.emplace_back(value, lfbt);
    }
  } else if (prop == propCOMPILE_OPTIONS) {
    this->impl->CompileOptionsEntries.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->CompileOptionsEntries.emplace_back(value, lfbt);
    }
  } else if (prop == propCOMPILE_FEATURES) {
    this->impl->CompileFeaturesEntries.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->CompileFeaturesEntries.emplace_back(value, lfbt);
    }
  } else if (prop == propCOMPILE_DEFINITIONS) {
    this->impl->CompileDefinitionsEntries.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->CompileDefinitionsEntries.emplace_back(value, lfbt);
    }
  } else if (prop == propLINK_OPTIONS) {
    this->impl->LinkOptionsEntries.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->LinkOptionsEntries.emplace_back(value, lfbt);
    }
  } else if (prop == propLINK_DIRECTORIES) {
    this->impl->LinkDirectoriesEntries.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->LinkDirectoriesEntries.emplace_back(value, lfbt);
    }
  } else if (prop == propPRECOMPILE_HEADERS) {
    this->impl->PrecompileHeadersEntries.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->PrecompileHeadersEntries.emplace_back(value, lfbt);
    }
  } else if (prop == propLINK_LIBRARIES) {
    this->impl->LinkImplementationPropertyEntries.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->LinkImplementationPropertyEntries.emplace_back(value, lfbt);
    }
  } else if (prop == propINTERFACE_LINK_LIBRARIES) {
    this->impl->LinkInterfacePropertyEntries.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->LinkInterfacePropertyEntries.emplace_back(value, lfbt);
    }
  } else if (prop == propINTERFACE_LINK_LIBRARIES_DIRECT) {
    this->impl->LinkInterfaceDirectPropertyEntries.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->LinkInterfaceDirectPropertyEntries.emplace_back(value, lfbt);
    }
  } else if (prop == propINTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE) {
    this->impl->LinkInterfaceDirectExcludePropertyEntries.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->LinkInterfaceDirectExcludePropertyEntries.emplace_back(value,
                                                                         lfbt);
    }
  } else if (prop == propSOURCES) {
    this->impl->SourceEntries.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->SourceEntries.emplace_back(value, lfbt);
    }
  } else if (prop == propIMPORTED_GLOBAL) {
    if (!cmIsOn(value)) {
      std::ostringstream e;
      e << "IMPORTED_GLOBAL property can't be set to FALSE on targets (\""
        << this->impl->Name << "\")\n";
      this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return;
    }
    /* no need to change anything if value does not change */
    if (!this->impl->ImportedGloballyVisible) {
      this->impl->ImportedGloballyVisible = true;
      this->GetGlobalGenerator()->IndexTarget(this);
    }
  } else if (cmHasLiteralPrefix(prop, "IMPORTED_LIBNAME") &&
             !this->impl->CheckImportedLibName(
               prop,
               value ? value
                     : std::string{})) { // NOLINT(bugprone-branch-clone)
    /* error was reported by check method */
  } else if (prop == propCUDA_PTX_COMPILATION &&
             this->GetType() != cmStateEnums::OBJECT_LIBRARY) {
    std::ostringstream e;
    e << "CUDA_PTX_COMPILATION property can only be applied to OBJECT "
         "targets (\""
      << this->impl->Name << "\")\n";
    this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  } else if (prop == propPRECOMPILE_HEADERS_REUSE_FROM) {
    if (this->GetProperty("PRECOMPILE_HEADERS")) {
      std::ostringstream e;
      e << "PRECOMPILE_HEADERS property is already set on target (\""
        << this->impl->Name << "\")\n";
      this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return;
    }
    auto* reusedTarget = this->impl->Makefile->GetCMakeInstance()
                           ->GetGlobalGenerator()
                           ->FindTarget(value);
    if (!reusedTarget) {
      const std::string e(
        "PRECOMPILE_HEADERS_REUSE_FROM set with non existing target");
      this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
      return;
    }

    std::string reusedFrom = reusedTarget->GetSafeProperty(prop);
    if (reusedFrom.empty()) {
      reusedFrom = ConvertToString(value);
    }

    this->impl->Properties.SetProperty(prop, reusedFrom);

    reusedTarget->SetProperty("COMPILE_PDB_NAME", reusedFrom);
    reusedTarget->SetProperty("COMPILE_PDB_OUTPUT_DIRECTORY",
                              cmStrCat(reusedFrom, ".dir/"));

    cmValue tmp = reusedTarget->GetProperty("COMPILE_PDB_NAME");
    this->SetProperty("COMPILE_PDB_NAME", tmp);
    this->AddUtility(reusedFrom, false, this->impl->Makefile);
  } else if (prop == propC_STANDARD || prop == propCXX_STANDARD ||
             prop == propCUDA_STANDARD || prop == propHIP_STANDARD ||
             prop == propOBJC_STANDARD || prop == propOBJCXX_STANDARD) {
    if (value) {
      this->impl->LanguageStandardProperties[prop] =
        BTs<std::string>(value, this->impl->Makefile->GetBacktrace());
    } else {
      this->impl->LanguageStandardProperties.erase(prop);
    }
  } else if (this->impl->HeadersFileSets.WriteProperties(
               this, this->impl.get(), prop, value, true)) {
    /* Handled in the `if` condition. */
  } else {
    this->impl->Properties.SetProperty(prop, value);
  }
}

void cmTarget::AppendProperty(const std::string& prop,
                              const std::string& value, bool asString)
{
  if (prop == "NAME") {
    this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                       "NAME property is read-only\n");
    return;
  }
  if (prop == "EXPORT_NAME" && this->IsImported()) {
    std::ostringstream e;
    e << "EXPORT_NAME property can't be set on imported targets (\""
      << this->impl->Name << "\")\n";
    this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }
  if (prop == "SOURCES" && this->IsImported()) {
    std::ostringstream e;
    e << "SOURCES property can't be set on imported targets (\""
      << this->impl->Name << "\")\n";
    this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }
  if (prop == "IMPORTED_GLOBAL") {
    std::ostringstream e;
    e << "IMPORTED_GLOBAL property can't be appended, only set on imported "
         "targets (\""
      << this->impl->Name << "\")\n";
    this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }
  if (prop == "INCLUDE_DIRECTORIES") {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->IncludeDirectoriesEntries.emplace_back(value, lfbt);
    }
  } else if (prop == "COMPILE_OPTIONS") {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->CompileOptionsEntries.emplace_back(value, lfbt);
    }
  } else if (prop == "COMPILE_FEATURES") {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->CompileFeaturesEntries.emplace_back(value, lfbt);
    }
  } else if (prop == "COMPILE_DEFINITIONS") {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->CompileDefinitionsEntries.emplace_back(value, lfbt);
    }
  } else if (prop == "LINK_OPTIONS") {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->LinkOptionsEntries.emplace_back(value, lfbt);
    }
  } else if (prop == "LINK_DIRECTORIES") {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->LinkDirectoriesEntries.emplace_back(value, lfbt);
    }
  } else if (prop == "PRECOMPILE_HEADERS") {
    if (this->GetProperty("PRECOMPILE_HEADERS_REUSE_FROM")) {
      std::ostringstream e;
      e << "PRECOMPILE_HEADERS_REUSE_FROM property is already set on target "
           "(\""
        << this->impl->Name << "\")\n";
      this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return;
    }
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->PrecompileHeadersEntries.emplace_back(value, lfbt);
    }
  } else if (prop == "LINK_LIBRARIES") {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->LinkImplementationPropertyEntries.emplace_back(value, lfbt);
    }
  } else if (prop == propINTERFACE_LINK_LIBRARIES) {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->LinkInterfacePropertyEntries.emplace_back(value, lfbt);
    }
  } else if (prop == propINTERFACE_LINK_LIBRARIES_DIRECT) {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->LinkInterfaceDirectPropertyEntries.emplace_back(value, lfbt);
    }
  } else if (prop == propINTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE) {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
      this->impl->LinkInterfaceDirectExcludePropertyEntries.emplace_back(value,
                                                                         lfbt);
    }
  } else if (prop == "SOURCES") {
    cmListFileBacktrace lfbt = this->impl->Makefile->GetBacktrace();
    this->impl->SourceEntries.emplace_back(value, lfbt);
  } else if (cmHasLiteralPrefix(prop, "IMPORTED_LIBNAME")) {
    this->impl->Makefile->IssueMessage(
      MessageType::FATAL_ERROR, prop + " property may not be APPENDed.");
  } else if (prop == "C_STANDARD" || prop == "CXX_STANDARD" ||
             prop == "CUDA_STANDARD" || prop == "HIP_STANDARD" ||
             prop == "OBJC_STANDARD" || prop == "OBJCXX_STANDARD") {
    this->impl->Makefile->IssueMessage(
      MessageType::FATAL_ERROR, prop + " property may not be appended.");
  } else if (this->impl->HeadersFileSets.WriteProperties(
               this, this->impl.get(), prop, value, false)) {
    /* Handled in the `if` condition. */
  } else {
    this->impl->Properties.AppendProperty(prop, value, asString);
  }
}

void cmTarget::SetProperty(const std::string& prop, const char* value)
{
  this->StoreProperty(prop, value);
}
void cmTarget::SetProperty(const std::string& prop, cmValue value)
{
  this->StoreProperty(prop, value);
}

template <typename ValueType>
void cmTargetInternals::AddDirectoryToFileSet(
  cmTarget* self, std::string const& fileSetName, ValueType value,
  cm::string_view fileSetType, cm::string_view description, bool clear)
{
  auto* fileSet = self->GetFileSet(fileSetName);
  if (!fileSet) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat(description, "has not yet been created."));
    return;
  }
  if (fileSet->GetType() != fileSetType) {
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                 cmStrCat("File set \"", fileSetName,
                                          "\" is not of type \"", fileSetType,
                                          "\"."));
    return;
  }
  if (clear) {
    fileSet->ClearDirectoryEntries();
  }
  if (!StringIsEmpty(value)) {
    fileSet->AddDirectoryEntry(
      BT<std::string>(value, this->Makefile->GetBacktrace()));
  }
}

template <typename ValueType>
void cmTargetInternals::AddPathToFileSet(
  cmTarget* self, std::string const& fileSetName, ValueType value,
  cm::string_view fileSetType, cm::string_view description, bool clear)
{
  auto* fileSet = self->GetFileSet(fileSetName);
  if (!fileSet) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat(description, "has not yet been created."));
    return;
  }
  if (fileSet->GetType() != fileSetType) {
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                 cmStrCat("File set \"", fileSetName,
                                          "\" is not of type \"", fileSetType,
                                          "\"."));
    return;
  }
  if (clear) {
    fileSet->ClearFileEntries();
  }
  if (!StringIsEmpty(value)) {
    fileSet->AddFileEntry(
      BT<std::string>(value, this->Makefile->GetBacktrace()));
  }
}

cmValue cmTargetInternals::GetFileSetDirectories(
  cmTarget const* self, std::string const& fileSetName,
  cm::string_view fileSetType) const
{
  auto const* fileSet = self->GetFileSet(fileSetName);
  if (!fileSet) {
    return nullptr;
  }
  if (fileSet->GetType() != fileSetType) {
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                 cmStrCat("File set \"", fileSetName,
                                          "\" is not of type \"", fileSetType,
                                          "\"."));
    return nullptr;
  }
  static std::string output;
  output = cmJoin(fileSet->GetDirectoryEntries(), ";"_s);
  return cmValue(output);
}

cmValue cmTargetInternals::GetFileSetPaths(cmTarget const* self,
                                           std::string const& fileSetName,
                                           cm::string_view fileSetType) const
{
  auto const* fileSet = self->GetFileSet(fileSetName);
  if (!fileSet) {
    return nullptr;
  }
  if (fileSet->GetType() != fileSetType) {
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                 cmStrCat("File set \"", fileSetName,
                                          "\" is not of type \"", fileSetType,
                                          "\"."));
    return nullptr;
  }
  static std::string output;
  output = cmJoin(fileSet->GetFileEntries(), ";"_s);
  return cmValue(output);
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
  if (this->impl->BuildInterfaceIncludesAppended) {
    return;
  }
  this->impl->BuildInterfaceIncludesAppended = true;

  if (this->impl->Makefile->IsOn("CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE")) {
    std::string dirs = this->impl->Makefile->GetCurrentBinaryDirectory();
    if (!dirs.empty()) {
      dirs += ';';
    }
    dirs += this->impl->Makefile->GetCurrentSourceDirectory();
    if (!dirs.empty()) {
      this->AppendProperty("INTERFACE_INCLUDE_DIRECTORIES",
                           ("$<BUILD_INTERFACE:" + dirs + ">"));
    }
  }
}

void cmTarget::FinalizeTargetCompileInfo(
  const cmBTStringRange& noConfigCompileDefinitions,
  cm::optional<std::map<std::string, cmValue>>& perConfigCompileDefinitions)
{
  if (this->GetType() == cmStateEnums::GLOBAL_TARGET) {
    return;
  }

  this->AppendBuildInterfaceIncludes();

  if (this->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    return;
  }

  for (auto const& def : noConfigCompileDefinitions) {
    this->InsertCompileDefinition(def);
  }

  auto* mf = this->GetMakefile();
  cmPolicies::PolicyStatus polSt = mf->GetPolicyStatus(cmPolicies::CMP0043);
  if (polSt == cmPolicies::WARN || polSt == cmPolicies::OLD) {
    if (perConfigCompileDefinitions) {
      for (auto const& it : *perConfigCompileDefinitions) {
        if (cmValue val = it.second) {
          this->AppendProperty(it.first, *val);
        }
      }
    } else {
      perConfigCompileDefinitions.emplace();
      std::vector<std::string> configs =
        mf->GetGeneratorConfigs(cmMakefile::ExcludeEmptyConfig);

      for (std::string const& c : configs) {
        std::string defPropName =
          cmStrCat("COMPILE_DEFINITIONS_", cmSystemTools::UpperCase(c));
        cmValue val = mf->GetProperty(defPropName);
        (*perConfigCompileDefinitions)[defPropName] = val;
        if (val) {
          this->AppendProperty(defPropName, *val);
        }
      }
    }
  }
}

void cmTarget::InsertInclude(BT<std::string> const& entry, bool before)
{
  auto position = before ? this->impl->IncludeDirectoriesEntries.begin()
                         : this->impl->IncludeDirectoriesEntries.end();

  this->impl->IncludeDirectoriesEntries.insert(position, entry);
}

void cmTarget::InsertCompileOption(BT<std::string> const& entry, bool before)
{
  auto position = before ? this->impl->CompileOptionsEntries.begin()
                         : this->impl->CompileOptionsEntries.end();

  this->impl->CompileOptionsEntries.insert(position, entry);
}

void cmTarget::InsertCompileDefinition(BT<std::string> const& entry)
{
  this->impl->CompileDefinitionsEntries.push_back(entry);
}

void cmTarget::InsertLinkOption(BT<std::string> const& entry, bool before)
{
  auto position = before ? this->impl->LinkOptionsEntries.begin()
                         : this->impl->LinkOptionsEntries.end();

  this->impl->LinkOptionsEntries.insert(position, entry);
}

void cmTarget::InsertLinkDirectory(BT<std::string> const& entry, bool before)
{
  auto position = before ? this->impl->LinkDirectoriesEntries.begin()
                         : this->impl->LinkDirectoriesEntries.end();

  this->impl->LinkDirectoriesEntries.insert(position, entry);
}

void cmTarget::InsertPrecompileHeader(BT<std::string> const& entry)
{
  this->impl->PrecompileHeadersEntries.push_back(entry);
}

namespace {
void CheckLinkLibraryPattern(const std::string& property,
                             const std::string& value, cmMakefile* context)
{
  // Look for <LINK_LIBRARY:> and </LINK_LIBRARY:> internal tags
  static cmsys::RegularExpression linkPattern(
    "(^|;)(</?LINK_(LIBRARY|GROUP):[^;>]*>)(;|$)");
  if (!linkPattern.find(value)) {
    return;
  }

  // Report an error.
  context->IssueMessage(
    MessageType::FATAL_ERROR,
    cmStrCat(
      "Property ", property, " contains the invalid item \"",
      linkPattern.match(2), "\". The ", property,
      " property may contain the generator-expression \"$<LINK_",
      linkPattern.match(3),
      ":...>\" which may be used to specify how the libraries are linked."));
}

void CheckLINK_INTERFACE_LIBRARIES(const std::string& prop,
                                   const std::string& value,
                                   cmMakefile* context, bool imported)
{
  // Support imported and non-imported versions of the property.
  const char* base = (imported ? "IMPORTED_LINK_INTERFACE_LIBRARIES"
                               : "LINK_INTERFACE_LIBRARIES");

  // Look for link-type keywords in the value.
  static cmsys::RegularExpression keys("(^|;)(debug|optimized|general)(;|$)");
  if (keys.find(value)) {
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

  CheckLinkLibraryPattern(base, value, context);
}

void CheckLINK_LIBRARIES(const std::string& value, cmMakefile* context)
{
  CheckLinkLibraryPattern("LINK_LIBRARIES", value, context);
}

void CheckINTERFACE_LINK_LIBRARIES(const std::string& value,
                                   cmMakefile* context)
{
  // Look for link-type keywords in the value.
  static cmsys::RegularExpression keys("(^|;)(debug|optimized|general)(;|$)");
  if (keys.find(value)) {
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

  CheckLinkLibraryPattern("INTERFACE_LINK_LIBRARIES", value, context);
}

void CheckIMPORTED_GLOBAL(const cmTarget* target, cmMakefile* context)
{
  const auto& targets = context->GetOwnedImportedTargets();
  auto it =
    std::find_if(targets.begin(), targets.end(),
                 [&](const std::unique_ptr<cmTarget>& importTarget) -> bool {
                   return target == importTarget.get();
                 });
  if (it == targets.end()) {
    std::ostringstream e;
    e << "Attempt to promote imported target \"" << target->GetName()
      << "\" to global scope (by setting IMPORTED_GLOBAL) "
         "which is not built in this directory.";
    context->IssueMessage(MessageType::FATAL_ERROR, e.str());
  }
}
}

void cmTarget::CheckProperty(const std::string& prop,
                             cmMakefile* context) const
{
  // Certain properties need checking.
  if (cmHasLiteralPrefix(prop, "LINK_INTERFACE_LIBRARIES")) {
    if (cmValue value = this->GetProperty(prop)) {
      CheckLINK_INTERFACE_LIBRARIES(prop, *value, context, false);
    }
  } else if (cmHasLiteralPrefix(prop, "IMPORTED_LINK_INTERFACE_LIBRARIES")) {
    if (cmValue value = this->GetProperty(prop)) {
      CheckLINK_INTERFACE_LIBRARIES(prop, *value, context, true);
    }
  } else if (prop == "LINK_LIBRARIES") {
    if (cmValue value = this->GetProperty(prop)) {
      CheckLINK_LIBRARIES(*value, context);
    }
  } else if (prop == "INTERFACE_LINK_LIBRARIES") {
    if (cmValue value = this->GetProperty(prop)) {
      CheckINTERFACE_LINK_LIBRARIES(*value, context);
    }
  } else if (prop == "IMPORTED_GLOBAL") {
    if (this->IsImported()) {
      CheckIMPORTED_GLOBAL(this, context);
    }
  }
}

cmValue cmTarget::GetComputedProperty(const std::string& prop,
                                      cmMakefile& mf) const
{
  return cmTargetPropertyComputer::GetProperty(this, prop, mf);
}

cmValue cmTarget::GetProperty(const std::string& prop) const
{
  static std::unordered_set<std::string> const specialProps{
    propC_STANDARD,
    propCXX_STANDARD,
    propCUDA_STANDARD,
    propOBJC_STANDARD,
    propOBJCXX_STANDARD,
    propLINK_LIBRARIES,
    propTYPE,
    propINCLUDE_DIRECTORIES,
    propCOMPILE_FEATURES,
    propCOMPILE_OPTIONS,
    propCOMPILE_DEFINITIONS,
    propPRECOMPILE_HEADERS,
    propLINK_OPTIONS,
    propLINK_DIRECTORIES,
    propIMPORTED,
    propIMPORTED_GLOBAL,
    propMANUALLY_ADDED_DEPENDENCIES,
    propNAME,
    propBINARY_DIR,
    propSOURCE_DIR,
    propSOURCES,
    propINTERFACE_LINK_LIBRARIES,
    propINTERFACE_LINK_LIBRARIES_DIRECT,
    propINTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE,
  };
  if (specialProps.count(prop)) {
    if (prop == propC_STANDARD || prop == propCXX_STANDARD ||
        prop == propCUDA_STANDARD || prop == propOBJC_STANDARD ||
        prop == propOBJCXX_STANDARD) {
      auto propertyIter = this->impl->LanguageStandardProperties.find(prop);
      if (propertyIter == this->impl->LanguageStandardProperties.end()) {
        return nullptr;
      }
      return cmValue(propertyIter->second.Value);
    }
    if (prop == propLINK_LIBRARIES) {
      if (this->impl->LinkImplementationPropertyEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(this->impl->LinkImplementationPropertyEntries, ";");
      return cmValue(output);
    }
    if (prop == propINTERFACE_LINK_LIBRARIES) {
      if (this->impl->LinkInterfacePropertyEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(this->impl->LinkInterfacePropertyEntries, ";");
      return cmValue(output);
    }
    if (prop == propINTERFACE_LINK_LIBRARIES_DIRECT) {
      if (this->impl->LinkInterfaceDirectPropertyEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(this->impl->LinkInterfaceDirectPropertyEntries, ";");
      return cmValue(output);
    }
    if (prop == propINTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE) {
      if (this->impl->LinkInterfaceDirectExcludePropertyEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output =
        cmJoin(this->impl->LinkInterfaceDirectExcludePropertyEntries, ";");
      return cmValue(output);
    }
    // the type property returns what type the target is
    if (prop == propTYPE) {
      return cmValue(cmState::GetTargetTypeName(this->GetType()));
    }
    if (prop == propINCLUDE_DIRECTORIES) {
      if (this->impl->IncludeDirectoriesEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(this->impl->IncludeDirectoriesEntries, ";");
      return cmValue(output);
    }
    if (prop == propCOMPILE_FEATURES) {
      if (this->impl->CompileFeaturesEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(this->impl->CompileFeaturesEntries, ";");
      return cmValue(output);
    }
    if (prop == propCOMPILE_OPTIONS) {
      if (this->impl->CompileOptionsEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(this->impl->CompileOptionsEntries, ";");
      return cmValue(output);
    }
    if (prop == propCOMPILE_DEFINITIONS) {
      if (this->impl->CompileDefinitionsEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(this->impl->CompileDefinitionsEntries, ";");
      return cmValue(output);
    }
    if (prop == propLINK_OPTIONS) {
      if (this->impl->LinkOptionsEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(this->impl->LinkOptionsEntries, ";");
      return cmValue(output);
    }
    if (prop == propLINK_DIRECTORIES) {
      if (this->impl->LinkDirectoriesEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(this->impl->LinkDirectoriesEntries, ";");

      return cmValue(output);
    }
    if (prop == propMANUALLY_ADDED_DEPENDENCIES) {
      if (this->impl->Utilities.empty()) {
        return nullptr;
      }

      static std::string output;
      static std::vector<std::string> utilities;
      utilities.resize(this->impl->Utilities.size());
      std::transform(
        this->impl->Utilities.cbegin(), this->impl->Utilities.cend(),
        utilities.begin(),
        [](const BT<std::pair<std::string, bool>>& item) -> std::string {
          return item.Value.first;
        });
      output = cmJoin(utilities, ";");
      return cmValue(output);
    }
    if (prop == propPRECOMPILE_HEADERS) {
      if (this->impl->PrecompileHeadersEntries.empty()) {
        return nullptr;
      }

      static std::string output;
      output = cmJoin(this->impl->PrecompileHeadersEntries, ";");
      return cmValue(output);
    }
    if (prop == propIMPORTED) {
      return this->IsImported() ? cmValue(propTRUE) : cmValue(propFALSE);
    }
    if (prop == propIMPORTED_GLOBAL) {
      return this->IsImportedGloballyVisible() ? cmValue(propTRUE)
                                               : cmValue(propFALSE);
    }
    if (prop == propNAME) {
      return cmValue(this->GetName());
    }
    if (prop == propBINARY_DIR) {
      return cmValue(this->impl->Makefile->GetStateSnapshot()
                       .GetDirectory()
                       .GetCurrentBinary());
    }
    if (prop == propSOURCE_DIR) {
      return cmValue(this->impl->Makefile->GetStateSnapshot()
                       .GetDirectory()
                       .GetCurrentSource());
    }
  }

  // Check fileset properties.
  {
    auto headers =
      this->impl->HeadersFileSets.ReadProperties(this, this->impl.get(), prop);
    if (headers.first) {
      return headers.second;
    }
  }

  cmValue retVal = this->impl->Properties.GetPropertyValue(prop);
  if (!retVal) {
    const bool chain = this->impl->Makefile->GetState()->IsPropertyChained(
      prop, cmProperty::TARGET);
    if (chain) {
      return this->impl->Makefile->GetStateSnapshot()
        .GetDirectory()
        .GetProperty(prop, chain);
    }
    return nullptr;
  }
  return retVal;
}

std::string const& cmTarget::GetSafeProperty(std::string const& prop) const
{
  cmValue ret = this->GetProperty(prop);
  if (ret) {
    return *ret;
  }

  static std::string const s_empty;
  return s_empty;
}

bool cmTarget::GetPropertyAsBool(const std::string& prop) const
{
  return cmIsOn(this->GetProperty(prop));
}

cmPropertyMap const& cmTarget::GetProperties() const
{
  return this->impl->Properties;
}

bool cmTarget::IsDLLPlatform() const
{
  return this->impl->IsDLLPlatform;
}

bool cmTarget::IsAIX() const
{
  return this->impl->IsAIX;
}

bool cmTarget::IsImported() const
{
  return this->impl->IsImportedTarget;
}

bool cmTarget::IsImportedGloballyVisible() const
{
  return this->impl->ImportedGloballyVisible;
}

bool cmTarget::IsPerConfig() const
{
  return this->impl->PerConfig;
}

bool cmTarget::CanCompileSources() const
{
  if (this->IsImported()) {
    return false;
  }
  switch (this->GetType()) {
    case cmStateEnums::EXECUTABLE:
    case cmStateEnums::STATIC_LIBRARY:
    case cmStateEnums::SHARED_LIBRARY:
    case cmStateEnums::MODULE_LIBRARY:
    case cmStateEnums::OBJECT_LIBRARY:
      return true;
    case cmStateEnums::UTILITY:
    case cmStateEnums::INTERFACE_LIBRARY:
    case cmStateEnums::GLOBAL_TARGET:
    case cmStateEnums::UNKNOWN_LIBRARY:
      break;
  }
  return false;
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
          return (this->IsAndroidGuiExecutable()
                    ? "CMAKE_SHARED_LIBRARY_SUFFIX"
                    : "CMAKE_EXECUTABLE_SUFFIX");
        case cmStateEnums::ImportLibraryArtifact:
          return (this->impl->IsAIX ? "CMAKE_AIX_IMPORT_FILE_SUFFIX"
                                    : "CMAKE_IMPORT_LIBRARY_SUFFIX");
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
          return (this->IsAndroidGuiExecutable()
                    ? "CMAKE_SHARED_LIBRARY_PREFIX"
                    : "");
        case cmStateEnums::ImportLibraryArtifact:
          return (this->impl->IsAIX ? "CMAKE_AIX_IMPORT_FILE_PREFIX"
                                    : "CMAKE_IMPORT_LIBRARY_PREFIX");
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

  cmValue loc = nullptr;
  cmValue imp = nullptr;
  std::string suffix;

  if (this->GetType() != cmStateEnums::INTERFACE_LIBRARY &&
      this->GetMappedConfig(desired_config, loc, imp, suffix)) {
    switch (artifact) {
      case cmStateEnums::RuntimeBinaryArtifact:
        if (loc) {
          result = *loc;
        } else {
          std::string impProp = cmStrCat("IMPORTED_LOCATION", suffix);
          if (cmValue config_location = this->GetProperty(impProp)) {
            result = *config_location;
          } else if (cmValue location =
                       this->GetProperty("IMPORTED_LOCATION")) {
            result = *location;
          }
        }
        break;

      case cmStateEnums::ImportLibraryArtifact:
        if (imp) {
          result = *imp;
        } else if (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
                   this->IsExecutableWithExports()) {
          std::string impProp = cmStrCat("IMPORTED_IMPLIB", suffix);
          if (cmValue config_implib = this->GetProperty(impProp)) {
            result = *config_implib;
          } else if (cmValue implib = this->GetProperty("IMPORTED_IMPLIB")) {
            result = *implib;
          }
        }
        break;
    }
  }

  if (result.empty()) {
    if (this->GetType() != cmStateEnums::INTERFACE_LIBRARY) {
      auto message = [&]() -> std::string {
        std::string unset;
        std::string configuration;

        if (artifact == cmStateEnums::RuntimeBinaryArtifact) {
          unset = "IMPORTED_LOCATION";
        } else if (artifact == cmStateEnums::ImportLibraryArtifact) {
          unset = "IMPORTED_IMPLIB";
        }

        if (!config.empty()) {
          configuration = cmStrCat(" configuration \"", config, "\"");
        }

        return cmStrCat(unset, " not set for imported target \"",
                        this->GetName(), "\"", configuration, ".");
      };

      switch (this->GetPolicyStatus(cmPolicies::CMP0111)) {
        case cmPolicies::WARN:
          this->impl->Makefile->IssueMessage(
            MessageType::AUTHOR_WARNING,
            cmPolicies::GetPolicyWarning(cmPolicies::CMP0111) + "\n" +
              message());
          CM_FALLTHROUGH;
        case cmPolicies::OLD:
          break;
        default:
          this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                             message());
      }
    }

    result = cmStrCat(this->GetName(), "-NOTFOUND");
  }
  return result;
}

const cmFileSet* cmTarget::GetFileSet(const std::string& name) const
{
  auto it = this->impl->FileSets.find(name);
  return it == this->impl->FileSets.end() ? nullptr : &it->second;
}

cmFileSet* cmTarget::GetFileSet(const std::string& name)
{
  auto it = this->impl->FileSets.find(name);
  return it == this->impl->FileSets.end() ? nullptr : &it->second;
}

std::pair<cmFileSet*, bool> cmTarget::GetOrCreateFileSet(
  const std::string& name, const std::string& type, cmFileSetVisibility vis)
{
  auto result = this->impl->FileSets.emplace(
    std::make_pair(name, cmFileSet(name, type, vis)));
  if (result.second) {
    auto bt = this->impl->Makefile->GetBacktrace();
    if (type == this->impl->HeadersFileSets.TypeName) {
      this->impl->HeadersFileSets.AddFileSet(name, vis, std::move(bt));
    }
  }
  return std::make_pair(&result.first->second, result.second);
}

std::string cmTarget::GetFileSetsPropertyName(const std::string& type)
{
  if (type == "HEADERS") {
    return "HEADER_SETS";
  }
  return "";
}

std::string cmTarget::GetInterfaceFileSetsPropertyName(const std::string& type)
{
  if (type == "HEADERS") {
    return "INTERFACE_HEADER_SETS";
  }
  return "";
}

std::vector<std::string> cmTarget::GetAllFileSetNames() const
{
  std::vector<std::string> result;

  for (auto const& it : this->impl->FileSets) {
    result.push_back(it.first);
  }

  return result;
}

std::vector<std::string> cmTarget::GetAllInterfaceFileSets() const
{
  std::vector<std::string> result;
  auto inserter = std::back_inserter(result);

  auto appendEntries = [=](const std::vector<BT<std::string>>& entries) {
    for (auto const& entry : entries) {
      auto expanded = cmExpandedList(entry.Value);
      std::copy(expanded.begin(), expanded.end(), inserter);
    }
  };

  appendEntries(this->impl->HeadersFileSets.InterfaceEntries.Entries);

  return result;
}

bool cmTargetInternals::CheckImportedLibName(std::string const& prop,
                                             std::string const& value) const
{
  if (this->TargetType != cmStateEnums::INTERFACE_LIBRARY ||
      !this->IsImportedTarget) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      prop +
        " property may be set only on imported INTERFACE library targets.");
    return false;
  }
  if (!value.empty()) {
    if (value[0] == '-') {
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                   prop + " property value\n  " + value +
                                     "\nmay not start with '-'.");
      return false;
    }
    std::string::size_type bad = value.find_first_of(":/\\;");
    if (bad != std::string::npos) {
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                   prop + " property value\n  " + value +
                                     "\nmay not contain '" +
                                     value.substr(bad, 1) + "'.");
      return false;
    }
  }
  return true;
}

bool cmTarget::GetMappedConfig(std::string const& desired_config, cmValue& loc,
                               cmValue& imp, std::string& suffix) const
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
  suffix = cmStrCat('_', config_upper);

  std::vector<std::string> mappedConfigs;
  {
    std::string mapProp = cmStrCat("MAP_IMPORTED_CONFIG_", config_upper);
    if (cmValue mapValue = this->GetProperty(mapProp)) {
      cmExpandList(*mapValue, mappedConfigs, true);
    }
  }

  // If we needed to find one of the mapped configurations but did not
  // On a DLL platform there may be only IMPORTED_IMPLIB for a shared
  // library or an executable with exports.
  bool allowImp = (this->IsDLLPlatform() &&
                   (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
                    this->IsExecutableWithExports())) ||
    (this->IsAIX() && this->IsExecutableWithExports());

  // If a mapping was found, check its configurations.
  for (auto mci = mappedConfigs.begin();
       !loc && !imp && mci != mappedConfigs.end(); ++mci) {
    // Look for this configuration.
    if (mci->empty()) {
      // An empty string in the mapping has a special meaning:
      // look up the config-less properties.
      loc = this->GetProperty(locPropBase);
      if (allowImp) {
        imp = this->GetProperty("IMPORTED_IMPLIB");
      }
      // If it was found, set the suffix.
      if (loc || imp) {
        suffix.clear();
      }
    } else {
      std::string mcUpper = cmSystemTools::UpperCase(*mci);
      std::string locProp = cmStrCat(locPropBase, '_', mcUpper);
      loc = this->GetProperty(locProp);
      if (allowImp) {
        std::string impProp = cmStrCat("IMPORTED_IMPLIB_", mcUpper);
        imp = this->GetProperty(impProp);
      }

      // If it was found, use it for all properties below.
      if (loc || imp) {
        suffix = cmStrCat('_', mcUpper);
      }
    }
  }

  // If we needed to find one of the mapped configurations but did not
  // then the target location is not found.  The project does not want
  // any other configuration.
  if (!mappedConfigs.empty() && !loc && !imp) {
    // Interface libraries are always available because their
    // library name is optional so it is okay to leave loc empty.
    return this->GetType() == cmStateEnums::INTERFACE_LIBRARY;
  }

  // If we have not yet found it then there are no mapped
  // configurations.  Look for an exact-match.
  if (!loc && !imp) {
    std::string locProp = cmStrCat(locPropBase, suffix);
    loc = this->GetProperty(locProp);
    if (allowImp) {
      std::string impProp = cmStrCat("IMPORTED_IMPLIB", suffix);
      imp = this->GetProperty(impProp);
    }
  }

  // If we have not yet found it then there are no mapped
  // configurations and no exact match.
  if (!loc && !imp) {
    // The suffix computed above is not useful.
    suffix.clear();

    // Look for a configuration-less location.  This may be set by
    // manually-written code.
    loc = this->GetProperty(locPropBase);
    if (allowImp) {
      imp = this->GetProperty("IMPORTED_IMPLIB");
    }
  }

  // If we have not yet found it then the project is willing to try
  // any available configuration.
  if (!loc && !imp) {
    std::vector<std::string> availableConfigs;
    if (cmValue iconfigs = this->GetProperty("IMPORTED_CONFIGURATIONS")) {
      cmExpandList(*iconfigs, availableConfigs);
    }
    for (auto aci = availableConfigs.begin();
         !loc && !imp && aci != availableConfigs.end(); ++aci) {
      suffix = cmStrCat('_', cmSystemTools::UpperCase(*aci));
      std::string locProp = cmStrCat(locPropBase, suffix);
      loc = this->GetProperty(locProp);
      if (allowImp) {
        std::string impProp = cmStrCat("IMPORTED_IMPLIB", suffix);
        imp = this->GetProperty(impProp);
      }
    }
  }
  // If we have not yet found it then the target location is not available.
  if (!loc && !imp) {
    // Interface libraries are always available because their
    // library name is optional so it is okay to leave loc empty.
    return this->GetType() == cmStateEnums::INTERFACE_LIBRARY;
  }

  return true;
}
