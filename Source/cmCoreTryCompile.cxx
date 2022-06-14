/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCoreTryCompile.h"

#include <cstdio>
#include <cstring>
#include <set>
#include <sstream>
#include <utility>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmsys/Directory.hxx"

#include "cmExportTryCompileFileGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmPolicies.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"
#include "cmVersion.h"
#include "cmake.h"

namespace {
class LanguageStandardState
{
public:
  LanguageStandardState(std::string&& lang)
    : StandardFlag(lang + "_STANDARD")
    , RequiredFlag(lang + "_STANDARD_REQUIRED")
    , ExtensionFlag(lang + "_EXTENSIONS")
  {
  }

  void Enabled(bool isEnabled) { this->IsEnabled = isEnabled; }

  bool UpdateIfMatches(std::vector<std::string> const& argv, size_t& index)
  {
    bool updated = false;
    if (argv[index] == this->StandardFlag) {
      this->DidStandard = true;
      this->StandardValue = argv[++index];
      updated = true;
    } else if (argv[index] == this->RequiredFlag) {
      this->DidStandardRequired = true;
      this->RequiredValue = argv[++index];
      updated = true;
    } else if (argv[index] == this->ExtensionFlag) {
      this->DidExtensions = true;
      this->ExtensionValue = argv[++index];
      updated = true;
    }
    return updated;
  }

  bool Validate(cmMakefile* const makefile) const
  {
    if (this->DidStandard) {
      makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat(this->StandardFlag,
                 " allowed only in source file signature."));
      return false;
    }
    if (this->DidStandardRequired) {
      makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat(this->RequiredFlag,
                 " allowed only in source file signature."));
      return false;
    }
    if (this->DidExtensions) {
      makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat(this->ExtensionFlag,
                 " allowed only in source file signature."));
      return false;
    }

    return true;
  }

  bool DidNone() const
  {
    return !this->DidStandard && !this->DidStandardRequired &&
      !this->DidExtensions;
  }

  void LoadUnsetPropertyValues(cmMakefile* const makefile, bool honorStandard,
                               bool warnCMP0067,
                               std::vector<std::string>& warnCMP0067Variables)
  {
    if (!this->IsEnabled) {
      return;
    }

    auto lookupStdVar = [&](std::string const& var) -> std::string {
      std::string value = makefile->GetSafeDefinition(var);
      if (warnCMP0067 && !value.empty()) {
        value.clear();
        warnCMP0067Variables.emplace_back(var);
      }
      return value;
    };

    if (honorStandard || warnCMP0067) {
      if (!this->DidStandard) {
        this->StandardValue =
          lookupStdVar(cmStrCat("CMAKE_", this->StandardFlag));
      }
      if (!this->DidStandardRequired) {
        this->RequiredValue =
          lookupStdVar(cmStrCat("CMAKE_", this->RequiredFlag));
      }
      if (!this->DidExtensions) {
        this->ExtensionValue =
          lookupStdVar(cmStrCat("CMAKE_", this->ExtensionFlag));
      }
    }
  }

  void WriteProperties(FILE* fout, std::string const& targetName) const
  {
    if (!this->IsEnabled) {
      return;
    }

    auto writeProp = [&](std::string const& prop, std::string const& value) {
      fprintf(fout, "set_property(TARGET %s PROPERTY %s %s)\n",
              targetName.c_str(),
              cmOutputConverter::EscapeForCMake(prop).c_str(),
              cmOutputConverter::EscapeForCMake(value).c_str());
    };

    if (!this->StandardValue.empty()) {
      writeProp(this->StandardFlag, this->StandardValue);
    }
    if (!this->RequiredValue.empty()) {
      writeProp(this->RequiredFlag, this->RequiredValue);
    }
    if (!this->ExtensionValue.empty()) {
      writeProp(this->ExtensionFlag, this->ExtensionValue);
    }
  }

private:
  bool IsEnabled = false;
  bool DidStandard = false;
  bool DidStandardRequired = false;
  bool DidExtensions = false;

  std::string StandardFlag;
  std::string RequiredFlag;
  std::string ExtensionFlag;

  std::string StandardValue;
  std::string RequiredValue;
  std::string ExtensionValue;
};

constexpr size_t lang_property_start = 0;
constexpr size_t lang_property_size = 4;
constexpr size_t pie_property_start = 4;
constexpr size_t pie_property_size = 2;
#define SETUP_LANGUAGE(name, lang)                                            \
  static const std::string name[lang_property_size + pie_property_size + 1] = \
    { "CMAKE_" #lang "_COMPILER_EXTERNAL_TOOLCHAIN",                          \
      "CMAKE_" #lang "_COMPILER_TARGET",                                      \
      "CMAKE_" #lang "_LINK_NO_PIE_SUPPORTED",                                \
      "CMAKE_" #lang "_PIE_SUPPORTED", "" }

// NOLINTNEXTLINE(bugprone-suspicious-missing-comma)
SETUP_LANGUAGE(c_properties, C);
// NOLINTNEXTLINE(bugprone-suspicious-missing-comma)
SETUP_LANGUAGE(cxx_properties, CXX);

// NOLINTNEXTLINE(bugprone-suspicious-missing-comma)
SETUP_LANGUAGE(cuda_properties, CUDA);
// NOLINTNEXTLINE(bugprone-suspicious-missing-comma)
SETUP_LANGUAGE(fortran_properties, Fortran);
// NOLINTNEXTLINE(bugprone-suspicious-missing-comma)
SETUP_LANGUAGE(hip_properties, HIP);
// NOLINTNEXTLINE(bugprone-suspicious-missing-comma)
SETUP_LANGUAGE(objc_properties, OBJC);
// NOLINTNEXTLINE(bugprone-suspicious-missing-comma)
SETUP_LANGUAGE(objcxx_properties, OBJCXX);
// NOLINTNEXTLINE(bugprone-suspicious-missing-comma)
SETUP_LANGUAGE(ispc_properties, ISPC);
// NOLINTNEXTLINE(bugprone-suspicious-missing-comma)
SETUP_LANGUAGE(swift_properties, Swift);
#undef SETUP_LANGUAGE

std::string const kCMAKE_CUDA_ARCHITECTURES = "CMAKE_CUDA_ARCHITECTURES";
std::string const kCMAKE_CUDA_RUNTIME_LIBRARY = "CMAKE_CUDA_RUNTIME_LIBRARY";
std::string const kCMAKE_ENABLE_EXPORTS = "CMAKE_ENABLE_EXPORTS";
std::string const kCMAKE_HIP_ARCHITECTURES = "CMAKE_HIP_ARCHITECTURES";
std::string const kCMAKE_HIP_RUNTIME_LIBRARY = "CMAKE_HIP_RUNTIME_LIBRARY";
std::string const kCMAKE_ISPC_INSTRUCTION_SETS = "CMAKE_ISPC_INSTRUCTION_SETS";
std::string const kCMAKE_ISPC_HEADER_SUFFIX = "CMAKE_ISPC_HEADER_SUFFIX";
std::string const kCMAKE_LINK_SEARCH_END_STATIC =
  "CMAKE_LINK_SEARCH_END_STATIC";
std::string const kCMAKE_LINK_SEARCH_START_STATIC =
  "CMAKE_LINK_SEARCH_START_STATIC";
std::string const kCMAKE_MSVC_RUNTIME_LIBRARY_DEFAULT =
  "CMAKE_MSVC_RUNTIME_LIBRARY_DEFAULT";
std::string const kCMAKE_OSX_ARCHITECTURES = "CMAKE_OSX_ARCHITECTURES";
std::string const kCMAKE_OSX_DEPLOYMENT_TARGET = "CMAKE_OSX_DEPLOYMENT_TARGET";
std::string const kCMAKE_OSX_SYSROOT = "CMAKE_OSX_SYSROOT";
std::string const kCMAKE_APPLE_ARCH_SYSROOTS = "CMAKE_APPLE_ARCH_SYSROOTS";
std::string const kCMAKE_POSITION_INDEPENDENT_CODE =
  "CMAKE_POSITION_INDEPENDENT_CODE";
std::string const kCMAKE_SYSROOT = "CMAKE_SYSROOT";
std::string const kCMAKE_SYSROOT_COMPILE = "CMAKE_SYSROOT_COMPILE";
std::string const kCMAKE_SYSROOT_LINK = "CMAKE_SYSROOT_LINK";
std::string const kCMAKE_ARMClang_CMP0123 = "CMAKE_ARMClang_CMP0123";
std::string const kCMAKE_TRY_COMPILE_OSX_ARCHITECTURES =
  "CMAKE_TRY_COMPILE_OSX_ARCHITECTURES";
std::string const kCMAKE_TRY_COMPILE_PLATFORM_VARIABLES =
  "CMAKE_TRY_COMPILE_PLATFORM_VARIABLES";
std::string const kCMAKE_WARN_DEPRECATED = "CMAKE_WARN_DEPRECATED";
std::string const kCMAKE_WATCOM_RUNTIME_LIBRARY_DEFAULT =
  "CMAKE_WATCOM_RUNTIME_LIBRARY_DEFAULT";

/* GHS Multi platform variables */
std::set<std::string> const ghs_platform_vars{
  "GHS_TARGET_PLATFORM", "GHS_PRIMARY_TARGET", "GHS_TOOLSET_ROOT",
  "GHS_OS_ROOT",         "GHS_OS_DIR",         "GHS_BSP_NAME",
  "GHS_OS_DIR_OPTION"
};
}

int cmCoreTryCompile::TryCompileCode(std::vector<std::string> const& argv,
                                     bool isTryRun)
{
  this->BinaryDirectory = argv[1];
  this->OutputFile.clear();
  // which signature were we called with ?
  this->SrcFileSignature = true;

  cmStateEnums::TargetType targetType = cmStateEnums::EXECUTABLE;
  cmValue tt = this->Makefile->GetDefinition("CMAKE_TRY_COMPILE_TARGET_TYPE");
  if (!isTryRun && cmNonempty(tt)) {
    if (*tt == cmState::GetTargetTypeName(cmStateEnums::EXECUTABLE)) {
      targetType = cmStateEnums::EXECUTABLE;
    } else if (*tt ==
               cmState::GetTargetTypeName(cmStateEnums::STATIC_LIBRARY)) {
      targetType = cmStateEnums::STATIC_LIBRARY;
    } else {
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Invalid value '", *tt,
                 "' for CMAKE_TRY_COMPILE_TARGET_TYPE.  Only '",
                 cmState::GetTargetTypeName(cmStateEnums::EXECUTABLE),
                 "' and '",
                 cmState::GetTargetTypeName(cmStateEnums::STATIC_LIBRARY),
                 "' are allowed."));
      return -1;
    }
  }

  std::string sourceDirectory = argv[2];
  std::string projectName;
  std::string targetName;
  std::vector<std::string> cmakeFlags(1, "CMAKE_FLAGS"); // fake argv[0]
  std::vector<std::string> compileDefs;
  std::string cmakeInternal;
  std::string outputVariable;
  std::string copyFile;
  std::string copyFileError;
  LanguageStandardState cState("C");
  LanguageStandardState cudaState("CUDA");
  LanguageStandardState cxxState("CXX");
  LanguageStandardState hipState("HIP");
  LanguageStandardState objcState("OBJC");
  LanguageStandardState objcxxState("OBJCXX");
  std::vector<std::string> targets;
  std::vector<std::string> linkOptions;
  std::string libsToLink = " ";
  bool useOldLinkLibs = true;
  char targetNameBuf[64];
  bool didOutputVariable = false;
  bool didCopyFile = false;
  bool didCopyFileError = false;
  bool useSources = argv[2] == "SOURCES";
  std::vector<std::string> sources;

  enum Doing
  {
    DoingNone,
    DoingCMakeFlags,
    DoingCompileDefinitions,
    DoingLinkOptions,
    DoingLinkLibraries,
    DoingOutputVariable,
    DoingCopyFile,
    DoingCopyFileError,
    DoingSources,
    DoingCMakeInternal
  };
  Doing doing = useSources ? DoingSources : DoingNone;
  for (size_t i = 3; i < argv.size(); ++i) {
    if (argv[i] == "CMAKE_FLAGS") {
      doing = DoingCMakeFlags;
    } else if (argv[i] == "COMPILE_DEFINITIONS") {
      doing = DoingCompileDefinitions;
    } else if (argv[i] == "LINK_OPTIONS") {
      doing = DoingLinkOptions;
    } else if (argv[i] == "LINK_LIBRARIES") {
      doing = DoingLinkLibraries;
      useOldLinkLibs = false;
    } else if (argv[i] == "OUTPUT_VARIABLE") {
      doing = DoingOutputVariable;
      didOutputVariable = true;
    } else if (argv[i] == "COPY_FILE") {
      doing = DoingCopyFile;
      didCopyFile = true;
    } else if (argv[i] == "COPY_FILE_ERROR") {
      doing = DoingCopyFileError;
      didCopyFileError = true;
    } else if (cState.UpdateIfMatches(argv, i) ||
               cxxState.UpdateIfMatches(argv, i) ||
               cudaState.UpdateIfMatches(argv, i) ||
               hipState.UpdateIfMatches(argv, i) ||
               objcState.UpdateIfMatches(argv, i) ||
               objcxxState.UpdateIfMatches(argv, i)) {
      continue;
    } else if (argv[i] == "__CMAKE_INTERNAL") {
      doing = DoingCMakeInternal;
    } else if (doing == DoingCMakeFlags) {
      cmakeFlags.emplace_back(argv[i]);
    } else if (doing == DoingCompileDefinitions) {
      cmExpandList(argv[i], compileDefs);
    } else if (doing == DoingLinkOptions) {
      linkOptions.emplace_back(argv[i]);
    } else if (doing == DoingLinkLibraries) {
      libsToLink += "\"" + cmTrimWhitespace(argv[i]) + "\" ";
      if (cmTarget* tgt = this->Makefile->FindTargetToUse(argv[i])) {
        switch (tgt->GetType()) {
          case cmStateEnums::SHARED_LIBRARY:
          case cmStateEnums::STATIC_LIBRARY:
          case cmStateEnums::INTERFACE_LIBRARY:
          case cmStateEnums::UNKNOWN_LIBRARY:
            break;
          case cmStateEnums::EXECUTABLE:
            if (tgt->IsExecutableWithExports()) {
              break;
            }
            CM_FALLTHROUGH;
          default:
            this->Makefile->IssueMessage(
              MessageType::FATAL_ERROR,
              cmStrCat("Only libraries may be used as try_compile or try_run "
                       "IMPORTED LINK_LIBRARIES.  Got ",
                       tgt->GetName(), " of type ",
                       cmState::GetTargetTypeName(tgt->GetType()), "."));
            return -1;
        }
        if (tgt->IsImported()) {
          targets.emplace_back(argv[i]);
        }
      }
    } else if (doing == DoingOutputVariable) {
      outputVariable = argv[i];
      doing = DoingNone;
    } else if (doing == DoingCopyFile) {
      copyFile = argv[i];
      doing = DoingNone;
    } else if (doing == DoingCopyFileError) {
      copyFileError = argv[i];
      doing = DoingNone;
    } else if (doing == DoingSources) {
      sources.emplace_back(argv[i]);
    } else if (doing == DoingCMakeInternal) {
      cmakeInternal = argv[i];
      doing = DoingNone;
    } else if (i == 3) {
      this->SrcFileSignature = false;
      projectName = argv[i];
    } else if (i == 4 && !this->SrcFileSignature) {
      targetName = argv[i];
    } else {
      std::ostringstream m;
      m << "try_compile given unknown argument \"" << argv[i] << "\".";
      this->Makefile->IssueMessage(MessageType::AUTHOR_WARNING, m.str());
    }
  }

  if (didCopyFile && copyFile.empty()) {
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                 "COPY_FILE must be followed by a file path");
    return -1;
  }

  if (didCopyFileError && copyFileError.empty()) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "COPY_FILE_ERROR must be followed by a variable name");
    return -1;
  }

  if (didCopyFileError && !didCopyFile) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "COPY_FILE_ERROR may be used only with COPY_FILE");
    return -1;
  }

  if (didOutputVariable && outputVariable.empty()) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "OUTPUT_VARIABLE must be followed by a variable name");
    return -1;
  }

  if (useSources && sources.empty()) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "SOURCES must be followed by at least one source file");
    return -1;
  }

  if (!this->SrcFileSignature) {
    if (!cState.Validate(this->Makefile)) {
      return -1;
    }
    if (!cudaState.Validate(this->Makefile)) {
      return -1;
    }
    if (!hipState.Validate(this->Makefile)) {
      return -1;
    }
    if (!cxxState.Validate(this->Makefile)) {
      return -1;
    }
    if (!objcState.Validate(this->Makefile)) {
      return -1;
    }
    if (!objcxxState.Validate(this->Makefile)) {
      return -1;
    }
  }

  // compute the binary dir when TRY_COMPILE is called with a src file
  // signature
  if (this->SrcFileSignature) {
    this->BinaryDirectory += "/CMakeFiles/CMakeTmp";
  } else {
    // only valid for srcfile signatures
    if (!compileDefs.empty()) {
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        "COMPILE_DEFINITIONS specified on a srcdir type TRY_COMPILE");
      return -1;
    }
    if (!copyFile.empty()) {
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        "COPY_FILE specified on a srcdir type TRY_COMPILE");
      return -1;
    }
  }
  // make sure the binary directory exists
  cmSystemTools::MakeDirectory(this->BinaryDirectory);

  // do not allow recursive try Compiles
  if (this->BinaryDirectory == this->Makefile->GetHomeOutputDirectory()) {
    std::ostringstream e;
    e << "Attempt at a recursive or nested TRY_COMPILE in directory\n"
      << "  " << this->BinaryDirectory << "\n";
    this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return -1;
  }

  std::string outFileName = this->BinaryDirectory + "/CMakeLists.txt";
  // which signature are we using? If we are using var srcfile bindir
  if (this->SrcFileSignature) {
    // remove any CMakeCache.txt files so we will have a clean test
    std::string ccFile = this->BinaryDirectory + "/CMakeCache.txt";
    cmSystemTools::RemoveFile(ccFile);

    // Choose sources.
    if (!useSources) {
      sources.emplace_back(argv[2]);
    }

    // Detect languages to enable.
    cmGlobalGenerator* gg = this->Makefile->GetGlobalGenerator();
    std::set<std::string> testLangs;
    for (std::string const& si : sources) {
      std::string ext = cmSystemTools::GetFilenameLastExtension(si);
      std::string lang = gg->GetLanguageFromExtension(ext.c_str());
      if (!lang.empty()) {
        testLangs.insert(lang);
      } else {
        std::ostringstream err;
        err << "Unknown extension \"" << ext << "\" for file\n"
            << "  " << si << "\n"
            << "try_compile() works only for enabled languages.  "
            << "Currently these are:\n  ";
        std::vector<std::string> langs;
        gg->GetEnabledLanguages(langs);
        err << cmJoin(langs, " ");
        err << "\nSee project() command to enable other languages.";
        this->Makefile->IssueMessage(MessageType::FATAL_ERROR, err.str());
        return -1;
      }
    }

    // when the only language is ISPC we know that the output
    // type must by a static library
    if (testLangs.size() == 1 && testLangs.count("ISPC") == 1) {
      targetType = cmStateEnums::STATIC_LIBRARY;
    }

    std::string const tcConfig =
      this->Makefile->GetSafeDefinition("CMAKE_TRY_COMPILE_CONFIGURATION");

    // we need to create a directory and CMakeLists file etc...
    // first create the directories
    sourceDirectory = this->BinaryDirectory;

    // now create a CMakeLists.txt file in that directory
    FILE* fout = cmsys::SystemTools::Fopen(outFileName, "w");
    if (!fout) {
      std::ostringstream e;
      /* clang-format off */
      e << "Failed to open\n"
        << "  " << outFileName << "\n"
        << cmSystemTools::GetLastSystemError();
      /* clang-format on */
      this->Makefile->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return -1;
    }

    cmValue def = this->Makefile->GetDefinition("CMAKE_MODULE_PATH");
    fprintf(fout, "cmake_minimum_required(VERSION %u.%u.%u.%u)\n",
            cmVersion::GetMajorVersion(), cmVersion::GetMinorVersion(),
            cmVersion::GetPatchVersion(), cmVersion::GetTweakVersion());
    if (def) {
      fprintf(fout, "set(CMAKE_MODULE_PATH \"%s\")\n", def->c_str());
    }

    /* Set MSVC runtime library policy to match our selection.  */
    if (cmValue msvcRuntimeLibraryDefault =
          this->Makefile->GetDefinition(kCMAKE_MSVC_RUNTIME_LIBRARY_DEFAULT)) {
      fprintf(fout, "cmake_policy(SET CMP0091 %s)\n",
              !msvcRuntimeLibraryDefault->empty() ? "NEW" : "OLD");
    }

    /* Set Watcom runtime library policy to match our selection.  */
    if (cmValue watcomRuntimeLibraryDefault = this->Makefile->GetDefinition(
          kCMAKE_WATCOM_RUNTIME_LIBRARY_DEFAULT)) {
      fprintf(fout, "cmake_policy(SET CMP0136 %s)\n",
              !watcomRuntimeLibraryDefault->empty() ? "NEW" : "OLD");
    }

    /* Set CUDA architectures policy to match outer project.  */
    if (this->Makefile->GetPolicyStatus(cmPolicies::CMP0104) !=
          cmPolicies::NEW &&
        testLangs.find("CUDA") != testLangs.end() &&
        this->Makefile->GetSafeDefinition(kCMAKE_CUDA_ARCHITECTURES).empty()) {
      fprintf(fout, "cmake_policy(SET CMP0104 OLD)\n");
    }

    /* Set ARMClang cpu/arch policy to match outer project.  */
    if (cmValue cmp0123 =
          this->Makefile->GetDefinition(kCMAKE_ARMClang_CMP0123)) {
      fprintf(fout, "cmake_policy(SET CMP0123 %s)\n",
              *cmp0123 == "NEW"_s ? "NEW" : "OLD");
    }

    /* Set cache/normal variable policy to match outer project.
       It may affect toolchain files.  */
    if (this->Makefile->GetPolicyStatus(cmPolicies::CMP0126) !=
        cmPolicies::NEW) {
      fprintf(fout, "cmake_policy(SET CMP0126 OLD)\n");
    }

    std::string projectLangs;
    for (std::string const& li : testLangs) {
      projectLangs += " " + li;
      std::string rulesOverrideBase = "CMAKE_USER_MAKE_RULES_OVERRIDE";
      std::string rulesOverrideLang = cmStrCat(rulesOverrideBase, "_", li);
      if (cmValue rulesOverridePath =
            this->Makefile->GetDefinition(rulesOverrideLang)) {
        fprintf(fout, "set(%s \"%s\")\n", rulesOverrideLang.c_str(),
                rulesOverridePath->c_str());
      } else if (cmValue rulesOverridePath2 =
                   this->Makefile->GetDefinition(rulesOverrideBase)) {
        fprintf(fout, "set(%s \"%s\")\n", rulesOverrideBase.c_str(),
                rulesOverridePath2->c_str());
      }
    }
    fprintf(fout, "project(CMAKE_TRY_COMPILE%s)\n", projectLangs.c_str());
    if (cmakeInternal == "ABI") {
      // This is the ABI detection step, also used for implicit includes.
      // Erase any include_directories() calls from the toolchain file so
      // that we do not see them as implicit.  Our ABI detection source
      // does not include any system headers anyway.
      fprintf(fout,
              "set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES \"\")\n");
    }
    fprintf(fout, "set(CMAKE_VERBOSE_MAKEFILE 1)\n");
    for (std::string const& li : testLangs) {
      std::string langFlags = "CMAKE_" + li + "_FLAGS";
      cmValue flags = this->Makefile->GetDefinition(langFlags);
      fprintf(fout, "set(CMAKE_%s_FLAGS %s)\n", li.c_str(),
              cmOutputConverter::EscapeForCMake(*flags).c_str());
      fprintf(fout,
              "set(CMAKE_%s_FLAGS \"${CMAKE_%s_FLAGS}"
              " ${COMPILE_DEFINITIONS}\")\n",
              li.c_str(), li.c_str());
    }
    switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0066)) {
      case cmPolicies::WARN:
        if (this->Makefile->PolicyOptionalWarningEnabled(
              "CMAKE_POLICY_WARNING_CMP0066")) {
          std::ostringstream w;
          /* clang-format off */
          w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0066) << "\n"
            "For compatibility with older versions of CMake, try_compile "
            "is not honoring caller config-specific compiler flags "
            "(e.g. CMAKE_C_FLAGS_DEBUG) in the test project."
            ;
          /* clang-format on */
          this->Makefile->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
        }
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        // OLD behavior is to do nothing.
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        this->Makefile->IssueMessage(
          MessageType::FATAL_ERROR,
          cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0066));
        CM_FALLTHROUGH;
      case cmPolicies::NEW: {
        // NEW behavior is to pass config-specific compiler flags.
        static std::string const cfgDefault = "DEBUG";
        std::string const cfg =
          !tcConfig.empty() ? cmSystemTools::UpperCase(tcConfig) : cfgDefault;
        for (std::string const& li : testLangs) {
          std::string const langFlagsCfg =
            cmStrCat("CMAKE_", li, "_FLAGS_", cfg);
          cmValue flagsCfg = this->Makefile->GetDefinition(langFlagsCfg);
          fprintf(fout, "set(%s %s)\n", langFlagsCfg.c_str(),
                  cmOutputConverter::EscapeForCMake(*flagsCfg).c_str());
        }
      } break;
    }
    switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0056)) {
      case cmPolicies::WARN:
        if (this->Makefile->PolicyOptionalWarningEnabled(
              "CMAKE_POLICY_WARNING_CMP0056")) {
          std::ostringstream w;
          /* clang-format off */
          w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0056) << "\n"
            "For compatibility with older versions of CMake, try_compile "
            "is not honoring caller link flags (e.g. CMAKE_EXE_LINKER_FLAGS) "
            "in the test project."
            ;
          /* clang-format on */
          this->Makefile->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
        }
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        // OLD behavior is to do nothing.
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        this->Makefile->IssueMessage(
          MessageType::FATAL_ERROR,
          cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0056));
        CM_FALLTHROUGH;
      case cmPolicies::NEW:
        // NEW behavior is to pass linker flags.
        {
          cmValue exeLinkFlags =
            this->Makefile->GetDefinition("CMAKE_EXE_LINKER_FLAGS");
          fprintf(fout, "set(CMAKE_EXE_LINKER_FLAGS %s)\n",
                  cmOutputConverter::EscapeForCMake(*exeLinkFlags).c_str());
        }
        break;
    }
    fprintf(fout,
            "set(CMAKE_EXE_LINKER_FLAGS \"${CMAKE_EXE_LINKER_FLAGS}"
            " ${EXE_LINKER_FLAGS}\")\n");
    fprintf(fout, "include_directories(${INCLUDE_DIRECTORIES})\n");
    fprintf(fout, "set(CMAKE_SUPPRESS_REGENERATION 1)\n");
    fprintf(fout, "link_directories(${LINK_DIRECTORIES})\n");
    // handle any compile flags we need to pass on
    if (!compileDefs.empty()) {
      // Pass using bracket arguments to preserve content.
      fprintf(fout, "add_definitions([==[%s]==])\n",
              cmJoin(compileDefs, "]==] [==[").c_str());
    }

    /* Use a random file name to avoid rapid creation and deletion
       of the same executable name (some filesystems fail on that).  */
    snprintf(targetNameBuf, sizeof(targetNameBuf), "cmTC_%05x",
             cmSystemTools::RandomSeed() & 0xFFFFF);
    targetName = targetNameBuf;

    if (!targets.empty()) {
      std::string fname = "/" + std::string(targetName) + "Targets.cmake";
      cmExportTryCompileFileGenerator tcfg(gg, targets, this->Makefile,
                                           testLangs);
      tcfg.SetExportFile((this->BinaryDirectory + fname).c_str());
      tcfg.SetConfig(tcConfig);

      if (!tcfg.GenerateImportFile()) {
        this->Makefile->IssueMessage(MessageType::FATAL_ERROR,
                                     "could not write export file.");
        fclose(fout);
        return -1;
      }
      fprintf(fout, "\ninclude(\"${CMAKE_CURRENT_LIST_DIR}/%s\")\n\n",
              fname.c_str());
    }

    /* Set the appropriate policy information for ENABLE_EXPORTS */
    fprintf(fout, "cmake_policy(SET CMP0065 %s)\n",
            this->Makefile->GetPolicyStatus(cmPolicies::CMP0065) ==
                cmPolicies::NEW
              ? "NEW"
              : "OLD");

    /* Set the appropriate policy information for PIE link flags */
    fprintf(fout, "cmake_policy(SET CMP0083 %s)\n",
            this->Makefile->GetPolicyStatus(cmPolicies::CMP0083) ==
                cmPolicies::NEW
              ? "NEW"
              : "OLD");

    // Workaround for -Wl,-headerpad_max_install_names issue until we can avoid
    // adding that flag in the platform and compiler language files
    fprintf(fout,
            "include(\"${CMAKE_ROOT}/Modules/Internal/"
            "HeaderpadWorkaround.cmake\")\n");

    if (targetType == cmStateEnums::EXECUTABLE) {
      /* Put the executable at a known location (for COPY_FILE).  */
      fprintf(fout, "set(CMAKE_RUNTIME_OUTPUT_DIRECTORY \"%s\")\n",
              this->BinaryDirectory.c_str());
      /* Create the actual executable.  */
      fprintf(fout, "add_executable(%s", targetName.c_str());
    } else // if (targetType == cmStateEnums::STATIC_LIBRARY)
    {
      /* Put the static library at a known location (for COPY_FILE).  */
      fprintf(fout, "set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY \"%s\")\n",
              this->BinaryDirectory.c_str());
      /* Create the actual static library.  */
      fprintf(fout, "add_library(%s STATIC", targetName.c_str());
    }
    for (std::string const& si : sources) {
      fprintf(fout, " \"%s\"", si.c_str());

      // Add dependencies on any non-temporary sources.
      if (si.find("CMakeTmp") == std::string::npos) {
        this->Makefile->AddCMakeDependFile(si);
      }
    }
    fprintf(fout, ")\n");

    cState.Enabled(testLangs.find("C") != testLangs.end());
    cxxState.Enabled(testLangs.find("CXX") != testLangs.end());
    cudaState.Enabled(testLangs.find("CUDA") != testLangs.end());
    hipState.Enabled(testLangs.find("HIP") != testLangs.end());
    objcState.Enabled(testLangs.find("OBJC") != testLangs.end());
    objcxxState.Enabled(testLangs.find("OBJCXX") != testLangs.end());

    bool warnCMP0067 = false;
    bool honorStandard = true;

    if (cState.DidNone() && cxxState.DidNone() && objcState.DidNone() &&
        objcxxState.DidNone() && cudaState.DidNone() && hipState.DidNone()) {
      switch (this->Makefile->GetPolicyStatus(cmPolicies::CMP0067)) {
        case cmPolicies::WARN:
          warnCMP0067 = this->Makefile->PolicyOptionalWarningEnabled(
            "CMAKE_POLICY_WARNING_CMP0067");
          CM_FALLTHROUGH;
        case cmPolicies::OLD:
          // OLD behavior is to not honor the language standard variables.
          honorStandard = false;
          break;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
          this->Makefile->IssueMessage(
            MessageType::FATAL_ERROR,
            cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0067));
          break;
        case cmPolicies::NEW:
          // NEW behavior is to honor the language standard variables.
          // We already initialized honorStandard to true.
          break;
      }
    }

    std::vector<std::string> warnCMP0067Variables;

    cState.LoadUnsetPropertyValues(this->Makefile, honorStandard, warnCMP0067,
                                   warnCMP0067Variables);
    cxxState.LoadUnsetPropertyValues(this->Makefile, honorStandard,
                                     warnCMP0067, warnCMP0067Variables);
    cudaState.LoadUnsetPropertyValues(this->Makefile, honorStandard,
                                      warnCMP0067, warnCMP0067Variables);
    hipState.LoadUnsetPropertyValues(this->Makefile, honorStandard,
                                     warnCMP0067, warnCMP0067Variables);
    objcState.LoadUnsetPropertyValues(this->Makefile, honorStandard,
                                      warnCMP0067, warnCMP0067Variables);
    objcxxState.LoadUnsetPropertyValues(this->Makefile, honorStandard,
                                        warnCMP0067, warnCMP0067Variables);

    if (!warnCMP0067Variables.empty()) {
      std::ostringstream w;
      /* clang-format off */
      w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0067) << "\n"
        "For compatibility with older versions of CMake, try_compile "
        "is not honoring language standard variables in the test project:\n"
        ;
      /* clang-format on */
      for (std::string const& vi : warnCMP0067Variables) {
        w << "  " << vi << "\n";
      }
      this->Makefile->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
    }

    cState.WriteProperties(fout, targetName);
    cxxState.WriteProperties(fout, targetName);
    cudaState.WriteProperties(fout, targetName);
    hipState.WriteProperties(fout, targetName);
    objcState.WriteProperties(fout, targetName);
    objcxxState.WriteProperties(fout, targetName);

    if (!linkOptions.empty()) {
      std::vector<std::string> options;
      options.reserve(linkOptions.size());
      for (const auto& option : linkOptions) {
        options.emplace_back(cmOutputConverter::EscapeForCMake(option));
      }

      if (targetType == cmStateEnums::STATIC_LIBRARY) {
        fprintf(fout,
                "set_property(TARGET %s PROPERTY STATIC_LIBRARY_OPTIONS %s)\n",
                targetName.c_str(), cmJoin(options, " ").c_str());
      } else {
        fprintf(fout, "target_link_options(%s PRIVATE %s)\n",
                targetName.c_str(), cmJoin(options, " ").c_str());
      }
    }

    if (useOldLinkLibs) {
      fprintf(fout, "target_link_libraries(%s ${LINK_LIBRARIES})\n",
              targetName.c_str());
    } else {
      fprintf(fout, "target_link_libraries(%s %s)\n", targetName.c_str(),
              libsToLink.c_str());
    }
    fclose(fout);
    projectName = "CMAKE_TRY_COMPILE";
  }

  // Forward a set of variables to the inner project cache.
  if ((this->SrcFileSignature ||
       this->Makefile->GetPolicyStatus(cmPolicies::CMP0137) ==
         cmPolicies::NEW) &&
      !this->Makefile->IsOn("CMAKE_TRY_COMPILE_NO_PLATFORM_VARIABLES")) {
    std::set<std::string> vars;
    vars.insert(&c_properties[lang_property_start],
                &c_properties[lang_property_start + lang_property_size]);
    vars.insert(&cxx_properties[lang_property_start],
                &cxx_properties[lang_property_start + lang_property_size]);
    vars.insert(&cuda_properties[lang_property_start],
                &cuda_properties[lang_property_start + lang_property_size]);
    vars.insert(&fortran_properties[lang_property_start],
                &fortran_properties[lang_property_start + lang_property_size]);
    vars.insert(&hip_properties[lang_property_start],
                &hip_properties[lang_property_start + lang_property_size]);
    vars.insert(&objc_properties[lang_property_start],
                &objc_properties[lang_property_start + lang_property_size]);
    vars.insert(&objcxx_properties[lang_property_start],
                &objcxx_properties[lang_property_start + lang_property_size]);
    vars.insert(&ispc_properties[lang_property_start],
                &ispc_properties[lang_property_start + lang_property_size]);
    vars.insert(&swift_properties[lang_property_start],
                &swift_properties[lang_property_start + lang_property_size]);
    vars.insert(kCMAKE_CUDA_ARCHITECTURES);
    vars.insert(kCMAKE_CUDA_RUNTIME_LIBRARY);
    vars.insert(kCMAKE_ENABLE_EXPORTS);
    vars.insert(kCMAKE_HIP_ARCHITECTURES);
    vars.insert(kCMAKE_HIP_RUNTIME_LIBRARY);
    vars.insert(kCMAKE_ISPC_INSTRUCTION_SETS);
    vars.insert(kCMAKE_ISPC_HEADER_SUFFIX);
    vars.insert(kCMAKE_LINK_SEARCH_END_STATIC);
    vars.insert(kCMAKE_LINK_SEARCH_START_STATIC);
    vars.insert(kCMAKE_OSX_ARCHITECTURES);
    vars.insert(kCMAKE_OSX_DEPLOYMENT_TARGET);
    vars.insert(kCMAKE_OSX_SYSROOT);
    vars.insert(kCMAKE_APPLE_ARCH_SYSROOTS);
    vars.insert(kCMAKE_POSITION_INDEPENDENT_CODE);
    vars.insert(kCMAKE_SYSROOT);
    vars.insert(kCMAKE_SYSROOT_COMPILE);
    vars.insert(kCMAKE_SYSROOT_LINK);
    vars.insert(kCMAKE_WARN_DEPRECATED);
    vars.emplace("CMAKE_MSVC_RUNTIME_LIBRARY"_s);
    vars.emplace("CMAKE_WATCOM_RUNTIME_LIBRARY"_s);

    if (cmValue varListStr = this->Makefile->GetDefinition(
          kCMAKE_TRY_COMPILE_PLATFORM_VARIABLES)) {
      std::vector<std::string> varList = cmExpandedList(*varListStr);
      vars.insert(varList.begin(), varList.end());
    }

    if (this->Makefile->GetPolicyStatus(cmPolicies::CMP0083) ==
        cmPolicies::NEW) {
      // To ensure full support of PIE, propagate cache variables
      // driving the link options
      vars.insert(&c_properties[pie_property_start],
                  &c_properties[pie_property_start + pie_property_size]);
      vars.insert(&cxx_properties[pie_property_start],
                  &cxx_properties[pie_property_start + pie_property_size]);
      vars.insert(&cuda_properties[pie_property_start],
                  &cuda_properties[pie_property_start + pie_property_size]);
      vars.insert(&fortran_properties[pie_property_start],
                  &fortran_properties[pie_property_start + pie_property_size]);
      vars.insert(&hip_properties[pie_property_start],
                  &hip_properties[pie_property_start + pie_property_size]);
      vars.insert(&objc_properties[pie_property_start],
                  &objc_properties[pie_property_start + pie_property_size]);
      vars.insert(&objcxx_properties[pie_property_start],
                  &objcxx_properties[pie_property_start + pie_property_size]);
      vars.insert(&ispc_properties[pie_property_start],
                  &ispc_properties[pie_property_start + pie_property_size]);
      vars.insert(&swift_properties[pie_property_start],
                  &swift_properties[pie_property_start + pie_property_size]);
    }

    /* for the TRY_COMPILEs we want to be able to specify the architecture.
       So the user can set CMAKE_OSX_ARCHITECTURES to i386;ppc and then set
       CMAKE_TRY_COMPILE_OSX_ARCHITECTURES first to i386 and then to ppc to
       have the tests run for each specific architecture. Since
       cmLocalGenerator doesn't allow building for "the other"
       architecture only via CMAKE_OSX_ARCHITECTURES.
       */
    if (cmValue tcArchs = this->Makefile->GetDefinition(
          kCMAKE_TRY_COMPILE_OSX_ARCHITECTURES)) {
      vars.erase(kCMAKE_OSX_ARCHITECTURES);
      std::string flag = "-DCMAKE_OSX_ARCHITECTURES=" + *tcArchs;
      cmakeFlags.emplace_back(std::move(flag));
    }

    for (std::string const& var : vars) {
      if (cmValue val = this->Makefile->GetDefinition(var)) {
        std::string flag = "-D" + var + "=" + *val;
        cmakeFlags.emplace_back(std::move(flag));
      }
    }
  }

  if (this->Makefile->GetState()->UseGhsMultiIDE()) {
    // Forward the GHS variables to the inner project cache.
    for (std::string const& var : ghs_platform_vars) {
      if (cmValue val = this->Makefile->GetDefinition(var)) {
        std::string flag = "-D" + var + "=" + "'" + *val + "'";
        cmakeFlags.emplace_back(std::move(flag));
      }
    }
  }

  bool erroroc = cmSystemTools::GetErrorOccurredFlag();
  cmSystemTools::ResetErrorOccurredFlag();
  std::string output;
  // actually do the try compile now that everything is setup
  int res = this->Makefile->TryCompile(
    sourceDirectory, this->BinaryDirectory, projectName, targetName,
    this->SrcFileSignature, cmake::NO_BUILD_PARALLEL_LEVEL, &cmakeFlags,
    output);
  if (erroroc) {
    cmSystemTools::SetErrorOccurred();
  }

  // set the result var to the return value to indicate success or failure
  this->Makefile->AddCacheDefinition(argv[0], (res == 0 ? "TRUE" : "FALSE"),
                                     "Result of TRY_COMPILE",
                                     cmStateEnums::INTERNAL);

  if (!outputVariable.empty()) {
    this->Makefile->AddDefinition(outputVariable, output);
  }

  if (this->SrcFileSignature) {
    std::string copyFileErrorMessage;
    this->FindOutputFile(targetName, targetType);

    if ((res == 0) && !copyFile.empty()) {
      if (this->OutputFile.empty() ||
          !cmSystemTools::CopyFileAlways(this->OutputFile, copyFile)) {
        std::ostringstream emsg;
        /* clang-format off */
        emsg << "Cannot copy output executable\n"
             << "  '" << this->OutputFile << "'\n"
             << "to destination specified by COPY_FILE:\n"
             << "  '" << copyFile << "'\n";
        /* clang-format on */
        if (!this->FindErrorMessage.empty()) {
          emsg << this->FindErrorMessage;
        }
        if (copyFileError.empty()) {
          this->Makefile->IssueMessage(MessageType::FATAL_ERROR, emsg.str());
          return -1;
        }
        copyFileErrorMessage = emsg.str();
      }
    }

    if (!copyFileError.empty()) {
      this->Makefile->AddDefinition(copyFileError, copyFileErrorMessage);
    }
  }
  return res;
}

void cmCoreTryCompile::CleanupFiles(std::string const& binDir)
{
  if (binDir.empty()) {
    return;
  }

  if (binDir.find("CMakeTmp") == std::string::npos) {
    cmSystemTools::Error(
      "TRY_COMPILE attempt to remove -rf directory that does not contain "
      "CMakeTmp:" +
      binDir);
    return;
  }

  cmsys::Directory dir;
  dir.Load(binDir);
  std::set<std::string> deletedFiles;
  for (unsigned long i = 0; i < dir.GetNumberOfFiles(); ++i) {
    const char* fileName = dir.GetFile(i);
    if (strcmp(fileName, ".") != 0 && strcmp(fileName, "..") != 0 &&
        // Do not delete NFS temporary files.
        !cmHasPrefix(fileName, ".nfs")) {
      if (deletedFiles.insert(fileName).second) {
        std::string const fullPath =
          std::string(binDir).append("/").append(fileName);
        if (cmSystemTools::FileIsSymlink(fullPath)) {
          cmSystemTools::RemoveFile(fullPath);
        } else if (cmSystemTools::FileIsDirectory(fullPath)) {
          this->CleanupFiles(fullPath);
          cmSystemTools::RemoveADirectory(fullPath);
        } else {
#ifdef _WIN32
          // Sometimes anti-virus software hangs on to new files so we
          // cannot delete them immediately.  Try a few times.
          cmSystemTools::WindowsFileRetry retry =
            cmSystemTools::GetWindowsFileRetry();
          cmsys::Status status;
          while (!((status = cmSystemTools::RemoveFile(fullPath))) &&
                 --retry.Count && cmSystemTools::FileExists(fullPath)) {
            cmSystemTools::Delay(retry.Delay);
          }
          if (retry.Count == 0)
#else
          cmsys::Status status = cmSystemTools::RemoveFile(fullPath);
          if (!status)
#endif
          {
            this->Makefile->IssueMessage(
              MessageType::FATAL_ERROR,
              cmStrCat("The file:\n  ", fullPath,
                       "\ncould not be removed:\n  ", status.GetString()));
          }
        }
      }
    }
  }
}

void cmCoreTryCompile::FindOutputFile(const std::string& targetName,
                                      cmStateEnums::TargetType targetType)
{
  this->FindErrorMessage.clear();
  this->OutputFile.clear();
  std::string tmpOutputFile = "/";
  if (targetType == cmStateEnums::EXECUTABLE) {
    tmpOutputFile += targetName;
    tmpOutputFile +=
      this->Makefile->GetSafeDefinition("CMAKE_EXECUTABLE_SUFFIX");
  } else // if (targetType == cmStateEnums::STATIC_LIBRARY)
  {
    tmpOutputFile +=
      this->Makefile->GetSafeDefinition("CMAKE_STATIC_LIBRARY_PREFIX");
    tmpOutputFile += targetName;
    tmpOutputFile +=
      this->Makefile->GetSafeDefinition("CMAKE_STATIC_LIBRARY_SUFFIX");
  }

  // a list of directories where to search for the compilation result
  // at first directly in the binary dir
  std::vector<std::string> searchDirs;
  searchDirs.emplace_back();

  cmValue config =
    this->Makefile->GetDefinition("CMAKE_TRY_COMPILE_CONFIGURATION");
  // if a config was specified try that first
  if (cmNonempty(config)) {
    std::string tmp = cmStrCat('/', *config);
    searchDirs.emplace_back(std::move(tmp));
  }
  searchDirs.emplace_back("/Debug");
#if defined(__APPLE__)
  std::string app = "/" + targetName + ".app";
  if (cmNonempty(config)) {
    std::string tmp = cmStrCat('/', *config, app);
    searchDirs.emplace_back(std::move(tmp));
  }
  std::string tmp = "/Debug" + app;
  searchDirs.emplace_back(std::move(tmp));
  searchDirs.emplace_back(std::move(app));
#endif
  searchDirs.emplace_back("/Development");

  for (std::string const& sdir : searchDirs) {
    std::string command = cmStrCat(this->BinaryDirectory, sdir, tmpOutputFile);
    if (cmSystemTools::FileExists(command)) {
      this->OutputFile = cmSystemTools::CollapseFullPath(command);
      return;
    }
  }

  std::ostringstream emsg;
  emsg << "Unable to find the executable at any of:\n";
  emsg << cmWrap("  " + this->BinaryDirectory, searchDirs, tmpOutputFile, "\n")
       << "\n";
  this->FindErrorMessage = emsg.str();
}
