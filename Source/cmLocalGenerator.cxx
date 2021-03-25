/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmLocalGenerator.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <unordered_set>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmAlgorithms.h"
#include "cmComputeLinkInformation.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandGenerator.h"
#include "cmCustomCommandLines.h"
#include "cmCustomCommandTypes.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionEvaluationFile.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmInstallGenerator.h"
#include "cmInstallScriptGenerator.h"
#include "cmInstallTargetGenerator.h"
#include "cmLinkLineComputer.h"
#include "cmMakefile.h"
#include "cmProperty.h"
#include "cmRulePlaceholderExpander.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmSourceFileLocationKind.h"
#include "cmStandardLevelResolver.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTestGenerator.h"
#include "cmVersion.h"
#include "cmake.h"

#if !defined(CMAKE_BOOTSTRAP)
#  define CM_LG_ENCODE_OBJECT_NAMES
#  include "cmCryptoHash.h"
#endif

#if defined(__HAIKU__)
#  include <FindDirectory.h>
#  include <StorageDefs.h>
#endif

// List of variables that are replaced when
// rules are expanced.  These variables are
// replaced in the form <var> with GetSafeDefinition(var).
// ${LANG} is replaced in the variable first with all enabled
// languages.
static auto ruleReplaceVars = { "CMAKE_${LANG}_COMPILER",
                                "CMAKE_SHARED_LIBRARY_CREATE_${LANG}_FLAGS",
                                "CMAKE_SHARED_MODULE_CREATE_${LANG}_FLAGS",
                                "CMAKE_SHARED_MODULE_${LANG}_FLAGS",
                                "CMAKE_SHARED_LIBRARY_${LANG}_FLAGS",
                                "CMAKE_${LANG}_LINK_FLAGS",
                                "CMAKE_SHARED_LIBRARY_SONAME_${LANG}_FLAG",
                                "CMAKE_${LANG}_ARCHIVE",
                                "CMAKE_AR",
                                "CMAKE_CURRENT_SOURCE_DIR",
                                "CMAKE_CURRENT_BINARY_DIR",
                                "CMAKE_RANLIB",
                                "CMAKE_LINKER",
                                "CMAKE_MT",
                                "CMAKE_CUDA_HOST_COMPILER",
                                "CMAKE_CUDA_HOST_LINK_LAUNCHER",
                                "CMAKE_CL_SHOWINCLUDES_PREFIX" };

cmLocalGenerator::cmLocalGenerator(cmGlobalGenerator* gg, cmMakefile* makefile)
  : cmOutputConverter(makefile->GetStateSnapshot())
  , StateSnapshot(makefile->GetStateSnapshot())
  , DirectoryBacktrace(makefile->GetBacktrace())
{
  this->GlobalGenerator = gg;

  this->Makefile = makefile;

  this->AliasTargets = makefile->GetAliasTargets();

  this->EmitUniversalBinaryFlags = true;
  this->BackwardsCompatibility = 0;
  this->BackwardsCompatibilityFinal = false;

  this->ComputeObjectMaxPath();

  // Canonicalize entries of the CPATH environment variable the same
  // way detection of CMAKE_<LANG>_IMPLICIT_INCLUDE_DIRECTORIES does.
  {
    std::vector<std::string> cpath;
    cmSystemTools::GetPath(cpath, "CPATH");
    for (std::string& cp : cpath) {
      if (cmSystemTools::FileIsFullPath(cp)) {
        cp = cmSystemTools::CollapseFullPath(cp);
        this->EnvCPATH.emplace(std::move(cp));
      }
    }
  }

  std::vector<std::string> enabledLanguages =
    this->GetState()->GetEnabledLanguages();

  if (cmProp sysrootCompile =
        this->Makefile->GetDefinition("CMAKE_SYSROOT_COMPILE")) {
    this->CompilerSysroot = *sysrootCompile;
  } else {
    this->CompilerSysroot = this->Makefile->GetSafeDefinition("CMAKE_SYSROOT");
  }

  if (cmProp sysrootLink =
        this->Makefile->GetDefinition("CMAKE_SYSROOT_LINK")) {
    this->LinkerSysroot = *sysrootLink;
  } else {
    this->LinkerSysroot = this->Makefile->GetSafeDefinition("CMAKE_SYSROOT");
  }

  if (cmProp appleArchSysroots =
        this->Makefile->GetDefinition("CMAKE_APPLE_ARCH_SYSROOTS")) {
    std::string const& appleArchs =
      this->Makefile->GetSafeDefinition("CMAKE_OSX_ARCHITECTURES");
    std::vector<std::string> archs;
    std::vector<std::string> sysroots;
    cmExpandList(appleArchs, archs);
    cmExpandList(*appleArchSysroots, sysroots, true);
    if (archs.size() == sysroots.size()) {
      for (size_t i = 0; i < archs.size(); ++i) {
        this->AppleArchSysroots[archs[i]] = sysroots[i];
      }
    } else {
      std::string const e =
        cmStrCat("CMAKE_APPLE_ARCH_SYSROOTS:\n  ", *appleArchSysroots,
                 "\n"
                 "is not the same length as CMAKE_OSX_ARCHITECTURES:\n  ",
                 appleArchs);
      this->IssueMessage(MessageType::FATAL_ERROR, e);
    }
  }

  for (std::string const& lang : enabledLanguages) {
    if (lang == "NONE") {
      continue;
    }
    this->Compilers["CMAKE_" + lang + "_COMPILER"] = lang;

    this->VariableMappings["CMAKE_" + lang + "_COMPILER"] =
      this->Makefile->GetSafeDefinition("CMAKE_" + lang + "_COMPILER");

    std::string const& compilerArg1 = "CMAKE_" + lang + "_COMPILER_ARG1";
    std::string const& compilerTarget = "CMAKE_" + lang + "_COMPILER_TARGET";
    std::string const& compilerOptionTarget =
      "CMAKE_" + lang + "_COMPILE_OPTIONS_TARGET";
    std::string const& compilerExternalToolchain =
      "CMAKE_" + lang + "_COMPILER_EXTERNAL_TOOLCHAIN";
    std::string const& compilerOptionExternalToolchain =
      "CMAKE_" + lang + "_COMPILE_OPTIONS_EXTERNAL_TOOLCHAIN";
    std::string const& compilerOptionSysroot =
      "CMAKE_" + lang + "_COMPILE_OPTIONS_SYSROOT";

    this->VariableMappings[compilerArg1] =
      this->Makefile->GetSafeDefinition(compilerArg1);
    this->VariableMappings[compilerTarget] =
      this->Makefile->GetSafeDefinition(compilerTarget);
    this->VariableMappings[compilerOptionTarget] =
      this->Makefile->GetSafeDefinition(compilerOptionTarget);
    this->VariableMappings[compilerExternalToolchain] =
      this->Makefile->GetSafeDefinition(compilerExternalToolchain);
    this->VariableMappings[compilerOptionExternalToolchain] =
      this->Makefile->GetSafeDefinition(compilerOptionExternalToolchain);
    this->VariableMappings[compilerOptionSysroot] =
      this->Makefile->GetSafeDefinition(compilerOptionSysroot);

    for (std::string replaceVar : ruleReplaceVars) {
      if (replaceVar.find("${LANG}") != std::string::npos) {
        cmSystemTools::ReplaceString(replaceVar, "${LANG}", lang);
      }

      this->VariableMappings[replaceVar] =
        this->Makefile->GetSafeDefinition(replaceVar);
    }
  }
}

cmRulePlaceholderExpander* cmLocalGenerator::CreateRulePlaceholderExpander()
  const
{
  return new cmRulePlaceholderExpander(this->Compilers, this->VariableMappings,
                                       this->CompilerSysroot,
                                       this->LinkerSysroot);
}

cmLocalGenerator::~cmLocalGenerator() = default;

void cmLocalGenerator::IssueMessage(MessageType t,
                                    std::string const& text) const
{
  this->GetCMakeInstance()->IssueMessage(t, text, this->DirectoryBacktrace);
}

void cmLocalGenerator::ComputeObjectMaxPath()
{
// Choose a maximum object file name length.
#if defined(_WIN32) || defined(__CYGWIN__)
  this->ObjectPathMax = 250;
#else
  this->ObjectPathMax = 1000;
#endif
  cmProp plen = this->Makefile->GetDefinition("CMAKE_OBJECT_PATH_MAX");
  if (cmNonempty(plen)) {
    unsigned int pmax;
    if (sscanf(plen->c_str(), "%u", &pmax) == 1) {
      if (pmax >= 128) {
        this->ObjectPathMax = pmax;
      } else {
        std::ostringstream w;
        w << "CMAKE_OBJECT_PATH_MAX is set to " << pmax
          << ", which is less than the minimum of 128.  "
          << "The value will be ignored.";
        this->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
      }
    } else {
      std::ostringstream w;
      w << "CMAKE_OBJECT_PATH_MAX is set to \"" << *plen
        << "\", which fails to parse as a positive integer.  "
        << "The value will be ignored.";
      this->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
    }
  }
  this->ObjectMaxPathViolations.clear();
}

static void MoveSystemIncludesToEnd(std::vector<std::string>& includeDirs,
                                    const std::string& config,
                                    const std::string& lang,
                                    const cmGeneratorTarget* target)
{
  if (!target) {
    return;
  }

  std::stable_sort(
    includeDirs.begin(), includeDirs.end(),
    [&target, &config, &lang](std::string const& a, std::string const& b) {
      return !target->IsSystemIncludeDirectory(a, config, lang) &&
        target->IsSystemIncludeDirectory(b, config, lang);
    });
}

static void MoveSystemIncludesToEnd(std::vector<BT<std::string>>& includeDirs,
                                    const std::string& config,
                                    const std::string& lang,
                                    const cmGeneratorTarget* target)
{
  if (!target) {
    return;
  }

  std::stable_sort(includeDirs.begin(), includeDirs.end(),
                   [target, &config, &lang](BT<std::string> const& a,
                                            BT<std::string> const& b) {
                     return !target->IsSystemIncludeDirectory(a.Value, config,
                                                              lang) &&
                       target->IsSystemIncludeDirectory(b.Value, config, lang);
                   });
}

void cmLocalGenerator::TraceDependencies() const
{
  // Generate the rule files for each target.
  const auto& targets = this->GetGeneratorTargets();
  for (const auto& target : targets) {
    if (!target->IsInBuildSystem()) {
      continue;
    }
    target->TraceDependencies();
  }
}

void cmLocalGenerator::GenerateTestFiles()
{
  if (!this->Makefile->IsOn("CMAKE_TESTING_ENABLED")) {
    return;
  }

  // Compute the set of configurations.
  std::vector<std::string> configurationTypes =
    this->Makefile->GetGeneratorConfigs(cmMakefile::OnlyMultiConfig);
  std::string config = this->Makefile->GetDefaultConfiguration();

  std::string file =
    cmStrCat(this->StateSnapshot.GetDirectory().GetCurrentBinary(),
             "/CTestTestfile.cmake");

  cmGeneratedFileStream fout(file);
  fout.SetCopyIfDifferent(true);

  fout << "# CMake generated Testfile for \n"
          "# Source directory: "
       << this->StateSnapshot.GetDirectory().GetCurrentSource()
       << "\n"
          "# Build directory: "
       << this->StateSnapshot.GetDirectory().GetCurrentBinary()
       << "\n"
          "# \n"
          "# This file includes the relevant testing commands "
          "required for \n"
          "# testing this directory and lists subdirectories to "
          "be tested as well.\n";

  std::string resourceSpecFile =
    this->Makefile->GetSafeDefinition("CTEST_RESOURCE_SPEC_FILE");
  if (!resourceSpecFile.empty()) {
    fout << "set(CTEST_RESOURCE_SPEC_FILE \"" << resourceSpecFile << "\")\n";
  }

  cmProp testIncludeFile = this->Makefile->GetProperty("TEST_INCLUDE_FILE");
  if (testIncludeFile) {
    fout << "include(\"" << *testIncludeFile << "\")\n";
  }

  cmProp testIncludeFiles = this->Makefile->GetProperty("TEST_INCLUDE_FILES");
  if (testIncludeFiles) {
    std::vector<std::string> includesList = cmExpandedList(*testIncludeFiles);
    for (std::string const& i : includesList) {
      fout << "include(\"" << i << "\")\n";
    }
  }

  // Ask each test generator to write its code.
  for (const auto& tester : this->Makefile->GetTestGenerators()) {
    tester->Compute(this);
    tester->Generate(fout, config, configurationTypes);
  }
  using vec_t = std::vector<cmStateSnapshot>;
  vec_t const& children = this->Makefile->GetStateSnapshot().GetChildren();
  std::string parentBinDir = this->GetCurrentBinaryDirectory();
  for (cmStateSnapshot const& i : children) {
    // TODO: Use add_subdirectory instead?
    std::string outP = i.GetDirectory().GetCurrentBinary();
    outP = this->MaybeConvertToRelativePath(parentBinDir, outP);
    outP = cmOutputConverter::EscapeForCMake(outP);
    fout << "subdirs(" << outP << ")\n";
  }

  // Add directory labels property
  cmProp directoryLabels =
    this->Makefile->GetDefinition("CMAKE_DIRECTORY_LABELS");
  cmProp labels = this->Makefile->GetProperty("LABELS");

  if (labels || directoryLabels) {
    fout << "set_directory_properties(PROPERTIES LABELS ";
    if (labels) {
      fout << cmOutputConverter::EscapeForCMake(*labels);
    }
    if (labels && directoryLabels) {
      fout << ";";
    }
    if (directoryLabels) {
      fout << cmOutputConverter::EscapeForCMake(*directoryLabels);
    }
    fout << ")\n";
  }
}

void cmLocalGenerator::CreateEvaluationFileOutputs()
{
  std::vector<std::string> const& configs =
    this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
  for (std::string const& c : configs) {
    this->CreateEvaluationFileOutputs(c);
  }
}

void cmLocalGenerator::CreateEvaluationFileOutputs(std::string const& config)
{
  for (const auto& geef : this->Makefile->GetEvaluationFiles()) {
    geef->CreateOutputFile(this, config);
  }
}

void cmLocalGenerator::ProcessEvaluationFiles(
  std::vector<std::string>& generatedFiles)
{
  for (const auto& geef : this->Makefile->GetEvaluationFiles()) {
    geef->Generate(this);
    if (cmSystemTools::GetFatalErrorOccured()) {
      return;
    }
    std::vector<std::string> files = geef->GetFiles();
    std::sort(files.begin(), files.end());

    std::vector<std::string> intersection;
    std::set_intersection(files.begin(), files.end(), generatedFiles.begin(),
                          generatedFiles.end(),
                          std::back_inserter(intersection));
    if (!intersection.empty()) {
      cmSystemTools::Error("Files to be generated by multiple different "
                           "commands: " +
                           cmWrap('"', intersection, '"', " "));
      return;
    }

    cm::append(generatedFiles, files);
    std::inplace_merge(generatedFiles.begin(),
                       generatedFiles.end() - files.size(),
                       generatedFiles.end());
  }
}

void cmLocalGenerator::GenerateInstallRules()
{
  // Compute the install prefix.
  const char* prefix =
    cmToCStr(this->Makefile->GetDefinition("CMAKE_INSTALL_PREFIX"));

#if defined(_WIN32) && !defined(__CYGWIN__)
  std::string prefix_win32;
  if (!prefix) {
    if (!cmSystemTools::GetEnv("SystemDrive", prefix_win32)) {
      prefix_win32 = "C:";
    }
    cmProp project_name = this->Makefile->GetDefinition("PROJECT_NAME");
    if (cmNonempty(project_name)) {
      prefix_win32 += "/Program Files/";
      prefix_win32 += *project_name;
    } else {
      prefix_win32 += "/InstalledCMakeProject";
    }
    prefix = prefix_win32.c_str();
  }
#elif defined(__HAIKU__)
  char dir[B_PATH_NAME_LENGTH];
  if (!prefix) {
    if (find_directory(B_SYSTEM_DIRECTORY, -1, false, dir, sizeof(dir)) ==
        B_OK) {
      prefix = dir;
    } else {
      prefix = "/boot/system";
    }
  }
#else
  if (!prefix) {
    prefix = "/usr/local";
  }
#endif
  if (cmProp stagingPrefix =
        this->Makefile->GetDefinition("CMAKE_STAGING_PREFIX")) {
    prefix = stagingPrefix->c_str();
  }

  // Compute the set of configurations.
  std::vector<std::string> configurationTypes =
    this->Makefile->GetGeneratorConfigs(cmMakefile::OnlyMultiConfig);
  std::string config = this->Makefile->GetDefaultConfiguration();

  // Choose a default install configuration.
  std::string default_config = config;
  const char* default_order[] = { "RELEASE", "MINSIZEREL", "RELWITHDEBINFO",
                                  "DEBUG", nullptr };
  for (const char** c = default_order; *c && default_config.empty(); ++c) {
    for (std::string const& configurationType : configurationTypes) {
      if (cmSystemTools::UpperCase(configurationType) == *c) {
        default_config = configurationType;
      }
    }
  }
  if (default_config.empty() && !configurationTypes.empty()) {
    default_config = configurationTypes[0];
  }

  // Create the install script file.
  std::string file = this->StateSnapshot.GetDirectory().GetCurrentBinary();
  std::string homedir = this->GetState()->GetBinaryDirectory();
  int toplevel_install = 0;
  if (file == homedir) {
    toplevel_install = 1;
  }
  file += "/cmake_install.cmake";
  cmGeneratedFileStream fout(file);
  fout.SetCopyIfDifferent(true);

  // Write the header.
  /* clang-format off */
  fout << "# Install script for directory: "
       << this->StateSnapshot.GetDirectory().GetCurrentSource()
       << "\n\n"
          "# Set the install prefix\n"
          "if(NOT DEFINED CMAKE_INSTALL_PREFIX)\n"
          "  set(CMAKE_INSTALL_PREFIX \"" << prefix << "\")\n"
          "endif()\n"
       << R"(string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX )"
       << "\"${CMAKE_INSTALL_PREFIX}\")\n\n";
  /* clang-format on */

  // Write support code for generating per-configuration install rules.
  /* clang-format off */
  fout <<
    "# Set the install configuration name.\n"
    "if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)\n"
    "  if(BUILD_TYPE)\n"
    "    string(REGEX REPLACE \"^[^A-Za-z0-9_]+\" \"\"\n"
    "           CMAKE_INSTALL_CONFIG_NAME \"${BUILD_TYPE}\")\n"
    "  else()\n"
    "    set(CMAKE_INSTALL_CONFIG_NAME \"" << default_config << "\")\n"
    "  endif()\n"
    "  message(STATUS \"Install configuration: "
    "\\\"${CMAKE_INSTALL_CONFIG_NAME}\\\"\")\n"
    "endif()\n"
    "\n";
  /* clang-format on */

  // Write support code for dealing with component-specific installs.
  /* clang-format off */
  fout <<
    "# Set the component getting installed.\n"
    "if(NOT CMAKE_INSTALL_COMPONENT)\n"
    "  if(COMPONENT)\n"
    "    message(STATUS \"Install component: \\\"${COMPONENT}\\\"\")\n"
    "    set(CMAKE_INSTALL_COMPONENT \"${COMPONENT}\")\n"
    "  else()\n"
    "    set(CMAKE_INSTALL_COMPONENT)\n"
    "  endif()\n"
    "endif()\n"
    "\n";
  /* clang-format on */

  // Copy user-specified install options to the install code.
  if (cmProp so_no_exe =
        this->Makefile->GetDefinition("CMAKE_INSTALL_SO_NO_EXE")) {
    /* clang-format off */
    fout <<
      "# Install shared libraries without execute permission?\n"
      "if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)\n"
      "  set(CMAKE_INSTALL_SO_NO_EXE \"" << *so_no_exe << "\")\n"
      "endif()\n"
      "\n";
    /* clang-format on */
  }

  // Copy cmake cross compile state to install code.
  if (cmProp crosscompiling =
        this->Makefile->GetDefinition("CMAKE_CROSSCOMPILING")) {
    /* clang-format off */
    fout <<
      "# Is this installation the result of a crosscompile?\n"
      "if(NOT DEFINED CMAKE_CROSSCOMPILING)\n"
      "  set(CMAKE_CROSSCOMPILING \"" << *crosscompiling << "\")\n"
      "endif()\n"
      "\n";
    /* clang-format on */
  }

  // Write default directory permissions.
  if (cmProp defaultDirPermissions = this->Makefile->GetDefinition(
        "CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS")) {
    /* clang-format off */
    fout <<
      "# Set default install directory permissions.\n"
      "if(NOT DEFINED CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS)\n"
      "  set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS \""
         << *defaultDirPermissions << "\")\n"
      "endif()\n"
      "\n";
    /* clang-format on */
  }

  // Write out CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM so that
  // installed code that uses `file(GET_RUNTIME_DEPENDENCIES)`
  // has same platform variable as when running cmake
  if (cmProp platform = this->Makefile->GetDefinition(
        "CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM")) {
    /* clang-format off */
    fout <<
      "# Set default install directory permissions.\n"
      "if(NOT DEFINED CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM)\n"
      "  set(CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM \""
         << *platform << "\")\n"
      "endif()\n"
      "\n";
    /* clang-format on */
  }

  // Write out CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL so that
  // installed code that uses `file(GET_RUNTIME_DEPENDENCIES)`
  // has same tool selected as when running cmake
  if (cmProp command =
        this->Makefile->GetDefinition("CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL")) {
    /* clang-format off */
    fout <<
      "# Set default install directory permissions.\n"
      "if(NOT DEFINED CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL)\n"
      "  set(CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL \""
         << *command << "\")\n"
      "endif()\n"
      "\n";
    /* clang-format on */
  }

  // Write out CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND so that
  // installed code that uses `file(GET_RUNTIME_DEPENDENCIES)`
  // has same path to the tool as when running cmake
  if (cmProp command = this->Makefile->GetDefinition(
        "CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND")) {
    /* clang-format off */
    fout <<
      "# Set default install directory permissions.\n"
      "if(NOT DEFINED CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND)\n"
      "  set(CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND \""
         << *command << "\")\n"
      "endif()\n"
      "\n";
    /* clang-format on */
  }

  // Write out CMAKE_OBJDUMP so that installed code that uses
  // `file(GET_RUNTIME_DEPENDENCIES)` and hasn't specified
  // CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND has consistent
  // logic to fallback to CMAKE_OBJDUMP when `objdump` is
  // not on the path
  if (cmProp command = this->Makefile->GetDefinition("CMAKE_OBJDUMP")) {
    /* clang-format off */
    fout <<
      "# Set default install directory permissions.\n"
      "if(NOT DEFINED CMAKE_OBJDUMP)\n"
      "  set(CMAKE_OBJDUMP \""
         << *command << "\")\n"
      "endif()\n"
      "\n";
    /* clang-format on */
  }

  this->AddGeneratorSpecificInstallSetup(fout);

  // Ask each install generator to write its code.
  cmPolicies::PolicyStatus status = this->GetPolicyStatus(cmPolicies::CMP0082);
  auto const& installers = this->Makefile->GetInstallGenerators();
  bool haveSubdirectoryInstall = false;
  bool haveInstallAfterSubdirectory = false;
  if (status == cmPolicies::WARN) {
    for (const auto& installer : installers) {
      installer->CheckCMP0082(haveSubdirectoryInstall,
                              haveInstallAfterSubdirectory);
      installer->Generate(fout, config, configurationTypes);
    }
  } else {
    for (const auto& installer : installers) {
      installer->Generate(fout, config, configurationTypes);
    }
  }

  // Write rules from old-style specification stored in targets.
  this->GenerateTargetInstallRules(fout, config, configurationTypes);

  // Include install scripts from subdirectories.
  switch (status) {
    case cmPolicies::WARN:
      if (haveInstallAfterSubdirectory &&
          this->Makefile->PolicyOptionalWarningEnabled(
            "CMAKE_POLICY_WARNING_CMP0082")) {
        std::ostringstream e;
        e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0082) << "\n";
        this->IssueMessage(MessageType::AUTHOR_WARNING, e.str());
      }
      CM_FALLTHROUGH;
    case cmPolicies::OLD: {
      std::vector<cmStateSnapshot> children =
        this->Makefile->GetStateSnapshot().GetChildren();
      if (!children.empty()) {
        fout << "if(NOT CMAKE_INSTALL_LOCAL_ONLY)\n";
        fout << "  # Include the install script for each subdirectory.\n";
        for (cmStateSnapshot const& c : children) {
          if (!c.GetDirectory().GetPropertyAsBool("EXCLUDE_FROM_ALL")) {
            std::string odir = c.GetDirectory().GetCurrentBinary();
            cmSystemTools::ConvertToUnixSlashes(odir);
            fout << "  include(\"" << odir << "/cmake_install.cmake\")\n";
          }
        }
        fout << "\n";
        fout << "endif()\n\n";
      }
    } break;

    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::NEW:
      // NEW behavior is handled in
      // cmInstallSubdirectoryGenerator::GenerateScript()
      break;
  }

  // Record the install manifest.
  if (toplevel_install) {
    /* clang-format off */
    fout <<
      "if(CMAKE_INSTALL_COMPONENT)\n"
      "  set(CMAKE_INSTALL_MANIFEST \"install_manifest_"
      "${CMAKE_INSTALL_COMPONENT}.txt\")\n"
      "else()\n"
      "  set(CMAKE_INSTALL_MANIFEST \"install_manifest.txt\")\n"
      "endif()\n"
      "\n"
      "string(REPLACE \";\" \"\\n\" CMAKE_INSTALL_MANIFEST_CONTENT\n"
      "       \"${CMAKE_INSTALL_MANIFEST_FILES}\")\n"
      "file(WRITE \"" << homedir << "/${CMAKE_INSTALL_MANIFEST}\"\n"
      "     \"${CMAKE_INSTALL_MANIFEST_CONTENT}\")\n";
    /* clang-format on */
  }
}

void cmLocalGenerator::AddGeneratorTarget(
  std::unique_ptr<cmGeneratorTarget> gt)
{
  cmGeneratorTarget* gt_ptr = gt.get();

  this->GeneratorTargets.push_back(std::move(gt));
  this->GeneratorTargetSearchIndex.emplace(gt_ptr->GetName(), gt_ptr);
  this->GlobalGenerator->IndexGeneratorTarget(gt_ptr);
}

void cmLocalGenerator::AddImportedGeneratorTarget(cmGeneratorTarget* gt)
{
  this->ImportedGeneratorTargets.emplace(gt->GetName(), gt);
  this->GlobalGenerator->IndexGeneratorTarget(gt);
}

void cmLocalGenerator::AddOwnedImportedGeneratorTarget(
  std::unique_ptr<cmGeneratorTarget> gt)
{
  this->OwnedImportedGeneratorTargets.push_back(std::move(gt));
}

cmGeneratorTarget* cmLocalGenerator::FindLocalNonAliasGeneratorTarget(
  const std::string& name) const
{
  auto ti = this->GeneratorTargetSearchIndex.find(name);
  if (ti != this->GeneratorTargetSearchIndex.end()) {
    return ti->second;
  }
  return nullptr;
}

void cmLocalGenerator::ComputeTargetManifest()
{
  // Collect the set of configuration types.
  std::vector<std::string> configNames =
    this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

  // Add our targets to the manifest for each configuration.
  const auto& targets = this->GetGeneratorTargets();
  for (const auto& target : targets) {
    if (!target->IsInBuildSystem()) {
      continue;
    }
    for (std::string const& c : configNames) {
      target->ComputeTargetManifest(c);
    }
  }
}

bool cmLocalGenerator::ComputeTargetCompileFeatures()
{
  // Collect the set of configuration types.
  std::vector<std::string> configNames =
    this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

  using LanguagePair = std::pair<std::string, std::string>;
  std::vector<LanguagePair> pairedLanguages{ { "OBJC", "C" },
                                             { "OBJCXX", "CXX" },
                                             { "CUDA", "CXX" } };
  std::set<LanguagePair> inferredEnabledLanguages;
  for (auto const& lang : pairedLanguages) {
    if (this->Makefile->GetState()->GetLanguageEnabled(lang.first)) {
      inferredEnabledLanguages.insert(lang);
    }
  }

  // Process compile features of all targets.
  const auto& targets = this->GetGeneratorTargets();
  for (const auto& target : targets) {
    for (std::string const& c : configNames) {
      if (!target->ComputeCompileFeatures(c)) {
        return false;
      }
    }

    // Now that C/C++ _STANDARD values have been computed
    // set the values to ObjC/ObjCXX _STANDARD variables
    if (target->CanCompileSources()) {
      for (std::string const& c : configNames) {
        target->ComputeCompileFeatures(c, inferredEnabledLanguages);
      }
    }
  }

  return true;
}

bool cmLocalGenerator::IsRootMakefile() const
{
  return !this->StateSnapshot.GetBuildsystemDirectoryParent().IsValid();
}

cmState* cmLocalGenerator::GetState() const
{
  return this->GlobalGenerator->GetCMakeInstance()->GetState();
}

cmStateSnapshot cmLocalGenerator::GetStateSnapshot() const
{
  return this->Makefile->GetStateSnapshot();
}

cmProp cmLocalGenerator::GetRuleLauncher(cmGeneratorTarget* target,
                                         const std::string& prop)
{
  if (target) {
    return target->GetProperty(prop);
  }
  return this->Makefile->GetProperty(prop);
}

std::string cmLocalGenerator::ConvertToIncludeReference(
  std::string const& path, OutputFormat format, bool forceFullPaths)
{
  static_cast<void>(forceFullPaths);
  return this->ConvertToOutputForExisting(path, format);
}

std::string cmLocalGenerator::GetIncludeFlags(
  const std::vector<std::string>& includeDirs, cmGeneratorTarget* target,
  const std::string& lang, bool forceFullPaths, bool forResponseFile,
  const std::string& config)
{
  if (lang.empty()) {
    return "";
  }

  std::vector<std::string> includes = includeDirs;
  MoveSystemIncludesToEnd(includes, config, lang, target);

  OutputFormat shellFormat = forResponseFile ? RESPONSE : SHELL;
  std::ostringstream includeFlags;

  std::string const& includeFlag =
    this->Makefile->GetSafeDefinition(cmStrCat("CMAKE_INCLUDE_FLAG_", lang));
  const char* sep = cmToCStr(
    this->Makefile->GetDefinition(cmStrCat("CMAKE_INCLUDE_FLAG_SEP_", lang)));
  bool quotePaths = false;
  if (this->Makefile->GetDefinition("CMAKE_QUOTE_INCLUDE_PATHS")) {
    quotePaths = true;
  }
  bool repeatFlag = true;
  // should the include flag be repeated like ie. -IA -IB
  if (!sep) {
    sep = " ";
  } else {
    // if there is a separator then the flag is not repeated but is only
    // given once i.e.  -classpath a:b:c
    repeatFlag = false;
  }

  // Support special system include flag if it is available and the
  // normal flag is repeated for each directory.
  cmProp sysIncludeFlag = nullptr;
  if (repeatFlag) {
    sysIncludeFlag = this->Makefile->GetDefinition(
      cmStrCat("CMAKE_INCLUDE_SYSTEM_FLAG_", lang));
  }

  cmProp fwSearchFlag = this->Makefile->GetDefinition(
    cmStrCat("CMAKE_", lang, "_FRAMEWORK_SEARCH_FLAG"));
  cmProp sysFwSearchFlag = this->Makefile->GetDefinition(
    cmStrCat("CMAKE_", lang, "_SYSTEM_FRAMEWORK_SEARCH_FLAG"));

  bool flagUsed = false;
  std::set<std::string> emitted;
#ifdef __APPLE__
  emitted.insert("/System/Library/Frameworks");
#endif
  for (std::string const& i : includes) {
    if (cmNonempty(fwSearchFlag) && this->Makefile->IsOn("APPLE") &&
        cmSystemTools::IsPathToFramework(i)) {
      std::string const frameworkDir =
        cmSystemTools::CollapseFullPath(cmStrCat(i, "/../"));
      if (emitted.insert(frameworkDir).second) {
        if (sysFwSearchFlag && target &&
            target->IsSystemIncludeDirectory(i, config, lang)) {
          includeFlags << *sysFwSearchFlag;
        } else {
          includeFlags << *fwSearchFlag;
        }
        includeFlags << this->ConvertToOutputFormat(frameworkDir, shellFormat)
                     << " ";
      }
      continue;
    }

    if (!flagUsed || repeatFlag) {
      if (sysIncludeFlag && target &&
          target->IsSystemIncludeDirectory(i, config, lang)) {
        includeFlags << *sysIncludeFlag;
      } else {
        includeFlags << includeFlag;
      }
      flagUsed = true;
    }
    std::string includePath =
      this->ConvertToIncludeReference(i, shellFormat, forceFullPaths);
    if (quotePaths && !includePath.empty() && includePath.front() != '\"') {
      includeFlags << "\"";
    }
    includeFlags << includePath;
    if (quotePaths && !includePath.empty() && includePath.front() != '\"') {
      includeFlags << "\"";
    }
    includeFlags << sep;
  }
  std::string flags = includeFlags.str();
  // remove trailing separators
  if ((sep[0] != ' ') && !flags.empty() && flags.back() == sep[0]) {
    flags.back() = ' ';
  }
  return cmTrimWhitespace(flags);
}

void cmLocalGenerator::AddCompileOptions(std::string& flags,
                                         cmGeneratorTarget* target,
                                         const std::string& lang,
                                         const std::string& config)
{
  std::vector<BT<std::string>> tmpFlags;
  this->AddCompileOptions(tmpFlags, target, lang, config);
  this->AppendFlags(flags, tmpFlags);
}

void cmLocalGenerator::AddCompileOptions(std::vector<BT<std::string>>& flags,
                                         cmGeneratorTarget* target,
                                         const std::string& lang,
                                         const std::string& config)
{
  std::string langFlagRegexVar = cmStrCat("CMAKE_", lang, "_FLAG_REGEX");

  if (cmProp langFlagRegexStr =
        this->Makefile->GetDefinition(langFlagRegexVar)) {
    // Filter flags acceptable to this language.
    if (cmProp targetFlags = target->GetProperty("COMPILE_FLAGS")) {
      std::vector<std::string> opts;
      cmSystemTools::ParseWindowsCommandLine(targetFlags->c_str(), opts);
      // Re-escape these flags since COMPILE_FLAGS were already parsed
      // as a command line above.
      std::string compileOpts;
      this->AppendCompileOptions(compileOpts, opts, langFlagRegexStr->c_str());
      if (!compileOpts.empty()) {
        flags.emplace_back(std::move(compileOpts));
      }
    }
    std::vector<BT<std::string>> targetCompileOpts =
      target->GetCompileOptions(config, lang);
    // COMPILE_OPTIONS are escaped.
    this->AppendCompileOptions(flags, targetCompileOpts,
                               langFlagRegexStr->c_str());
  } else {
    // Use all flags.
    if (cmProp targetFlags = target->GetProperty("COMPILE_FLAGS")) {
      // COMPILE_FLAGS are not escaped for historical reasons.
      std::string compileFlags;
      this->AppendFlags(compileFlags, *targetFlags);
      if (!compileFlags.empty()) {
        flags.emplace_back(std::move(compileFlags));
      }
    }
    std::vector<BT<std::string>> targetCompileOpts =
      target->GetCompileOptions(config, lang);
    // COMPILE_OPTIONS are escaped.
    this->AppendCompileOptions(flags, targetCompileOpts);
  }

  cmStandardLevelResolver standardResolver(this->Makefile);
  for (auto const& it : target->GetMaxLanguageStandards()) {
    cmProp standard = target->GetLanguageStandard(it.first, config);
    if (!standard) {
      continue;
    }
    if (standardResolver.IsLaterStandard(it.first, *standard, it.second)) {
      std::ostringstream e;
      e << "The COMPILE_FEATURES property of target \"" << target->GetName()
        << "\" was evaluated when computing the link "
           "implementation, and the \""
        << it.first << "_STANDARD\" was \"" << it.second
        << "\" for that computation.  Computing the "
           "COMPILE_FEATURES based on the link implementation resulted in a "
           "higher \""
        << it.first << "_STANDARD\" \"" << *standard
        << "\".  "
           "This is not permitted. The COMPILE_FEATURES may not both depend "
           "on "
           "and be depended on by the link implementation.\n";
      this->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return;
    }
  }

  std::string compReqFlag;
  this->AddCompilerRequirementFlag(compReqFlag, target, lang, config);
  if (!compReqFlag.empty()) {
    flags.emplace_back(std::move(compReqFlag));
  }

  // Add compile flag for the MSVC compiler only.
  cmMakefile* mf = this->GetMakefile();
  if (cmProp jmc =
        mf->GetDefinition("CMAKE_" + lang + "_COMPILE_OPTIONS_JMC")) {

    // Handle Just My Code debugging flags, /JMC.
    // If the target is a Managed C++ one, /JMC is not compatible.
    if (target->GetManagedType(config) !=
        cmGeneratorTarget::ManagedType::Managed) {
      // add /JMC flags if target property VS_JUST_MY_CODE_DEBUGGING is set
      // to ON
      if (cmProp jmcExprGen =
            target->GetProperty("VS_JUST_MY_CODE_DEBUGGING")) {
        std::string isJMCEnabled =
          cmGeneratorExpression::Evaluate(*jmcExprGen, this, config);
        if (cmIsOn(isJMCEnabled)) {
          std::vector<std::string> optVec = cmExpandedList(*jmc);
          std::string jmcFlags;
          this->AppendCompileOptions(jmcFlags, optVec);
          if (!jmcFlags.empty()) {
            flags.emplace_back(std::move(jmcFlags));
          }
        }
      }
    }
  }
}

cmTarget* cmLocalGenerator::AddCustomCommandToTarget(
  const std::string& target, const std::vector<std::string>& byproducts,
  const std::vector<std::string>& depends,
  const cmCustomCommandLines& commandLines, cmCustomCommandType type,
  const char* comment, const char* workingDir,
  cmPolicies::PolicyStatus cmp0116, bool escapeOldStyle, bool uses_terminal,
  const std::string& depfile, const std::string& job_pool,
  bool command_expand_lists, cmObjectLibraryCommands objLibCommands,
  bool stdPipesUTF8)
{
  cmTarget* t = this->Makefile->GetCustomCommandTarget(
    target, objLibCommands, this->DirectoryBacktrace);
  if (!t) {
    return nullptr;
  }

  detail::AddCustomCommandToTarget(
    *this, this->DirectoryBacktrace, cmCommandOrigin::Generator, t, byproducts,
    depends, commandLines, type, comment, workingDir, escapeOldStyle,
    uses_terminal, depfile, job_pool, command_expand_lists, stdPipesUTF8,
    cmp0116);

  return t;
}

cmSourceFile* cmLocalGenerator::AddCustomCommandToOutput(
  const std::string& output, const std::vector<std::string>& depends,
  const std::string& main_dependency, const cmCustomCommandLines& commandLines,
  const char* comment, const char* workingDir,
  cmPolicies::PolicyStatus cmp0116, bool replace, bool escapeOldStyle,
  bool uses_terminal, bool command_expand_lists, const std::string& depfile,
  const std::string& job_pool, bool stdPipesUTF8)
{
  std::vector<std::string> no_byproducts;
  cmImplicitDependsList no_implicit_depends;
  return this->AddCustomCommandToOutput(
    { output }, no_byproducts, depends, main_dependency, no_implicit_depends,
    commandLines, comment, workingDir, cmp0116, replace, escapeOldStyle,
    uses_terminal, command_expand_lists, depfile, job_pool, stdPipesUTF8);
}

cmSourceFile* cmLocalGenerator::AddCustomCommandToOutput(
  const std::vector<std::string>& outputs,
  const std::vector<std::string>& byproducts,
  const std::vector<std::string>& depends, const std::string& main_dependency,
  const cmImplicitDependsList& implicit_depends,
  const cmCustomCommandLines& commandLines, const char* comment,
  const char* workingDir, cmPolicies::PolicyStatus cmp0116, bool replace,
  bool escapeOldStyle, bool uses_terminal, bool command_expand_lists,
  const std::string& depfile, const std::string& job_pool, bool stdPipesUTF8)
{
  // Make sure there is at least one output.
  if (outputs.empty()) {
    cmSystemTools::Error("Attempt to add a custom rule with no output!");
    return nullptr;
  }

  return detail::AddCustomCommandToOutput(
    *this, this->DirectoryBacktrace, cmCommandOrigin::Generator, outputs,
    byproducts, depends, main_dependency, implicit_depends, commandLines,
    comment, workingDir, replace, escapeOldStyle, uses_terminal,
    command_expand_lists, depfile, job_pool, stdPipesUTF8, cmp0116);
}

cmTarget* cmLocalGenerator::AddUtilityCommand(
  const std::string& utilityName, bool excludeFromAll, const char* workingDir,
  const std::vector<std::string>& byproducts,
  const std::vector<std::string>& depends,
  const cmCustomCommandLines& commandLines, cmPolicies::PolicyStatus cmp0116,
  bool escapeOldStyle, const char* comment, bool uses_terminal,
  bool command_expand_lists, const std::string& job_pool, bool stdPipesUTF8)
{
  cmTarget* target =
    this->Makefile->AddNewUtilityTarget(utilityName, excludeFromAll);
  target->SetIsGeneratorProvided(true);

  if (commandLines.empty() && depends.empty()) {
    return target;
  }

  detail::AddUtilityCommand(
    *this, this->DirectoryBacktrace, cmCommandOrigin::Generator, target,
    workingDir, byproducts, depends, commandLines, escapeOldStyle, comment,
    uses_terminal, command_expand_lists, job_pool, stdPipesUTF8, cmp0116);

  return target;
}

std::vector<BT<std::string>> cmLocalGenerator::GetIncludeDirectoriesImplicit(
  cmGeneratorTarget const* target, std::string const& lang,
  std::string const& config, bool stripImplicitDirs,
  bool appendAllImplicitDirs) const
{
  std::vector<BT<std::string>> result;
  // Do not repeat an include path.
  std::set<std::string> emitted;

  auto emitDir = [&result, &emitted](std::string const& dir) {
    if (emitted.insert(dir).second) {
      result.emplace_back(dir);
    }
  };

  auto emitBT = [&result, &emitted](BT<std::string> const& dir) {
    if (emitted.insert(dir.Value).second) {
      result.emplace_back(dir);
    }
  };

  // When automatic include directories are requested for a build then
  // include the source and binary directories at the beginning of the
  // include path to approximate include file behavior for an
  // in-source build.  This does not account for the case of a source
  // file in a subdirectory of the current source directory but we
  // cannot fix this because not all native build tools support
  // per-source-file include paths.
  if (this->Makefile->IsOn("CMAKE_INCLUDE_CURRENT_DIR")) {
    // Current binary directory
    emitDir(this->StateSnapshot.GetDirectory().GetCurrentBinary());
    // Current source directory
    emitDir(this->StateSnapshot.GetDirectory().GetCurrentSource());
  }

  if (!target) {
    return result;
  }

  // Standard include directories to be added unconditionally at the end.
  // These are intended to simulate additional implicit include directories.
  std::vector<std::string> userStandardDirs;
  {
    std::string const value = this->Makefile->GetSafeDefinition(
      cmStrCat("CMAKE_", lang, "_STANDARD_INCLUDE_DIRECTORIES"));
    cmExpandList(value, userStandardDirs);
    for (std::string& usd : userStandardDirs) {
      cmSystemTools::ConvertToUnixSlashes(usd);
    }
  }

  // Implicit include directories
  std::vector<std::string> implicitDirs;
  std::set<std::string> implicitSet;
  // Include directories to be excluded as if they were implicit.
  std::set<std::string> implicitExclude;
  {
    // Raw list of implicit include directories
    // Start with "standard" directories that we unconditionally add below.
    std::vector<std::string> impDirVec = userStandardDirs;

    // Load implicit include directories for this language.
    // We ignore this for Fortran because:
    // * There are no standard library headers to avoid overriding.
    // * Compilers like gfortran do not search their own implicit include
    //   directories for modules ('.mod' files).
    if (lang != "Fortran") {
      size_t const impDirVecOldSize = impDirVec.size();
      if (this->Makefile->GetDefExpandList(
            cmStrCat("CMAKE_", lang, "_IMPLICIT_INCLUDE_DIRECTORIES"),
            impDirVec)) {
        // FIXME: Use cmRange with 'advance()' when it supports non-const.
        for (size_t i = impDirVecOldSize; i < impDirVec.size(); ++i) {
          cmSystemTools::ConvertToUnixSlashes(impDirVec[i]);
        }
      }
    }

    // The Platform/UnixPaths module used to hard-code /usr/include for C, CXX,
    // and CUDA in CMAKE_<LANG>_IMPLICIT_INCLUDE_DIRECTORIES, but those
    // variables are now computed.  On macOS the /usr/include directory is
    // inside the platform SDK so the computed value does not contain it
    // directly.  In this case adding -I/usr/include can hide SDK headers so we
    // must still exclude it.
    if ((lang == "C" || lang == "CXX" || lang == "CUDA") &&
        !cm::contains(impDirVec, "/usr/include") &&
        std::find_if(impDirVec.begin(), impDirVec.end(),
                     [](std::string const& d) {
                       return cmHasLiteralSuffix(d, "/usr/include");
                     }) != impDirVec.end()) {
      // Only exclude this hard coded path for backwards compatibility.
      implicitExclude.emplace("/usr/include");
    }

    for (std::string const& i : impDirVec) {
      if (implicitSet.insert(this->GlobalGenerator->GetRealPath(i)).second) {
        implicitDirs.emplace_back(i);
      }
    }
  }

  // Checks if this is not an excluded (implicit) include directory.
  auto notExcluded = [this, &implicitSet, &implicitExclude,
                      &lang](std::string const& dir) {
    return (
      // Do not exclude directories that are not in an excluded set.
      ((!cm::contains(implicitSet, this->GlobalGenerator->GetRealPath(dir))) &&
       (!cm::contains(implicitExclude, dir)))
      // Do not exclude entries of the CPATH environment variable even though
      // they are implicitly searched by the compiler.  They are meant to be
      // user-specified directories that can be re-ordered or converted to
      // -isystem without breaking real compiler builtin headers.
      ||
      ((lang == "C" || lang == "CXX") && cm::contains(this->EnvCPATH, dir)));
  };

  // Get the target-specific include directories.
  std::vector<BT<std::string>> userDirs =
    target->GetIncludeDirectories(config, lang);

  // Support putting all the in-project include directories first if
  // it is requested by the project.
  if (this->Makefile->IsOn("CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE")) {
    std::string const& topSourceDir = this->GetState()->GetSourceDirectory();
    std::string const& topBinaryDir = this->GetState()->GetBinaryDirectory();
    for (BT<std::string> const& udr : userDirs) {
      // Emit this directory only if it is a subdirectory of the
      // top-level source or binary tree.
      if (cmSystemTools::ComparePath(udr.Value, topSourceDir) ||
          cmSystemTools::ComparePath(udr.Value, topBinaryDir) ||
          cmSystemTools::IsSubDirectory(udr.Value, topSourceDir) ||
          cmSystemTools::IsSubDirectory(udr.Value, topBinaryDir)) {
        if (notExcluded(udr.Value)) {
          emitBT(udr);
        }
      }
    }
  }

  // Emit remaining non implicit user directories.
  for (BT<std::string> const& udr : userDirs) {
    if (notExcluded(udr.Value)) {
      emitBT(udr);
    }
  }

  // Sort result
  MoveSystemIncludesToEnd(result, config, lang, target);

  // Append standard include directories for this language.
  userDirs.reserve(userDirs.size() + userStandardDirs.size());
  for (std::string& usd : userStandardDirs) {
    emitDir(usd);
    userDirs.emplace_back(std::move(usd));
  }

  // Append compiler implicit include directories
  if (!stripImplicitDirs) {
    // Append implicit directories that were requested by the user only
    for (BT<std::string> const& udr : userDirs) {
      if (cm::contains(implicitSet, cmSystemTools::GetRealPath(udr.Value))) {
        emitBT(udr);
      }
    }
    // Append remaining implicit directories (on demand)
    if (appendAllImplicitDirs) {
      for (std::string& imd : implicitDirs) {
        emitDir(imd);
      }
    }
  }

  return result;
}

void cmLocalGenerator::GetIncludeDirectoriesImplicit(
  std::vector<std::string>& dirs, cmGeneratorTarget const* target,
  const std::string& lang, const std::string& config, bool stripImplicitDirs,
  bool appendAllImplicitDirs) const
{
  std::vector<BT<std::string>> tmp = this->GetIncludeDirectoriesImplicit(
    target, lang, config, stripImplicitDirs, appendAllImplicitDirs);
  dirs.reserve(dirs.size() + tmp.size());
  for (BT<std::string>& v : tmp) {
    dirs.emplace_back(std::move(v.Value));
  }
}

std::vector<BT<std::string>> cmLocalGenerator::GetIncludeDirectories(
  cmGeneratorTarget const* target, std::string const& lang,
  std::string const& config) const
{
  return this->GetIncludeDirectoriesImplicit(target, lang, config);
}

void cmLocalGenerator::GetIncludeDirectories(std::vector<std::string>& dirs,
                                             cmGeneratorTarget const* target,
                                             const std::string& lang,
                                             const std::string& config) const
{
  this->GetIncludeDirectoriesImplicit(dirs, target, lang, config);
}

void cmLocalGenerator::GetStaticLibraryFlags(std::string& flags,
                                             std::string const& config,
                                             std::string const& linkLanguage,
                                             cmGeneratorTarget* target)
{
  std::vector<BT<std::string>> tmpFlags =
    this->GetStaticLibraryFlags(config, linkLanguage, target);
  this->AppendFlags(flags, tmpFlags);
}

std::vector<BT<std::string>> cmLocalGenerator::GetStaticLibraryFlags(
  std::string const& config, std::string const& linkLanguage,
  cmGeneratorTarget* target)
{
  const std::string configUpper = cmSystemTools::UpperCase(config);
  std::vector<BT<std::string>> flags;
  if (linkLanguage != "Swift") {
    std::string staticLibFlags;
    this->AppendFlags(
      staticLibFlags,
      this->Makefile->GetSafeDefinition("CMAKE_STATIC_LINKER_FLAGS"));
    if (!configUpper.empty()) {
      std::string name = "CMAKE_STATIC_LINKER_FLAGS_" + configUpper;
      this->AppendFlags(staticLibFlags,
                        this->Makefile->GetSafeDefinition(name));
    }
    if (!staticLibFlags.empty()) {
      flags.emplace_back(std::move(staticLibFlags));
    }
  }

  std::string staticLibFlags;
  this->AppendFlags(staticLibFlags,
                    target->GetSafeProperty("STATIC_LIBRARY_FLAGS"));
  if (!configUpper.empty()) {
    std::string name = "STATIC_LIBRARY_FLAGS_" + configUpper;
    this->AppendFlags(staticLibFlags, target->GetSafeProperty(name));
  }

  if (!staticLibFlags.empty()) {
    flags.emplace_back(std::move(staticLibFlags));
  }

  std::vector<BT<std::string>> staticLibOpts =
    target->GetStaticLibraryLinkOptions(config, linkLanguage);
  // STATIC_LIBRARY_OPTIONS are escaped.
  this->AppendCompileOptions(flags, staticLibOpts);

  return flags;
}

void cmLocalGenerator::GetDeviceLinkFlags(
  cmLinkLineComputer* linkLineComputer, const std::string& config,
  std::string& linkLibs, std::string& linkFlags, std::string& frameworkPath,
  std::string& linkPath, cmGeneratorTarget* target)
{
  cmGeneratorTarget::DeviceLinkSetter setter(*target);

  cmComputeLinkInformation* pcli = target->GetLinkInformation(config);
  const std::string linkLanguage =
    linkLineComputer->GetLinkerLanguage(target, config);

  if (pcli) {
    // Compute the required cuda device link libraries when
    // resolving cuda device symbols
    this->OutputLinkLibraries(pcli, linkLineComputer, linkLibs, frameworkPath,
                              linkPath);
  }

  std::vector<std::string> linkOpts;
  target->GetLinkOptions(linkOpts, config, linkLanguage);
  // LINK_OPTIONS are escaped.
  this->AppendCompileOptions(linkFlags, linkOpts);
}

void cmLocalGenerator::GetTargetFlags(
  cmLinkLineComputer* linkLineComputer, const std::string& config,
  std::string& linkLibs, std::string& flags, std::string& linkFlags,
  std::string& frameworkPath, std::string& linkPath, cmGeneratorTarget* target)
{
  std::vector<BT<std::string>> linkFlagsList;
  std::vector<BT<std::string>> linkPathList;
  std::vector<BT<std::string>> linkLibsList;
  this->GetTargetFlags(linkLineComputer, config, linkLibsList, flags,
                       linkFlagsList, frameworkPath, linkPathList, target);
  this->AppendFlags(linkFlags, linkFlagsList);
  this->AppendFlags(linkPath, linkPathList);
  this->AppendFlags(linkLibs, linkLibsList);
}

void cmLocalGenerator::GetTargetFlags(
  cmLinkLineComputer* linkLineComputer, const std::string& config,
  std::vector<BT<std::string>>& linkLibs, std::string& flags,
  std::vector<BT<std::string>>& linkFlags, std::string& frameworkPath,
  std::vector<BT<std::string>>& linkPath, cmGeneratorTarget* target)
{
  const std::string configUpper = cmSystemTools::UpperCase(config);
  cmComputeLinkInformation* pcli = target->GetLinkInformation(config);
  const char* libraryLinkVariable =
    "CMAKE_SHARED_LINKER_FLAGS"; // default to shared library

  const std::string linkLanguage =
    linkLineComputer->GetLinkerLanguage(target, config);

  switch (target->GetType()) {
    case cmStateEnums::STATIC_LIBRARY:
      linkFlags = this->GetStaticLibraryFlags(config, linkLanguage, target);
      break;
    case cmStateEnums::MODULE_LIBRARY:
      libraryLinkVariable = "CMAKE_MODULE_LINKER_FLAGS";
      CM_FALLTHROUGH;
    case cmStateEnums::SHARED_LIBRARY: {
      std::string sharedLibFlags;
      if (linkLanguage != "Swift") {
        sharedLibFlags = cmStrCat(
          this->Makefile->GetSafeDefinition(libraryLinkVariable), ' ');
        if (!configUpper.empty()) {
          std::string build = cmStrCat(libraryLinkVariable, '_', configUpper);
          sharedLibFlags += this->Makefile->GetSafeDefinition(build);
          sharedLibFlags += " ";
        }
        if (this->Makefile->IsOn("WIN32") &&
            !(this->Makefile->IsOn("CYGWIN") ||
              this->Makefile->IsOn("MINGW"))) {
          std::vector<cmSourceFile*> sources;
          target->GetSourceFiles(sources, config);
          std::string defFlag =
            this->Makefile->GetSafeDefinition("CMAKE_LINK_DEF_FILE_FLAG");
          for (cmSourceFile* sf : sources) {
            if (sf->GetExtension() == "def") {
              sharedLibFlags += defFlag;
              sharedLibFlags +=
                this->ConvertToOutputFormat(sf->ResolveFullPath(), SHELL);
              sharedLibFlags += " ";
            }
          }
        }
      }

      cmProp targetLinkFlags = target->GetProperty("LINK_FLAGS");
      if (targetLinkFlags) {
        sharedLibFlags += *targetLinkFlags;
        sharedLibFlags += " ";
      }
      if (!configUpper.empty()) {
        targetLinkFlags =
          target->GetProperty(cmStrCat("LINK_FLAGS_", configUpper));
        if (targetLinkFlags) {
          sharedLibFlags += *targetLinkFlags;
          sharedLibFlags += " ";
        }
      }

      if (!sharedLibFlags.empty()) {
        linkFlags.emplace_back(std::move(sharedLibFlags));
      }

      std::vector<BT<std::string>> linkOpts =
        target->GetLinkOptions(config, linkLanguage);
      // LINK_OPTIONS are escaped.
      this->AppendCompileOptions(linkFlags, linkOpts);
      if (pcli) {
        this->OutputLinkLibraries(pcli, linkLineComputer, linkLibs,
                                  frameworkPath, linkPath);
      }
    } break;
    case cmStateEnums::EXECUTABLE: {
      std::string exeFlags;
      if (linkLanguage != "Swift") {
        exeFlags = this->Makefile->GetSafeDefinition("CMAKE_EXE_LINKER_FLAGS");
        exeFlags += " ";
        if (!configUpper.empty()) {
          exeFlags += this->Makefile->GetSafeDefinition(
            cmStrCat("CMAKE_EXE_LINKER_FLAGS_", configUpper));
          exeFlags += " ";
        }
        if (linkLanguage.empty()) {
          cmSystemTools::Error(
            "CMake can not determine linker language for target: " +
            target->GetName());
          return;
        }

        if (target->IsWin32Executable(config)) {
          exeFlags +=
            this->Makefile->GetSafeDefinition("CMAKE_CREATE_WIN32_EXE");
          exeFlags += " ";
        } else {
          exeFlags +=
            this->Makefile->GetSafeDefinition("CMAKE_CREATE_CONSOLE_EXE");
          exeFlags += " ";
        }

        if (target->IsExecutableWithExports()) {
          exeFlags += this->Makefile->GetSafeDefinition(
            cmStrCat("CMAKE_EXE_EXPORTS_", linkLanguage, "_FLAG"));
          exeFlags += " ";
        }
      }

      this->AddLanguageFlagsForLinking(flags, target, linkLanguage, config);
      if (pcli) {
        this->OutputLinkLibraries(pcli, linkLineComputer, linkLibs,
                                  frameworkPath, linkPath);
      }

      if (this->Makefile->IsOn("BUILD_SHARED_LIBS")) {
        std::string sFlagVar = "CMAKE_SHARED_BUILD_" + linkLanguage + "_FLAGS";
        exeFlags += this->Makefile->GetSafeDefinition(sFlagVar);
        exeFlags += " ";
      }

      std::string cmp0065Flags =
        this->GetLinkLibsCMP0065(linkLanguage, *target);
      if (!cmp0065Flags.empty()) {
        exeFlags += cmp0065Flags;
        exeFlags += " ";
      }

      cmProp targetLinkFlags = target->GetProperty("LINK_FLAGS");
      if (targetLinkFlags) {
        exeFlags += *targetLinkFlags;
        exeFlags += " ";
      }
      if (!configUpper.empty()) {
        targetLinkFlags =
          target->GetProperty(cmStrCat("LINK_FLAGS_", configUpper));
        if (targetLinkFlags) {
          exeFlags += *targetLinkFlags;
          exeFlags += " ";
        }
      }

      if (!exeFlags.empty()) {
        linkFlags.emplace_back(std::move(exeFlags));
      }

      std::vector<BT<std::string>> linkOpts =
        target->GetLinkOptions(config, linkLanguage);
      // LINK_OPTIONS are escaped.
      this->AppendCompileOptions(linkFlags, linkOpts);
    } break;
    default:
      break;
  }

  std::string extraLinkFlags;
  this->AppendPositionIndependentLinkerFlags(extraLinkFlags, target, config,
                                             linkLanguage);
  this->AppendIPOLinkerFlags(extraLinkFlags, target, config, linkLanguage);

  if (!extraLinkFlags.empty()) {
    linkFlags.emplace_back(std::move(extraLinkFlags));
  }
}

void cmLocalGenerator::GetTargetCompileFlags(cmGeneratorTarget* target,
                                             std::string const& config,
                                             std::string const& lang,
                                             std::string& flags,
                                             std::string const& arch)
{
  std::vector<BT<std::string>> tmpFlags =
    this->GetTargetCompileFlags(target, config, lang, arch);
  this->AppendFlags(flags, tmpFlags);
}

std::vector<BT<std::string>> cmLocalGenerator::GetTargetCompileFlags(
  cmGeneratorTarget* target, std::string const& config,
  std::string const& lang, std::string const& arch)
{
  std::vector<BT<std::string>> flags;
  std::string compileFlags;

  cmMakefile* mf = this->GetMakefile();

  // Add language-specific flags.
  this->AddLanguageFlags(compileFlags, target, lang, config);

  if (target->IsIPOEnabled(lang, config)) {
    this->AppendFeatureOptions(compileFlags, lang, "IPO");
  }

  this->AddArchitectureFlags(compileFlags, target, lang, config, arch);

  if (lang == "Fortran") {
    this->AppendFlags(compileFlags,
                      this->GetTargetFortranFlags(target, config));
  }

  this->AddCMP0018Flags(compileFlags, target, lang, config);
  this->AddVisibilityPresetFlags(compileFlags, target, lang);
  this->AppendFlags(compileFlags, mf->GetDefineFlags());
  this->AppendFlags(compileFlags,
                    this->GetFrameworkFlags(lang, config, target));

  if (!compileFlags.empty()) {
    flags.emplace_back(std::move(compileFlags));
  }
  this->AddCompileOptions(flags, target, lang, config);
  return flags;
}

static std::string GetFrameworkFlags(const std::string& lang,
                                     const std::string& config,
                                     cmGeneratorTarget* target)
{
  cmLocalGenerator* lg = target->GetLocalGenerator();
  cmMakefile* mf = lg->GetMakefile();

  if (!mf->IsOn("APPLE")) {
    return std::string();
  }

  std::string fwSearchFlagVar = "CMAKE_" + lang + "_FRAMEWORK_SEARCH_FLAG";
  cmProp fwSearchFlag = mf->GetDefinition(fwSearchFlagVar);
  if (!cmNonempty(fwSearchFlag)) {
    return std::string();
  }

  std::set<std::string> emitted;
#ifdef __APPLE__ /* don't insert this when crosscompiling e.g. to iphone */
  emitted.insert("/System/Library/Frameworks");
#endif
  std::vector<std::string> includes;

  lg->GetIncludeDirectories(includes, target, "C", config);
  // check all include directories for frameworks as this
  // will already have added a -F for the framework
  for (std::string const& include : includes) {
    if (lg->GetGlobalGenerator()->NameResolvesToFramework(include)) {
      std::string frameworkDir = cmStrCat(include, "/../");
      frameworkDir = cmSystemTools::CollapseFullPath(frameworkDir);
      emitted.insert(frameworkDir);
    }
  }

  std::string flags;
  if (cmComputeLinkInformation* cli = target->GetLinkInformation(config)) {
    std::vector<std::string> const& frameworks = cli->GetFrameworkPaths();
    for (std::string const& framework : frameworks) {
      if (emitted.insert(framework).second) {
        flags += *fwSearchFlag;
        flags +=
          lg->ConvertToOutputFormat(framework, cmOutputConverter::SHELL);
        flags += " ";
      }
    }
  }
  return flags;
}

std::string cmLocalGenerator::GetFrameworkFlags(std::string const& l,
                                                std::string const& config,
                                                cmGeneratorTarget* target)
{
  return ::GetFrameworkFlags(l, config, target);
}

void cmLocalGenerator::GetTargetDefines(cmGeneratorTarget const* target,
                                        std::string const& config,
                                        std::string const& lang,
                                        std::set<std::string>& defines) const
{
  std::set<BT<std::string>> tmp = this->GetTargetDefines(target, config, lang);
  for (BT<std::string> const& v : tmp) {
    defines.emplace(v.Value);
  }
}

std::set<BT<std::string>> cmLocalGenerator::GetTargetDefines(
  cmGeneratorTarget const* target, std::string const& config,
  std::string const& lang) const
{
  std::set<BT<std::string>> defines;

  // Add the export symbol definition for shared library objects.
  if (const std::string* exportMacro = target->GetExportMacro()) {
    this->AppendDefines(defines, *exportMacro);
  }

  // Add preprocessor definitions for this target and configuration.
  std::vector<BT<std::string>> targetDefines =
    target->GetCompileDefinitions(config, lang);
  this->AppendDefines(defines, targetDefines);

  return defines;
}

std::string cmLocalGenerator::GetTargetFortranFlags(
  cmGeneratorTarget const* /*unused*/, std::string const& /*unused*/)
{
  // Implemented by specific generators that override this.
  return std::string();
}

/**
 * Output the linking rules on a command line.  For executables,
 * targetLibrary should be a NULL pointer.  For libraries, it should point
 * to the name of the library.  This will not link a library against itself.
 */
void cmLocalGenerator::OutputLinkLibraries(
  cmComputeLinkInformation* pcli, cmLinkLineComputer* linkLineComputer,
  std::string& linkLibraries, std::string& frameworkPath,
  std::string& linkPath)
{
  std::vector<BT<std::string>> linkLibrariesList;
  std::vector<BT<std::string>> linkPathList;
  this->OutputLinkLibraries(pcli, linkLineComputer, linkLibrariesList,
                            frameworkPath, linkPathList);
  pcli->AppendValues(linkLibraries, linkLibrariesList);
  pcli->AppendValues(linkPath, linkPathList);
}

void cmLocalGenerator::OutputLinkLibraries(
  cmComputeLinkInformation* pcli, cmLinkLineComputer* linkLineComputer,
  std::vector<BT<std::string>>& linkLibraries, std::string& frameworkPath,
  std::vector<BT<std::string>>& linkPath)
{
  cmComputeLinkInformation& cli = *pcli;

  std::string linkLanguage = cli.GetLinkLanguage();

  std::string libPathFlag;
  if (cmProp value = this->Makefile->GetDefinition(
        "CMAKE_" + cli.GetLinkLanguage() + "_LIBRARY_PATH_FLAG")) {
    libPathFlag = *value;
  } else {
    libPathFlag =
      this->Makefile->GetRequiredDefinition("CMAKE_LIBRARY_PATH_FLAG");
  }

  std::string libPathTerminator;
  if (cmProp value = this->Makefile->GetDefinition(
        "CMAKE_" + cli.GetLinkLanguage() + "_LIBRARY_PATH_TERMINATOR")) {
    libPathTerminator = *value;
  } else {
    libPathTerminator =
      this->Makefile->GetRequiredDefinition("CMAKE_LIBRARY_PATH_TERMINATOR");
  }

  // Add standard libraries for this language.
  std::string stdLibString = this->Makefile->GetSafeDefinition(
    cmStrCat("CMAKE_", cli.GetLinkLanguage(), "_STANDARD_LIBRARIES"));

  // Append the framework search path flags.
  std::string fwSearchFlag = this->Makefile->GetSafeDefinition(
    cmStrCat("CMAKE_", linkLanguage, "_FRAMEWORK_SEARCH_FLAG"));

  frameworkPath = linkLineComputer->ComputeFrameworkPath(cli, fwSearchFlag);
  linkLineComputer->ComputeLinkPath(cli, libPathFlag, libPathTerminator,
                                    linkPath);
  linkLineComputer->ComputeLinkLibraries(cli, stdLibString, linkLibraries);
}

std::string cmLocalGenerator::GetLinkLibsCMP0065(
  std::string const& linkLanguage, cmGeneratorTarget& tgt) const
{
  std::string linkFlags;

  // Flags to link an executable to shared libraries.
  if (tgt.GetType() == cmStateEnums::EXECUTABLE &&
      this->StateSnapshot.GetState()->GetGlobalPropertyAsBool(
        "TARGET_SUPPORTS_SHARED_LIBS")) {
    bool add_shlib_flags = false;
    switch (tgt.GetPolicyStatusCMP0065()) {
      case cmPolicies::WARN:
        if (!tgt.GetPropertyAsBool("ENABLE_EXPORTS") &&
            this->Makefile->PolicyOptionalWarningEnabled(
              "CMAKE_POLICY_WARNING_CMP0065")) {
          std::ostringstream w;
          /* clang-format off */
            w << cmPolicies::GetPolicyWarning(cmPolicies::CMP0065) << "\n"
              "For compatibility with older versions of CMake, "
              "additional flags may be added to export symbols on all "
              "executables regardless of their ENABLE_EXPORTS property.";
          /* clang-format on */
          this->IssueMessage(MessageType::AUTHOR_WARNING, w.str());
        }
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        // OLD behavior is to always add the flags, except on AIX where
        // we compute symbol exports if ENABLE_EXPORTS is on.
        add_shlib_flags =
          !(tgt.Target->IsAIX() && tgt.GetPropertyAsBool("ENABLE_EXPORTS"));
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        this->IssueMessage(
          MessageType::FATAL_ERROR,
          cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0065));
        CM_FALLTHROUGH;
      case cmPolicies::NEW:
        // NEW behavior is to only add the flags if ENABLE_EXPORTS is on,
        // except on AIX where we compute symbol exports.
        add_shlib_flags =
          !tgt.Target->IsAIX() && tgt.GetPropertyAsBool("ENABLE_EXPORTS");
        break;
    }

    if (add_shlib_flags) {
      linkFlags = this->Makefile->GetSafeDefinition(
        cmStrCat("CMAKE_SHARED_LIBRARY_LINK_", linkLanguage, "_FLAGS"));
    }
  }
  return linkFlags;
}

bool cmLocalGenerator::AllAppleArchSysrootsAreTheSame(
  const std::vector<std::string>& archs, const char* sysroot)
{
  if (!sysroot) {
    return false;
  }

  return std::all_of(archs.begin(), archs.end(),
                     [this, &sysroot](std::string const& arch) -> bool {
                       std::string const& archSysroot =
                         this->AppleArchSysroots[arch];
                       return cmIsOff(archSysroot) || archSysroot == sysroot;
                     });
}

void cmLocalGenerator::AddArchitectureFlags(std::string& flags,
                                            cmGeneratorTarget const* target,
                                            const std::string& lang,
                                            const std::string& config,
                                            const std::string& filterArch)
{
  // Only add Apple specific flags on Apple platforms
  if (this->Makefile->IsOn("APPLE") && this->EmitUniversalBinaryFlags) {
    std::vector<std::string> archs;
    target->GetAppleArchs(config, archs);
    if (!archs.empty() &&
        (lang == "C" || lang == "CXX" || lang == "OBJ" || lang == "OBJCXX" ||
         lang == "ASM")) {
      for (std::string const& arch : archs) {
        if (filterArch.empty() || filterArch == arch) {
          flags += " -arch ";
          flags += arch;
        }
      }
    }

    cmProp sysroot = this->Makefile->GetDefinition("CMAKE_OSX_SYSROOT");
    if (sysroot && *sysroot == "/") {
      sysroot = nullptr;
    }
    std::string sysrootFlagVar = "CMAKE_" + lang + "_SYSROOT_FLAG";
    cmProp sysrootFlag = this->Makefile->GetDefinition(sysrootFlagVar);
    if (cmNonempty(sysrootFlag)) {
      if (!this->AppleArchSysroots.empty() &&
          !this->AllAppleArchSysrootsAreTheSame(archs, cmToCStr(sysroot))) {
        for (std::string const& arch : archs) {
          std::string const& archSysroot = this->AppleArchSysroots[arch];
          if (cmIsOff(archSysroot)) {
            continue;
          }
          if (filterArch.empty() || filterArch == arch) {
            flags += " -Xarch_" + arch + " ";
            // Combine sysroot flag and path to work with -Xarch
            std::string arch_sysroot = *sysrootFlag + archSysroot;
            flags += this->ConvertToOutputFormat(arch_sysroot, SHELL);
          }
        }
      } else if (cmNonempty(sysroot)) {
        flags += " ";
        flags += *sysrootFlag;
        flags += " ";
        flags += this->ConvertToOutputFormat(*sysroot, SHELL);
      }
    }

    cmProp deploymentTarget =
      this->Makefile->GetDefinition("CMAKE_OSX_DEPLOYMENT_TARGET");
    std::string deploymentTargetFlagVar =
      "CMAKE_" + lang + "_OSX_DEPLOYMENT_TARGET_FLAG";
    cmProp deploymentTargetFlag =
      this->Makefile->GetDefinition(deploymentTargetFlagVar);
    if (cmNonempty(deploymentTargetFlag) && cmNonempty(deploymentTarget)) {
      flags += " ";
      flags += *deploymentTargetFlag;
      flags += *deploymentTarget;
    }
  }
}

void cmLocalGenerator::AddLanguageFlags(std::string& flags,
                                        cmGeneratorTarget const* target,
                                        const std::string& lang,
                                        const std::string& config)
{
  // Add language-specific flags.
  this->AddConfigVariableFlags(flags, cmStrCat("CMAKE_", lang, "_FLAGS"),
                               config);

  std::string compiler = this->Makefile->GetSafeDefinition(
    cmStrCat("CMAKE_", lang, "_COMPILER_ID"));

  std::string compilerSimulateId = this->Makefile->GetSafeDefinition(
    cmStrCat("CMAKE_", lang, "_SIMULATE_ID"));
  if (lang == "Swift") {
    if (cmProp v = target->GetProperty("Swift_LANGUAGE_VERSION")) {
      if (cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER_EQUAL,
                                        cmToCStr(this->Makefile->GetDefinition(
                                          "CMAKE_Swift_COMPILER_VERSION")),
                                        "4.2")) {
        this->AppendFlags(flags, "-swift-version " + *v);
      }
    }
  } else if (lang == "CUDA") {
    target->AddCUDAArchitectureFlags(flags);
    target->AddCUDAToolkitFlags(flags);
  } else if (lang == "ISPC") {
    target->AddISPCTargetFlags(flags);
  } else if (lang == "RC" &&
             this->Makefile->GetSafeDefinition("CMAKE_RC_COMPILER")
                 .find("llvm-rc") != std::string::npos) {
    compiler = this->Makefile->GetSafeDefinition("CMAKE_C_COMPILER_ID");
    if (!compiler.empty()) {
      compilerSimulateId =
        this->Makefile->GetSafeDefinition("CMAKE_C_SIMULATE_ID");
    } else {
      compiler = this->Makefile->GetSafeDefinition("CMAKE_CXX_COMPILER_ID");
      compilerSimulateId =
        this->Makefile->GetSafeDefinition("CMAKE_CXX_SIMULATE_ID");
    }
  }

  // Add VFS Overlay for Clang compiliers
  if (compiler == "Clang") {
    if (cmProp vfsOverlay =
          this->Makefile->GetDefinition("CMAKE_CLANG_VFS_OVERLAY")) {
      if (compilerSimulateId == "MSVC") {
        this->AppendCompileOptions(
          flags,
          std::vector<std::string>{ "-Xclang", "-ivfsoverlay", "-Xclang",
                                    *vfsOverlay });
      } else {
        this->AppendCompileOptions(
          flags, std::vector<std::string>{ "-ivfsoverlay", *vfsOverlay });
      }
    }
  }
  // Add MSVC runtime library flags.  This is activated by the presence
  // of a default selection whether or not it is overridden by a property.
  cmProp msvcRuntimeLibraryDefault =
    this->Makefile->GetDefinition("CMAKE_MSVC_RUNTIME_LIBRARY_DEFAULT");
  if (cmNonempty(msvcRuntimeLibraryDefault)) {
    cmProp msvcRuntimeLibraryValue =
      target->GetProperty("MSVC_RUNTIME_LIBRARY");
    if (!msvcRuntimeLibraryValue) {
      msvcRuntimeLibraryValue = msvcRuntimeLibraryDefault;
    }
    std::string const msvcRuntimeLibrary = cmGeneratorExpression::Evaluate(
      *msvcRuntimeLibraryValue, this, config, target);
    if (!msvcRuntimeLibrary.empty()) {
      if (cmProp msvcRuntimeLibraryOptions = this->Makefile->GetDefinition(
            "CMAKE_" + lang + "_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_" +
            msvcRuntimeLibrary)) {
        this->AppendCompileOptions(flags, *msvcRuntimeLibraryOptions);
      } else if ((this->Makefile->GetSafeDefinition(
                    "CMAKE_" + lang + "_COMPILER_ID") == "MSVC" ||
                  this->Makefile->GetSafeDefinition(
                    "CMAKE_" + lang + "_SIMULATE_ID") == "MSVC") &&
                 !cmSystemTools::GetErrorOccuredFlag()) {
        // The compiler uses the MSVC ABI so it needs a known runtime library.
        this->IssueMessage(MessageType::FATAL_ERROR,
                           "MSVC_RUNTIME_LIBRARY value '" +
                             msvcRuntimeLibrary + "' not known for this " +
                             lang + " compiler.");
      }
    }
  }
}

void cmLocalGenerator::AddLanguageFlagsForLinking(
  std::string& flags, cmGeneratorTarget const* target, const std::string& lang,
  const std::string& config)
{
  if (this->Makefile->IsOn("CMAKE_" + lang +
                           "_LINK_WITH_STANDARD_COMPILE_OPTION")) {
    // This toolchain requires use of the language standard flag
    // when linking in order to use the matching standard library.
    // FIXME: If CMake gains an abstraction for standard library
    // selection, this will have to be reconciled with it.
    this->AddCompilerRequirementFlag(flags, target, lang, config);
  }

  this->AddLanguageFlags(flags, target, lang, config);

  if (target->IsIPOEnabled(lang, config)) {
    this->AppendFeatureOptions(flags, lang, "IPO");
  }
}

cmGeneratorTarget* cmLocalGenerator::FindGeneratorTargetToUse(
  const std::string& name) const
{
  auto imported = this->ImportedGeneratorTargets.find(name);
  if (imported != this->ImportedGeneratorTargets.end()) {
    return imported->second;
  }

  // find local alias to imported target
  auto aliased = this->AliasTargets.find(name);
  if (aliased != this->AliasTargets.end()) {
    imported = this->ImportedGeneratorTargets.find(aliased->second);
    if (imported != this->ImportedGeneratorTargets.end()) {
      return imported->second;
    }
  }

  if (cmGeneratorTarget* t = this->FindLocalNonAliasGeneratorTarget(name)) {
    return t;
  }

  return this->GetGlobalGenerator()->FindGeneratorTarget(name);
}

bool cmLocalGenerator::GetRealDependency(const std::string& inName,
                                         const std::string& config,
                                         std::string& dep)
{
  // Older CMake code may specify the dependency using the target
  // output file rather than the target name.  Such code would have
  // been written before there was support for target properties that
  // modify the name so stripping down to just the file name should
  // produce the target name in this case.
  std::string name = cmSystemTools::GetFilenameName(inName);

  // If the input name is the empty string, there is no real
  // dependency. Short-circuit the other checks:
  if (name.empty()) {
    return false;
  }
  if (cmSystemTools::GetFilenameLastExtension(name) == ".exe") {
    name = cmSystemTools::GetFilenameWithoutLastExtension(name);
  }

  // Look for a CMake target with the given name.
  if (cmGeneratorTarget* target = this->FindGeneratorTargetToUse(name)) {
    // make sure it is not just a coincidence that the target name
    // found is part of the inName
    if (cmSystemTools::FileIsFullPath(inName)) {
      std::string tLocation;
      if (target->GetType() >= cmStateEnums::EXECUTABLE &&
          target->GetType() <= cmStateEnums::MODULE_LIBRARY) {
        tLocation = target->GetLocation(config);
        tLocation = cmSystemTools::GetFilenamePath(tLocation);
        tLocation = cmSystemTools::CollapseFullPath(tLocation);
      }
      std::string depLocation =
        cmSystemTools::GetFilenamePath(std::string(inName));
      depLocation = cmSystemTools::CollapseFullPath(depLocation);
      if (depLocation != tLocation) {
        // it is a full path to a depend that has the same name
        // as a target but is in a different location so do not use
        // the target as the depend
        dep = inName;
        return true;
      }
    }
    switch (target->GetType()) {
      case cmStateEnums::EXECUTABLE:
      case cmStateEnums::STATIC_LIBRARY:
      case cmStateEnums::SHARED_LIBRARY:
      case cmStateEnums::MODULE_LIBRARY:
      case cmStateEnums::UNKNOWN_LIBRARY:
        dep = target->GetFullPath(config, cmStateEnums::RuntimeBinaryArtifact,
                                  /*realname=*/true);
        return true;
      case cmStateEnums::OBJECT_LIBRARY:
        // An object library has no single file on which to depend.
        // This was listed to get the target-level dependency.
      case cmStateEnums::INTERFACE_LIBRARY:
        // An interface library has no file on which to depend.
        // This was listed to get the target-level dependency.
      case cmStateEnums::UTILITY:
      case cmStateEnums::GLOBAL_TARGET:
        // A utility target has no file on which to depend.  This was listed
        // only to get the target-level dependency.
        return false;
    }
  }

  // The name was not that of a CMake target.  It must name a file.
  if (cmSystemTools::FileIsFullPath(inName)) {
    // This is a full path.  Return it as given.
    dep = inName;
    return true;
  }

  // Check for a source file in this directory that matches the
  // dependency.
  if (cmSourceFile* sf = this->Makefile->GetSource(inName)) {
    dep = sf->ResolveFullPath();
    return true;
  }

  // Treat the name as relative to the source directory in which it
  // was given.
  dep = cmStrCat(this->GetCurrentSourceDirectory(), '/', inName);

  // If the in-source path does not exist, assume it instead lives in the
  // binary directory.
  if (!cmSystemTools::FileExists(dep)) {
    dep = cmStrCat(this->GetCurrentBinaryDirectory(), '/', inName);
  }

  dep = cmSystemTools::CollapseFullPath(dep, this->GetBinaryDirectory());

  return true;
}

void cmLocalGenerator::AddSharedFlags(std::string& flags,
                                      const std::string& lang, bool shared)
{
  std::string flagsVar;

  // Add flags for dealing with shared libraries for this language.
  if (shared) {
    this->AppendFlags(flags,
                      this->Makefile->GetSafeDefinition(
                        cmStrCat("CMAKE_SHARED_LIBRARY_", lang, "_FLAGS")));
  }
}

void cmLocalGenerator::AddCompilerRequirementFlag(
  std::string& flags, cmGeneratorTarget const* target, const std::string& lang,
  const std::string& config)
{
  cmStandardLevelResolver standardResolver(this->Makefile);

  std::string const& optionFlagDef =
    standardResolver.GetCompileOptionDef(target, lang, config);
  if (!optionFlagDef.empty()) {
    cmProp opt = target->Target->GetMakefile()->GetDefinition(optionFlagDef);
    if (opt) {
      std::vector<std::string> optVec = cmExpandedList(*opt);
      for (std::string const& i : optVec) {
        this->AppendFlagEscape(flags, i);
      }
    }
  }
}

static void AddVisibilityCompileOption(std::string& flags,
                                       cmGeneratorTarget const* target,
                                       cmLocalGenerator* lg,
                                       const std::string& lang,
                                       std::string* warnCMP0063)
{
  std::string compileOption = "CMAKE_" + lang + "_COMPILE_OPTIONS_VISIBILITY";
  cmProp opt = lg->GetMakefile()->GetDefinition(compileOption);
  if (!opt) {
    return;
  }
  std::string flagDefine = lang + "_VISIBILITY_PRESET";

  cmProp prop = target->GetProperty(flagDefine);
  if (!prop) {
    return;
  }
  if (warnCMP0063) {
    *warnCMP0063 += "  " + flagDefine + "\n";
    return;
  }
  if ((*prop != "hidden") && (*prop != "default") && (*prop != "protected") &&
      (*prop != "internal")) {
    std::ostringstream e;
    e << "Target " << target->GetName() << " uses unsupported value \""
      << *prop << "\" for " << flagDefine << "."
      << " The supported values are: default, hidden, protected, and "
         "internal.";
    cmSystemTools::Error(e.str());
    return;
  }
  std::string option = *opt + *prop;
  lg->AppendFlags(flags, option);
}

static void AddInlineVisibilityCompileOption(std::string& flags,
                                             cmGeneratorTarget const* target,
                                             cmLocalGenerator* lg,
                                             std::string* warnCMP0063,
                                             const std::string& lang)
{
  std::string compileOption =
    cmStrCat("CMAKE_", lang, "_COMPILE_OPTIONS_VISIBILITY_INLINES_HIDDEN");
  cmProp opt = lg->GetMakefile()->GetDefinition(compileOption);
  if (!opt) {
    return;
  }

  bool prop = target->GetPropertyAsBool("VISIBILITY_INLINES_HIDDEN");
  if (!prop) {
    return;
  }
  if (warnCMP0063) {
    *warnCMP0063 += "  VISIBILITY_INLINES_HIDDEN\n";
    return;
  }
  lg->AppendFlags(flags, *opt);
}

void cmLocalGenerator::AddVisibilityPresetFlags(
  std::string& flags, cmGeneratorTarget const* target, const std::string& lang)
{
  if (lang.empty()) {
    return;
  }

  std::string warnCMP0063;
  std::string* pWarnCMP0063 = nullptr;
  if (target->GetType() != cmStateEnums::SHARED_LIBRARY &&
      target->GetType() != cmStateEnums::MODULE_LIBRARY &&
      !target->IsExecutableWithExports()) {
    switch (target->GetPolicyStatusCMP0063()) {
      case cmPolicies::OLD:
        return;
      case cmPolicies::WARN:
        pWarnCMP0063 = &warnCMP0063;
        break;
      default:
        break;
    }
  }

  AddVisibilityCompileOption(flags, target, this, lang, pWarnCMP0063);

  if (lang == "CXX" || lang == "OBJCXX") {
    AddInlineVisibilityCompileOption(flags, target, this, pWarnCMP0063, lang);
  }

  if (!warnCMP0063.empty() && this->WarnCMP0063.insert(target).second) {
    std::ostringstream w;
    /* clang-format off */
    w <<
      cmPolicies::GetPolicyWarning(cmPolicies::CMP0063) << "\n"
      "Target \"" << target->GetName() << "\" of "
      "type \"" << cmState::GetTargetTypeName(target->GetType()) << "\" "
      "has the following visibility properties set for " << lang << ":\n" <<
      warnCMP0063 <<
      "For compatibility CMake is not honoring them for this target.";
    /* clang-format on */
    target->GetLocalGenerator()->GetCMakeInstance()->IssueMessage(
      MessageType::AUTHOR_WARNING, w.str(), target->GetBacktrace());
  }
}

void cmLocalGenerator::AddCMP0018Flags(std::string& flags,
                                       cmGeneratorTarget const* target,
                                       std::string const& lang,
                                       const std::string& config)
{
  int targetType = target->GetType();

  bool shared = ((targetType == cmStateEnums::SHARED_LIBRARY) ||
                 (targetType == cmStateEnums::MODULE_LIBRARY));

  if (this->GetShouldUseOldFlags(shared, lang)) {
    this->AddSharedFlags(flags, lang, shared);
  } else {
    if (target->GetType() == cmStateEnums::OBJECT_LIBRARY) {
      if (target->GetPropertyAsBool("POSITION_INDEPENDENT_CODE")) {
        this->AddPositionIndependentFlags(flags, lang, targetType);
      }
      return;
    }

    if (target->GetLinkInterfaceDependentBoolProperty(
          "POSITION_INDEPENDENT_CODE", config)) {
      this->AddPositionIndependentFlags(flags, lang, targetType);
    }
    if (shared) {
      this->AppendFeatureOptions(flags, lang, "DLL");
    }
  }
}

bool cmLocalGenerator::GetShouldUseOldFlags(bool shared,
                                            const std::string& lang) const
{
  std::string originalFlags =
    this->GlobalGenerator->GetSharedLibFlagsForLanguage(lang);
  if (shared) {
    std::string flagsVar = cmStrCat("CMAKE_SHARED_LIBRARY_", lang, "_FLAGS");
    std::string const& flags = this->Makefile->GetSafeDefinition(flagsVar);

    if (flags != originalFlags) {
      switch (this->GetPolicyStatus(cmPolicies::CMP0018)) {
        case cmPolicies::WARN: {
          std::ostringstream e;
          e << "Variable " << flagsVar
            << " has been modified. CMake "
               "will ignore the POSITION_INDEPENDENT_CODE target property "
               "for "
               "shared libraries and will use the "
            << flagsVar
            << " variable "
               "instead.  This may cause errors if the original content of "
            << flagsVar << " was removed.\n"
            << cmPolicies::GetPolicyWarning(cmPolicies::CMP0018);

          this->IssueMessage(MessageType::AUTHOR_WARNING, e.str());
          CM_FALLTHROUGH;
        }
        case cmPolicies::OLD:
          return true;
        case cmPolicies::REQUIRED_IF_USED:
        case cmPolicies::REQUIRED_ALWAYS:
        case cmPolicies::NEW:
          return false;
      }
    }
  }
  return false;
}

void cmLocalGenerator::AddPositionIndependentFlags(std::string& flags,
                                                   std::string const& lang,
                                                   int targetType)
{
  std::string picFlags;

  if (targetType == cmStateEnums::EXECUTABLE) {
    picFlags = this->Makefile->GetSafeDefinition(
      cmStrCat("CMAKE_", lang, "_COMPILE_OPTIONS_PIE"));
  }
  if (picFlags.empty()) {
    picFlags = this->Makefile->GetSafeDefinition(
      cmStrCat("CMAKE_", lang, "_COMPILE_OPTIONS_PIC"));
  }
  if (!picFlags.empty()) {
    std::vector<std::string> options = cmExpandedList(picFlags);
    for (std::string const& o : options) {
      this->AppendFlagEscape(flags, o);
    }
  }
}

void cmLocalGenerator::AddConfigVariableFlags(std::string& flags,
                                              const std::string& var,
                                              const std::string& config)
{
  // Add the flags from the variable itself.
  this->AppendFlags(flags, this->Makefile->GetSafeDefinition(var));
  // Add the flags from the build-type specific variable.
  if (!config.empty()) {
    const std::string flagsVar =
      cmStrCat(var, '_', cmSystemTools::UpperCase(config));
    this->AppendFlags(flags, this->Makefile->GetSafeDefinition(flagsVar));
  }
}

void cmLocalGenerator::AppendFlags(std::string& flags,
                                   const std::string& newFlags) const
{
  bool allSpaces = std::all_of(newFlags.begin(), newFlags.end(), cmIsSpace);

  if (!newFlags.empty() && !allSpaces) {
    if (!flags.empty()) {
      flags += " ";
    }
    flags += newFlags;
  }
}

void cmLocalGenerator::AppendFlags(
  std::string& flags, const std::vector<BT<std::string>>& newFlags) const
{
  for (BT<std::string> const& flag : newFlags) {
    this->AppendFlags(flags, flag.Value);
  }
}

void cmLocalGenerator::AppendFlagEscape(std::string& flags,
                                        const std::string& rawFlag) const
{
  this->AppendFlags(
    flags,
    this->EscapeForShell(rawFlag, false, false, false, this->IsNinjaMulti()));
}

void cmLocalGenerator::AddISPCDependencies(cmGeneratorTarget* target)
{
  std::vector<std::string> enabledLanguages =
    this->GetState()->GetEnabledLanguages();
  if (std::find(enabledLanguages.begin(), enabledLanguages.end(), "ISPC") ==
      enabledLanguages.end()) {
    return;
  }

  cmProp ispcHeaderSuffixProp = target->GetProperty("ISPC_HEADER_SUFFIX");
  assert(ispcHeaderSuffixProp != nullptr);

  std::vector<std::string> ispcArchSuffixes =
    detail::ComputeISPCObjectSuffixes(target);
  const bool extra_objects = (ispcArchSuffixes.size() > 1);

  std::vector<std::string> configsList =
    this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
  for (std::string const& config : configsList) {

    std::string rootObjectDir = target->GetObjectDirectory(config);
    std::string headerDir = rootObjectDir;
    if (cmProp prop = target->GetProperty("ISPC_HEADER_DIRECTORY")) {
      headerDir = cmSystemTools::CollapseFullPath(
        cmStrCat(this->GetBinaryDirectory(), '/', *prop));
    }

    std::vector<cmSourceFile*> sources;
    target->GetSourceFiles(sources, config);

    // build up the list of ispc headers and extra objects that this target is
    // generating
    for (cmSourceFile const* sf : sources) {
      // Generate this object file's rule file.
      const std::string& lang = sf->GetLanguage();
      if (lang == "ISPC") {
        std::string const& objectName = target->GetObjectName(sf);

        // Drop both ".obj" and the source file extension
        std::string ispcSource =
          cmSystemTools::GetFilenameWithoutLastExtension(objectName);
        ispcSource =
          cmSystemTools::GetFilenameWithoutLastExtension(ispcSource);

        auto headerPath =
          cmStrCat(headerDir, '/', ispcSource, *ispcHeaderSuffixProp);
        target->AddISPCGeneratedHeader(headerPath, config);
        if (extra_objects) {
          std::vector<std::string> objs = detail::ComputeISPCExtraObjects(
            objectName, rootObjectDir, ispcArchSuffixes);
          target->AddISPCGeneratedObject(std::move(objs), config);
        }
      }
    }
  }
}

void cmLocalGenerator::AddPchDependencies(cmGeneratorTarget* target)
{
  std::vector<std::string> configsList =
    this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

  for (std::string const& config : configsList) {
    // FIXME: Refactor collection of sources to not evaluate object
    // libraries.
    std::vector<cmSourceFile*> sources;
    target->GetSourceFiles(sources, config);

    const std::string configUpper = cmSystemTools::UpperCase(config);
    static const std::array<std::string, 4> langs = { { "C", "CXX", "OBJC",
                                                        "OBJCXX" } };

    for (const std::string& lang : langs) {
      auto langSources = std::count_if(
        sources.begin(), sources.end(), [lang](cmSourceFile* sf) {
          return lang == sf->GetLanguage() &&
            !sf->GetProperty("SKIP_PRECOMPILE_HEADERS");
        });
      if (langSources == 0) {
        continue;
      }

      std::vector<std::string> architectures;
      if (!this->GetGlobalGenerator()->IsXcode()) {
        target->GetAppleArchs(config, architectures);
      }
      if (architectures.empty()) {
        architectures.emplace_back();
      } else {
        std::string useMultiArchPch;
        for (const std::string& arch : architectures) {
          const std::string pchHeader =
            target->GetPchHeader(config, lang, arch);
          if (!pchHeader.empty()) {
            useMultiArchPch = cmStrCat(useMultiArchPch, ";-Xarch_", arch,
                                       ";-include", pchHeader);
          }
        }

        if (!useMultiArchPch.empty()) {

          target->Target->AppendProperty(
            cmStrCat(lang, "_COMPILE_OPTIONS_USE_PCH"),
            cmStrCat("$<$<CONFIG:", config, ">:", useMultiArchPch, ">"));
        }
      }

      for (const std::string& arch : architectures) {
        const std::string pchSource = target->GetPchSource(config, lang, arch);
        const std::string pchHeader = target->GetPchHeader(config, lang, arch);

        if (pchSource.empty() || pchHeader.empty()) {
          continue;
        }

        const std::string pchExtension =
          this->Makefile->GetSafeDefinition("CMAKE_PCH_EXTENSION");

        if (pchExtension.empty()) {
          continue;
        }

        cmProp ReuseFrom =
          target->GetProperty("PRECOMPILE_HEADERS_REUSE_FROM");

        auto* pch_sf = this->Makefile->GetOrCreateSource(
          pchSource, false, cmSourceFileLocationKind::Known);

        if (!this->GetGlobalGenerator()->IsXcode()) {
          if (!ReuseFrom) {
            target->AddSource(pchSource, true);
          }

          const std::string pchFile = target->GetPchFile(config, lang, arch);

          // Exclude the pch files from linking
          if (this->Makefile->IsOn("CMAKE_LINK_PCH")) {
            if (!ReuseFrom) {
              pch_sf->AppendProperty(
                "OBJECT_OUTPUTS",
                cmStrCat("$<$<CONFIG:", config, ">:", pchFile, ">"));
            } else {
              auto* reuseTarget =
                this->GlobalGenerator->FindGeneratorTarget(*ReuseFrom);

              if (this->Makefile->IsOn("CMAKE_PCH_COPY_COMPILE_PDB")) {

                const std::string compilerId =
                  this->Makefile->GetSafeDefinition(
                    cmStrCat("CMAKE_", lang, "_COMPILER_ID"));

                const std::string compilerVersion =
                  this->Makefile->GetSafeDefinition(
                    cmStrCat("CMAKE_", lang, "_COMPILER_VERSION"));

                const std::string langFlags =
                  this->Makefile->GetSafeDefinition(
                    cmStrCat("CMAKE_", lang, "_FLAGS_", configUpper));

                bool editAndContinueDebugInfo =
                  langFlags.find("/ZI") != std::string::npos ||
                  langFlags.find("-ZI") != std::string::npos;

                bool enableDebuggingInformation =
                  langFlags.find("/Zi") != std::string::npos ||
                  langFlags.find("-Zi") != std::string::npos;

                // MSVC 2008 is producing both .pdb and .idb files with /Zi.
                bool msvc2008OrLess =
                  cmSystemTools::VersionCompare(
                    cmSystemTools::OP_LESS, compilerVersion.c_str(), "16.0") &&
                  compilerId == "MSVC";
                // but not when used via toolset -Tv90
                if (this->Makefile->GetSafeDefinition(
                      "CMAKE_VS_PLATFORM_TOOLSET") == "v90") {
                  msvc2008OrLess = false;
                }

                if (editAndContinueDebugInfo || msvc2008OrLess) {
                  this->CopyPchCompilePdb(config, target, *ReuseFrom,
                                          reuseTarget, { ".pdb", ".idb" });
                } else if (enableDebuggingInformation) {
                  this->CopyPchCompilePdb(config, target, *ReuseFrom,
                                          reuseTarget, { ".pdb" });
                }
              }

              // Link to the pch object file
              std::string pchSourceObj =
                reuseTarget->GetPchFileObject(config, lang, arch);

              if (target->GetType() != cmStateEnums::OBJECT_LIBRARY) {
                std::string linkerProperty = "LINK_FLAGS_";
                if (target->GetType() == cmStateEnums::STATIC_LIBRARY) {
                  linkerProperty = "STATIC_LIBRARY_FLAGS_";
                }
                target->Target->AppendProperty(
                  cmStrCat(linkerProperty, configUpper),
                  cmStrCat(" ",
                           this->ConvertToOutputFormat(pchSourceObj, SHELL)),
                  true);
              } else {
                target->Target->AppendProperty(
                  "INTERFACE_LINK_LIBRARIES",
                  cmStrCat("$<$<CONFIG:", config,
                           ">:$<LINK_ONLY:", pchSourceObj, ">>"));
              }
            }
          } else {
            pch_sf->SetProperty("PCH_EXTENSION", pchExtension.c_str());
          }

          // Add pchHeader to source files, which will
          // be grouped as "Precompile Header File"
          auto* pchHeader_sf = this->Makefile->GetOrCreateSource(
            pchHeader, false, cmSourceFileLocationKind::Known);
          std::string err;
          pchHeader_sf->ResolveFullPath(&err);
          if (!err.empty()) {
            std::ostringstream msg;
            msg << "Unable to resolve full path of PCH-header '" << pchHeader
                << "' assigned to target " << target->GetName()
                << ", although its path is supposed to be known!";
            this->IssueMessage(MessageType::FATAL_ERROR, msg.str());
          }
          target->AddSource(pchHeader);
        }
      }
    }
  }
}

void cmLocalGenerator::CopyPchCompilePdb(
  const std::string& config, cmGeneratorTarget* target,
  const std::string& ReuseFrom, cmGeneratorTarget* reuseTarget,
  const std::vector<std::string>& extensions)
{
  const std::string pdb_prefix =
    this->GetGlobalGenerator()->IsMultiConfig() ? cmStrCat(config, "/") : "";

  const std::string target_compile_pdb_dir =
    cmStrCat(target->GetLocalGenerator()->GetCurrentBinaryDirectory(), "/",
             target->GetName(), ".dir/");

  const std::string copy_script =
    cmStrCat(target_compile_pdb_dir, "copy_idb_pdb.cmake");
  cmGeneratedFileStream file(copy_script);

  file << "# CMake generated file\n";

  file << "# The compiler generated pdb file needs to be written to disk\n"
       << "# by mspdbsrv. The foreach retry loop is needed to make sure\n"
       << "# the pdb file is ready to be copied.\n\n";

  for (auto const& extension : extensions) {
    const std::string from_file =
      cmStrCat(reuseTarget->GetLocalGenerator()->GetCurrentBinaryDirectory(),
               "/", ReuseFrom, ".dir/${PDB_PREFIX}", ReuseFrom, extension);

    const std::string to_dir =
      cmStrCat(target->GetLocalGenerator()->GetCurrentBinaryDirectory(), "/",
               target->GetName(), ".dir/${PDB_PREFIX}");

    const std::string to_file = cmStrCat(to_dir, ReuseFrom, extension);

    std::string dest_file = to_file;

    std::string const& prefix = target->GetSafeProperty("PREFIX");
    if (!prefix.empty()) {
      dest_file = cmStrCat(to_dir, prefix, ReuseFrom, extension);
    }

    file << "foreach(retry RANGE 1 30)\n";
    file << "  if (EXISTS \"" << from_file << "\" AND \"" << from_file
         << "  \" IS_NEWER_THAN \"" << dest_file << "\")\n";
    file << "    execute_process(COMMAND ${CMAKE_COMMAND} -E copy";
    file << " \"" << from_file << "\""
         << " \"" << to_dir << "\" RESULT_VARIABLE result "
         << " ERROR_QUIET)\n";
    file << "    if (NOT result EQUAL 0)\n"
         << "      execute_process(COMMAND ${CMAKE_COMMAND}"
         << " -E sleep 1)\n"
         << "    else()\n";
    if (!prefix.empty()) {
      file << "  file(REMOVE \"" << dest_file << "\")\n";
      file << "  file(RENAME \"" << to_file << "\" \"" << dest_file << "\")\n";
    }
    file << "      break()\n"
         << "    endif()\n";
    file << "  elseif(NOT EXISTS \"" << from_file << "\")\n"
         << "    execute_process(COMMAND ${CMAKE_COMMAND}"
         << " -E sleep 1)\n"
         << "  endif()\n";
    file << "endforeach()\n";
  }

  bool stdPipesUTF8 = true;

  auto configGenex = [&](cm::string_view expr) -> std::string {
    if (this->GetGlobalGenerator()->IsVisualStudio()) {
      return cmStrCat("$<$<CONFIG:", config, ">:", expr, ">");
    }
    return std::string(expr);
  };

  cmCustomCommandLines commandLines = cmMakeSingleCommandLine(
    { configGenex(cmSystemTools::GetCMakeCommand()),
      configGenex(cmStrCat("-DPDB_PREFIX=", pdb_prefix)), configGenex("-P"),
      configGenex(copy_script) });

  const std::string no_main_dependency;
  const std::vector<std::string> no_deps;
  const char* no_message = "";
  const char* no_current_dir = nullptr;
  const cmPolicies::PolicyStatus cmp0116_new = cmPolicies::NEW;
  std::vector<std::string> no_byproducts;

  std::vector<std::string> outputs;
  outputs.push_back(
    cmStrCat(target_compile_pdb_dir, pdb_prefix, ReuseFrom, ".pdb"));

  if (this->GetGlobalGenerator()->IsVisualStudio()) {
    this->AddCustomCommandToTarget(
      target->GetName(), outputs, no_deps, commandLines,
      cmCustomCommandType::PRE_BUILD, no_message, no_current_dir, cmp0116_new,
      true, false, "", "", false, cmObjectLibraryCommands::Accept,
      stdPipesUTF8);
  } else {
    cmImplicitDependsList no_implicit_depends;
    cmSourceFile* copy_rule = this->AddCustomCommandToOutput(
      outputs, no_byproducts, no_deps, no_main_dependency, no_implicit_depends,
      commandLines, no_message, no_current_dir, cmp0116_new, false, true,
      false, false, "", "", stdPipesUTF8);

    if (copy_rule) {
      target->AddSource(copy_rule->ResolveFullPath());
    }
  }

  target->Target->SetProperty("COMPILE_PDB_OUTPUT_DIRECTORY",
                              target_compile_pdb_dir);
}

namespace {

inline void RegisterUnitySources(cmGeneratorTarget* target, cmSourceFile* sf,
                                 std::string const& filename)
{
  target->AddSourceFileToUnityBatch(sf->ResolveFullPath());
  sf->SetProperty("UNITY_SOURCE_FILE", filename.c_str());
}
}

void cmLocalGenerator::IncludeFileInUnitySources(
  cmGeneratedFileStream& unity_file, std::string const& sf_full_path,
  cmProp beforeInclude, cmProp afterInclude, cmProp uniqueIdName) const
{
  if (uniqueIdName && !uniqueIdName->empty()) {
    std::string pathToHash;
    auto PathEqOrSubDir = [](std::string const& a, std::string const& b) {
      return (cmSystemTools::ComparePath(a, b) ||
              cmSystemTools::IsSubDirectory(a, b));
    };
    const auto path = cmSystemTools::GetFilenamePath(sf_full_path);
    if (PathEqOrSubDir(path, this->GetBinaryDirectory())) {
      pathToHash = "BLD_" +
        cmSystemTools::RelativePath(this->GetBinaryDirectory(), sf_full_path);
    } else if (PathEqOrSubDir(path, this->GetSourceDirectory())) {
      pathToHash = "SRC_" +
        cmSystemTools::RelativePath(this->GetSourceDirectory(), sf_full_path);
    } else {
      pathToHash = "ABS_" + sf_full_path;
    }
    unity_file << "/* " << pathToHash << " */\n"
               << "#undef " << *uniqueIdName << "\n"
               << "#define " << *uniqueIdName << " unity_"
               << cmSystemTools::ComputeStringMD5(pathToHash) << "\n";
  }

  if (beforeInclude) {
    unity_file << *beforeInclude << "\n";
  }

  unity_file << "#include \"" << sf_full_path << "\"\n";

  if (afterInclude) {
    unity_file << *afterInclude << "\n";
  }
  unity_file << "\n";
}

std::vector<std::string> cmLocalGenerator::AddUnityFilesModeAuto(
  cmGeneratorTarget* target, std::string const& lang,
  std::vector<cmSourceFile*> const& filtered_sources, cmProp beforeInclude,
  cmProp afterInclude, std::string const& filename_base, size_t batchSize)
{
  if (batchSize == 0) {
    batchSize = filtered_sources.size();
  }

  cmProp uniqueIdName = target->GetProperty("UNITY_BUILD_UNIQUE_ID");

  std::vector<std::string> unity_files;
  for (size_t itemsLeft = filtered_sources.size(), chunk, batch = 0;
       itemsLeft > 0; itemsLeft -= chunk, ++batch) {

    chunk = std::min(itemsLeft, batchSize);

    std::string filename = cmStrCat(filename_base, "unity_", batch,
                                    (lang == "C") ? "_c.c" : "_cxx.cxx");

    const std::string filename_tmp = cmStrCat(filename, ".tmp");
    {
      size_t begin = batch * batchSize;
      size_t end = begin + chunk;

      cmGeneratedFileStream file(
        filename_tmp, false,
        target->GetGlobalGenerator()->GetMakefileEncoding());
      file << "/* generated by CMake */\n\n";

      for (; begin != end; ++begin) {
        cmSourceFile* sf = filtered_sources[begin];
        RegisterUnitySources(target, sf, filename);
        IncludeFileInUnitySources(file, sf->ResolveFullPath(), beforeInclude,
                                  afterInclude, uniqueIdName);
      }
    }
    cmSystemTools::MoveFileIfDifferent(filename_tmp, filename);
    unity_files.emplace_back(std::move(filename));
  }
  return unity_files;
}

std::vector<std::string> cmLocalGenerator::AddUnityFilesModeGroup(
  cmGeneratorTarget* target, std::string const& lang,
  std::vector<cmSourceFile*> const& filtered_sources, cmProp beforeInclude,
  cmProp afterInclude, std::string const& filename_base)
{
  std::vector<std::string> unity_files;

  // sources organized by group name. Drop any source
  // without a group
  std::unordered_map<std::string, std::vector<cmSourceFile*>> explicit_mapping;
  for (cmSourceFile* sf : filtered_sources) {
    if (cmProp value = sf->GetProperty("UNITY_GROUP")) {
      auto i = explicit_mapping.find(*value);
      if (i == explicit_mapping.end()) {
        std::vector<cmSourceFile*> sources{ sf };
        explicit_mapping.emplace(*value, sources);
      } else {
        i->second.emplace_back(sf);
      }
    }
  }

  cmProp uniqueIdName = target->GetProperty("UNITY_BUILD_UNIQUE_ID");

  for (auto const& item : explicit_mapping) {
    auto const& name = item.first;
    std::string filename = cmStrCat(filename_base, "unity_", name,
                                    (lang == "C") ? "_c.c" : "_cxx.cxx");

    const std::string filename_tmp = cmStrCat(filename, ".tmp");
    {
      cmGeneratedFileStream file(
        filename_tmp, false,
        target->GetGlobalGenerator()->GetMakefileEncoding());
      file << "/* generated by CMake */\n\n";

      for (cmSourceFile* sf : item.second) {
        RegisterUnitySources(target, sf, filename);
        IncludeFileInUnitySources(file, sf->ResolveFullPath(), beforeInclude,
                                  afterInclude, uniqueIdName);
      }
    }
    cmSystemTools::MoveFileIfDifferent(filename_tmp, filename);
    unity_files.emplace_back(std::move(filename));
  }

  return unity_files;
}

void cmLocalGenerator::AddUnityBuild(cmGeneratorTarget* target)
{
  if (!target->GetPropertyAsBool("UNITY_BUILD")) {
    return;
  }

  // FIXME: Handle all configurations in multi-config generators.
  std::string config;
  if (!this->GetGlobalGenerator()->IsMultiConfig()) {
    config = this->Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
  }

  std::string filename_base =
    cmStrCat(this->GetCurrentBinaryDirectory(), "/CMakeFiles/",
             target->GetName(), ".dir/Unity/");

  // FIXME: Refactor collection of sources to not evaluate object libraries.
  std::vector<cmSourceFile*> sources;
  target->GetSourceFiles(sources, config);

  cmProp batchSizeString = target->GetProperty("UNITY_BUILD_BATCH_SIZE");
  const size_t unityBatchSize = batchSizeString
    ? static_cast<size_t>(std::atoi(batchSizeString->c_str()))
    : 0;

  cmProp beforeInclude =
    target->GetProperty("UNITY_BUILD_CODE_BEFORE_INCLUDE");
  cmProp afterInclude = target->GetProperty("UNITY_BUILD_CODE_AFTER_INCLUDE");
  cmProp unityMode = target->GetProperty("UNITY_BUILD_MODE");

  for (std::string lang : { "C", "CXX" }) {
    std::vector<cmSourceFile*> filtered_sources;
    std::copy_if(sources.begin(), sources.end(),
                 std::back_inserter(filtered_sources), [&](cmSourceFile* sf) {
                   return sf->GetLanguage() == lang &&
                     !sf->GetPropertyAsBool("SKIP_UNITY_BUILD_INCLUSION") &&
                     !sf->GetPropertyAsBool("HEADER_FILE_ONLY") &&
                     !sf->GetProperty("COMPILE_OPTIONS") &&
                     !sf->GetProperty("COMPILE_DEFINITIONS") &&
                     !sf->GetProperty("COMPILE_FLAGS") &&
                     !sf->GetProperty("INCLUDE_DIRECTORIES");
                 });

    std::vector<std::string> unity_files;
    if (!unityMode || *unityMode == "BATCH") {
      unity_files =
        AddUnityFilesModeAuto(target, lang, filtered_sources, beforeInclude,
                              afterInclude, filename_base, unityBatchSize);
    } else if (unityMode && *unityMode == "GROUP") {
      unity_files =
        AddUnityFilesModeGroup(target, lang, filtered_sources, beforeInclude,
                               afterInclude, filename_base);
    } else {
      // unity mode is set to an unsupported value
      std::string e("Invalid UNITY_BUILD_MODE value of " + *unityMode +
                    " assigned to target " + target->GetName() +
                    ". Acceptable values are BATCH and GROUP.");
      this->IssueMessage(MessageType::FATAL_ERROR, e);
    }

    for (auto const& file : unity_files) {
      auto* unity = this->GetMakefile()->GetOrCreateSource(file);
      target->AddSource(file, true);
      unity->SetProperty("SKIP_UNITY_BUILD_INCLUSION", "ON");
      unity->SetProperty("UNITY_SOURCE_FILE", file.c_str());
    }
  }
}

void cmLocalGenerator::AppendIPOLinkerFlags(std::string& flags,
                                            cmGeneratorTarget* target,
                                            const std::string& config,
                                            const std::string& lang)
{
  if (!target->IsIPOEnabled(lang, config)) {
    return;
  }

  switch (target->GetType()) {
    case cmStateEnums::EXECUTABLE:
    case cmStateEnums::SHARED_LIBRARY:
    case cmStateEnums::MODULE_LIBRARY:
      break;
    default:
      return;
  }

  const std::string name = "CMAKE_" + lang + "_LINK_OPTIONS_IPO";
  cmProp rawFlagsList = this->Makefile->GetDefinition(name);
  if (rawFlagsList == nullptr) {
    return;
  }

  std::vector<std::string> flagsList = cmExpandedList(*rawFlagsList);
  for (std::string const& o : flagsList) {
    this->AppendFlagEscape(flags, o);
  }
}

void cmLocalGenerator::AppendPositionIndependentLinkerFlags(
  std::string& flags, cmGeneratorTarget* target, const std::string& config,
  const std::string& lang)
{
  // For now, only EXECUTABLE is concerned
  if (target->GetType() != cmStateEnums::EXECUTABLE) {
    return;
  }

  const char* PICValue = target->GetLinkPIEProperty(config);
  if (PICValue == nullptr) {
    // POSITION_INDEPENDENT_CODE is not set
    return;
  }

  const std::string mode = cmIsOn(PICValue) ? "PIE" : "NO_PIE";

  std::string supported = "CMAKE_" + lang + "_LINK_" + mode + "_SUPPORTED";
  if (cmIsOff(this->Makefile->GetDefinition(supported))) {
    return;
  }

  std::string name = "CMAKE_" + lang + "_LINK_OPTIONS_" + mode;

  auto pieFlags = this->Makefile->GetSafeDefinition(name);
  if (pieFlags.empty()) {
    return;
  }

  std::vector<std::string> flagsList = cmExpandedList(pieFlags);
  for (const auto& flag : flagsList) {
    this->AppendFlagEscape(flags, flag);
  }
}

void cmLocalGenerator::AppendCompileOptions(std::string& options,
                                            std::string const& options_list,
                                            const char* regex) const
{
  // Short-circuit if there are no options.
  if (options_list.empty()) {
    return;
  }

  // Expand the list of options.
  std::vector<std::string> options_vec = cmExpandedList(options_list);
  this->AppendCompileOptions(options, options_vec, regex);
}

void cmLocalGenerator::AppendCompileOptions(
  std::string& options, const std::vector<std::string>& options_vec,
  const char* regex) const
{
  if (regex != nullptr) {
    // Filter flags upon specified reges.
    cmsys::RegularExpression r(regex);

    for (std::string const& opt : options_vec) {
      if (r.find(opt)) {
        this->AppendFlagEscape(options, opt);
      }
    }
  } else {
    for (std::string const& opt : options_vec) {
      this->AppendFlagEscape(options, opt);
    }
  }
}

void cmLocalGenerator::AppendCompileOptions(
  std::vector<BT<std::string>>& options,
  const std::vector<BT<std::string>>& options_vec, const char* regex) const
{
  if (regex != nullptr) {
    // Filter flags upon specified regular expressions.
    cmsys::RegularExpression r(regex);

    for (BT<std::string> const& opt : options_vec) {
      if (r.find(opt.Value)) {
        std::string flag;
        this->AppendFlagEscape(flag, opt.Value);
        options.emplace_back(std::move(flag), opt.Backtrace);
      }
    }
  } else {
    for (BT<std::string> const& opt : options_vec) {
      std::string flag;
      this->AppendFlagEscape(flag, opt.Value);
      options.emplace_back(std::move(flag), opt.Backtrace);
    }
  }
}

void cmLocalGenerator::AppendIncludeDirectories(
  std::vector<std::string>& includes, const std::string& includes_list,
  const cmSourceFile& sourceFile) const
{
  // Short-circuit if there are no includes.
  if (includes_list.empty()) {
    return;
  }

  // Expand the list of includes.
  std::vector<std::string> includes_vec = cmExpandedList(includes_list);
  this->AppendIncludeDirectories(includes, includes_vec, sourceFile);
}

void cmLocalGenerator::AppendIncludeDirectories(
  std::vector<std::string>& includes,
  const std::vector<std::string>& includes_vec,
  const cmSourceFile& sourceFile) const
{
  std::unordered_set<std::string> uniqueIncludes;

  for (const std::string& include : includes_vec) {
    if (!cmSystemTools::FileIsFullPath(include)) {
      std::ostringstream e;
      e << "Found relative path while evaluating include directories of "
           "\""
        << sourceFile.GetLocation().GetName() << "\":\n  \"" << include
        << "\"\n";

      this->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return;
    }

    std::string inc = include;

    if (!cmIsOff(inc)) {
      cmSystemTools::ConvertToUnixSlashes(inc);
    }

    if (uniqueIncludes.insert(inc).second) {
      includes.push_back(std::move(inc));
    }
  }
}

void cmLocalGenerator::AppendDefines(std::set<std::string>& defines,
                                     std::string const& defines_list) const
{
  std::set<BT<std::string>> tmp;
  this->AppendDefines(tmp, ExpandListWithBacktrace(defines_list));
  for (BT<std::string> const& i : tmp) {
    defines.emplace(i.Value);
  }
}

void cmLocalGenerator::AppendDefines(std::set<BT<std::string>>& defines,
                                     std::string const& defines_list) const
{
  // Short-circuit if there are no definitions.
  if (defines_list.empty()) {
    return;
  }

  // Expand the list of definitions.
  this->AppendDefines(defines, ExpandListWithBacktrace(defines_list));
}

void cmLocalGenerator::AppendDefines(
  std::set<BT<std::string>>& defines,
  const std::vector<BT<std::string>>& defines_vec) const
{
  for (BT<std::string> const& d : defines_vec) {
    // Skip unsupported definitions.
    if (!this->CheckDefinition(d.Value)) {
      continue;
    }
    defines.insert(d);
  }
}

void cmLocalGenerator::JoinDefines(const std::set<std::string>& defines,
                                   std::string& definesString,
                                   const std::string& lang)
{
  // Lookup the define flag for the current language.
  std::string dflag = "-D";
  if (!lang.empty()) {
    cmProp df =
      this->Makefile->GetDefinition(cmStrCat("CMAKE_", lang, "_DEFINE_FLAG"));
    if (cmNonempty(df)) {
      dflag = *df;
    }
  }
  const char* itemSeparator = definesString.empty() ? "" : " ";
  for (std::string const& define : defines) {
    // Append the definition with proper escaping.
    std::string def = dflag;
    if (this->GetState()->UseWatcomWMake()) {
      // The Watcom compiler does its own command line parsing instead
      // of using the windows shell rules.  Definitions are one of
      //   -DNAME
      //   -DNAME=<cpp-token>
      //   -DNAME="c-string with spaces and other characters(?@#$)"
      //
      // Watcom will properly parse each of these cases from the
      // command line without any escapes.  However we still have to
      // get the '$' and '#' characters through WMake as '$$' and
      // '$#'.
      for (char c : define) {
        if (c == '$' || c == '#') {
          def += '$';
        }
        def += c;
      }
    } else {
      // Make the definition appear properly on the command line.  Use
      // -DNAME="value" instead of -D"NAME=value" for historical reasons.
      std::string::size_type eq = define.find('=');
      def += define.substr(0, eq);
      if (eq != std::string::npos) {
        def += "=";
        def += this->EscapeForShell(define.substr(eq + 1), true);
      }
    }
    definesString += itemSeparator;
    itemSeparator = " ";
    definesString += def;
  }
}

void cmLocalGenerator::AppendFeatureOptions(std::string& flags,
                                            const std::string& lang,
                                            const char* feature)
{
  cmProp optionList = this->Makefile->GetDefinition(
    cmStrCat("CMAKE_", lang, "_COMPILE_OPTIONS_", feature));
  if (optionList != nullptr) {
    std::vector<std::string> options = cmExpandedList(*optionList);
    for (std::string const& o : options) {
      this->AppendFlagEscape(flags, o);
    }
  }
}

cmProp cmLocalGenerator::GetFeature(const std::string& feature,
                                    const std::string& config)
{
  std::string featureName = feature;
  // TODO: Define accumulation policy for features (prepend, append,
  // replace). Currently we always replace.
  if (!config.empty()) {
    featureName += "_";
    featureName += cmSystemTools::UpperCase(config);
  }
  cmStateSnapshot snp = this->StateSnapshot;
  while (snp.IsValid()) {
    if (cmProp value = snp.GetDirectory().GetProperty(featureName)) {
      return value;
    }
    snp = snp.GetBuildsystemDirectoryParent();
  }
  return nullptr;
}

std::string cmLocalGenerator::GetProjectName() const
{
  return this->StateSnapshot.GetProjectName();
}

std::string cmLocalGenerator::ConstructComment(
  cmCustomCommandGenerator const& ccg, const char* default_comment) const
{
  // Check for a comment provided with the command.
  if (ccg.GetComment()) {
    return ccg.GetComment();
  }

  // Construct a reasonable default comment if possible.
  if (!ccg.GetOutputs().empty()) {
    std::string comment;
    comment = "Generating ";
    const char* sep = "";
    std::string currentBinaryDir = this->GetCurrentBinaryDirectory();
    for (std::string const& o : ccg.GetOutputs()) {
      comment += sep;
      comment += this->MaybeConvertToRelativePath(currentBinaryDir, o);
      sep = ", ";
    }
    return comment;
  }

  // Otherwise use the provided default.
  return default_comment;
}

class cmInstallTargetGeneratorLocal : public cmInstallTargetGenerator
{
public:
  cmInstallTargetGeneratorLocal(cmLocalGenerator* lg, std::string const& t,
                                std::string const& dest, bool implib)
    : cmInstallTargetGenerator(
        t, dest, implib, "", std::vector<std::string>(), "Unspecified",
        cmInstallGenerator::SelectMessageLevel(lg->GetMakefile()), false,
        false)
  {
    this->Compute(lg);
  }
};

void cmLocalGenerator::GenerateTargetInstallRules(
  std::ostream& os, const std::string& config,
  std::vector<std::string> const& configurationTypes)
{
  // Convert the old-style install specification from each target to
  // an install generator and run it.
  const auto& tgts = this->GetGeneratorTargets();
  for (const auto& l : tgts) {
    if (l->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
      continue;
    }

    // Include the user-specified pre-install script for this target.
    if (cmProp preinstall = l->GetProperty("PRE_INSTALL_SCRIPT")) {
      cmInstallScriptGenerator g(*preinstall, false, "", false);
      g.Generate(os, config, configurationTypes);
    }

    // Install this target if a destination is given.
    if (!l->Target->GetInstallPath().empty()) {
      // Compute the full install destination.  Note that converting
      // to unix slashes also removes any trailing slash.
      // We also skip over the leading slash given by the user.
      std::string destination = l->Target->GetInstallPath().substr(1);
      cmSystemTools::ConvertToUnixSlashes(destination);
      if (destination.empty()) {
        destination = ".";
      }

      // Generate the proper install generator for this target type.
      switch (l->GetType()) {
        case cmStateEnums::EXECUTABLE:
        case cmStateEnums::STATIC_LIBRARY:
        case cmStateEnums::MODULE_LIBRARY: {
          // Use a target install generator.
          cmInstallTargetGeneratorLocal g(this, l->GetName(), destination,
                                          false);
          g.Generate(os, config, configurationTypes);
        } break;
        case cmStateEnums::SHARED_LIBRARY: {
#if defined(_WIN32) || defined(__CYGWIN__)
          // Special code to handle DLL.  Install the import library
          // to the normal destination and the DLL to the runtime
          // destination.
          cmInstallTargetGeneratorLocal g1(this, l->GetName(), destination,
                                           true);
          g1.Generate(os, config, configurationTypes);
          // We also skip over the leading slash given by the user.
          destination = l->Target->GetRuntimeInstallPath().substr(1);
          cmSystemTools::ConvertToUnixSlashes(destination);
          cmInstallTargetGeneratorLocal g2(this, l->GetName(), destination,
                                           false);
          g2.Generate(os, config, configurationTypes);
#else
          // Use a target install generator.
          cmInstallTargetGeneratorLocal g(this, l->GetName(), destination,
                                          false);
          g.Generate(os, config, configurationTypes);
#endif
        } break;
        default:
          break;
      }
    }

    // Include the user-specified post-install script for this target.
    if (cmProp postinstall = l->GetProperty("POST_INSTALL_SCRIPT")) {
      cmInstallScriptGenerator g(*postinstall, false, "", false);
      g.Generate(os, config, configurationTypes);
    }
  }
}

#if defined(CM_LG_ENCODE_OBJECT_NAMES)
static bool cmLocalGeneratorShortenObjectName(std::string& objName,
                                              std::string::size_type max_len)
{
  // Replace the beginning of the path portion of the object name with
  // its own md5 sum.
  std::string::size_type pos =
    objName.find('/', objName.size() - max_len + 32);
  if (pos != std::string::npos) {
    cmCryptoHash md5(cmCryptoHash::AlgoMD5);
    std::string md5name = cmStrCat(md5.HashString(objName.substr(0, pos)),
                                   cm::string_view(objName).substr(pos));
    objName = md5name;

    // The object name is now short enough.
    return true;
  }
  // The object name could not be shortened enough.
  return false;
}

bool cmLocalGeneratorCheckObjectName(std::string& objName,
                                     std::string::size_type dir_len,
                                     std::string::size_type max_total_len)
{
  // Enforce the maximum file name length if possible.
  std::string::size_type max_obj_len = max_total_len;
  if (dir_len < max_total_len) {
    max_obj_len = max_total_len - dir_len;
    if (objName.size() > max_obj_len) {
      // The current object file name is too long.  Try to shorten it.
      return cmLocalGeneratorShortenObjectName(objName, max_obj_len);
    }
    // The object file name is short enough.
    return true;
  }
  // The build directory in which the object will be stored is
  // already too deep.
  return false;
}
#endif

std::string& cmLocalGenerator::CreateSafeUniqueObjectFileName(
  const std::string& sin, std::string const& dir_max)
{
  // Look for an existing mapped name for this object file.
  auto it = this->UniqueObjectNamesMap.find(sin);

  // If no entry exists create one.
  if (it == this->UniqueObjectNamesMap.end()) {
    // Start with the original name.
    std::string ssin = sin;

    // Avoid full paths by removing leading slashes.
    ssin.erase(0, ssin.find_first_not_of('/'));

    // Avoid full paths by removing colons.
    std::replace(ssin.begin(), ssin.end(), ':', '_');

    // Avoid relative paths that go up the tree.
    cmSystemTools::ReplaceString(ssin, "../", "__/");

    // Avoid spaces.
    std::replace(ssin.begin(), ssin.end(), ' ', '_');

    // Mangle the name if necessary.
    if (this->Makefile->IsOn("CMAKE_MANGLE_OBJECT_FILE_NAMES")) {
      bool done;
      int cc = 0;
      char rpstr[100];
      sprintf(rpstr, "_p_");
      cmSystemTools::ReplaceString(ssin, "+", rpstr);
      std::string sssin = sin;
      do {
        done = true;
        for (it = this->UniqueObjectNamesMap.begin();
             it != this->UniqueObjectNamesMap.end(); ++it) {
          if (it->second == ssin) {
            done = false;
          }
        }
        if (done) {
          break;
        }
        sssin = ssin;
        cmSystemTools::ReplaceString(ssin, "_p_", rpstr);
        sprintf(rpstr, "_p%d_", cc++);
      } while (!done);
    }

#if defined(CM_LG_ENCODE_OBJECT_NAMES)
    if (!cmLocalGeneratorCheckObjectName(ssin, dir_max.size(),
                                         this->ObjectPathMax)) {
      // Warn if this is the first time the path has been seen.
      if (this->ObjectMaxPathViolations.insert(dir_max).second) {
        std::ostringstream m;
        /* clang-format off */
        m << "The object file directory\n"
          << "  " << dir_max << "\n"
          << "has " << dir_max.size() << " characters.  "
          << "The maximum full path to an object file is "
          << this->ObjectPathMax << " characters "
          << "(see CMAKE_OBJECT_PATH_MAX).  "
          << "Object file\n"
          << "  " << ssin << "\n"
          << "cannot be safely placed under this directory.  "
          << "The build may not work correctly.";
        /* clang-format on */
        this->IssueMessage(MessageType::WARNING, m.str());
      }
    }
#else
    (void)dir_max;
#endif

    // Insert the newly mapped object file name.
    std::map<std::string, std::string>::value_type e(sin, ssin);
    it = this->UniqueObjectNamesMap.insert(e).first;
  }

  // Return the map entry.
  return it->second;
}

void cmLocalGenerator::ComputeObjectFilenames(
  std::map<cmSourceFile const*, std::string>& /*unused*/,
  cmGeneratorTarget const* /*unused*/)
{
}

bool cmLocalGenerator::IsWindowsShell() const
{
  return this->GetState()->UseWindowsShell();
}

bool cmLocalGenerator::IsWatcomWMake() const
{
  return this->GetState()->UseWatcomWMake();
}

bool cmLocalGenerator::IsMinGWMake() const
{
  return this->GetState()->UseMinGWMake();
}

bool cmLocalGenerator::IsNMake() const
{
  return this->GetState()->UseNMake();
}

bool cmLocalGenerator::IsNinjaMulti() const
{
  return this->GetState()->UseNinjaMulti();
}

std::string cmLocalGenerator::GetObjectFileNameWithoutTarget(
  const cmSourceFile& source, std::string const& dir_max,
  bool* hasSourceExtension, char const* customOutputExtension)
{
  // Construct the object file name using the full path to the source
  // file which is its only unique identification.
  std::string const& fullPath = source.GetFullPath();

  // Try referencing the source relative to the source tree.
  std::string relFromSource = this->MaybeConvertToRelativePath(
    this->GetCurrentSourceDirectory(), fullPath);
  assert(!relFromSource.empty());
  bool relSource = !cmSystemTools::FileIsFullPath(relFromSource);
  bool subSource = relSource && relFromSource[0] != '.';

  // Try referencing the source relative to the binary tree.
  std::string relFromBinary = this->MaybeConvertToRelativePath(
    this->GetCurrentBinaryDirectory(), fullPath);
  assert(!relFromBinary.empty());
  bool relBinary = !cmSystemTools::FileIsFullPath(relFromBinary);
  bool subBinary = relBinary && relFromBinary[0] != '.';

  // Select a nice-looking reference to the source file to construct
  // the object file name.
  std::string objectName;
  // XXX(clang-tidy): https://bugs.llvm.org/show_bug.cgi?id=44165
  // NOLINTNEXTLINE(bugprone-branch-clone)
  if ((relSource && !relBinary) || (subSource && !subBinary)) {
    objectName = relFromSource;
  } else if ((relBinary && !relSource) || (subBinary && !subSource) ||
             relFromBinary.length() < relFromSource.length()) {
    objectName = relFromBinary;
  } else {
    objectName = relFromSource;
  }

  // if it is still a full path check for the try compile case
  // try compile never have in source sources, and should not
  // have conflicting source file names in the same target
  if (cmSystemTools::FileIsFullPath(objectName)) {
    if (this->GetGlobalGenerator()->GetCMakeInstance()->GetIsInTryCompile()) {
      objectName = cmSystemTools::GetFilenameName(source.GetFullPath());
    }
  }

  // Ensure that for the CMakeFiles/<target>.dir/generated_source_file
  // we don't end up having:
  // CMakeFiles/<target>.dir/CMakeFiles/<target>.dir/generated_source_file.obj
  cmProp unitySourceFile = source.GetProperty("UNITY_SOURCE_FILE");
  cmProp pchExtension = source.GetProperty("PCH_EXTENSION");
  const bool isPchObject = objectName.find("cmake_pch") != std::string::npos;
  if (unitySourceFile || pchExtension || isPchObject) {
    if (pchExtension) {
      customOutputExtension = pchExtension->c_str();
    }

    cmsys::RegularExpression var("(CMakeFiles/[^/]+.dir/)");
    if (var.find(objectName)) {
      objectName.erase(var.start(), var.end() - var.start());
    }
  }

  // Replace the original source file extension with the object file
  // extension.
  bool keptSourceExtension = true;
  if (!source.GetPropertyAsBool("KEEP_EXTENSION")) {
    // Decide whether this language wants to replace the source
    // extension with the object extension.  For CMake 2.4
    // compatibility do this by default.
    bool replaceExt = this->NeedBackwardsCompatibility_2_4();
    if (!replaceExt) {
      std::string lang = source.GetLanguage();
      if (!lang.empty()) {
        replaceExt = this->Makefile->IsOn(
          cmStrCat("CMAKE_", lang, "_OUTPUT_EXTENSION_REPLACE"));
      }
    }

    // Remove the source extension if it is to be replaced.
    if (replaceExt || customOutputExtension) {
      keptSourceExtension = false;
      std::string::size_type dot_pos = objectName.rfind('.');
      if (dot_pos != std::string::npos) {
        objectName = objectName.substr(0, dot_pos);
      }
    }

    // Store the new extension.
    if (customOutputExtension) {
      objectName += customOutputExtension;
    } else {
      objectName += this->GlobalGenerator->GetLanguageOutputExtension(source);
    }
  }
  if (hasSourceExtension) {
    *hasSourceExtension = keptSourceExtension;
  }

  // Convert to a safe name.
  return this->CreateSafeUniqueObjectFileName(objectName, dir_max);
}

std::string cmLocalGenerator::GetSourceFileLanguage(const cmSourceFile& source)
{
  return source.GetLanguage();
}

cmake* cmLocalGenerator::GetCMakeInstance() const
{
  return this->GlobalGenerator->GetCMakeInstance();
}

std::string const& cmLocalGenerator::GetSourceDirectory() const
{
  return this->GetCMakeInstance()->GetHomeDirectory();
}

std::string const& cmLocalGenerator::GetBinaryDirectory() const
{
  return this->GetCMakeInstance()->GetHomeOutputDirectory();
}

std::string const& cmLocalGenerator::GetCurrentBinaryDirectory() const
{
  return this->StateSnapshot.GetDirectory().GetCurrentBinary();
}

std::string const& cmLocalGenerator::GetCurrentSourceDirectory() const
{
  return this->StateSnapshot.GetDirectory().GetCurrentSource();
}

std::string cmLocalGenerator::MaybeConvertToRelativePath(
  std::string const& local_path, std::string const& remote_path) const
{
  return this->StateSnapshot.GetDirectory().ConvertToRelPathIfNotContained(
    local_path, remote_path);
}

std::string cmLocalGenerator::GetTargetDirectory(
  const cmGeneratorTarget* /*unused*/) const
{
  cmSystemTools::Error("GetTargetDirectory"
                       " called on cmLocalGenerator");
  return "";
}

KWIML_INT_uint64_t cmLocalGenerator::GetBackwardsCompatibility()
{
  // The computed version may change until the project is fully
  // configured.
  if (!this->BackwardsCompatibilityFinal) {
    unsigned int major = 0;
    unsigned int minor = 0;
    unsigned int patch = 0;
    if (cmProp value =
          this->Makefile->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY")) {
      switch (sscanf(value->c_str(), "%u.%u.%u", &major, &minor, &patch)) {
        case 2:
          patch = 0;
          break;
        case 1:
          minor = 0;
          patch = 0;
          break;
        default:
          break;
      }
    }
    this->BackwardsCompatibility = CMake_VERSION_ENCODE(major, minor, patch);
    this->BackwardsCompatibilityFinal = true;
  }

  return this->BackwardsCompatibility;
}

bool cmLocalGenerator::NeedBackwardsCompatibility_2_4()
{
  // Check the policy to decide whether to pay attention to this
  // variable.
  switch (this->GetPolicyStatus(cmPolicies::CMP0001)) {
    case cmPolicies::WARN:
    // WARN is just OLD without warning because user code does not
    // always affect whether this check is done.
    case cmPolicies::OLD:
      // Old behavior is to check the variable.
      break;
    case cmPolicies::NEW:
      // New behavior is to ignore the variable.
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
      // This will never be the case because the only way to require
      // the setting is to require the user to specify version policy
      // 2.6 or higher.  Once we add that requirement then this whole
      // method can be removed anyway.
      return false;
  }

  // Compatibility is needed if CMAKE_BACKWARDS_COMPATIBILITY is set
  // equal to or lower than the given version.
  KWIML_INT_uint64_t actual_compat = this->GetBackwardsCompatibility();
  return (actual_compat && actual_compat <= CMake_VERSION_ENCODE(2, 4, 255));
}

cmPolicies::PolicyStatus cmLocalGenerator::GetPolicyStatus(
  cmPolicies::PolicyID id) const
{
  return this->Makefile->GetPolicyStatus(id);
}

bool cmLocalGenerator::CheckDefinition(std::string const& define) const
{
  // Many compilers do not support -DNAME(arg)=sdf so we disable it.
  std::string::size_type pos = define.find_first_of("(=");
  if (pos != std::string::npos) {
    if (define[pos] == '(') {
      std::ostringstream e;
      /* clang-format off */
      e << "WARNING: Function-style preprocessor definitions may not be "
        << "passed on the compiler command line because many compilers "
        << "do not support it.\n"
        << "CMake is dropping a preprocessor definition: " << define << "\n"
        << "Consider defining the macro in a (configured) header file.\n";
      /* clang-format on */
      cmSystemTools::Message(e.str());
      return false;
    }
  }

  // Many compilers do not support # in the value so we disable it.
  if (define.find_first_of('#') != std::string::npos) {
    std::ostringstream e;
    /* clang-format off */
    e << "WARNING: Preprocessor definitions containing '#' may not be "
      << "passed on the compiler command line because many compilers "
      << "do not support it.\n"
      << "CMake is dropping a preprocessor definition: " << define << "\n"
      << "Consider defining the macro in a (configured) header file.\n";
    /* clang-format on */
    cmSystemTools::Message(e.str());
    return false;
  }

  // Assume it is supported.
  return true;
}

static void cmLGInfoProp(cmMakefile* mf, cmGeneratorTarget* target,
                         const std::string& prop)
{
  if (cmProp val = target->GetProperty(prop)) {
    mf->AddDefinition(prop, *val);
  }
}

void cmLocalGenerator::GenerateAppleInfoPList(cmGeneratorTarget* target,
                                              const std::string& targetName,
                                              const std::string& fname)
{
  // Find the Info.plist template.
  cmProp in = target->GetProperty("MACOSX_BUNDLE_INFO_PLIST");
  std::string inFile = cmNonempty(in) ? *in : "MacOSXBundleInfo.plist.in";
  if (!cmSystemTools::FileIsFullPath(inFile)) {
    std::string inMod = this->Makefile->GetModulesFile(inFile);
    if (!inMod.empty()) {
      inFile = inMod;
    }
  }
  if (!cmSystemTools::FileExists(inFile, true)) {
    std::ostringstream e;
    e << "Target " << target->GetName() << " Info.plist template \"" << inFile
      << "\" could not be found.";
    cmSystemTools::Error(e.str());
    return;
  }

  // Convert target properties to variables in an isolated makefile
  // scope to configure the file.  If properties are set they will
  // override user make variables.  If not the configuration will fall
  // back to the directory-level values set by the user.
  cmMakefile* mf = this->Makefile;
  cmMakefile::ScopePushPop varScope(mf);
  mf->AddDefinition("MACOSX_BUNDLE_EXECUTABLE_NAME", targetName);
  cmLGInfoProp(mf, target, "MACOSX_BUNDLE_INFO_STRING");
  cmLGInfoProp(mf, target, "MACOSX_BUNDLE_ICON_FILE");
  cmLGInfoProp(mf, target, "MACOSX_BUNDLE_GUI_IDENTIFIER");
  cmLGInfoProp(mf, target, "MACOSX_BUNDLE_LONG_VERSION_STRING");
  cmLGInfoProp(mf, target, "MACOSX_BUNDLE_BUNDLE_NAME");
  cmLGInfoProp(mf, target, "MACOSX_BUNDLE_SHORT_VERSION_STRING");
  cmLGInfoProp(mf, target, "MACOSX_BUNDLE_BUNDLE_VERSION");
  cmLGInfoProp(mf, target, "MACOSX_BUNDLE_COPYRIGHT");
  mf->ConfigureFile(inFile, fname, false, false, false);
}

void cmLocalGenerator::GenerateFrameworkInfoPList(
  cmGeneratorTarget* target, const std::string& targetName,
  const std::string& fname)
{
  // Find the Info.plist template.
  cmProp in = target->GetProperty("MACOSX_FRAMEWORK_INFO_PLIST");
  std::string inFile = cmNonempty(in) ? *in : "MacOSXFrameworkInfo.plist.in";
  if (!cmSystemTools::FileIsFullPath(inFile)) {
    std::string inMod = this->Makefile->GetModulesFile(inFile);
    if (!inMod.empty()) {
      inFile = inMod;
    }
  }
  if (!cmSystemTools::FileExists(inFile, true)) {
    std::ostringstream e;
    e << "Target " << target->GetName() << " Info.plist template \"" << inFile
      << "\" could not be found.";
    cmSystemTools::Error(e.str());
    return;
  }

  // Convert target properties to variables in an isolated makefile
  // scope to configure the file.  If properties are set they will
  // override user make variables.  If not the configuration will fall
  // back to the directory-level values set by the user.
  cmMakefile* mf = this->Makefile;
  cmMakefile::ScopePushPop varScope(mf);
  mf->AddDefinition("MACOSX_FRAMEWORK_NAME", targetName);
  cmLGInfoProp(mf, target, "MACOSX_FRAMEWORK_ICON_FILE");
  cmLGInfoProp(mf, target, "MACOSX_FRAMEWORK_IDENTIFIER");
  cmLGInfoProp(mf, target, "MACOSX_FRAMEWORK_SHORT_VERSION_STRING");
  cmLGInfoProp(mf, target, "MACOSX_FRAMEWORK_BUNDLE_VERSION");
  mf->ConfigureFile(inFile, fname, false, false, false);
}

namespace {
cm::string_view CustomOutputRoleKeyword(cmLocalGenerator::OutputRole role)
{
  return (role == cmLocalGenerator::OutputRole::Primary ? "OUTPUT"_s
                                                        : "BYPRODUCTS"_s);
}

void CreateGeneratedSource(cmLocalGenerator& lg, const std::string& output,
                           cmLocalGenerator::OutputRole role,
                           cmCommandOrigin origin,
                           const cmListFileBacktrace& lfbt)
{
  if (cmGeneratorExpression::Find(output) != std::string::npos) {
    lg.GetCMakeInstance()->IssueMessage(
      MessageType::FATAL_ERROR,
      "Generator expressions in custom command outputs are not implemented!",
      lfbt);
    return;
  }

  // Make sure the file will not be generated into the source
  // directory during an out of source build.
  if (!lg.GetMakefile()->CanIWriteThisFile(output)) {
    lg.GetCMakeInstance()->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat(CustomOutputRoleKeyword(role), " path\n  ", output,
               "\nin a source directory as an output of custom command."),
      lfbt);
    return;
  }

  // Make sure the output file name has no invalid characters.
  std::string::size_type pos = output.find_first_of("#<>");
  if (pos != std::string::npos) {
    lg.GetCMakeInstance()->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat(CustomOutputRoleKeyword(role), " containing a \"", output[pos],
               "\" is not allowed."),
      lfbt);
    return;
  }

  // Outputs without generator expressions from the project are already
  // created and marked as generated.  Do not mark them again, because
  // other commands might have overwritten the property.
  if (origin == cmCommandOrigin::Generator) {
    lg.GetMakefile()->GetOrCreateGeneratedSource(output);
  }
}

std::string ComputeCustomCommandRuleFileName(cmLocalGenerator& lg,
                                             cmListFileBacktrace const& bt,
                                             std::string const& output)
{
  // If the output path has no generator expressions, use it directly.
  if (cmGeneratorExpression::Find(output) == std::string::npos) {
    return output;
  }

  // The output path contains a generator expression, but we must choose
  // a single source file path to which to attach the custom command.
  // Use some heuristics to provie a nice-looking name when possible.

  // If the only genex is $<CONFIG>, replace that gracefully.
  {
    std::string simple = output;
    cmSystemTools::ReplaceString(simple, "$<CONFIG>", "(CONFIG)");
    if (cmGeneratorExpression::Find(simple) == std::string::npos) {
      return simple;
    }
  }

  // If the genex evaluates to the same value in all configurations, use that.
  {
    std::vector<std::string> allConfigOutputs =
      lg.ExpandCustomCommandOutputGenex(output, bt);
    if (allConfigOutputs.size() == 1) {
      return allConfigOutputs.front();
    }
  }

  // Fall back to a deterministic unique name.
  cmCryptoHash h(cmCryptoHash::AlgoSHA256);
  return cmStrCat(lg.GetCurrentBinaryDirectory(), "/CMakeFiles/",
                  h.HashString(output).substr(0, 16));
}

cmSourceFile* AddCustomCommand(
  cmLocalGenerator& lg, const cmListFileBacktrace& lfbt,
  cmCommandOrigin origin, const std::vector<std::string>& outputs,
  const std::vector<std::string>& byproducts,
  const std::vector<std::string>& depends, const std::string& main_dependency,
  const cmImplicitDependsList& implicit_depends,
  const cmCustomCommandLines& commandLines, const char* comment,
  const char* workingDir, bool replace, bool escapeOldStyle,
  bool uses_terminal, bool command_expand_lists, const std::string& depfile,
  const std::string& job_pool, bool stdPipesUTF8,
  cmPolicies::PolicyStatus cmp0116)
{
  cmMakefile* mf = lg.GetMakefile();

  // Choose a source file on which to store the custom command.
  cmSourceFile* file = nullptr;
  if (!commandLines.empty() && !main_dependency.empty()) {
    // The main dependency was specified.  Use it unless a different
    // custom command already used it.
    file = mf->GetSource(main_dependency);
    if (file && file->GetCustomCommand() && !replace) {
      // The main dependency already has a custom command.
      if (commandLines == file->GetCustomCommand()->GetCommandLines()) {
        // The existing custom command is identical.  Silently ignore
        // the duplicate.
        return file;
      }
      // The existing custom command is different.  We need to
      // generate a rule file for this new command.
      file = nullptr;
    } else if (!file) {
      file = mf->CreateSource(main_dependency);
    }
  }

  // Generate a rule file if the main dependency is not available.
  if (!file) {
    cmGlobalGenerator* gg = lg.GetGlobalGenerator();

    // Construct a rule file associated with the first output produced.
    std::string outName = gg->GenerateRuleFile(
      ComputeCustomCommandRuleFileName(lg, lfbt, outputs[0]));

    // Check if the rule file already exists.
    file = mf->GetSource(outName, cmSourceFileLocationKind::Known);
    if (file && file->GetCustomCommand() && !replace) {
      // The rule file already exists.
      if (commandLines != file->GetCustomCommand()->GetCommandLines()) {
        lg.GetCMakeInstance()->IssueMessage(
          MessageType::FATAL_ERROR,
          cmStrCat("Attempt to add a custom rule to output\n  ", outName,
                   "\nwhich already has a custom rule."),
          lfbt);
      }
      return file;
    }

    // Create a cmSourceFile for the rule file.
    if (!file) {
      file = mf->CreateSource(outName, true, cmSourceFileLocationKind::Known);
    }
    file->SetProperty("__CMAKE_RULE", "1");
  }

  // Attach the custom command to the file.
  if (file) {
    // Construct a complete list of dependencies.
    std::vector<std::string> depends2(depends);
    if (!main_dependency.empty()) {
      depends2.push_back(main_dependency);
    }

    std::unique_ptr<cmCustomCommand> cc = cm::make_unique<cmCustomCommand>(
      outputs, byproducts, depends2, commandLines, lfbt, comment, workingDir,
      stdPipesUTF8);
    cc->SetEscapeOldStyle(escapeOldStyle);
    cc->SetEscapeAllowMakeVars(true);
    cc->SetImplicitDepends(implicit_depends);
    cc->SetUsesTerminal(uses_terminal);
    cc->SetCommandExpandLists(command_expand_lists);
    cc->SetDepfile(depfile);
    cc->SetJobPool(job_pool);
    cc->SetCMP0116Status(cmp0116);
    file->SetCustomCommand(std::move(cc));

    lg.AddSourceOutputs(file, outputs, cmLocalGenerator::OutputRole::Primary,
                        lfbt, origin);
    lg.AddSourceOutputs(file, byproducts,
                        cmLocalGenerator::OutputRole::Byproduct, lfbt, origin);
  }
  return file;
}

bool AnyOutputMatches(const std::string& name,
                      const std::vector<std::string>& outputs)
{
  return std::any_of(outputs.begin(), outputs.end(),
                     [&name](std::string const& output) -> bool {
                       std::string::size_type pos = output.rfind(name);
                       // If the output matches exactly
                       return (pos != std::string::npos &&
                               pos == output.size() - name.size() &&
                               (pos == 0 || output[pos - 1] == '/'));
                     });
}

bool AnyTargetCommandOutputMatches(
  const std::string& name, const std::vector<cmCustomCommand>& commands)
{
  return std::any_of(commands.begin(), commands.end(),
                     [&name](cmCustomCommand const& command) -> bool {
                       return AnyOutputMatches(name, command.GetByproducts());
                     });
}
}

namespace detail {
void AddCustomCommandToTarget(cmLocalGenerator& lg,
                              const cmListFileBacktrace& lfbt,
                              cmCommandOrigin origin, cmTarget* target,
                              const std::vector<std::string>& byproducts,
                              const std::vector<std::string>& depends,
                              const cmCustomCommandLines& commandLines,
                              cmCustomCommandType type, const char* comment,
                              const char* workingDir, bool escapeOldStyle,
                              bool uses_terminal, const std::string& depfile,
                              const std::string& job_pool,
                              bool command_expand_lists, bool stdPipesUTF8,
                              cmPolicies::PolicyStatus cmp0116)
{
  // Add the command to the appropriate build step for the target.
  std::vector<std::string> no_output;
  cmCustomCommand cc(no_output, byproducts, depends, commandLines, lfbt,
                     comment, workingDir, stdPipesUTF8);
  cc.SetEscapeOldStyle(escapeOldStyle);
  cc.SetEscapeAllowMakeVars(true);
  cc.SetUsesTerminal(uses_terminal);
  cc.SetCommandExpandLists(command_expand_lists);
  cc.SetDepfile(depfile);
  cc.SetJobPool(job_pool);
  cc.SetCMP0116Status(cmp0116);
  switch (type) {
    case cmCustomCommandType::PRE_BUILD:
      target->AddPreBuildCommand(std::move(cc));
      break;
    case cmCustomCommandType::PRE_LINK:
      target->AddPreLinkCommand(std::move(cc));
      break;
    case cmCustomCommandType::POST_BUILD:
      target->AddPostBuildCommand(std::move(cc));
      break;
  }

  lg.AddTargetByproducts(target, byproducts, lfbt, origin);
}

cmSourceFile* AddCustomCommandToOutput(
  cmLocalGenerator& lg, const cmListFileBacktrace& lfbt,
  cmCommandOrigin origin, const std::vector<std::string>& outputs,
  const std::vector<std::string>& byproducts,
  const std::vector<std::string>& depends, const std::string& main_dependency,
  const cmImplicitDependsList& implicit_depends,
  const cmCustomCommandLines& commandLines, const char* comment,
  const char* workingDir, bool replace, bool escapeOldStyle,
  bool uses_terminal, bool command_expand_lists, const std::string& depfile,
  const std::string& job_pool, bool stdPipesUTF8,
  cmPolicies::PolicyStatus cmp0116)
{
  return AddCustomCommand(lg, lfbt, origin, outputs, byproducts, depends,
                          main_dependency, implicit_depends, commandLines,
                          comment, workingDir, replace, escapeOldStyle,
                          uses_terminal, command_expand_lists, depfile,
                          job_pool, stdPipesUTF8, cmp0116);
}

void AppendCustomCommandToOutput(cmLocalGenerator& lg,
                                 const cmListFileBacktrace& lfbt,
                                 const std::string& output,
                                 const std::vector<std::string>& depends,
                                 const cmImplicitDependsList& implicit_depends,
                                 const cmCustomCommandLines& commandLines)
{
  // Lookup an existing command.
  cmSourceFile* sf = nullptr;
  if (cmGeneratorExpression::Find(output) == std::string::npos) {
    sf = lg.GetSourceFileWithOutput(output);
  } else {
    // This output path has a generator expression.  Evaluate it to
    // find the output for any configurations.
    for (std::string const& out :
         lg.ExpandCustomCommandOutputGenex(output, lfbt)) {
      sf = lg.GetSourceFileWithOutput(out);
      if (sf) {
        break;
      }
    }
  }

  if (sf) {
    if (cmCustomCommand* cc = sf->GetCustomCommand()) {
      cc->AppendCommands(commandLines);
      cc->AppendDepends(depends);
      cc->AppendImplicitDepends(implicit_depends);
      return;
    }
  }

  // No existing command found.
  lg.GetCMakeInstance()->IssueMessage(
    MessageType::FATAL_ERROR,
    cmStrCat("Attempt to APPEND to custom command with output\n  ", output,
             "\nwhich is not already a custom command output."),
    lfbt);
}

void AddUtilityCommand(cmLocalGenerator& lg, const cmListFileBacktrace& lfbt,
                       cmCommandOrigin origin, cmTarget* target,
                       const char* workingDir,
                       const std::vector<std::string>& byproducts,
                       const std::vector<std::string>& depends,
                       const cmCustomCommandLines& commandLines,
                       bool escapeOldStyle, const char* comment,
                       bool uses_terminal, bool command_expand_lists,
                       const std::string& job_pool, bool stdPipesUTF8,
                       cmPolicies::PolicyStatus cmp0116)
{
  // Use an empty comment to avoid generation of default comment.
  if (!comment) {
    comment = "";
  }

  // Create the generated symbolic output name of the utility target.
  std::string output =
    lg.CreateUtilityOutput(target->GetName(), byproducts, lfbt);

  std::string no_main_dependency;
  cmImplicitDependsList no_implicit_depends;
  cmSourceFile* rule = AddCustomCommand(
    lg, lfbt, origin, { output }, byproducts, depends, no_main_dependency,
    no_implicit_depends, commandLines, comment, workingDir,
    /*replace=*/false, escapeOldStyle, uses_terminal, command_expand_lists,
    /*depfile=*/"", job_pool, stdPipesUTF8, cmp0116);
  if (rule) {
    lg.AddTargetByproducts(target, byproducts, lfbt, origin);
  }

  target->AddSource(output);
}

std::vector<std::string> ComputeISPCObjectSuffixes(cmGeneratorTarget* target)
{
  const std::string& targetProperty =
    target->GetSafeProperty("ISPC_INSTRUCTION_SETS");
  std::vector<std::string> ispcTargets;

  if (!cmIsOff(targetProperty)) {
    cmExpandList(targetProperty, ispcTargets);
    for (auto& ispcTarget : ispcTargets) {
      // transform targets into the suffixes
      auto pos = ispcTarget.find('-');
      auto target_suffix = ispcTarget.substr(0, pos);
      if (target_suffix ==
          "avx1") { // when targetting avx1 ISPC uses the 'avx' output string
        target_suffix = "avx";
      }
      ispcTarget = target_suffix;
    }
  }
  return ispcTargets;
}

std::vector<std::string> ComputeISPCExtraObjects(
  std::string const& objectName, std::string const& buildDirectory,
  std::vector<std::string> const& ispcSuffixes)
{
  auto normalizedDir = cmSystemTools::CollapseFullPath(buildDirectory);
  std::vector<std::string> computedObjects;
  computedObjects.reserve(ispcSuffixes.size());

  auto extension = cmSystemTools::GetFilenameLastExtension(objectName);

  // We can't use cmSystemTools::GetFilenameWithoutLastExtension as it
  // drops any directories in objectName
  auto objNameNoExt = objectName;
  std::string::size_type dot_pos = objectName.rfind('.');
  if (dot_pos != std::string::npos) {
    objNameNoExt.resize(dot_pos);
  }

  for (const auto& ispcTarget : ispcSuffixes) {
    computedObjects.emplace_back(
      cmStrCat(normalizedDir, "/", objNameNoExt, "_", ispcTarget, extension));
  }

  return computedObjects;
}
}

cmSourcesWithOutput cmLocalGenerator::GetSourcesWithOutput(
  const std::string& name) const
{
  // Linear search?  Also see GetSourceFileWithOutput for detail.
  if (!cmSystemTools::FileIsFullPath(name)) {
    cmSourcesWithOutput sources;
    sources.Target = this->LinearGetTargetWithOutput(name);
    sources.Source = this->LinearGetSourceFileWithOutput(
      name, cmSourceOutputKind::OutputOrByproduct, sources.SourceIsByproduct);
    return sources;
  }
  // Otherwise we use an efficient lookup map.
  auto o = this->OutputToSource.find(name);
  if (o != this->OutputToSource.end()) {
    return o->second.Sources;
  }
  return {};
}

cmSourceFile* cmLocalGenerator::GetSourceFileWithOutput(
  const std::string& name, cmSourceOutputKind kind) const
{
  // If the queried path is not absolute we use the backward compatible
  // linear-time search for an output with a matching suffix.
  if (!cmSystemTools::FileIsFullPath(name)) {
    bool byproduct = false;
    return this->LinearGetSourceFileWithOutput(name, kind, byproduct);
  }
  // Otherwise we use an efficient lookup map.
  auto o = this->OutputToSource.find(name);
  if (o != this->OutputToSource.end() &&
      (!o->second.Sources.SourceIsByproduct ||
       kind == cmSourceOutputKind::OutputOrByproduct)) {
    // Source file could also be null pointer for example if we found the
    // byproduct of a utility target, a PRE_BUILD, PRE_LINK, or POST_BUILD
    // command of a target, or a not yet created custom command.
    return o->second.Sources.Source;
  }
  return nullptr;
}

std::string cmLocalGenerator::CreateUtilityOutput(
  std::string const& targetName, std::vector<std::string> const&,
  cmListFileBacktrace const&)
{
  std::string force =
    cmStrCat(this->GetCurrentBinaryDirectory(), "/CMakeFiles/", targetName);
  // The output is not actually created so mark it symbolic.
  if (cmSourceFile* sf = this->Makefile->GetOrCreateGeneratedSource(force)) {
    sf->SetProperty("SYMBOLIC", "1");
  } else {
    cmSystemTools::Error("Could not get source file entry for " + force);
  }
  return force;
}

std::vector<cmCustomCommandGenerator>
cmLocalGenerator::MakeCustomCommandGenerators(cmCustomCommand const& cc,
                                              std::string const& config)
{
  std::vector<cmCustomCommandGenerator> ccgs;
  ccgs.emplace_back(cc, config, this);
  return ccgs;
}

std::vector<std::string> cmLocalGenerator::ExpandCustomCommandOutputPaths(
  cmCompiledGeneratorExpression const& cge, std::string const& config)
{
  std::vector<std::string> paths = cmExpandedList(cge.Evaluate(this, config));
  for (std::string& p : paths) {
    p = cmSystemTools::CollapseFullPath(p, this->GetCurrentBinaryDirectory());
  }
  return paths;
}

std::vector<std::string> cmLocalGenerator::ExpandCustomCommandOutputGenex(
  std::string const& o, cmListFileBacktrace const& bt)
{
  std::vector<std::string> allConfigOutputs;
  cmGeneratorExpression ge(bt);
  std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(o);
  std::vector<std::string> configs =
    this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
  for (std::string const& config : configs) {
    std::vector<std::string> configOutputs =
      this->ExpandCustomCommandOutputPaths(*cge, config);
    allConfigOutputs.reserve(allConfigOutputs.size() + configOutputs.size());
    std::move(configOutputs.begin(), configOutputs.end(),
              std::back_inserter(allConfigOutputs));
  }
  auto endUnique =
    cmRemoveDuplicates(allConfigOutputs.begin(), allConfigOutputs.end());
  allConfigOutputs.erase(endUnique, allConfigOutputs.end());
  return allConfigOutputs;
}

void cmLocalGenerator::AddTargetByproducts(
  cmTarget* target, const std::vector<std::string>& byproducts,
  cmListFileBacktrace const& bt, cmCommandOrigin origin)
{
  for (std::string const& o : byproducts) {
    if (cmGeneratorExpression::Find(o) == std::string::npos) {
      this->UpdateOutputToSourceMap(o, target, bt, origin);
      continue;
    }

    // This byproduct path has a generator expression.  Evaluate it to
    // register the byproducts for all configurations.
    for (std::string const& b : this->ExpandCustomCommandOutputGenex(o, bt)) {
      this->UpdateOutputToSourceMap(b, target, bt, cmCommandOrigin::Generator);
    }
  }
}

void cmLocalGenerator::AddSourceOutputs(
  cmSourceFile* source, const std::vector<std::string>& outputs,
  OutputRole role, cmListFileBacktrace const& bt, cmCommandOrigin origin)
{
  for (std::string const& o : outputs) {
    if (cmGeneratorExpression::Find(o) == std::string::npos) {
      this->UpdateOutputToSourceMap(o, source, role, bt, origin);
      continue;
    }

    // This output path has a generator expression.  Evaluate it to
    // register the outputs for all configurations.
    for (std::string const& out :
         this->ExpandCustomCommandOutputGenex(o, bt)) {
      this->UpdateOutputToSourceMap(out, source, role, bt,
                                    cmCommandOrigin::Generator);
    }
  }
}

void cmLocalGenerator::UpdateOutputToSourceMap(std::string const& byproduct,
                                               cmTarget* target,
                                               cmListFileBacktrace const& bt,
                                               cmCommandOrigin origin)
{
  SourceEntry entry;
  entry.Sources.Target = target;

  auto pr = this->OutputToSource.emplace(byproduct, entry);
  if (pr.second) {
    CreateGeneratedSource(*this, byproduct, OutputRole::Byproduct, origin, bt);
  } else {
    SourceEntry& current = pr.first->second;
    // Has the target already been set?
    if (!current.Sources.Target) {
      current.Sources.Target = target;
    } else {
      // Multiple custom commands/targets produce the same output (source file
      // or target).  See also comment in other UpdateOutputToSourceMap
      // overload.
      //
      // TODO: Warn the user about this case.
    }
  }
}

void cmLocalGenerator::UpdateOutputToSourceMap(std::string const& output,
                                               cmSourceFile* source,
                                               OutputRole role,
                                               cmListFileBacktrace const& bt,
                                               cmCommandOrigin origin)
{
  SourceEntry entry;
  entry.Sources.Source = source;
  entry.Sources.SourceIsByproduct = role == OutputRole::Byproduct;

  auto pr = this->OutputToSource.emplace(output, entry);
  if (pr.second) {
    CreateGeneratedSource(*this, output, role, origin, bt);
  } else {
    SourceEntry& current = pr.first->second;
    // Outputs take precedence over byproducts
    if (!current.Sources.Source ||
        (current.Sources.SourceIsByproduct && role == OutputRole::Primary)) {
      current.Sources.Source = source;
      current.Sources.SourceIsByproduct = false;
    } else {
      // Multiple custom commands produce the same output but may
      // be attached to a different source file (MAIN_DEPENDENCY).
      // LinearGetSourceFileWithOutput would return the first one,
      // so keep the mapping for the first one.
      //
      // TODO: Warn the user about this case.  However, the VS 8 generator
      // triggers it for separate generate.stamp rules in ZERO_CHECK and
      // individual targets.
    }
  }
}

cmTarget* cmLocalGenerator::LinearGetTargetWithOutput(
  const std::string& name) const
{
  // We go through the ordered vector of targets to get reproducible results
  // should multiple names match.
  for (cmTarget* t : this->Makefile->GetOrderedTargets()) {
    // Does the output of any command match the source file name?
    if (AnyTargetCommandOutputMatches(name, t->GetPreBuildCommands())) {
      return t;
    }
    if (AnyTargetCommandOutputMatches(name, t->GetPreLinkCommands())) {
      return t;
    }
    if (AnyTargetCommandOutputMatches(name, t->GetPostBuildCommands())) {
      return t;
    }
  }
  return nullptr;
}

cmSourceFile* cmLocalGenerator::LinearGetSourceFileWithOutput(
  const std::string& name, cmSourceOutputKind kind, bool& byproduct) const
{
  // Outputs take precedence over byproducts.
  byproduct = false;
  cmSourceFile* fallback = nullptr;

  // Look through all the source files that have custom commands and see if the
  // custom command has the passed source file as an output.
  for (const auto& src : this->Makefile->GetSourceFiles()) {
    // Does this source file have a custom command?
    if (src->GetCustomCommand()) {
      // Does the output of the custom command match the source file name?
      if (AnyOutputMatches(name, src->GetCustomCommand()->GetOutputs())) {
        // Return the first matching output.
        return src.get();
      }
      if (kind == cmSourceOutputKind::OutputOrByproduct) {
        if (AnyOutputMatches(name, src->GetCustomCommand()->GetByproducts())) {
          // Do not return the source yet as there might be a matching output.
          fallback = src.get();
        }
      }
    }
  }

  // Did we find a byproduct?
  byproduct = fallback != nullptr;
  return fallback;
}
