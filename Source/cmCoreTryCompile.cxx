/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCoreTryCompile.h"

#include <cstdio>
#include <cstring>
#include <set>
#include <sstream>
#include <utility>

#include "cmsys/Directory.hxx"

#include "cm_static_string_view.hxx"

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
#include "cmVersion.h"
#include "cmake.h"

static std::string const kCMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN =
  "CMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN";
static std::string const kCMAKE_C_COMPILER_TARGET = "CMAKE_C_COMPILER_TARGET";
static std::string const kCMAKE_C_LINK_NO_PIE_SUPPORTED =
  "CMAKE_C_LINK_NO_PIE_SUPPORTED";
static std::string const kCMAKE_C_LINK_PIE_SUPPORTED =
  "CMAKE_C_LINK_PIE_SUPPORTED";
static std::string const kCMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN =
  "CMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN";
static std::string const kCMAKE_CXX_COMPILER_TARGET =
  "CMAKE_CXX_COMPILER_TARGET";
static std::string const kCMAKE_CXX_LINK_NO_PIE_SUPPORTED =
  "CMAKE_CXX_LINK_NO_PIE_SUPPORTED";
static std::string const kCMAKE_CXX_LINK_PIE_SUPPORTED =
  "CMAKE_CXX_LINK_PIE_SUPPORTED";
static std::string const kCMAKE_CUDA_RUNTIME_LIBRARY =
  "CMAKE_CUDA_RUNTIME_LIBRARY";
static std::string const kCMAKE_ENABLE_EXPORTS = "CMAKE_ENABLE_EXPORTS";
static std::string const kCMAKE_LINK_SEARCH_END_STATIC =
  "CMAKE_LINK_SEARCH_END_STATIC";
static std::string const kCMAKE_LINK_SEARCH_START_STATIC =
  "CMAKE_LINK_SEARCH_START_STATIC";
static std::string const kCMAKE_MSVC_RUNTIME_LIBRARY_DEFAULT =
  "CMAKE_MSVC_RUNTIME_LIBRARY_DEFAULT";
static std::string const kCMAKE_OSX_ARCHITECTURES = "CMAKE_OSX_ARCHITECTURES";
static std::string const kCMAKE_OSX_DEPLOYMENT_TARGET =
  "CMAKE_OSX_DEPLOYMENT_TARGET";
static std::string const kCMAKE_OSX_SYSROOT = "CMAKE_OSX_SYSROOT";
static std::string const kCMAKE_APPLE_ARCH_SYSROOTS =
  "CMAKE_APPLE_ARCH_SYSROOTS";
static std::string const kCMAKE_POSITION_INDEPENDENT_CODE =
  "CMAKE_POSITION_INDEPENDENT_CODE";
static std::string const kCMAKE_SYSROOT = "CMAKE_SYSROOT";
static std::string const kCMAKE_SYSROOT_COMPILE = "CMAKE_SYSROOT_COMPILE";
static std::string const kCMAKE_SYSROOT_LINK = "CMAKE_SYSROOT_LINK";
static std::string const kCMAKE_Swift_COMPILER_TARGET =
  "CMAKE_Swift_COMPILER_TARGET";
static std::string const kCMAKE_TRY_COMPILE_OSX_ARCHITECTURES =
  "CMAKE_TRY_COMPILE_OSX_ARCHITECTURES";
static std::string const kCMAKE_TRY_COMPILE_PLATFORM_VARIABLES =
  "CMAKE_TRY_COMPILE_PLATFORM_VARIABLES";
static std::string const kCMAKE_WARN_DEPRECATED = "CMAKE_WARN_DEPRECATED";

/* GHS Multi platform variables */
static std::set<std::string> ghs_platform_vars{
  "GHS_TARGET_PLATFORM", "GHS_PRIMARY_TARGET", "GHS_TOOLSET_ROOT",
  "GHS_OS_ROOT",         "GHS_OS_DIR",         "GHS_BSP_NAME",
  "GHS_OS_DIR_OPTION"
};

static void writeProperty(FILE* fout, std::string const& targetName,
                          std::string const& prop, std::string const& value)
{
  fprintf(fout, "set_property(TARGET %s PROPERTY %s %s)\n", targetName.c_str(),
          cmOutputConverter::EscapeForCMake(prop).c_str(),
          cmOutputConverter::EscapeForCMake(value).c_str());
}

std::string cmCoreTryCompile::LookupStdVar(std::string const& var,
                                           bool warnCMP0067)
{
  std::string value = this->Makefile->GetSafeDefinition(var);
  if (warnCMP0067 && !value.empty()) {
    value.clear();
    this->WarnCMP0067.push_back(var);
  }
  return value;
}

int cmCoreTryCompile::TryCompileCode(std::vector<std::string> const& argv,
                                     bool isTryRun)
{
  this->BinaryDirectory = argv[1];
  this->OutputFile.clear();
  // which signature were we called with ?
  this->SrcFileSignature = true;

  cmStateEnums::TargetType targetType = cmStateEnums::EXECUTABLE;
  const char* tt =
    this->Makefile->GetDefinition("CMAKE_TRY_COMPILE_TARGET_TYPE");
  if (!isTryRun && tt && *tt) {
    if (strcmp(tt, cmState::GetTargetTypeName(cmStateEnums::EXECUTABLE)) ==
        0) {
      targetType = cmStateEnums::EXECUTABLE;
    } else if (strcmp(tt,
                      cmState::GetTargetTypeName(
                        cmStateEnums::STATIC_LIBRARY)) == 0) {
      targetType = cmStateEnums::STATIC_LIBRARY;
    } else {
      this->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        std::string("Invalid value '") + tt +
          "' for "
          "CMAKE_TRY_COMPILE_TARGET_TYPE.  Only "
          "'" +
          cmState::GetTargetTypeName(cmStateEnums::EXECUTABLE) +
          "' and "
          "'" +
          cmState::GetTargetTypeName(cmStateEnums::STATIC_LIBRARY) +
          "' "
          "are allowed.");
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
  std::string cStandard;
  std::string objcStandard;
  std::string cxxStandard;
  std::string objcxxStandard;
  std::string cudaStandard;
  std::string cStandardRequired;
  std::string cxxStandardRequired;
  std::string objcStandardRequired;
  std::string objcxxStandardRequired;
  std::string cudaStandardRequired;
  std::string cExtensions;
  std::string cxxExtensions;
  std::string objcExtensions;
  std::string objcxxExtensions;
  std::string cudaExtensions;
  std::vector<std::string> targets;
  std::vector<std::string> linkOptions;
  std::string libsToLink = " ";
  bool useOldLinkLibs = true;
  char targetNameBuf[64];
  bool didOutputVariable = false;
  bool didCopyFile = false;
  bool didCopyFileError = false;
  bool didCStandard = false;
  bool didCxxStandard = false;
  bool didObjCStandard = false;
  bool didObjCxxStandard = false;
  bool didCudaStandard = false;
  bool didCStandardRequired = false;
  bool didCxxStandardRequired = false;
  bool didObjCStandardRequired = false;
  bool didObjCxxStandardRequired = false;
  bool didCudaStandardRequired = false;
  bool didCExtensions = false;
  bool didCxxExtensions = false;
  bool didObjCExtensions = false;
  bool didObjCxxExtensions = false;
  bool didCudaExtensions = false;
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
    DoingCStandard,
    DoingCxxStandard,
    DoingObjCStandard,
    DoingObjCxxStandard,
    DoingCudaStandard,
    DoingCStandardRequired,
    DoingCxxStandardRequired,
    DoingObjCStandardRequired,
    DoingObjCxxStandardRequired,
    DoingCudaStandardRequired,
    DoingCExtensions,
    DoingCxxExtensions,
    DoingObjCExtensions,
    DoingObjCxxExtensions,
    DoingCudaExtensions,
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
    } else if (argv[i] == "C_STANDARD") {
      doing = DoingCStandard;
      didCStandard = true;
    } else if (argv[i] == "CXX_STANDARD") {
      doing = DoingCxxStandard;
      didCxxStandard = true;
    } else if (argv[i] == "OBJC_STANDARD") {
      doing = DoingObjCStandard;
      didObjCStandard = true;
    } else if (argv[i] == "OBJCXX_STANDARD") {
      doing = DoingObjCxxStandard;
      didObjCxxStandard = true;
    } else if (argv[i] == "CUDA_STANDARD") {
      doing = DoingCudaStandard;
      didCudaStandard = true;
    } else if (argv[i] == "C_STANDARD_REQUIRED") {
      doing = DoingCStandardRequired;
      didCStandardRequired = true;
    } else if (argv[i] == "CXX_STANDARD_REQUIRED") {
      doing = DoingCxxStandardRequired;
      didCxxStandardRequired = true;
    } else if (argv[i] == "OBJC_STANDARD_REQUIRED") {
      doing = DoingObjCStandardRequired;
      didObjCStandardRequired = true;
    } else if (argv[i] == "OBJCXX_STANDARD_REQUIRED") {
      doing = DoingObjCxxStandardRequired;
      didObjCxxStandardRequired = true;
    } else if (argv[i] == "CUDA_STANDARD_REQUIRED") {
      doing = DoingCudaStandardRequired;
      didCudaStandardRequired = true;
    } else if (argv[i] == "C_EXTENSIONS") {
      doing = DoingCExtensions;
      didCExtensions = true;
    } else if (argv[i] == "CXX_EXTENSIONS") {
      doing = DoingCxxExtensions;
      didCxxExtensions = true;
    } else if (argv[i] == "OBJC_EXTENSIONS") {
      doing = DoingObjCExtensions;
      didObjCExtensions = true;
    } else if (argv[i] == "OBJCXX_EXTENSIONS") {
      doing = DoingObjCxxExtensions;
      didObjCxxExtensions = true;
    } else if (argv[i] == "CUDA_EXTENSIONS") {
      doing = DoingCudaExtensions;
      didCudaExtensions = true;
    } else if (argv[i] == "__CMAKE_INTERNAL") {
      doing = DoingCMakeInternal;
    } else if (doing == DoingCMakeFlags) {
      cmakeFlags.push_back(argv[i]);
    } else if (doing == DoingCompileDefinitions) {
      cmExpandList(argv[i], compileDefs);
    } else if (doing == DoingLinkOptions) {
      linkOptions.push_back(argv[i]);
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
              "Only libraries may be used as try_compile or try_run IMPORTED "
              "LINK_LIBRARIES.  Got " +
                std::string(tgt->GetName()) +
                " of "
                "type " +
                cmState::GetTargetTypeName(tgt->GetType()) + ".");
            return -1;
        }
        if (tgt->IsImported()) {
          targets.push_back(argv[i]);
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
    } else if (doing == DoingCStandard) {
      cStandard = argv[i];
      doing = DoingNone;
    } else if (doing == DoingCxxStandard) {
      cxxStandard = argv[i];
      doing = DoingNone;
    } else if (doing == DoingObjCStandard) {
      objcStandard = argv[i];
      doing = DoingNone;
    } else if (doing == DoingObjCxxStandard) {
      objcxxStandard = argv[i];
      doing = DoingNone;
    } else if (doing == DoingCudaStandard) {
      cudaStandard = argv[i];
      doing = DoingNone;
    } else if (doing == DoingCStandardRequired) {
      cStandardRequired = argv[i];
      doing = DoingNone;
    } else if (doing == DoingCxxStandardRequired) {
      cxxStandardRequired = argv[i];
      doing = DoingNone;
    } else if (doing == DoingObjCStandardRequired) {
      objcStandardRequired = argv[i];
      doing = DoingNone;
    } else if (doing == DoingObjCxxStandardRequired) {
      objcxxStandardRequired = argv[i];
      doing = DoingNone;
    } else if (doing == DoingCudaStandardRequired) {
      cudaStandardRequired = argv[i];
      doing = DoingNone;
    } else if (doing == DoingCExtensions) {
      cExtensions = argv[i];
      doing = DoingNone;
    } else if (doing == DoingCxxExtensions) {
      cxxExtensions = argv[i];
      doing = DoingNone;
    } else if (doing == DoingObjCExtensions) {
      objcExtensions = argv[i];
      doing = DoingNone;
    } else if (doing == DoingObjCxxExtensions) {
      objcxxExtensions = argv[i];
      doing = DoingNone;
    } else if (doing == DoingCudaExtensions) {
      cudaExtensions = argv[i];
      doing = DoingNone;
    } else if (doing == DoingSources) {
      sources.push_back(argv[i]);
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

  if (didCStandard && !this->SrcFileSignature) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "C_STANDARD allowed only in source file signature.");
    return -1;
  }
  if (didCxxStandard && !this->SrcFileSignature) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "CXX_STANDARD allowed only in source file signature.");
    return -1;
  }
  if (didCudaStandard && !this->SrcFileSignature) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "CUDA_STANDARD allowed only in source file signature.");
    return -1;
  }
  if (didCStandardRequired && !this->SrcFileSignature) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "C_STANDARD_REQUIRED allowed only in source file signature.");
    return -1;
  }
  if (didCxxStandardRequired && !this->SrcFileSignature) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "CXX_STANDARD_REQUIRED allowed only in source file signature.");
    return -1;
  }
  if (didCudaStandardRequired && !this->SrcFileSignature) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "CUDA_STANDARD_REQUIRED allowed only in source file signature.");
    return -1;
  }
  if (didCExtensions && !this->SrcFileSignature) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "C_EXTENSIONS allowed only in source file signature.");
    return -1;
  }
  if (didCxxExtensions && !this->SrcFileSignature) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "CXX_EXTENSIONS allowed only in source file signature.");
    return -1;
  }
  if (didCudaExtensions && !this->SrcFileSignature) {
    this->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      "CUDA_EXTENSIONS allowed only in source file signature.");
    return -1;
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
      sources.push_back(argv[2]);
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

    const char* def = this->Makefile->GetDefinition("CMAKE_MODULE_PATH");
    fprintf(fout, "cmake_minimum_required(VERSION %u.%u.%u.%u)\n",
            cmVersion::GetMajorVersion(), cmVersion::GetMinorVersion(),
            cmVersion::GetPatchVersion(), cmVersion::GetTweakVersion());
    if (def) {
      fprintf(fout, "set(CMAKE_MODULE_PATH \"%s\")\n", def);
    }

    /* Set MSVC runtime library policy to match our selection.  */
    if (const char* msvcRuntimeLibraryDefault =
          this->Makefile->GetDefinition(kCMAKE_MSVC_RUNTIME_LIBRARY_DEFAULT)) {
      fprintf(fout, "cmake_policy(SET CMP0091 %s)\n",
              *msvcRuntimeLibraryDefault ? "NEW" : "OLD");
    }

    std::string projectLangs;
    for (std::string const& li : testLangs) {
      projectLangs += " " + li;
      std::string rulesOverrideBase = "CMAKE_USER_MAKE_RULES_OVERRIDE";
      std::string rulesOverrideLang = cmStrCat(rulesOverrideBase, "_", li);
      if (const char* rulesOverridePath =
            this->Makefile->GetDefinition(rulesOverrideLang)) {
        fprintf(fout, "set(%s \"%s\")\n", rulesOverrideLang.c_str(),
                rulesOverridePath);
      } else if (const char* rulesOverridePath2 =
                   this->Makefile->GetDefinition(rulesOverrideBase)) {
        fprintf(fout, "set(%s \"%s\")\n", rulesOverrideBase.c_str(),
                rulesOverridePath2);
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
      const char* flags = this->Makefile->GetDefinition(langFlags);
      fprintf(fout, "set(CMAKE_%s_FLAGS %s)\n", li.c_str(),
              cmOutputConverter::EscapeForCMake(flags ? flags : "").c_str());
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
          const char* flagsCfg = this->Makefile->GetDefinition(langFlagsCfg);
          fprintf(fout, "set(%s %s)\n", langFlagsCfg.c_str(),
                  cmOutputConverter::EscapeForCMake(flagsCfg ? flagsCfg : "")
                    .c_str());
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
          const char* exeLinkFlags =
            this->Makefile->GetDefinition("CMAKE_EXE_LINKER_FLAGS");
          fprintf(
            fout, "set(CMAKE_EXE_LINKER_FLAGS %s)\n",
            cmOutputConverter::EscapeForCMake(exeLinkFlags ? exeLinkFlags : "")
              .c_str());
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
    sprintf(targetNameBuf, "cmTC_%05x", cmSystemTools::RandomSeed() & 0xFFFFF);
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

    // Forward a set of variables to the inner project cache.
    {
      std::set<std::string> vars;
      vars.insert(kCMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN);
      vars.insert(kCMAKE_C_COMPILER_TARGET);
      vars.insert(kCMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN);
      vars.insert(kCMAKE_CXX_COMPILER_TARGET);
      vars.insert(kCMAKE_CUDA_RUNTIME_LIBRARY);
      vars.insert(kCMAKE_ENABLE_EXPORTS);
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
      vars.insert(kCMAKE_Swift_COMPILER_TARGET);
      vars.insert(kCMAKE_WARN_DEPRECATED);
      vars.emplace("CMAKE_MSVC_RUNTIME_LIBRARY"_s);

      if (const char* varListStr = this->Makefile->GetDefinition(
            kCMAKE_TRY_COMPILE_PLATFORM_VARIABLES)) {
        std::vector<std::string> varList = cmExpandedList(varListStr);
        vars.insert(varList.begin(), varList.end());
      }

      if (this->Makefile->GetPolicyStatus(cmPolicies::CMP0083) ==
          cmPolicies::NEW) {
        // To ensure full support of PIE, propagate cache variables
        // driving the link options
        vars.insert(kCMAKE_C_LINK_PIE_SUPPORTED);
        vars.insert(kCMAKE_C_LINK_NO_PIE_SUPPORTED);
        vars.insert(kCMAKE_CXX_LINK_PIE_SUPPORTED);
        vars.insert(kCMAKE_CXX_LINK_NO_PIE_SUPPORTED);
      }

      /* for the TRY_COMPILEs we want to be able to specify the architecture.
         So the user can set CMAKE_OSX_ARCHITECTURES to i386;ppc and then set
         CMAKE_TRY_COMPILE_OSX_ARCHITECTURES first to i386 and then to ppc to
         have the tests run for each specific architecture. Since
         cmLocalGenerator doesn't allow building for "the other"
         architecture only via CMAKE_OSX_ARCHITECTURES.
         */
      if (const char* tcArchs = this->Makefile->GetDefinition(
            kCMAKE_TRY_COMPILE_OSX_ARCHITECTURES)) {
        vars.erase(kCMAKE_OSX_ARCHITECTURES);
        std::string flag = "-DCMAKE_OSX_ARCHITECTURES=" + std::string(tcArchs);
        cmakeFlags.push_back(std::move(flag));
      }

      for (std::string const& var : vars) {
        if (const char* val = this->Makefile->GetDefinition(var)) {
          std::string flag = "-D" + var + "=" + val;
          cmakeFlags.push_back(std::move(flag));
        }
      }
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

    bool const testC = testLangs.find("C") != testLangs.end();
    bool const testObjC = testLangs.find("OBJC") != testLangs.end();
    bool const testCxx = testLangs.find("CXX") != testLangs.end();
    bool const testObjCxx = testLangs.find("OBJCXX") != testLangs.end();
    bool const testCuda = testLangs.find("CUDA") != testLangs.end();

    bool warnCMP0067 = false;
    bool honorStandard = true;

    if (!didCStandard && !didCxxStandard && !didObjCStandard &&
        !didObjCxxStandard && !didCudaStandard && !didCStandardRequired &&
        !didCxxStandardRequired && !didObjCStandardRequired &&
        !didObjCxxStandardRequired && !didCudaStandardRequired &&
        !didCExtensions && !didCxxExtensions && !didObjCExtensions &&
        !didObjCxxExtensions && !didCudaExtensions) {
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
        case cmPolicies::NEW:
          // NEW behavior is to honor the language standard variables.
          // We already initialized honorStandard to true.
          break;
      }
    }

    if (honorStandard || warnCMP0067) {

      auto testLanguage =
        [&](bool testLang, bool didLangStandard, bool didLangStandardRequired,
            bool didLangExtensions, std::string& langStandard,
            std::string& langStandardRequired, std::string& langExtensions,
            const std::string& lang) {
          if (testLang) {
            if (!didLangStandard) {
              langStandard = this->LookupStdVar(
                cmStrCat("CMAKE_", lang, "_STANDARD"), warnCMP0067);
            }
            if (!didLangStandardRequired) {
              langStandardRequired = this->LookupStdVar(
                cmStrCat("CMAKE_", lang, "_STANDARD_REQUIRED"), warnCMP0067);
            }
            if (!didLangExtensions) {
              langExtensions = this->LookupStdVar(
                cmStrCat("CMAKE_", lang, "_EXTENSIONS"), warnCMP0067);
            }
          }
        };

      testLanguage(testC, didCStandard, didCStandardRequired, didCExtensions,
                   cStandard, cStandardRequired, cExtensions, "C");
      testLanguage(testObjC, didObjCStandard, didObjCStandardRequired,
                   didObjCExtensions, objcStandard, objcStandardRequired,
                   objcExtensions, "OBJC");
      testLanguage(testCxx, didCxxStandard, didCxxStandardRequired,
                   didCxxExtensions, cxxStandard, cxxStandardRequired,
                   cxxExtensions, "CXX");
      testLanguage(testObjCxx, didObjCxxStandard, didObjCxxStandardRequired,
                   didObjCxxExtensions, objcxxStandard, objcxxStandardRequired,
                   objcxxExtensions, "OBJCXX");
      testLanguage(testCuda, didCudaStandard, didCudaStandardRequired,
                   didCudaExtensions, cudaStandard, cudaStandardRequired,
                   cudaExtensions, "CUDA");
    }

    if (!this->WarnCMP0067.empty()) {
      std::ostringstream w;
      /* clang-format off */
      w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0067) << "\n"
        "For compatibility with older versions of CMake, try_compile "
        "is not honoring language standard variables in the test project:\n"
        ;
      /* clang-format on */
      for (std::string const& vi : this->WarnCMP0067) {
        w << "  " << vi << "\n";
      }
      this->Makefile->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
    }

    auto writeLanguageProperties = [&](bool testLang,
                                       const std::string& langStandard,
                                       const std::string& langStandardRequired,
                                       const std::string& langExtensions,
                                       const std::string& lang) {
      if (testLang) {
        if (!langStandard.empty()) {
          writeProperty(fout, targetName, cmStrCat(lang, "_STANDARD"),
                        langStandard);
        }
        if (!langStandardRequired.empty()) {
          writeProperty(fout, targetName, cmStrCat(lang, "_STANDARD_REQUIRED"),
                        langStandardRequired);
        }
        if (!langExtensions.empty()) {
          writeProperty(fout, targetName, cmStrCat(lang, "_EXTENSIONS"),
                        langExtensions);
        }
      }
    };

    writeLanguageProperties(testC, cStandard, cStandardRequired, cExtensions,
                            "C");
    writeLanguageProperties(testObjC, objcStandard, objcStandardRequired,
                            objcExtensions, "OBJC");
    writeLanguageProperties(testCxx, cxxStandard, cxxStandardRequired,
                            cxxExtensions, "CXX");
    writeLanguageProperties(testObjCxx, objcxxStandard, objcxxStandardRequired,
                            objcxxExtensions, "OBJCXX");
    writeLanguageProperties(testCuda, cudaStandard, cudaStandardRequired,
                            cudaExtensions, "CUDA");

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

  if (this->Makefile->GetState()->UseGhsMultiIDE()) {
    // Forward the GHS variables to the inner project cache.
    for (std::string const& var : ghs_platform_vars) {
      if (const char* val = this->Makefile->GetDefinition(var)) {
        std::string flag = "-D" + var + "=" + "'" + val + "'";
        cmakeFlags.push_back(std::move(flag));
      }
    }
  }

  bool erroroc = cmSystemTools::GetErrorOccuredFlag();
  cmSystemTools::ResetErrorOccuredFlag();
  std::string output;
  // actually do the try compile now that everything is setup
  int res = this->Makefile->TryCompile(
    sourceDirectory, this->BinaryDirectory, projectName, targetName,
    this->SrcFileSignature, cmake::NO_BUILD_PARALLEL_LEVEL, &cmakeFlags,
    output);
  if (erroroc) {
    cmSystemTools::SetErrorOccured();
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
    if (strcmp(fileName, ".") != 0 && strcmp(fileName, "..") != 0) {
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
          while (!cmSystemTools::RemoveFile(fullPath) && --retry.Count &&
                 cmSystemTools::FileExists(fullPath)) {
            cmSystemTools::Delay(retry.Delay);
          }
          if (retry.Count == 0)
#else
          if (!cmSystemTools::RemoveFile(fullPath))
#endif
          {
            std::string m = "Remove failed on file: " + fullPath;
            cmSystemTools::ReportLastSystemError(m.c_str());
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

  const char* config =
    this->Makefile->GetDefinition("CMAKE_TRY_COMPILE_CONFIGURATION");
  // if a config was specified try that first
  if (config && config[0]) {
    std::string tmp = cmStrCat('/', config);
    searchDirs.push_back(std::move(tmp));
  }
  searchDirs.emplace_back("/Debug");
#if defined(__APPLE__)
  std::string app = "/" + targetName + ".app";
  if (config && config[0]) {
    std::string tmp = cmStrCat('/', config, app);
    searchDirs.push_back(std::move(tmp));
  }
  std::string tmp = "/Debug" + app;
  searchDirs.emplace_back(std::move(tmp));
  searchDirs.push_back(std::move(app));
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
