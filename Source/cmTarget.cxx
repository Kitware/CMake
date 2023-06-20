/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTarget.h"

#include <algorithm>
#include <cassert>
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
#include "cmList.h"
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
    cmList files{ entry.Value };
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

  enum class Action
  {
    Set,
    Append,
  };

  template <typename ValueType>
  bool WriteProperties(cmTarget* tgt, cmTargetInternals* impl,
                       const std::string& prop, ValueType value,
                       Action action);
  std::pair<bool, cmValue> ReadProperties(cmTarget const* tgt,
                                          cmTargetInternals const* impl,
                                          const std::string& prop) const;

  void AddFileSet(const std::string& name, cmFileSetVisibility vis,
                  cmListFileBacktrace bt);
};

struct UsageRequirementProperty
{
  enum class AppendEmpty
  {
    Yes,
    No,
  };

  UsageRequirementProperty(cm::static_string_view name,
                           AppendEmpty appendEmpty = AppendEmpty::No)
    : Name(name)
    , AppendBehavior(appendEmpty)
  {
  }

  void CopyFromDirectory(cmBTStringRange directoryEntries)
  {
    return cm::append(this->Entries, directoryEntries);
  }

  enum class Action
  {
    Set,
    Prepend,
    Append,
  };

  template <typename ValueType>
  bool Write(cmTargetInternals const* impl,
             cm::optional<cmListFileBacktrace> const& bt,
             const std::string& prop, ValueType value, Action action);
  template <typename ValueType>
  void WriteDirect(cmTargetInternals const* impl,
                   cm::optional<cmListFileBacktrace> const& bt,
                   ValueType value, Action action);
  void WriteDirect(BT<std::string> value, Action action);
  std::pair<bool, cmValue> Read(const std::string& prop) const;

  cm::static_string_view const Name;
  AppendEmpty const AppendBehavior;

  std::vector<BT<std::string>> Entries;
};

struct TargetProperty
{
  enum class InitCondition
  {
    // Always initialize the property.
    Always,
    // Never initialize the property.
    Never,
    // Only initialize if the target can compile sources.
    CanCompileSources,
    // Only apply to Xcode generators.
    NeedsXcode,
    // Only apply to Xcode generators on targets that can compile sources.
    NeedsXcodeAndCanCompileSources,
    // Needs to be a "normal" target (any non-global, non-utility target).
    NormalTarget,
    // Any non-imported target.
    NonImportedTarget,
    // Needs to be a "normal" target (any non-global, non-utility target) that
    // is not `IMPORTED`.
    NormalNonImportedTarget,
    // Needs to be a "normal" target with an artifact (no `INTERFACE`
    // libraries).
    TargetWithArtifact,
    // Needs to be a "normal" target with an artifact that is not an
    // executable.
    NonExecutableWithArtifact,
    // Needs to be a linkable library target (no `OBJECT` or `MODULE`
    // libraries).
    LinkableLibraryTarget,
    // Needs to be an executable.
    ExecutableTarget,
    // Needs to be a shared library (`SHARED`).
    SharedLibraryTarget,
    // Needs to be a target with meaningful symbol exports (`SHARED` or
    // `EXECUTABLE`).
    TargetWithSymbolExports,
    // Targets with "commands" associated with them. Basically everything
    // except global and `INTERFACE` targets.
    TargetWithCommands,
  };

  enum class Repetition
  {
    Once,
    PerConfig,
    PerConfigPrefix,
  };

  TargetProperty(cm::static_string_view name)
    : Name(name)
  {
  }

  TargetProperty(cm::static_string_view name, cm::static_string_view dflt,
                 InitCondition init)
    : Name(name)
    , Default(dflt)
    , InitConditional(init)
  {
  }

  TargetProperty(cm::static_string_view name, InitCondition init)
    : Name(name)
    , InitConditional(init)
  {
  }

  TargetProperty(cm::static_string_view name, InitCondition init,
                 Repetition repeat)
    : Name(name)
    , InitConditional(init)
    , Repeat(repeat)
  {
  }

  cm::static_string_view const Name;
  cm::optional<cm::static_string_view> const Default = {};
  InitCondition const InitConditional = InitCondition::Always;
  Repetition const Repeat = Repetition::Once;
};

#define IC TargetProperty::InitCondition
#define R TargetProperty::Repetition

/* clang-format off */
#define COMMON_LANGUAGE_PROPERTIES(lang)                                      \
  { #lang "_COMPILER_LAUNCHER"_s, IC::CanCompileSources },                    \
  { #lang "_STANDARD"_s, IC::CanCompileSources },                             \
  { #lang "_STANDARD_REQUIRED"_s, IC::CanCompileSources },                    \
  { #lang "_EXTENSIONS"_s, IC::CanCompileSources },                           \
  { #lang "_VISIBILITY_PRESET"_s, IC::CanCompileSources }
/* clang-format on */

TargetProperty const StaticTargetProperties[] = {
  /* clang-format off */
  // Compilation properties
  { "COMPILE_WARNING_AS_ERROR"_s, IC::CanCompileSources },
  { "INTERPROCEDURAL_OPTIMIZATION"_s, IC::CanCompileSources },
  { "INTERPROCEDURAL_OPTIMIZATION_"_s, IC::TargetWithArtifact, R::PerConfig },
  { "NO_SYSTEM_FROM_IMPORTED"_s, IC::CanCompileSources },
  // Set to `True` for `SHARED` and `MODULE` targets.
  { "POSITION_INDEPENDENT_CODE"_s, IC::CanCompileSources },
  { "VISIBILITY_INLINES_HIDDEN"_s, IC::CanCompileSources },
  // -- Features
  // ---- PCH
  { "DISABLE_PRECOMPILE_HEADERS"_s, IC::CanCompileSources },
  { "PCH_WARN_INVALID"_s, "ON"_s, IC::CanCompileSources },
  { "PCH_INSTANTIATE_TEMPLATES"_s, "ON"_s, IC::CanCompileSources },
  // -- Platforms
  // ---- Android
  { "ANDROID_API"_s, IC::CanCompileSources },
  { "ANDROID_API_MIN"_s, IC::CanCompileSources },
  { "ANDROID_ARCH"_s, IC::CanCompileSources },
  { "ANDROID_ASSETS_DIRECTORIES"_s, IC::CanCompileSources },
  { "ANDROID_JAVA_SOURCE_DIR"_s, IC::CanCompileSources },
  { "ANDROID_STL_TYPE"_s, IC::CanCompileSources },
  // ---- macOS
  { "OSX_ARCHITECTURES"_s, IC::CanCompileSources },
  // ---- Windows
  { "MSVC_DEBUG_INFORMATION_FORMAT"_s, IC::CanCompileSources },
  { "MSVC_RUNTIME_LIBRARY"_s, IC::CanCompileSources },
  { "VS_JUST_MY_CODE_DEBUGGING"_s, IC::CanCompileSources },
  { "VS_DEBUGGER_COMMAND"_s, IC::ExecutableTarget },
  { "VS_DEBUGGER_COMMAND_ARGUMENTS"_s, IC::ExecutableTarget },
  { "VS_DEBUGGER_ENVIRONMENT"_s, IC::ExecutableTarget },
  { "VS_DEBUGGER_WORKING_DIRECTORY"_s, IC::ExecutableTarget },
  // ---- OpenWatcom
  { "WATCOM_RUNTIME_LIBRARY"_s, IC::CanCompileSources },
  // -- Language
  // ---- C
  COMMON_LANGUAGE_PROPERTIES(C),
  // ---- C++
  COMMON_LANGUAGE_PROPERTIES(CXX),
  // ---- CSharp
  { "DOTNET_SDK"_s, IC::NonImportedTarget },
  { "DOTNET_TARGET_FRAMEWORK"_s, IC::TargetWithCommands },
  { "DOTNET_TARGET_FRAMEWORK_VERSION"_s, IC::TargetWithCommands },
  // ---- CUDA
  COMMON_LANGUAGE_PROPERTIES(CUDA),
  { "CUDA_SEPARABLE_COMPILATION"_s, IC::CanCompileSources },
  { "CUDA_ARCHITECTURES"_s, IC::CanCompileSources },
  // ---- Fortran
  { "Fortran_FORMAT"_s, IC::CanCompileSources },
  { "Fortran_MODULE_DIRECTORY"_s, IC::CanCompileSources },
  { "Fortran_COMPILER_LAUNCHER"_s, IC::CanCompileSources },
  { "Fortran_PREPRPOCESS"_s, IC::CanCompileSources },
  { "Fortran_VISIBILITY_PRESET"_s, IC::CanCompileSources },
  // ---- HIP
  COMMON_LANGUAGE_PROPERTIES(HIP),
  { "HIP_ARCHITECTURES"_s, IC::CanCompileSources },
  // ---- ISPC
  { "ISPC_COMPILER_LAUNCHER"_s, IC::CanCompileSources },
  { "ISPC_HEADER_DIRECTORY"_s, IC::CanCompileSources },
  { "ISPC_HEADER_SUFFIX"_s, "_ispc.h"_s, IC::CanCompileSources },
  { "ISPC_INSTRUCTION_SETS"_s, IC::CanCompileSources },
  // ---- Objective C
  COMMON_LANGUAGE_PROPERTIES(OBJC),
  // ---- Objective C++
  COMMON_LANGUAGE_PROPERTIES(OBJCXX),
  // ---- Swift
  { "Swift_LANGUAGE_VERSION"_s, IC::CanCompileSources },
  { "Swift_MODULE_DIRECTORY"_s, IC::CanCompileSources },
  // ---- moc
  { "AUTOMOC"_s, IC::CanCompileSources },
  { "AUTOMOC_COMPILER_PREDEFINES"_s, IC::CanCompileSources },
  { "AUTOMOC_MACRO_NAMES"_s, IC::CanCompileSources },
  { "AUTOMOC_MOC_OPTIONS"_s, IC::CanCompileSources },
  { "AUTOMOC_PATH_PREFIX"_s, IC::CanCompileSources },
  { "AUTOMOC_EXECUTABLE"_s, IC::CanCompileSources },
  // ---- uic
  { "AUTOUIC"_s, IC::CanCompileSources },
  { "AUTOUIC_OPTIONS"_s, IC::CanCompileSources },
  { "AUTOUIC_SEARCH_PATHS"_s, IC::CanCompileSources },
  { "AUTOUIC_EXECUTABLE"_s, IC::CanCompileSources },
  // ---- rcc
  { "AUTORCC"_s, IC::CanCompileSources },
  { "AUTORCC_OPTIONS"_s, IC::CanCompileSources },
  { "AUTORCC_EXECUTABLE"_s, IC::CanCompileSources },

  // Linking properties
  { "ENABLE_EXPORTS"_s, IC::TargetWithSymbolExports },
  { "LINK_LIBRARIES_ONLY_TARGETS"_s, IC::NormalNonImportedTarget },
  { "LINK_SEARCH_START_STATIC"_s, IC::CanCompileSources },
  { "LINK_SEARCH_END_STATIC"_s, IC::CanCompileSources },
  // Initialize per-configuration name postfix property from the variable only
  // for non-executable targets.  This preserves compatibility with previous
  // CMake versions in which executables did not support this variable.
  // Projects may still specify the property directly.
  { "_POSTFIX"_s, IC::NonExecutableWithArtifact, R::PerConfigPrefix },
  // -- Dependent library lookup
  { "MACOSX_RPATH"_s, IC::CanCompileSources },
  // ---- Build
  { "BUILD_RPATH"_s, IC::CanCompileSources },
  { "BUILD_RPATH_USE_ORIGIN"_s, IC::CanCompileSources },
  { "SKIP_BUILD_RPATH"_s, "OFF"_s, IC::CanCompileSources },
  { "BUILD_WITH_INSTALL_RPATH"_s, "OFF"_s, IC::CanCompileSources },
  { "BUILD_WITH_INSTALL_NAME_DIR"_s, IC::CanCompileSources },
  // ---- Install
  { "INSTALL_NAME_DIR"_s, IC::CanCompileSources },
  { "INSTALL_REMOVE_ENVIRONMENT_RPATH"_s, IC::CanCompileSources },
  { "INSTALL_RPATH"_s, ""_s, IC::CanCompileSources },
  { "INSTALL_RPATH_USE_LINK_PATH"_s, "OFF"_s, IC::CanCompileSources },
  // -- Platforms
  // ---- AIX
  { "AIX_EXPORT_ALL_SYMBOLS"_s, IC::TargetWithSymbolExports },
  // ---- Android
  { "ANDROID_GUI"_s, IC::ExecutableTarget },
  { "ANDROID_JAR_DIRECTORIES"_s, IC::CanCompileSources },
  { "ANDROID_JAR_DEPENDENCIES"_s, IC::CanCompileSources },
  { "ANDROID_NATIVE_LIB_DIRECTORIES"_s, IC::CanCompileSources },
  { "ANDROID_NATIVE_LIB_DEPENDENCIES"_s, IC::CanCompileSources },
  { "ANDROID_PROGUARD"_s, IC::CanCompileSources },
  { "ANDROID_PROGUARD_CONFIG_PATH"_s, IC::CanCompileSources },
  { "ANDROID_SECURE_PROPS_PATH"_s, IC::CanCompileSources },
  // ---- iOS
  { "IOS_INSTALL_COMBINED"_s, IC::CanCompileSources },
  // ---- macOS
  { "FRAMEWORK_MULTI_CONFIG_POSTFIX_"_s, IC::LinkableLibraryTarget, R::PerConfig },
  // ---- Windows
  { "DLL_NAME_WITH_SOVERSION"_s, IC::SharedLibraryTarget },
  { "GNUtoMS"_s, IC::CanCompileSources },
  { "WIN32_EXECUTABLE"_s, IC::CanCompileSources },
  { "WINDOWS_EXPORT_ALL_SYMBOLS"_s, IC::TargetWithSymbolExports },
  // -- Languages
  // ---- C
  { "C_LINKER_LAUNCHER"_s, IC::CanCompileSources },
  // ---- C++
  { "CXX_LINKER_LAUNCHER"_s, IC::CanCompileSources },
  // ---- CUDA
  { "CUDA_RESOLVE_DEVICE_SYMBOLS"_s, IC::CanCompileSources },
  { "CUDA_RUNTIME_LIBRARY"_s, IC::CanCompileSources },
  // ---- HIP
  { "HIP_RUNTIME_LIBRARY"_s, IC::CanCompileSources },
  // ---- Objective C
  { "OBJC_LINKER_LAUNCHER"_s, IC::CanCompileSources },
  // ---- Objective C++
  { "OBJCXX_LINKER_LAUNCHER"_s, IC::CanCompileSources },

  // Static analysis
  // -- C
  { "C_CLANG_TIDY"_s, IC::CanCompileSources },
  { "C_CLANG_TIDY_EXPORT_FIXES_DIR"_s, IC::CanCompileSources },
  { "C_CPPLINT"_s, IC::CanCompileSources },
  { "C_CPPCHECK"_s, IC::CanCompileSources },
  { "C_INCLUDE_WHAT_YOU_USE"_s, IC::CanCompileSources },
  // -- C++
  { "CXX_CLANG_TIDY"_s, IC::CanCompileSources },
  { "CXX_CLANG_TIDY_EXPORT_FIXES_DIR"_s, IC::CanCompileSources },
  { "CXX_CPPLINT"_s, IC::CanCompileSources },
  { "CXX_CPPCHECK"_s, IC::CanCompileSources },
  { "CXX_INCLUDE_WHAT_YOU_USE"_s, IC::CanCompileSources },
  // -- Objective C
  { "OBJC_CLANG_TIDY"_s, IC::CanCompileSources },
  { "OBJC_CLANG_TIDY_EXPORT_FIXES_DIR"_s, IC::CanCompileSources },
  // -- Objective C++
  { "OBJCXX_CLANG_TIDY"_s, IC::CanCompileSources },
  { "OBJCXX_CLANG_TIDY_EXPORT_FIXES_DIR"_s, IC::CanCompileSources },
  // -- Linking
  { "LINK_WHAT_YOU_USE"_s, IC::CanCompileSources },

  // Build graph properties
  { "LINK_DEPENDS_NO_SHARED"_s, IC::CanCompileSources },
  { "UNITY_BUILD"_s, IC::CanCompileSources },
  { "UNITY_BUILD_UNIQUE_ID"_s, IC::CanCompileSources },
  { "UNITY_BUILD_BATCH_SIZE"_s, "8"_s, IC::CanCompileSources },
  { "UNITY_BUILD_MODE"_s, "BATCH"_s, IC::CanCompileSources },
  { "OPTIMIZE_DEPENDENCIES"_s, IC::CanCompileSources },
  { "VERIFY_INTERFACE_HEADER_SETS"_s },
  // -- Android
  { "ANDROID_ANT_ADDITIONAL_OPTIONS"_s, IC::CanCompileSources },
  { "ANDROID_PROCESS_MAX"_s, IC::CanCompileSources },
  { "ANDROID_SKIP_ANT_STEP"_s, IC::CanCompileSources },
  // -- Autogen
  { "AUTOGEN_ORIGIN_DEPENDS"_s, IC::CanCompileSources },
  { "AUTOGEN_PARALLEL"_s, IC::CanCompileSources },
  { "AUTOGEN_USE_SYSTEM_INCLUDE"_s, IC::CanCompileSources },
  // -- moc
  { "AUTOMOC_DEPEND_FILTERS"_s, IC::CanCompileSources },
  // -- C++
  { "CXX_SCAN_FOR_MODULES"_s, IC::CanCompileSources },
  // -- Ninja
  { "JOB_POOL_COMPILE"_s, IC::CanCompileSources },
  { "JOB_POOL_LINK"_s, IC::CanCompileSources },
  { "JOB_POOL_PRECOMPILE_HEADER"_s, IC::CanCompileSources },
  // -- Visual Studio
  { "VS_NO_COMPILE_BATCHING"_s, IC::CanCompileSources },
  { "VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION"_s, IC::CanCompileSources},

  // Output location properties
  { "ARCHIVE_OUTPUT_DIRECTORY"_s, IC::CanCompileSources },
  { "ARCHIVE_OUTPUT_DIRECTORY_"_s, IC::TargetWithArtifact, R::PerConfig },
  { "COMPILE_PDB_OUTPUT_DIRECTORY"_s, IC::CanCompileSources },
  { "COMPILE_PDB_OUTPUT_DIRECTORY_"_s, IC::TargetWithArtifact, R::PerConfig },
  { "LIBRARY_OUTPUT_DIRECTORY"_s, IC::CanCompileSources },
  { "LIBRARY_OUTPUT_DIRECTORY_"_s, IC::TargetWithArtifact, R::PerConfig },
  { "PDB_OUTPUT_DIRECTORY"_s, IC::CanCompileSources },
  { "PDB_OUTPUT_DIRECTORY_"_s, IC::TargetWithArtifact, R::PerConfig },
  { "RUNTIME_OUTPUT_DIRECTORY"_s, IC::CanCompileSources },
  { "RUNTIME_OUTPUT_DIRECTORY_"_s, IC::TargetWithArtifact, R::PerConfig },

  // macOS bundle properties
  { "FRAMEWORK"_s, IC::CanCompileSources },
  { "FRAMEWORK_MULTI_CONFIG_POSTFIX"_s, IC::CanCompileSources },
  { "MACOSX_BUNDLE"_s, IC::CanCompileSources },

  // Usage requirement properties
  { "LINK_INTERFACE_LIBRARIES"_s, IC::CanCompileSources },
  { "MAP_IMPORTED_CONFIG_"_s, IC::NormalTarget, R::PerConfig },

  // Metadata
  { "CROSSCOMPILING_EMULATOR"_s, IC::ExecutableTarget },
  { "EXPORT_COMPILE_COMMANDS"_s, IC::CanCompileSources },
  { "FOLDER"_s },

  // Xcode properties
  { "XCODE_GENERATE_SCHEME"_s, IC::NeedsXcode },

#ifdef __APPLE__
  { "XCODE_SCHEME_ADDRESS_SANITIZER"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_ADDRESS_SANITIZER_USE_AFTER_RETURN"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_DEBUG_DOCUMENT_VERSIONING"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_THREAD_SANITIZER"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_THREAD_SANITIZER_STOP"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER_STOP"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_LAUNCH_CONFIGURATION"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_ENABLE_GPU_API_VALIDATION"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_ENABLE_GPU_SHADER_VALIDATION"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_WORKING_DIRECTORY"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_DISABLE_MAIN_THREAD_CHECKER"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_MAIN_THREAD_CHECKER_STOP"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_MALLOC_SCRIBBLE"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_MALLOC_GUARD_EDGES"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_GUARD_MALLOC"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_LAUNCH_MODE"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_ZOMBIE_OBJECTS"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_MALLOC_STACK"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_DYNAMIC_LINKER_API_USAGE"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_DYNAMIC_LIBRARY_LOADS"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_SCHEME_ENVIRONMENT"_s, IC::NeedsXcodeAndCanCompileSources },
  { "XCODE_LINK_BUILD_PHASE_MODE"_s, "NONE"_s, IC::NeedsXcodeAndCanCompileSources },
#endif
  /* clang-format on */
};

#undef COMMON_LANGUAGE_PROPERTIES
#undef IC
#undef R
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
  bool IsApple;
  bool IsAndroid;
  bool BuildInterfaceIncludesAppended;
  bool PerConfig;
  cmTarget::Visibility TargetVisibility;
  std::set<BT<std::pair<std::string, bool>>> Utilities;
  std::vector<cmCustomCommand> PreBuildCommands;
  std::vector<cmCustomCommand> PreLinkCommands;
  std::vector<cmCustomCommand> PostBuildCommands;
  std::vector<cmInstallTargetGenerator*> InstallGenerators;
  std::set<std::string> SystemIncludeDirectories;
  cmTarget::LinkLibraryVectorType OriginalLinkLibraries;
  std::map<std::string, BTs<std::string>> LanguageStandardProperties;
  std::map<cmTargetExport const*, std::vector<std::string>>
    InstallIncludeDirectoriesEntries;
  std::vector<std::pair<cmTarget::TLLSignature, cmListFileContext>>
    TLLCommands;
  std::map<std::string, cmFileSet> FileSets;
  cmListFileBacktrace Backtrace;

  UsageRequirementProperty IncludeDirectories;
  UsageRequirementProperty CompileOptions;
  UsageRequirementProperty CompileFeatures;
  UsageRequirementProperty CompileDefinitions;
  UsageRequirementProperty PrecompileHeaders;
  UsageRequirementProperty Sources;
  UsageRequirementProperty LinkOptions;
  UsageRequirementProperty LinkDirectories;
  UsageRequirementProperty LinkLibraries;
  UsageRequirementProperty InterfaceLinkLibraries;
  UsageRequirementProperty InterfaceLinkLibrariesDirect;
  UsageRequirementProperty InterfaceLinkLibrariesDirectExclude;

  FileSetType HeadersFileSets;
  FileSetType CxxModulesFileSets;

  cmTargetInternals();

  bool IsImported() const;

  bool CheckImportedLibName(std::string const& prop,
                            std::string const& value) const;

  std::string ProcessSourceItemCMP0049(const std::string& s) const;

  template <typename ValueType>
  void AddDirectoryToFileSet(cmTarget* self, std::string const& fileSetName,
                             ValueType value, cm::string_view fileSetType,
                             cm::string_view description,
                             FileSetType::Action action);
  template <typename ValueType>
  void AddPathToFileSet(cmTarget* self, std::string const& fileSetName,
                        ValueType value, cm::string_view fileSetType,
                        cm::string_view description,
                        FileSetType::Action action);
  cmValue GetFileSetDirectories(cmTarget const* self,
                                std::string const& fileSetName,
                                cm::string_view fileSetType) const;
  cmValue GetFileSetPaths(cmTarget const* self, std::string const& fileSetName,
                          cm::string_view fileSetType) const;

  cmListFileBacktrace GetBacktrace(
    cm::optional<cmListFileBacktrace> const& bt) const
  {
    return bt ? *bt : this->Makefile->GetBacktrace();
  }
};

cmTargetInternals::cmTargetInternals()
  : IncludeDirectories("INCLUDE_DIRECTORIES"_s)
  , CompileOptions("COMPILE_OPTIONS"_s)
  , CompileFeatures("COMPILE_FEATURES"_s)
  , CompileDefinitions("COMPILE_DEFINITIONS"_s)
  , PrecompileHeaders("PRECOMPILE_HEADERS"_s)
  , Sources("SOURCES"_s, UsageRequirementProperty::AppendEmpty::Yes)
  , LinkOptions("LINK_OPTIONS"_s)
  , LinkDirectories("LINK_DIRECTORIES"_s)
  , LinkLibraries("LINK_LIBRARIES"_s)
  , InterfaceLinkLibraries("INTERFACE_LINK_LIBRARIES"_s)
  , InterfaceLinkLibrariesDirect("INTERFACE_LINK_LIBRARIES_DIRECT"_s)
  , InterfaceLinkLibrariesDirectExclude(
      "INTERFACE_LINK_LIBRARIES_DIRECT_EXCLUDE"_s)
  , HeadersFileSets("HEADERS"_s, "HEADER_DIRS"_s, "HEADER_SET"_s,
                    "HEADER_DIRS_"_s, "HEADER_SET_"_s, "Header"_s,
                    "The default header set"_s, "Header set"_s,
                    FileSetEntries("HEADER_SETS"_s),
                    FileSetEntries("INTERFACE_HEADER_SETS"_s))
  , CxxModulesFileSets("CXX_MODULES"_s, "CXX_MODULE_DIRS"_s,
                       "CXX_MODULE_SET"_s, "CXX_MODULE_DIRS_"_s,
                       "CXX_MODULE_SET_"_s, "C++ module"_s,
                       "The default C++ module set"_s, "C++ module set"_s,
                       FileSetEntries("CXX_MODULE_SETS"_s),
                       FileSetEntries("INTERFACE_CXX_MODULE_SETS"_s))
{
}

template <typename ValueType>
bool FileSetType::WriteProperties(cmTarget* tgt, cmTargetInternals* impl,
                                  const std::string& prop, ValueType value,
                                  Action action)
{
  if (prop == this->DefaultDirectoryProperty) {
    impl->AddDirectoryToFileSet(tgt, std::string(this->TypeName), value,
                                this->TypeName, this->DefaultDescription,
                                action);
    return true;
  }
  if (prop == this->DefaultPathProperty) {
    impl->AddPathToFileSet(tgt, std::string(this->TypeName), value,
                           this->TypeName, this->DefaultDescription, action);
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
        cmStrCat(this->ArbitraryDescription, " \"", fileSetName, "\""),
        action);
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
        cmStrCat(this->ArbitraryDescription, " \"", fileSetName, "\""),
        action);
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
    output = cmList::to_string(this->SelfEntries.Entries);
    value = cmValue(output);
    did_read = true;
  } else if (prop == this->InterfaceEntries.PropertyName) {
    static std::string output;
    output = cmList::to_string(this->InterfaceEntries.Entries);
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

template <typename ValueType>
bool UsageRequirementProperty::Write(
  cmTargetInternals const* impl, cm::optional<cmListFileBacktrace> const& bt,
  const std::string& prop, ValueType value, Action action)
{
  if (prop == this->Name) {
    this->WriteDirect(impl, bt, value, action);
    return true;
  }
  return false;
}

template <typename ValueType>
void UsageRequirementProperty::WriteDirect(
  cmTargetInternals const* impl, cm::optional<cmListFileBacktrace> const& bt,
  ValueType value, Action action)
{
  if (action == Action::Set) {
    this->Entries.clear();
  }
  if (value) {
    cmListFileBacktrace lfbt = impl->GetBacktrace(bt);
    if (action == Action::Prepend) {
      this->Entries.emplace(this->Entries.begin(), value, lfbt);
    } else if (action == Action::Set || cmNonempty(value) ||
               this->AppendBehavior == AppendEmpty::Yes) {
      this->Entries.emplace_back(value, lfbt);
    }
  }
}

void UsageRequirementProperty::WriteDirect(BT<std::string> value,
                                           Action action)
{
  if (action == Action::Set) {
    this->Entries.clear();
  }
  if (action == Action::Prepend) {
    this->Entries.emplace(this->Entries.begin(), std::move(value));
  } else {
    this->Entries.emplace_back(std::move(value));
  }
}

std::pair<bool, cmValue> UsageRequirementProperty::Read(
  const std::string& prop) const
{
  bool did_read = false;
  cmValue value = nullptr;
  if (prop == this->Name) {
    if (!this->Entries.empty()) {
      // Storage to back the returned `cmValue`.
      static std::string output;
      output = cmList::to_string(this->Entries);
      value = cmValue(output);
    }
    did_read = true;
  }
  return { did_read, value };
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
  this->impl->IsApple = false;
  this->impl->IsAndroid = false;
  this->impl->TargetVisibility = vis;
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

  // Check whether we are targeting Apple.
  this->impl->IsApple = this->impl->Makefile->IsOn("APPLE");

  // Check whether we are targeting an Android platform.
  this->impl->IsAndroid = (this->impl->Makefile->GetSafeDefinition(
                             "CMAKE_SYSTEM_NAME") == "Android");

  // Save the backtrace of target construction.
  this->impl->Backtrace = this->impl->Makefile->GetBacktrace();

  if (this->IsNormal()) {
    // Initialize the INCLUDE_DIRECTORIES property based on the current value
    // of the same directory property:
    this->impl->IncludeDirectories.CopyFromDirectory(
      this->impl->Makefile->GetIncludeDirectoriesEntries());

    {
      auto const& sysInc = this->impl->Makefile->GetSystemIncludeDirectories();
      this->impl->SystemIncludeDirectories.insert(sysInc.begin(),
                                                  sysInc.end());
    }

    this->impl->CompileOptions.CopyFromDirectory(
      this->impl->Makefile->GetCompileOptionsEntries());
    this->impl->LinkOptions.CopyFromDirectory(
      this->impl->Makefile->GetLinkOptionsEntries());
    this->impl->LinkDirectories.CopyFromDirectory(
      this->impl->Makefile->GetLinkDirectoriesEntries());
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

  std::set<TargetProperty::InitCondition> metConditions;
  metConditions.insert(TargetProperty::InitCondition::Always);
  if (this->CanCompileSources()) {
    metConditions.insert(TargetProperty::InitCondition::CanCompileSources);
  }
  if (this->GetGlobalGenerator()->IsXcode()) {
    metConditions.insert(TargetProperty::InitCondition::NeedsXcode);
    if (this->CanCompileSources()) {
      metConditions.insert(
        TargetProperty::InitCondition::NeedsXcodeAndCanCompileSources);
    }
  }
  if (!this->IsImported()) {
    metConditions.insert(TargetProperty::InitCondition::NonImportedTarget);
  }
  if (this->impl->TargetType != cmStateEnums::UTILITY &&
      this->impl->TargetType != cmStateEnums::GLOBAL_TARGET) {
    metConditions.insert(TargetProperty::InitCondition::NormalTarget);
    if (this->IsNormal()) {
      metConditions.insert(
        TargetProperty::InitCondition::NormalNonImportedTarget);
    }
    if (this->impl->TargetType != cmStateEnums::INTERFACE_LIBRARY) {
      metConditions.insert(TargetProperty::InitCondition::TargetWithArtifact);
      if (this->impl->TargetType != cmStateEnums::EXECUTABLE) {
        metConditions.insert(
          TargetProperty::InitCondition::NonExecutableWithArtifact);
      }
    }
    if (this->impl->TargetType == cmStateEnums::SHARED_LIBRARY ||
        this->impl->TargetType == cmStateEnums::STATIC_LIBRARY) {
      metConditions.insert(
        TargetProperty::InitCondition::LinkableLibraryTarget);
    }
    if (this->impl->TargetType == cmStateEnums::SHARED_LIBRARY) {
      metConditions.insert(TargetProperty::InitCondition::SharedLibraryTarget);
    }
  }
  if (this->impl->TargetType == cmStateEnums::EXECUTABLE) {
    metConditions.insert(TargetProperty::InitCondition::ExecutableTarget);
  }
  if (this->impl->TargetType == cmStateEnums::SHARED_LIBRARY ||
      this->impl->TargetType == cmStateEnums::EXECUTABLE) {
    metConditions.insert(
      TargetProperty::InitCondition::TargetWithSymbolExports);
  }
  if (this->impl->TargetType <= cmStateEnums::GLOBAL_TARGET) {
    metConditions.insert(TargetProperty::InitCondition::TargetWithCommands);
  }

  std::vector<std::string> configNames =
    mf->GetGeneratorConfigs(cmMakefile::ExcludeEmptyConfig);
  for (auto& config : configNames) {
    config = cmSystemTools::UpperCase(config);
  }

  std::string defKey;
  defKey.reserve(128);
  defKey += "CMAKE_";
  auto initProperty = [this, mf, &defKey](const std::string& property,
                                          const char* default_value) {
    // special init for ENABLE_EXPORTS
    // For SHARED_LIBRARY, only CMAKE_SHARED_LIBRARY_ENABLE_EXPORTS variable
    // is used
    // For EXECUTABLE, CMAKE_EXECUTABLE_ENABLE_EXPORTS or else
    // CMAKE_ENABLE_EXPORTS variables are used
    if (property == "ENABLE_EXPORTS"_s) {
      // Replace everything after "CMAKE_"
      defKey.replace(
        defKey.begin() + 6, defKey.end(),
        cmStrCat(this->impl->TargetType == cmStateEnums::EXECUTABLE
                   ? "EXECUTABLE"
                   : "SHARED_LIBRARY",
                 '_', property));
      if (cmValue value = mf->GetDefinition(defKey)) {
        this->SetProperty(property, value);
        return;
      }
      if (this->impl->TargetType == cmStateEnums::SHARED_LIBRARY) {
        if (default_value) {
          this->SetProperty(property, default_value);
        }
        return;
      }
    }

    // Replace everything after "CMAKE_"
    defKey.replace(defKey.begin() + 6, defKey.end(), property);
    if (cmValue value = mf->GetDefinition(defKey)) {
      this->SetProperty(property, value);
    } else if (default_value) {
      this->SetProperty(property, default_value);
    }
  };

  std::string dflt_storage;
  for (auto const& tp : StaticTargetProperties) {
    // Ignore properties that we have not met the condition for.
    if (!metConditions.count(tp.InitConditional)) {
      continue;
    }

    const char* dflt = nullptr;
    if (tp.Default) {
      dflt_storage = std::string(*tp.Default);
      dflt = dflt_storage.c_str();
    }

    if (tp.Repeat == TargetProperty::Repetition::Once) {
      initProperty(std::string(tp.Name), dflt);
    } else {
      std::string propertyName;
      for (auto const& configName : configNames) {
        if (tp.Repeat == TargetProperty::Repetition::PerConfig) {
          propertyName = cmStrCat(tp.Name, configName);
        } else if (tp.Repeat == TargetProperty::Repetition::PerConfigPrefix) {
          propertyName = cmStrCat(configName, tp.Name);
        }
        initProperty(propertyName, dflt);
      }
    }
  }

  // Clean up some property defaults.
  if (this->impl->TargetType == cmStateEnums::SHARED_LIBRARY ||
      this->impl->TargetType == cmStateEnums::MODULE_LIBRARY) {
    this->SetProperty("POSITION_INDEPENDENT_CODE", "True");
  }

  // check for "CMAKE_VS_GLOBALS" variable and set up target properties
  // if any
  cmValue globals = mf->GetDefinition("CMAKE_VS_GLOBALS");
  if (globals) {
    const std::string genName = mf->GetGlobalGenerator()->GetName();
    if (cmHasLiteralPrefix(genName, "Visual Studio")) {
      cmList props{ *globals };
      const std::string vsGlobal = "VS_GLOBAL_";
      for (const std::string& i : props) {
        // split NAME=VALUE
        const std::string::size_type assignment = i.find('=');
        if (assignment != std::string::npos) {
          const std::string propName = vsGlobal + i.substr(0, assignment);
          const std::string propValue = i.substr(assignment + 1);
          initProperty(propName, propValue.c_str());
        }
      }
    }
  }

  if (!this->IsNormal() || mf->GetPropertyAsBool("SYSTEM")) {
    this->SetProperty("SYSTEM", "ON");
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
  for (auto const& entry : this->impl->CompileFeatures.Entries) {
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

void cmTarget::AddUtility(std::string const& name, bool cross,
                          cmMakefile const* mf)
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

bool cmTarget::IsSharedLibraryWithExports() const
{
  return (this->GetType() == cmStateEnums::SHARED_LIBRARY &&
          this->GetPropertyAsBool("ENABLE_EXPORTS"));
}

bool cmTarget::IsFrameworkOnApple() const
{
  return ((this->GetType() == cmStateEnums::SHARED_LIBRARY ||
           this->GetType() == cmStateEnums::STATIC_LIBRARY) &&
          this->IsApple() && this->GetPropertyAsBool("FRAMEWORK"));
}

bool cmTarget::IsAppBundleOnApple() const
{
  return (this->GetType() == cmStateEnums::EXECUTABLE && this->IsApple() &&
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
    this->impl->Sources.WriteDirect(this->impl.get(), {},
                                    cmValue(cmJoin(srcs, ";")),
                                    UsageRequirementProperty::Action::Append);
  }
}

void cmTarget::AddSources(std::vector<std::string> const& srcs)
{
  std::vector<std::string> srcFiles;
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
    srcFiles.emplace_back(filename);
  }
  this->AddTracedSources(srcFiles);
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
    cmList files{ entry.Value };
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
  auto const& sources = this->impl->Sources.Entries;
  if (std::find_if(sources.begin(), sources.end(),
                   TargetPropertyEntryFinder(sfl)) == sources.end()) {
    this->impl->Sources.WriteDirect(
      this->impl.get(), {}, cmValue(src),
      before ? UsageRequirementProperty::Action::Prepend
             : UsageRequirementProperty::Action::Append);
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
                         this->GetDebugGeneratorExpressions(libName, llt),
                         mf.GetBacktrace());
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
  return cmMakeRange(this->impl->IncludeDirectories.Entries);
}

cmBTStringRange cmTarget::GetCompileOptionsEntries() const
{
  return cmMakeRange(this->impl->CompileOptions.Entries);
}

cmBTStringRange cmTarget::GetCompileFeaturesEntries() const
{
  return cmMakeRange(this->impl->CompileFeatures.Entries);
}

cmBTStringRange cmTarget::GetCompileDefinitionsEntries() const
{
  return cmMakeRange(this->impl->CompileDefinitions.Entries);
}

cmBTStringRange cmTarget::GetPrecompileHeadersEntries() const
{
  return cmMakeRange(this->impl->PrecompileHeaders.Entries);
}

cmBTStringRange cmTarget::GetSourceEntries() const
{
  return cmMakeRange(this->impl->Sources.Entries);
}

cmBTStringRange cmTarget::GetLinkOptionsEntries() const
{
  return cmMakeRange(this->impl->LinkOptions.Entries);
}

cmBTStringRange cmTarget::GetLinkDirectoriesEntries() const
{
  return cmMakeRange(this->impl->LinkDirectories.Entries);
}

cmBTStringRange cmTarget::GetLinkImplementationEntries() const
{
  return cmMakeRange(this->impl->LinkLibraries.Entries);
}

cmBTStringRange cmTarget::GetLinkInterfaceEntries() const
{
  return cmMakeRange(this->impl->InterfaceLinkLibraries.Entries);
}

cmBTStringRange cmTarget::GetLinkInterfaceDirectEntries() const
{
  return cmMakeRange(this->impl->InterfaceLinkLibrariesDirect.Entries);
}

cmBTStringRange cmTarget::GetLinkInterfaceDirectExcludeEntries() const
{
  return cmMakeRange(this->impl->InterfaceLinkLibrariesDirectExclude.Entries);
}

cmBTStringRange cmTarget::GetHeaderSetsEntries() const
{
  return cmMakeRange(this->impl->HeadersFileSets.SelfEntries.Entries);
}

cmBTStringRange cmTarget::GetCxxModuleSetsEntries() const
{
  return cmMakeRange(this->impl->CxxModulesFileSets.SelfEntries.Entries);
}

cmBTStringRange cmTarget::GetInterfaceHeaderSetsEntries() const
{
  return cmMakeRange(this->impl->HeadersFileSets.InterfaceEntries.Entries);
}

cmBTStringRange cmTarget::GetInterfaceCxxModuleSetsEntries() const
{
  return cmMakeRange(this->impl->CxxModulesFileSets.InterfaceEntries.Entries);
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
MAKE_PROP(CUDA_CUBIN_COMPILATION);
MAKE_PROP(CUDA_FATBIN_COMPILATION);
MAKE_PROP(CUDA_OPTIX_COMPILATION);
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

void cmTarget::SetProperty(const std::string& prop, cmValue value)
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

  UsageRequirementProperty* usageRequirements[] = {
    &this->impl->IncludeDirectories,
    &this->impl->CompileOptions,
    &this->impl->CompileFeatures,
    &this->impl->CompileDefinitions,
    &this->impl->PrecompileHeaders,
    &this->impl->Sources,
    &this->impl->LinkOptions,
    &this->impl->LinkDirectories,
    &this->impl->LinkLibraries,
    &this->impl->InterfaceLinkLibraries,
    &this->impl->InterfaceLinkLibrariesDirect,
    &this->impl->InterfaceLinkLibrariesDirectExclude,
  };

  for (auto* usageRequirement : usageRequirements) {
    if (usageRequirement->Write(this->impl.get(), {}, prop, value,
                                UsageRequirementProperty::Action::Set)) {
      return;
    }
  }

  FileSetType* fileSetTypes[] = {
    &this->impl->HeadersFileSets,
    &this->impl->CxxModulesFileSets,
  };

  for (auto* fileSetType : fileSetTypes) {
    if (fileSetType->WriteProperties(this, this->impl.get(), prop, value,
                                     FileSetType::Action::Set)) {
      return;
    }
  }

  if (prop == propIMPORTED_GLOBAL) {
    if (!cmIsOn(value)) {
      std::ostringstream e;
      e << "IMPORTED_GLOBAL property can't be set to FALSE on targets (\""
        << this->impl->Name << "\")\n";
      this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return;
    }
    /* no need to change anything if value does not change */
    if (!this->IsImportedGloballyVisible()) {
      this->impl->TargetVisibility = Visibility::ImportedGlobally;
      this->GetGlobalGenerator()->IndexTarget(this);
    }
  } else if (cmHasLiteralPrefix(prop, "IMPORTED_LIBNAME") &&
             !this->impl->CheckImportedLibName(
               prop,
               value ? value
                     : std::string{})) { // NOLINT(bugprone-branch-clone)
    /* error was reported by check method */
  } else if (prop == propCUDA_CUBIN_COMPILATION ||
             prop == propCUDA_FATBIN_COMPILATION ||
             prop == propCUDA_OPTIX_COMPILATION ||
             prop == propCUDA_PTX_COMPILATION) {
    auto const& compiler =
      this->impl->Makefile->GetSafeDefinition("CMAKE_CUDA_COMPILER_ID");
    auto const& compilerVersion =
      this->impl->Makefile->GetSafeDefinition("CMAKE_CUDA_COMPILER_VERSION");
    if (this->GetType() != cmStateEnums::OBJECT_LIBRARY) {
      auto e =
        cmStrCat(prop, " property can only be applied to OBJECT targets(",
                 this->impl->Name, ")\n");
      this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
      return;
    }
    const bool flag_found =
      (prop == propCUDA_PTX_COMPILATION &&
       this->impl->Makefile->GetDefinition("_CMAKE_CUDA_PTX_FLAG")) ||
      (prop == propCUDA_CUBIN_COMPILATION &&
       this->impl->Makefile->GetDefinition("_CMAKE_CUDA_CUBIN_FLAG")) ||
      (prop == propCUDA_FATBIN_COMPILATION &&
       this->impl->Makefile->GetDefinition("_CMAKE_CUDA_FATBIN_FLAG")) ||
      (prop == propCUDA_OPTIX_COMPILATION &&
       this->impl->Makefile->GetDefinition("_CMAKE_CUDA_OPTIX_FLAG"));
    if (flag_found) {
      this->impl->Properties.SetProperty(prop, value);
    } else {
      auto e = cmStrCat(prop, " property is not supported by ", compiler,
                        "  compiler version ", compilerVersion, ".");
      this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e);
      return;
    }
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
      reusedFrom = *value;
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
  } else {
    this->impl->Properties.SetProperty(prop, value);
  }
}

void cmTarget::AppendProperty(const std::string& prop,
                              const std::string& value,
                              cm::optional<cmListFileBacktrace> const& bt,
                              bool asString)
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
  if (prop == propPRECOMPILE_HEADERS &&
      this->GetProperty("PRECOMPILE_HEADERS_REUSE_FROM")) {
    std::ostringstream e;
    e << "PRECOMPILE_HEADERS_REUSE_FROM property is already set on target "
         "(\""
      << this->impl->Name << "\")\n";
    this->impl->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }

  UsageRequirementProperty* usageRequirements[] = {
    &this->impl->IncludeDirectories,
    &this->impl->CompileOptions,
    &this->impl->CompileFeatures,
    &this->impl->CompileDefinitions,
    &this->impl->PrecompileHeaders,
    &this->impl->Sources,
    &this->impl->LinkOptions,
    &this->impl->LinkDirectories,
    &this->impl->LinkLibraries,
    &this->impl->InterfaceLinkLibraries,
    &this->impl->InterfaceLinkLibrariesDirect,
    &this->impl->InterfaceLinkLibrariesDirectExclude,
  };

  for (auto* usageRequirement : usageRequirements) {
    if (usageRequirement->Write(this->impl.get(), bt, prop, cmValue(value),
                                UsageRequirementProperty::Action::Append)) {
      return;
    }
  }

  FileSetType* fileSetTypes[] = {
    &this->impl->HeadersFileSets,
    &this->impl->CxxModulesFileSets,
  };

  for (auto* fileSetType : fileSetTypes) {
    if (fileSetType->WriteProperties(this, this->impl.get(), prop, value,
                                     FileSetType::Action::Append)) {
      return;
    }
  }

  if (cmHasLiteralPrefix(prop, "IMPORTED_LIBNAME")) {
    this->impl->Makefile->IssueMessage(
      MessageType::FATAL_ERROR, prop + " property may not be APPENDed.");
  } else if (prop == "C_STANDARD" || prop == "CXX_STANDARD" ||
             prop == "CUDA_STANDARD" || prop == "HIP_STANDARD" ||
             prop == "OBJC_STANDARD" || prop == "OBJCXX_STANDARD") {
    this->impl->Makefile->IssueMessage(
      MessageType::FATAL_ERROR, prop + " property may not be appended.");
  } else {
    this->impl->Properties.AppendProperty(prop, value, asString);
  }
}

template <typename ValueType>
void cmTargetInternals::AddDirectoryToFileSet(cmTarget* self,
                                              std::string const& fileSetName,
                                              ValueType value,
                                              cm::string_view fileSetType,
                                              cm::string_view description,
                                              FileSetType::Action action)
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
  if (action == FileSetType::Action::Set) {
    fileSet->ClearDirectoryEntries();
  }
  if (cmNonempty(value)) {
    fileSet->AddDirectoryEntry(
      BT<std::string>(value, this->Makefile->GetBacktrace()));
  }
}

template <typename ValueType>
void cmTargetInternals::AddPathToFileSet(cmTarget* self,
                                         std::string const& fileSetName,
                                         ValueType value,
                                         cm::string_view fileSetType,
                                         cm::string_view description,
                                         FileSetType::Action action)
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
  if (action == FileSetType::Action::Set) {
    fileSet->ClearFileEntries();
  }
  if (cmNonempty(value)) {
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
  output = cmList::to_string(fileSet->GetDirectoryEntries());
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
  output = cmList::to_string(fileSet->GetFileEntries());
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

namespace {
bool CheckLinkLibraryPattern(UsageRequirementProperty const& usage,
                             cmake* context)
{
  // Look for <LINK_LIBRARY:> and </LINK_LIBRARY:> internal tags
  static cmsys::RegularExpression linkPattern(
    "(^|;)(</?LINK_(LIBRARY|GROUP):[^;>]*>)(;|$)");

  bool isValid = true;

  for (const auto& item : usage.Entries) {
    if (!linkPattern.find(item.Value)) {
      continue;
    }

    isValid = false;

    // Report an error.
    context->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat(
        "Property ", usage.Name, " contains the invalid item \"",
        linkPattern.match(2), "\". The ", usage.Name,
        " property may contain the generator-expression \"$<LINK_",
        linkPattern.match(3),
        ":...>\" which may be used to specify how the libraries are linked."),
      item.Backtrace);
  }

  return isValid;
}
}

void cmTarget::FinalizeTargetConfiguration(
  const cmBTStringRange& noConfigCompileDefinitions,
  cm::optional<std::map<std::string, cmValue>>& perConfigCompileDefinitions)
{
  if (this->GetType() == cmStateEnums::GLOBAL_TARGET) {
    return;
  }

  if (!CheckLinkLibraryPattern(this->impl->LinkLibraries,
                               this->GetMakefile()->GetCMakeInstance()) ||
      !CheckLinkLibraryPattern(this->impl->InterfaceLinkLibraries,
                               this->GetMakefile()->GetCMakeInstance()) ||
      !CheckLinkLibraryPattern(this->impl->InterfaceLinkLibrariesDirect,
                               this->GetMakefile()->GetCMakeInstance())) {
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
  this->impl->IncludeDirectories.WriteDirect(
    entry,
    before ? UsageRequirementProperty::Action::Prepend
           : UsageRequirementProperty::Action::Append);
}

void cmTarget::InsertCompileOption(BT<std::string> const& entry, bool before)
{
  this->impl->CompileOptions.WriteDirect(
    entry,
    before ? UsageRequirementProperty::Action::Prepend
           : UsageRequirementProperty::Action::Append);
}

void cmTarget::InsertCompileDefinition(BT<std::string> const& entry)
{
  this->impl->CompileDefinitions.WriteDirect(
    entry, UsageRequirementProperty::Action::Append);
}

void cmTarget::InsertLinkOption(BT<std::string> const& entry, bool before)
{
  this->impl->LinkOptions.WriteDirect(
    entry,
    before ? UsageRequirementProperty::Action::Prepend
           : UsageRequirementProperty::Action::Append);
}

void cmTarget::InsertLinkDirectory(BT<std::string> const& entry, bool before)
{
  this->impl->LinkDirectories.WriteDirect(
    entry,
    before ? UsageRequirementProperty::Action::Prepend
           : UsageRequirementProperty::Action::Append);
}

void cmTarget::InsertPrecompileHeader(BT<std::string> const& entry)
{
  this->impl->PrecompileHeaders.WriteDirect(
    entry, UsageRequirementProperty::Action::Append);
}

namespace {
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

    UsageRequirementProperty const* usageRequirements[] = {
      &this->impl->IncludeDirectories,
      &this->impl->CompileOptions,
      &this->impl->CompileFeatures,
      &this->impl->CompileDefinitions,
      &this->impl->PrecompileHeaders,
      &this->impl->Sources,
      &this->impl->LinkOptions,
      &this->impl->LinkDirectories,
      &this->impl->LinkLibraries,
      &this->impl->InterfaceLinkLibraries,
      &this->impl->InterfaceLinkLibrariesDirect,
      &this->impl->InterfaceLinkLibrariesDirectExclude,
    };

    for (auto const* usageRequirement : usageRequirements) {
      auto value = usageRequirement->Read(prop);
      if (value.first) {
        return value.second;
      }
    }

    // the type property returns what type the target is
    if (prop == propTYPE) {
      return cmValue(cmState::GetTargetTypeName(this->GetType()));
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
      output = cmList::to_string(utilities);
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
    FileSetType* fileSetTypes[] = {
      &this->impl->HeadersFileSets,
      &this->impl->CxxModulesFileSets,
    };

    for (auto* fileSetType : fileSetTypes) {
      auto value = fileSetType->ReadProperties(this, this->impl.get(), prop);
      if (value.first) {
        return value.second;
      }
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
bool cmTarget::IsApple() const
{
  return this->impl->IsApple;
}

bool cmTarget::IsNormal() const
{
  switch (this->impl->TargetVisibility) {
    case Visibility::Normal:
      return true;
    case Visibility::Generated:
    case Visibility::Imported:
    case Visibility::ImportedGlobally:
      return false;
  }
  assert(false && "unknown visibility (IsNormal)");
  return false;
}

bool cmTarget::IsSynthetic() const
{
  switch (this->impl->TargetVisibility) {
    case Visibility::Generated:
      return true;
    case Visibility::Normal:
    case Visibility::Imported:
    case Visibility::ImportedGlobally:
      return false;
  }
  assert(false && "unknown visibility (IsSynthetic)");
  return false;
}

bool cmTargetInternals::IsImported() const
{
  switch (this->TargetVisibility) {
    case cmTarget::Visibility::Imported:
    case cmTarget::Visibility::ImportedGlobally:
      return true;
    case cmTarget::Visibility::Normal:
    case cmTarget::Visibility::Generated:
      return false;
  }
  assert(false && "unknown visibility (IsImported)");
  return false;
}

bool cmTarget::IsImported() const
{
  return this->impl->IsImported();
}

bool cmTarget::IsImportedGloballyVisible() const
{
  switch (this->impl->TargetVisibility) {
    case Visibility::ImportedGlobally:
      return true;
    case Visibility::Normal:
    case Visibility::Generated:
    case Visibility::Imported:
      return false;
  }
  assert(false && "unknown visibility (IsImportedGloballyVisible)");
  return false;
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
          return this->IsApple() ? "CMAKE_APPLE_IMPORT_FILE_SUFFIX"
                                 : "CMAKE_IMPORT_LIBRARY_SUFFIX";
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
          return this->IsApple() ? "CMAKE_APPLE_IMPORT_FILE_PREFIX"
                                 : "CMAKE_IMPORT_LIBRARY_PREFIX";
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
    name,
    cmFileSet(*this->GetMakefile()->GetCMakeInstance(), name, type, vis));
  if (result.second) {
    auto bt = this->impl->Makefile->GetBacktrace();
    if (type == this->impl->HeadersFileSets.TypeName) {
      this->impl->HeadersFileSets.AddFileSet(name, vis, std::move(bt));
    } else if (type == this->impl->CxxModulesFileSets.TypeName) {
      this->impl->CxxModulesFileSets.AddFileSet(name, vis, std::move(bt));
    }
  }
  return std::make_pair(&result.first->second, result.second);
}

std::string cmTarget::GetFileSetsPropertyName(const std::string& type)
{
  if (type == "HEADERS") {
    return "HEADER_SETS";
  }
  if (type == "CXX_MODULES") {
    return "CXX_MODULE_SETS";
  }
  return "";
}

std::string cmTarget::GetInterfaceFileSetsPropertyName(const std::string& type)
{
  if (type == "HEADERS") {
    return "INTERFACE_HEADER_SETS";
  }
  if (type == "CXX_MODULES") {
    return "INTERFACE_CXX_MODULE_SETS";
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
      cmList expanded{ entry.Value };
      std::copy(expanded.begin(), expanded.end(), inserter);
    }
  };

  appendEntries(this->impl->HeadersFileSets.InterfaceEntries.Entries);
  appendEntries(this->impl->CxxModulesFileSets.InterfaceEntries.Entries);

  return result;
}

bool cmTargetInternals::CheckImportedLibName(std::string const& prop,
                                             std::string const& value) const
{
  if (this->TargetType != cmStateEnums::INTERFACE_LIBRARY ||
      !this->IsImported()) {
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

  cmList mappedConfigs;
  {
    std::string mapProp = cmStrCat("MAP_IMPORTED_CONFIG_", config_upper);
    if (cmValue mapValue = this->GetProperty(mapProp)) {
      mappedConfigs.assign(*mapValue, cmList::EmptyElements::Yes);
    }
  }

  // If we needed to find one of the mapped configurations but did not
  // On a DLL platform there may be only IMPORTED_IMPLIB for a shared
  // library or an executable with exports.
  bool allowImp = (this->IsDLLPlatform() &&
                   (this->GetType() == cmStateEnums::SHARED_LIBRARY ||
                    this->IsExecutableWithExports())) ||
    (this->IsAIX() && this->IsExecutableWithExports()) ||
    (this->GetMakefile()->PlatformSupportsAppleTextStubs() &&
     this->IsSharedLibraryWithExports());

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
    cmList availableConfigs;
    if (cmValue iconfigs = this->GetProperty("IMPORTED_CONFIGURATIONS")) {
      availableConfigs.assign(*iconfigs);
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
