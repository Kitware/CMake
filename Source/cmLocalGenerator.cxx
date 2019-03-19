/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmLocalGenerator.h"

#include "cmAlgorithms.h"
#include "cmComputeLinkInformation.h"
#include "cmCustomCommandGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpressionEvaluationFile.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmInstallGenerator.h"
#include "cmInstallScriptGenerator.h"
#include "cmInstallTargetGenerator.h"
#include "cmLinkLineComputer.h"
#include "cmMakefile.h"
#include "cmRulePlaceholderExpander.h"
#include "cmSourceFile.h"
#include "cmSourceFileLocation.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTestGenerator.h"
#include "cmVersion.h"
#include "cmake.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#  define CM_LG_ENCODE_OBJECT_NAMES
#  include "cmCryptoHash.h"
#endif

#include "cmsys/RegularExpression.hxx"
#include <algorithm>
#include <assert.h>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <unordered_set>
#include <utility>

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

  std::vector<std::string> enabledLanguages =
    this->GetState()->GetEnabledLanguages();

  if (const char* sysrootCompile =
        this->Makefile->GetDefinition("CMAKE_SYSROOT_COMPILE")) {
    this->CompilerSysroot = sysrootCompile;
  } else {
    this->CompilerSysroot = this->Makefile->GetSafeDefinition("CMAKE_SYSROOT");
  }

  if (const char* sysrootLink =
        this->Makefile->GetDefinition("CMAKE_SYSROOT_LINK")) {
    this->LinkerSysroot = sysrootLink;
  } else {
    this->LinkerSysroot = this->Makefile->GetSafeDefinition("CMAKE_SYSROOT");
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

cmLocalGenerator::~cmLocalGenerator()
{
  cmDeleteAll(this->GeneratorTargets);
  cmDeleteAll(this->OwnedImportedGeneratorTargets);
}

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
  const char* plen = this->Makefile->GetDefinition("CMAKE_OBJECT_PATH_MAX");
  if (plen && *plen) {
    unsigned int pmax;
    if (sscanf(plen, "%u", &pmax) == 1) {
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
      w << "CMAKE_OBJECT_PATH_MAX is set to \"" << plen
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

void cmLocalGenerator::TraceDependencies()
{
  std::vector<std::string> configs;
  this->Makefile->GetConfigurations(configs);
  if (configs.empty()) {
    configs.emplace_back();
  }
  for (std::string const& c : configs) {
    this->GlobalGenerator->CreateEvaluationSourceFiles(c);
  }
  // Generate the rule files for each target.
  const std::vector<cmGeneratorTarget*>& targets = this->GetGeneratorTargets();
  for (cmGeneratorTarget* target : targets) {
    if (target->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
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
  std::vector<std::string> configurationTypes;
  const std::string& config =
    this->Makefile->GetConfigurations(configurationTypes, false);

  std::string file = this->StateSnapshot.GetDirectory().GetCurrentBinary();
  file += "/";
  file += "CTestTestfile.cmake";

  cmGeneratedFileStream fout(file);
  fout.SetCopyIfDifferent(true);

  fout << "# CMake generated Testfile for " << std::endl
       << "# Source directory: "
       << this->StateSnapshot.GetDirectory().GetCurrentSource() << std::endl
       << "# Build directory: "
       << this->StateSnapshot.GetDirectory().GetCurrentBinary() << std::endl
       << "# " << std::endl
       << "# This file includes the relevant testing commands "
       << "required for " << std::endl
       << "# testing this directory and lists subdirectories to "
       << "be tested as well." << std::endl;

  const char* testIncludeFile =
    this->Makefile->GetProperty("TEST_INCLUDE_FILE");
  if (testIncludeFile) {
    fout << "include(\"" << testIncludeFile << "\")" << std::endl;
  }

  const char* testIncludeFiles =
    this->Makefile->GetProperty("TEST_INCLUDE_FILES");
  if (testIncludeFiles) {
    std::vector<std::string> includesList;
    cmSystemTools::ExpandListArgument(testIncludeFiles, includesList);
    for (std::string const& i : includesList) {
      fout << "include(\"" << i << "\")" << std::endl;
    }
  }

  // Ask each test generator to write its code.
  std::vector<cmTestGenerator*> const& testers =
    this->Makefile->GetTestGenerators();
  for (cmTestGenerator* tester : testers) {
    tester->Compute(this);
    tester->Generate(fout, config, configurationTypes);
  }
  typedef std::vector<cmStateSnapshot> vec_t;
  vec_t const& children = this->Makefile->GetStateSnapshot().GetChildren();
  std::string parentBinDir = this->GetCurrentBinaryDirectory();
  for (cmStateSnapshot const& i : children) {
    // TODO: Use add_subdirectory instead?
    std::string outP = i.GetDirectory().GetCurrentBinary();
    outP = this->MaybeConvertToRelativePath(parentBinDir, outP);
    outP = cmOutputConverter::EscapeForCMake(outP);
    fout << "subdirs(" << outP << ")" << std::endl;
  }

  // Add directory labels property
  const char* directoryLabels =
    this->Makefile->GetDefinition("CMAKE_DIRECTORY_LABELS");
  const char* labels = this->Makefile->GetProperty("LABELS");

  if (labels || directoryLabels) {
    fout << "set_directory_properties(PROPERTIES LABELS ";
    if (labels) {
      fout << cmOutputConverter::EscapeForCMake(labels);
    }
    if (labels && directoryLabels) {
      fout << ";";
    }
    if (directoryLabels) {
      fout << cmOutputConverter::EscapeForCMake(directoryLabels);
    }
    fout << ")" << std::endl;
  }
}

void cmLocalGenerator::CreateEvaluationFileOutputs(std::string const& config)
{
  std::vector<cmGeneratorExpressionEvaluationFile*> ef =
    this->Makefile->GetEvaluationFiles();
  for (cmGeneratorExpressionEvaluationFile* geef : ef) {
    geef->CreateOutputFile(this, config);
  }
}

void cmLocalGenerator::ProcessEvaluationFiles(
  std::vector<std::string>& generatedFiles)
{
  std::vector<cmGeneratorExpressionEvaluationFile*> ef =
    this->Makefile->GetEvaluationFiles();
  for (cmGeneratorExpressionEvaluationFile* geef : ef) {
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

    generatedFiles.insert(generatedFiles.end(), files.begin(), files.end());
    std::vector<std::string>::iterator newIt =
      generatedFiles.end() - files.size();
    std::inplace_merge(generatedFiles.begin(), newIt, generatedFiles.end());
  }
}

void cmLocalGenerator::GenerateInstallRules()
{
  // Compute the install prefix.
  const char* prefix = this->Makefile->GetDefinition("CMAKE_INSTALL_PREFIX");

#if defined(_WIN32) && !defined(__CYGWIN__)
  std::string prefix_win32;
  if (!prefix) {
    if (!cmSystemTools::GetEnv("SystemDrive", prefix_win32)) {
      prefix_win32 = "C:";
    }
    const char* project_name = this->Makefile->GetDefinition("PROJECT_NAME");
    if (project_name && project_name[0]) {
      prefix_win32 += "/Program Files/";
      prefix_win32 += project_name;
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
  if (const char* stagingPrefix =
        this->Makefile->GetDefinition("CMAKE_STAGING_PREFIX")) {
    prefix = stagingPrefix;
  }

  // Compute the set of configurations.
  std::vector<std::string> configurationTypes;
  const std::string& config =
    this->Makefile->GetConfigurations(configurationTypes, false);

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
  fout << "# Install script for directory: "
       << this->StateSnapshot.GetDirectory().GetCurrentSource() << std::endl
       << std::endl;
  fout << "# Set the install prefix" << std::endl
       << "if(NOT DEFINED CMAKE_INSTALL_PREFIX)" << std::endl
       << "  set(CMAKE_INSTALL_PREFIX \"" << prefix << "\")" << std::endl
       << "endif()" << std::endl
       << "string(REGEX REPLACE \"/$\" \"\" CMAKE_INSTALL_PREFIX "
       << "\"${CMAKE_INSTALL_PREFIX}\")" << std::endl
       << std::endl;

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
  if (const char* so_no_exe =
        this->Makefile->GetDefinition("CMAKE_INSTALL_SO_NO_EXE")) {
    /* clang-format off */
    fout <<
      "# Install shared libraries without execute permission?\n"
      "if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)\n"
      "  set(CMAKE_INSTALL_SO_NO_EXE \"" << so_no_exe << "\")\n"
      "endif()\n"
      "\n";
    /* clang-format on */
  }

  // Copy cmake cross compile state to install code.
  if (const char* crosscompiling =
        this->Makefile->GetDefinition("CMAKE_CROSSCOMPILING")) {
    /* clang-format off */
    fout <<
      "# Is this installation the result of a crosscompile?\n"
      "if(NOT DEFINED CMAKE_CROSSCOMPILING)\n"
      "  set(CMAKE_CROSSCOMPILING \"" << crosscompiling << "\")\n"
      "endif()\n"
      "\n";
    /* clang-format on */
  }

  // Write default directory permissions.
  if (const char* defaultDirPermissions = this->Makefile->GetDefinition(
        "CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS")) {
    /* clang-format off */
    fout <<
      "# Set default install directory permissions.\n"
      "if(NOT DEFINED CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS)\n"
      "  set(CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS \""
         << defaultDirPermissions << "\")\n"
      "endif()\n"
      "\n";
    /* clang-format on */
  }

  // Ask each install generator to write its code.
  cmPolicies::PolicyStatus status = this->GetPolicyStatus(cmPolicies::CMP0082);
  std::vector<cmInstallGenerator*> const& installers =
    this->Makefile->GetInstallGenerators();
  bool haveSubdirectoryInstall = false;
  bool haveInstallAfterSubdirectory = false;
  if (status == cmPolicies::WARN) {
    for (cmInstallGenerator* installer : installers) {
      installer->CheckCMP0082(haveSubdirectoryInstall,
                              haveInstallAfterSubdirectory);
      installer->Generate(fout, config, configurationTypes);
    }
  } else {
    for (cmInstallGenerator* installer : installers) {
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
            fout << "  include(\"" << odir << "/cmake_install.cmake\")"
                 << std::endl;
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

void cmLocalGenerator::AddGeneratorTarget(cmGeneratorTarget* gt)
{
  this->GeneratorTargets.push_back(gt);
  this->GeneratorTargetSearchIndex.emplace(gt->GetName(), gt);
  this->GlobalGenerator->IndexGeneratorTarget(gt);
}

void cmLocalGenerator::AddImportedGeneratorTarget(cmGeneratorTarget* gt)
{
  this->ImportedGeneratorTargets.emplace(gt->GetName(), gt);
  this->GlobalGenerator->IndexGeneratorTarget(gt);
}

void cmLocalGenerator::AddOwnedImportedGeneratorTarget(cmGeneratorTarget* gt)
{
  this->OwnedImportedGeneratorTargets.push_back(gt);
}

cmGeneratorTarget* cmLocalGenerator::FindLocalNonAliasGeneratorTarget(
  const std::string& name) const
{
  GeneratorTargetMap::const_iterator ti =
    this->GeneratorTargetSearchIndex.find(name);
  if (ti != this->GeneratorTargetSearchIndex.end()) {
    return ti->second;
  }
  return nullptr;
}

void cmLocalGenerator::ComputeTargetManifest()
{
  // Collect the set of configuration types.
  std::vector<std::string> configNames;
  this->Makefile->GetConfigurations(configNames);
  if (configNames.empty()) {
    configNames.emplace_back();
  }

  // Add our targets to the manifest for each configuration.
  const std::vector<cmGeneratorTarget*>& targets = this->GetGeneratorTargets();
  for (cmGeneratorTarget* target : targets) {
    if (target->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
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
  std::vector<std::string> configNames;
  this->Makefile->GetConfigurations(configNames);
  if (configNames.empty()) {
    configNames.emplace_back();
  }

  // Process compile features of all targets.
  const std::vector<cmGeneratorTarget*>& targets = this->GetGeneratorTargets();
  for (cmGeneratorTarget* target : targets) {
    for (std::string const& c : configNames) {
      if (!target->ComputeCompileFeatures(c)) {
        return false;
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

const char* cmLocalGenerator::GetRuleLauncher(cmGeneratorTarget* target,
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

  std::string flagVar = "CMAKE_INCLUDE_FLAG_";
  flagVar += lang;
  std::string const& includeFlag = this->Makefile->GetSafeDefinition(flagVar);
  flagVar = "CMAKE_INCLUDE_FLAG_SEP_";
  flagVar += lang;
  const char* sep = this->Makefile->GetDefinition(flagVar);
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
  std::string sysFlagVar = "CMAKE_INCLUDE_SYSTEM_FLAG_";
  sysFlagVar += lang;
  const char* sysIncludeFlag = nullptr;
  if (repeatFlag) {
    sysIncludeFlag = this->Makefile->GetDefinition(sysFlagVar);
  }

  std::string fwSearchFlagVar = "CMAKE_";
  fwSearchFlagVar += lang;
  fwSearchFlagVar += "_FRAMEWORK_SEARCH_FLAG";
  const char* fwSearchFlag = this->Makefile->GetDefinition(fwSearchFlagVar);

  std::string sysFwSearchFlagVar = "CMAKE_";
  sysFwSearchFlagVar += lang;
  sysFwSearchFlagVar += "_SYSTEM_FRAMEWORK_SEARCH_FLAG";
  const char* sysFwSearchFlag =
    this->Makefile->GetDefinition(sysFwSearchFlagVar);

  bool flagUsed = false;
  std::set<std::string> emitted;
#ifdef __APPLE__
  emitted.insert("/System/Library/Frameworks");
#endif
  for (std::string const& i : includes) {
    if (fwSearchFlag && *fwSearchFlag && this->Makefile->IsOn("APPLE") &&
        cmSystemTools::IsPathToFramework(i)) {
      std::string frameworkDir = i;
      frameworkDir += "/../";
      frameworkDir = cmSystemTools::CollapseFullPath(frameworkDir);
      if (emitted.insert(frameworkDir).second) {
        if (sysFwSearchFlag && target &&
            target->IsSystemIncludeDirectory(i, config, lang)) {
          includeFlags << sysFwSearchFlag;
        } else {
          includeFlags << fwSearchFlag;
        }
        includeFlags << this->ConvertToOutputFormat(frameworkDir, shellFormat)
                     << " ";
      }
      continue;
    }

    if (!flagUsed || repeatFlag) {
      if (sysIncludeFlag && target &&
          target->IsSystemIncludeDirectory(i, config, lang)) {
        includeFlags << sysIncludeFlag;
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
  return flags;
}

void cmLocalGenerator::AddCompileOptions(std::string& flags,
                                         cmGeneratorTarget* target,
                                         const std::string& lang,
                                         const std::string& config)
{
  std::string langFlagRegexVar = std::string("CMAKE_") + lang + "_FLAG_REGEX";

  if (const char* langFlagRegexStr =
        this->Makefile->GetDefinition(langFlagRegexVar)) {
    // Filter flags acceptable to this language.
    std::vector<std::string> opts;
    if (const char* targetFlags = target->GetProperty("COMPILE_FLAGS")) {
      cmSystemTools::ParseWindowsCommandLine(targetFlags, opts);
    }
    target->GetCompileOptions(opts, config, lang);
    // (Re-)Escape these flags.  COMPILE_FLAGS were already parsed
    // as a command line above, and COMPILE_OPTIONS are escaped.
    this->AppendCompileOptions(flags, opts, langFlagRegexStr);
  } else {
    // Use all flags.
    if (const char* targetFlags = target->GetProperty("COMPILE_FLAGS")) {
      // COMPILE_FLAGS are not escaped for historical reasons.
      this->AppendFlags(flags, targetFlags);
    }
    std::vector<std::string> opts;
    target->GetCompileOptions(opts, config, lang);
    // COMPILE_OPTIONS are escaped.
    this->AppendCompileOptions(flags, opts);
  }

  for (auto const& it : target->GetMaxLanguageStandards()) {
    const char* standard = target->GetProperty(it.first + "_STANDARD");
    if (!standard) {
      continue;
    }
    if (this->Makefile->IsLaterStandard(it.first, standard, it.second)) {
      std::ostringstream e;
      e << "The COMPILE_FEATURES property of target \"" << target->GetName()
        << "\" was evaluated when computing the link "
           "implementation, and the \""
        << it.first << "_STANDARD\" was \"" << it.second
        << "\" for that computation.  Computing the "
           "COMPILE_FEATURES based on the link implementation resulted in a "
           "higher \""
        << it.first << "_STANDARD\" \"" << standard
        << "\".  "
           "This is not permitted. The COMPILE_FEATURES may not both depend "
           "on "
           "and be depended on by the link implementation."
        << std::endl;
      this->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return;
    }
  }
  this->AddCompilerRequirementFlag(flags, target, lang);
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
    std::string key = "CMAKE_";
    key += lang;
    key += "_STANDARD_INCLUDE_DIRECTORIES";
    std::string const value = this->Makefile->GetSafeDefinition(key);
    cmSystemTools::ExpandListArgument(value, userStandardDirs);
    for (std::string& usd : userStandardDirs) {
      cmSystemTools::ConvertToUnixSlashes(usd);
    }
  }

  // Implicit include directories
  std::vector<std::string> implicitDirs;
  std::set<std::string> implicitSet;
  // Checks if this is not an implicit include directory
  auto notImplicit = [&implicitSet](std::string const& dir) {
    return (implicitSet.find(dir) == implicitSet.end());
  };
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
      std::string key = "CMAKE_";
      key += lang;
      key += "_IMPLICIT_INCLUDE_DIRECTORIES";
      if (const char* value = this->Makefile->GetDefinition(key)) {
        size_t const impDirVecOldSize = impDirVec.size();
        cmSystemTools::ExpandListArgument(value, impDirVec);
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
        std::find(impDirVec.begin(), impDirVec.end(), "/usr/include") ==
          impDirVec.end() &&
        std::find_if(impDirVec.begin(), impDirVec.end(),
                     [](std::string const& d) {
                       return cmHasLiteralSuffix(d, "/usr/include");
                     }) != impDirVec.end()) {
      impDirVec.emplace_back("/usr/include");
    }

    for (std::string const& i : impDirVec) {
      if (implicitSet.insert(i).second) {
        implicitDirs.emplace_back(i);
      }
    }
  }

  // Get the target-specific include directories.
  std::vector<BT<std::string>> userDirs =
    target->GetIncludeDirectories(config, lang);

  // Support putting all the in-project include directories first if
  // it is requested by the project.
  if (this->Makefile->IsOn("CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE")) {
    std::string const &topSourceDir = this->GetState()->GetSourceDirectory(),
                      &topBinaryDir = this->GetState()->GetBinaryDirectory();
    for (BT<std::string> const& udr : userDirs) {
      // Emit this directory only if it is a subdirectory of the
      // top-level source or binary tree.
      if (cmSystemTools::ComparePath(udr.Value, topSourceDir) ||
          cmSystemTools::ComparePath(udr.Value, topBinaryDir) ||
          cmSystemTools::IsSubDirectory(udr.Value, topSourceDir) ||
          cmSystemTools::IsSubDirectory(udr.Value, topBinaryDir)) {
        if (notImplicit(udr.Value)) {
          emitBT(udr);
        }
      }
    }
  }

  // Emit remaining non implicit user direcories.
  for (BT<std::string> const& udr : userDirs) {
    if (notImplicit(udr.Value)) {
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
      if (!notImplicit(udr.Value)) {
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
  this->AppendFlags(
    flags, this->Makefile->GetSafeDefinition("CMAKE_STATIC_LINKER_FLAGS"));
  if (!config.empty()) {
    std::string name = "CMAKE_STATIC_LINKER_FLAGS_" + config;
    this->AppendFlags(flags, this->Makefile->GetSafeDefinition(name));
  }
  this->AppendFlags(flags, target->GetProperty("STATIC_LIBRARY_FLAGS"));
  if (!config.empty()) {
    std::string name = "STATIC_LIBRARY_FLAGS_" + config;
    this->AppendFlags(flags, target->GetProperty(name));
  }

  std::vector<std::string> options;
  target->GetStaticLibraryLinkOptions(options, config, linkLanguage);
  // STATIC_LIBRARY_OPTIONS are escaped.
  this->AppendCompileOptions(flags, options);
}

void cmLocalGenerator::GetTargetFlags(
  cmLinkLineComputer* linkLineComputer, const std::string& config,
  std::string& linkLibs, std::string& flags, std::string& linkFlags,
  std::string& frameworkPath, std::string& linkPath, cmGeneratorTarget* target)
{
  const std::string buildType = cmSystemTools::UpperCase(config);
  cmComputeLinkInformation* pcli = target->GetLinkInformation(config);
  const char* libraryLinkVariable =
    "CMAKE_SHARED_LINKER_FLAGS"; // default to shared library

  const std::string linkLanguage =
    linkLineComputer->GetLinkerLanguage(target, buildType);

  switch (target->GetType()) {
    case cmStateEnums::STATIC_LIBRARY:
      this->GetStaticLibraryFlags(linkFlags, buildType, linkLanguage, target);
      break;
    case cmStateEnums::MODULE_LIBRARY:
      libraryLinkVariable = "CMAKE_MODULE_LINKER_FLAGS";
      CM_FALLTHROUGH;
    case cmStateEnums::SHARED_LIBRARY: {
      linkFlags = this->Makefile->GetSafeDefinition(libraryLinkVariable);
      linkFlags += " ";
      if (!buildType.empty()) {
        std::string build = libraryLinkVariable;
        build += "_";
        build += buildType;
        linkFlags += this->Makefile->GetSafeDefinition(build);
        linkFlags += " ";
      }
      if (this->Makefile->IsOn("WIN32") &&
          !(this->Makefile->IsOn("CYGWIN") || this->Makefile->IsOn("MINGW"))) {
        std::vector<cmSourceFile*> sources;
        target->GetSourceFiles(sources, buildType);
        std::string defFlag =
          this->Makefile->GetSafeDefinition("CMAKE_LINK_DEF_FILE_FLAG");
        for (cmSourceFile* sf : sources) {
          if (sf->GetExtension() == "def") {
            linkFlags += defFlag;
            linkFlags += this->ConvertToOutputFormat(
              cmSystemTools::CollapseFullPath(sf->GetFullPath()), SHELL);
            linkFlags += " ";
          }
        }
      }
      const char* targetLinkFlags = target->GetProperty("LINK_FLAGS");
      if (targetLinkFlags) {
        linkFlags += targetLinkFlags;
        linkFlags += " ";
      }
      if (!buildType.empty()) {
        std::string configLinkFlags = "LINK_FLAGS_";
        configLinkFlags += buildType;
        targetLinkFlags = target->GetProperty(configLinkFlags);
        if (targetLinkFlags) {
          linkFlags += targetLinkFlags;
          linkFlags += " ";
        }
      }
      std::vector<std::string> opts;
      target->GetLinkOptions(opts, config, linkLanguage);
      // LINK_OPTIONS are escaped.
      this->AppendCompileOptions(linkFlags, opts);
      if (pcli) {
        this->OutputLinkLibraries(pcli, linkLineComputer, linkLibs,
                                  frameworkPath, linkPath);
      }
    } break;
    case cmStateEnums::EXECUTABLE: {
      linkFlags += this->Makefile->GetSafeDefinition("CMAKE_EXE_LINKER_FLAGS");
      linkFlags += " ";
      if (!buildType.empty()) {
        std::string build = "CMAKE_EXE_LINKER_FLAGS_";
        build += buildType;
        linkFlags += this->Makefile->GetSafeDefinition(build);
        linkFlags += " ";
      }
      if (linkLanguage.empty()) {
        cmSystemTools::Error(
          "CMake can not determine linker language for target: " +
          target->GetName());
        return;
      }
      this->AddLanguageFlagsForLinking(flags, target, linkLanguage, buildType);
      if (pcli) {
        this->OutputLinkLibraries(pcli, linkLineComputer, linkLibs,
                                  frameworkPath, linkPath);
      }
      if (cmSystemTools::IsOn(
            this->Makefile->GetDefinition("BUILD_SHARED_LIBS"))) {
        std::string sFlagVar = std::string("CMAKE_SHARED_BUILD_") +
          linkLanguage + std::string("_FLAGS");
        linkFlags += this->Makefile->GetSafeDefinition(sFlagVar);
        linkFlags += " ";
      }
      if (target->GetPropertyAsBool("WIN32_EXECUTABLE")) {
        linkFlags +=
          this->Makefile->GetSafeDefinition("CMAKE_CREATE_WIN32_EXE");
        linkFlags += " ";
      } else {
        linkFlags +=
          this->Makefile->GetSafeDefinition("CMAKE_CREATE_CONSOLE_EXE");
        linkFlags += " ";
      }
      if (target->IsExecutableWithExports()) {
        std::string exportFlagVar = "CMAKE_EXE_EXPORTS_";
        exportFlagVar += linkLanguage;
        exportFlagVar += "_FLAG";

        linkFlags += this->Makefile->GetSafeDefinition(exportFlagVar);
        linkFlags += " ";
      }

      std::string cmp0065Flags =
        this->GetLinkLibsCMP0065(linkLanguage, *target);
      if (!cmp0065Flags.empty()) {
        linkFlags += cmp0065Flags;
        linkFlags += " ";
      }

      const char* targetLinkFlags = target->GetProperty("LINK_FLAGS");
      if (targetLinkFlags) {
        linkFlags += targetLinkFlags;
        linkFlags += " ";
      }
      if (!buildType.empty()) {
        std::string configLinkFlags = "LINK_FLAGS_";
        configLinkFlags += buildType;
        targetLinkFlags = target->GetProperty(configLinkFlags);
        if (targetLinkFlags) {
          linkFlags += targetLinkFlags;
          linkFlags += " ";
        }
      }
      std::vector<std::string> opts;
      target->GetLinkOptions(opts, config, linkLanguage);
      // LINK_OPTIONS are escaped.
      this->AppendCompileOptions(linkFlags, opts);
    } break;
    default:
      break;
  }

  this->AppendPositionIndependentLinkerFlags(linkFlags, target, config,
                                             linkLanguage);
  this->AppendIPOLinkerFlags(linkFlags, target, config, linkLanguage);
}

void cmLocalGenerator::GetTargetCompileFlags(cmGeneratorTarget* target,
                                             std::string const& config,
                                             std::string const& lang,
                                             std::string& flags)
{
  cmMakefile* mf = this->GetMakefile();

  // Add language-specific flags.
  this->AddLanguageFlags(flags, target, lang, config);

  this->AddArchitectureFlags(flags, target, lang, config);

  if (lang == "Fortran") {
    this->AppendFlags(flags, this->GetTargetFortranFlags(target, config));
  }

  this->AddCMP0018Flags(flags, target, lang, config);
  this->AddVisibilityPresetFlags(flags, target, lang);
  this->AppendFlags(flags, mf->GetDefineFlags());
  this->AppendFlags(flags, this->GetFrameworkFlags(lang, config, target));
  this->AddCompileOptions(flags, target, lang, config);
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
  const char* fwSearchFlag = mf->GetDefinition(fwSearchFlagVar);
  if (!(fwSearchFlag && *fwSearchFlag)) {
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
      std::string frameworkDir = include;
      frameworkDir += "/../";
      frameworkDir = cmSystemTools::CollapseFullPath(frameworkDir);
      emitted.insert(frameworkDir);
    }
  }

  std::string flags;
  if (cmComputeLinkInformation* cli = target->GetLinkInformation(config)) {
    std::vector<std::string> const& frameworks = cli->GetFrameworkPaths();
    for (std::string const& framework : frameworks) {
      if (emitted.insert(framework).second) {
        flags += fwSearchFlag;
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
  cmComputeLinkInformation& cli = *pcli;

  std::string linkLanguage = cli.GetLinkLanguage();

  const std::string& libPathFlag =
    this->Makefile->GetRequiredDefinition("CMAKE_LIBRARY_PATH_FLAG");
  const std::string& libPathTerminator =
    this->Makefile->GetSafeDefinition("CMAKE_LIBRARY_PATH_TERMINATOR");

  // Add standard libraries for this language.
  std::string standardLibsVar = "CMAKE_";
  standardLibsVar += cli.GetLinkLanguage();
  standardLibsVar += "_STANDARD_LIBRARIES";
  std::string stdLibString;
  if (const char* stdLibs = this->Makefile->GetDefinition(standardLibsVar)) {
    stdLibString = stdLibs;
  }

  // Append the framework search path flags.
  std::string fwSearchFlagVar = "CMAKE_";
  fwSearchFlagVar += linkLanguage;
  fwSearchFlagVar += "_FRAMEWORK_SEARCH_FLAG";
  std::string fwSearchFlag =
    this->Makefile->GetSafeDefinition(fwSearchFlagVar);

  frameworkPath = linkLineComputer->ComputeFrameworkPath(cli, fwSearchFlag);
  linkPath =
    linkLineComputer->ComputeLinkPath(cli, libPathFlag, libPathTerminator);

  linkLibraries = linkLineComputer->ComputeLinkLibraries(cli, stdLibString);
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
        // OLD behavior is to always add the flags
        add_shlib_flags = true;
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        this->IssueMessage(
          MessageType::FATAL_ERROR,
          cmPolicies::GetRequiredPolicyError(cmPolicies::CMP0065));
        CM_FALLTHROUGH;
      case cmPolicies::NEW:
        // NEW behavior is to only add the flags if ENABLE_EXPORTS is on
        add_shlib_flags = tgt.GetPropertyAsBool("ENABLE_EXPORTS");
        break;
    }

    if (add_shlib_flags) {
      std::string linkFlagsVar = "CMAKE_SHARED_LIBRARY_LINK_";
      linkFlagsVar += linkLanguage;
      linkFlagsVar += "_FLAGS";
      linkFlags = this->Makefile->GetSafeDefinition(linkFlagsVar);
    }
  }
  return linkFlags;
}

void cmLocalGenerator::AddArchitectureFlags(std::string& flags,
                                            cmGeneratorTarget const* target,
                                            const std::string& lang,
                                            const std::string& config)
{
  // Only add macOS specific flags on Darwin platforms (macOS and iOS):
  if (this->Makefile->IsOn("APPLE") && this->EmitUniversalBinaryFlags) {
    std::vector<std::string> archs;
    target->GetAppleArchs(config, archs);
    const char* sysroot = this->Makefile->GetDefinition("CMAKE_OSX_SYSROOT");
    if (sysroot && sysroot[0] == '/' && !sysroot[1]) {
      sysroot = nullptr;
    }
    std::string sysrootFlagVar =
      std::string("CMAKE_") + lang + "_SYSROOT_FLAG";
    const char* sysrootFlag = this->Makefile->GetDefinition(sysrootFlagVar);
    const char* deploymentTarget =
      this->Makefile->GetDefinition("CMAKE_OSX_DEPLOYMENT_TARGET");
    std::string deploymentTargetFlagVar =
      std::string("CMAKE_") + lang + "_OSX_DEPLOYMENT_TARGET_FLAG";
    const char* deploymentTargetFlag =
      this->Makefile->GetDefinition(deploymentTargetFlagVar);
    if (!archs.empty() && !lang.empty() &&
        (lang[0] == 'C' || lang[0] == 'F')) {
      for (std::string const& arch : archs) {
        flags += " -arch ";
        flags += arch;
      }
    }

    if (sysrootFlag && *sysrootFlag && sysroot && *sysroot) {
      flags += " ";
      flags += sysrootFlag;
      flags += " ";
      flags += this->ConvertToOutputFormat(sysroot, SHELL);
    }

    if (deploymentTargetFlag && *deploymentTargetFlag && deploymentTarget &&
        *deploymentTarget) {
      flags += " ";
      flags += deploymentTargetFlag;
      flags += deploymentTarget;
    }
  }
}

void cmLocalGenerator::AddLanguageFlags(std::string& flags,
                                        cmGeneratorTarget const* target,
                                        const std::string& lang,
                                        const std::string& config)
{
  // Add language-specific flags.
  std::string flagsVar = "CMAKE_";
  flagsVar += lang;
  flagsVar += "_FLAGS";
  this->AddConfigVariableFlags(flags, flagsVar, config);

  if (target->IsIPOEnabled(lang, config)) {
    this->AppendFeatureOptions(flags, lang, "IPO");
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
    this->AddCompilerRequirementFlag(flags, target, lang);
  }

  this->AddLanguageFlags(flags, target, lang, config);
}

cmGeneratorTarget* cmLocalGenerator::FindGeneratorTargetToUse(
  const std::string& name) const
{
  GeneratorTargetMap::const_iterator imported =
    this->ImportedGeneratorTargets.find(name);
  if (imported != this->ImportedGeneratorTargets.end()) {
    return imported->second;
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
        dep = target->GetLocation(config);
        return true;
      case cmStateEnums::OBJECT_LIBRARY:
        // An object library has no single file on which to depend.
        // This was listed to get the target-level dependency.
        return false;
      case cmStateEnums::INTERFACE_LIBRARY:
        // An interface library has no file on which to depend.
        // This was listed to get the target-level dependency.
        return false;
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
    dep = sf->GetFullPath();
    return true;
  }

  // Treat the name as relative to the source directory in which it
  // was given.
  dep = this->StateSnapshot.GetDirectory().GetCurrentSource();
  dep += "/";
  dep += inName;
  return true;
}

void cmLocalGenerator::AddSharedFlags(std::string& flags,
                                      const std::string& lang, bool shared)
{
  std::string flagsVar;

  // Add flags for dealing with shared libraries for this language.
  if (shared) {
    flagsVar = "CMAKE_SHARED_LIBRARY_";
    flagsVar += lang;
    flagsVar += "_FLAGS";
    this->AppendFlags(flags, this->Makefile->GetDefinition(flagsVar));
  }
}

void cmLocalGenerator::AddCompilerRequirementFlag(
  std::string& flags, cmGeneratorTarget const* target, const std::string& lang)
{
  if (lang.empty()) {
    return;
  }
  const char* defaultStd =
    this->Makefile->GetDefinition("CMAKE_" + lang + "_STANDARD_DEFAULT");
  if (!defaultStd || !*defaultStd) {
    // This compiler has no notion of language standard levels.
    return;
  }
  std::string extProp = lang + "_EXTENSIONS";
  bool ext = true;
  if (const char* extPropValue = target->GetProperty(extProp)) {
    if (cmSystemTools::IsOff(extPropValue)) {
      ext = false;
    }
  }
  std::string stdProp = lang + "_STANDARD";
  const char* standardProp = target->GetProperty(stdProp);
  if (!standardProp) {
    if (ext) {
      // No language standard is specified and extensions are not disabled.
      // Check if this compiler needs a flag to enable extensions.
      std::string const option_flag =
        "CMAKE_" + lang + "_EXTENSION_COMPILE_OPTION";
      if (const char* opt =
            target->Target->GetMakefile()->GetDefinition(option_flag)) {
        std::vector<std::string> optVec;
        cmSystemTools::ExpandListArgument(opt, optVec);
        for (std::string const& i : optVec) {
          this->AppendFlagEscape(flags, i);
        }
      }
    }
    return;
  }

  std::string const type = ext ? "EXTENSION" : "STANDARD";

  if (target->GetPropertyAsBool(lang + "_STANDARD_REQUIRED")) {
    std::string option_flag =
      "CMAKE_" + lang + standardProp + "_" + type + "_COMPILE_OPTION";

    const char* opt =
      target->Target->GetMakefile()->GetDefinition(option_flag);
    if (!opt) {
      std::ostringstream e;
      e << "Target \"" << target->GetName()
        << "\" requires the language "
           "dialect \""
        << lang << standardProp << "\" "
        << (ext ? "(with compiler extensions)" : "")
        << ", but CMake "
           "does not know the compile flags to use to enable it.";
      this->IssueMessage(MessageType::FATAL_ERROR, e.str());
    } else {
      std::vector<std::string> optVec;
      cmSystemTools::ExpandListArgument(opt, optVec);
      for (std::string const& i : optVec) {
        this->AppendFlagEscape(flags, i);
      }
    }
    return;
  }

  static std::map<std::string, std::vector<std::string>> langStdMap;
  if (langStdMap.empty()) {
    // Maintain sorted order, most recent first.
    langStdMap["CXX"].emplace_back("20");
    langStdMap["CXX"].emplace_back("17");
    langStdMap["CXX"].emplace_back("14");
    langStdMap["CXX"].emplace_back("11");
    langStdMap["CXX"].emplace_back("98");

    langStdMap["C"].emplace_back("11");
    langStdMap["C"].emplace_back("99");
    langStdMap["C"].emplace_back("90");

    langStdMap["CUDA"].emplace_back("14");
    langStdMap["CUDA"].emplace_back("11");
    langStdMap["CUDA"].emplace_back("98");
  }

  std::string standard(standardProp);

  std::vector<std::string>& stds = langStdMap[lang];

  std::vector<std::string>::const_iterator stdIt =
    std::find(stds.begin(), stds.end(), standard);
  if (stdIt == stds.end()) {
    std::string e =
      lang + "_STANDARD is set to invalid value '" + standard + "'";
    this->GetGlobalGenerator()->GetCMakeInstance()->IssueMessage(
      MessageType::FATAL_ERROR, e, target->GetBacktrace());
    return;
  }

  std::vector<std::string>::const_iterator defaultStdIt =
    std::find(stds.begin(), stds.end(), defaultStd);
  if (defaultStdIt == stds.end()) {
    std::string e = "CMAKE_" + lang +
      "_STANDARD_DEFAULT is set to invalid value '" + std::string(defaultStd) +
      "'";
    this->IssueMessage(MessageType::INTERNAL_ERROR, e);
    return;
  }

  // If the standard requested is older than the compiler's default
  // then we need to use a flag to change it.  The comparison is
  // greater-or-equal because the standards are stored in backward
  // chronological order.
  if (stdIt >= defaultStdIt) {
    std::string option_flag =
      "CMAKE_" + lang + *stdIt + "_" + type + "_COMPILE_OPTION";

    std::string const& opt =
      target->Target->GetMakefile()->GetRequiredDefinition(option_flag);
    std::vector<std::string> optVec;
    cmSystemTools::ExpandListArgument(opt, optVec);
    for (std::string const& i : optVec) {
      this->AppendFlagEscape(flags, i);
    }
    return;
  }

  // The standard requested is at least as new as the compiler's default,
  // and the standard request is not required.  Decay to the newest standard
  // for which a flag is defined.
  for (; stdIt < defaultStdIt; ++stdIt) {
    std::string option_flag =
      "CMAKE_" + lang + *stdIt + "_" + type + "_COMPILE_OPTION";

    if (const char* opt =
          target->Target->GetMakefile()->GetDefinition(option_flag)) {
      std::vector<std::string> optVec;
      cmSystemTools::ExpandListArgument(opt, optVec);
      for (std::string const& i : optVec) {
        this->AppendFlagEscape(flags, i);
      }
      return;
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
  const char* opt = lg->GetMakefile()->GetDefinition(compileOption);
  if (!opt) {
    return;
  }
  std::string flagDefine = lang + "_VISIBILITY_PRESET";

  const char* prop = target->GetProperty(flagDefine);
  if (!prop) {
    return;
  }
  if (warnCMP0063) {
    *warnCMP0063 += "  " + flagDefine + "\n";
    return;
  }
  if (strcmp(prop, "hidden") != 0 && strcmp(prop, "default") != 0 &&
      strcmp(prop, "protected") != 0 && strcmp(prop, "internal") != 0) {
    std::ostringstream e;
    e << "Target " << target->GetName() << " uses unsupported value \"" << prop
      << "\" for " << flagDefine << ".";
    cmSystemTools::Error(e.str());
    return;
  }
  std::string option = std::string(opt) + prop;
  lg->AppendFlags(flags, option);
}

static void AddInlineVisibilityCompileOption(std::string& flags,
                                             cmGeneratorTarget const* target,
                                             cmLocalGenerator* lg,
                                             std::string* warnCMP0063)
{
  std::string compileOption =
    "CMAKE_CXX_COMPILE_OPTIONS_VISIBILITY_INLINES_HIDDEN";
  const char* opt = lg->GetMakefile()->GetDefinition(compileOption);
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
  lg->AppendFlags(flags, opt);
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

  if (lang == "CXX") {
    AddInlineVisibilityCompileOption(flags, target, this, pWarnCMP0063);
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
    std::string flagsVar = "CMAKE_SHARED_LIBRARY_";
    flagsVar += lang;
    flagsVar += "_FLAGS";
    std::string const& flags = this->Makefile->GetSafeDefinition(flagsVar);

    if (flags != originalFlags) {
      switch (this->GetPolicyStatus(cmPolicies::CMP0018)) {
        case cmPolicies::WARN: {
          std::ostringstream e;
          e << "Variable " << flagsVar
            << " has been modified. CMake "
               "will ignore the POSITION_INDEPENDENT_CODE target property for "
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
    std::string flagsVar = "CMAKE_";
    flagsVar += lang;
    flagsVar += "_COMPILE_OPTIONS_PIE";
    picFlags = this->Makefile->GetSafeDefinition(flagsVar);
  }
  if (picFlags.empty()) {
    std::string flagsVar = "CMAKE_";
    flagsVar += lang;
    flagsVar += "_COMPILE_OPTIONS_PIC";
    picFlags = this->Makefile->GetSafeDefinition(flagsVar);
  }
  if (!picFlags.empty()) {
    std::vector<std::string> options;
    cmSystemTools::ExpandListArgument(picFlags, options);
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
  std::string flagsVar = var;
  this->AppendFlags(flags, this->Makefile->GetDefinition(flagsVar));
  // Add the flags from the build-type specific variable.
  if (!config.empty()) {
    flagsVar += "_";
    flagsVar += cmSystemTools::UpperCase(config);
    this->AppendFlags(flags, this->Makefile->GetDefinition(flagsVar));
  }
}

void cmLocalGenerator::AppendFlags(std::string& flags,
                                   const std::string& newFlags) const
{
  if (!newFlags.empty()) {
    if (!flags.empty()) {
      flags += " ";
    }
    flags += newFlags;
  }
}

void cmLocalGenerator::AppendFlags(std::string& flags,
                                   const char* newFlags) const
{
  if (newFlags && *newFlags) {
    this->AppendFlags(flags, std::string(newFlags));
  }
}

void cmLocalGenerator::AppendFlagEscape(std::string& flags,
                                        const std::string& rawFlag) const
{
  this->AppendFlags(flags, this->EscapeForShell(rawFlag));
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
  const char* rawFlagsList = this->Makefile->GetDefinition(name);
  if (rawFlagsList == nullptr) {
    return;
  }

  std::vector<std::string> flagsList;
  cmSystemTools::ExpandListArgument(rawFlagsList, flagsList);
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

  const std::string mode = cmSystemTools::IsOn(PICValue) ? "PIE" : "NO_PIE";

  std::string supported = "CMAKE_" + lang + "_LINK_" + mode + "_SUPPORTED";
  if (cmSystemTools::IsOff(this->Makefile->GetDefinition(supported))) {
    return;
  }

  std::string name = "CMAKE_" + lang + "_LINK_OPTIONS_" + mode;

  auto pieFlags = this->Makefile->GetSafeDefinition(name);
  if (pieFlags.empty()) {
    return;
  }

  std::vector<std::string> flagsList;
  cmSystemTools::ExpandListArgument(pieFlags, flagsList);
  for (const auto& flag : flagsList) {
    this->AppendFlagEscape(flags, flag);
  }
}

void cmLocalGenerator::AppendCompileOptions(std::string& options,
                                            const char* options_list,
                                            const char* regex) const
{
  // Short-circuit if there are no options.
  if (!options_list) {
    return;
  }

  // Expand the list of options.
  std::vector<std::string> options_vec;
  cmSystemTools::ExpandListArgument(options_list, options_vec);
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

void cmLocalGenerator::AppendIncludeDirectories(
  std::vector<std::string>& includes, const char* includes_list,
  const cmSourceFile& sourceFile) const
{
  // Short-circuit if there are no includes.
  if (!includes_list) {
    return;
  }

  // Expand the list of includes.
  std::vector<std::string> includes_vec;
  cmSystemTools::ExpandListArgument(includes_list, includes_vec);
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

    if (!cmSystemTools::IsOff(inc)) {
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
    std::string defineFlagVar = "CMAKE_";
    defineFlagVar += lang;
    defineFlagVar += "_DEFINE_FLAG";
    const char* df = this->Makefile->GetDefinition(defineFlagVar);
    if (df && *df) {
      dflag = df;
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
      for (const char* c = define.c_str(); *c; ++c) {
        if (*c == '$' || *c == '#') {
          def += '$';
        }
        def += *c;
      }
    } else {
      // Make the definition appear properly on the command line.  Use
      // -DNAME="value" instead of -D"NAME=value" for historical reasons.
      std::string::size_type eq = define.find('=');
      def += define.substr(0, eq);
      if (eq != std::string::npos) {
        def += "=";
        def += this->EscapeForShell(define.c_str() + eq + 1, true);
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
  std::string optVar = "CMAKE_";
  optVar += lang;
  optVar += "_COMPILE_OPTIONS_";
  optVar += feature;
  if (const char* optionList = this->Makefile->GetDefinition(optVar)) {
    std::vector<std::string> options;
    cmSystemTools::ExpandListArgument(optionList, options);
    for (std::string const& o : options) {
      this->AppendFlagEscape(flags, o);
    }
  }
}

const char* cmLocalGenerator::GetFeature(const std::string& feature,
                                         const std::string& config)
{
  std::string featureName = feature;
  // TODO: Define accumulation policy for features (prepend, append, replace).
  // Currently we always replace.
  if (!config.empty()) {
    featureName += "_";
    featureName += cmSystemTools::UpperCase(config);
  }
  cmStateSnapshot snp = this->StateSnapshot;
  while (snp.IsValid()) {
    if (const char* value = snp.GetDirectory().GetProperty(featureName)) {
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
  cmCustomCommandGenerator const& ccg, const char* default_comment)
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
                                const char* dest, bool implib)
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
  const std::vector<cmGeneratorTarget*>& tgts = this->GetGeneratorTargets();
  for (cmGeneratorTarget* l : tgts) {
    if (l->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
      continue;
    }

    // Include the user-specified pre-install script for this target.
    if (const char* preinstall = l->GetProperty("PRE_INSTALL_SCRIPT")) {
      cmInstallScriptGenerator g(preinstall, false, nullptr, false);
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
          cmInstallTargetGeneratorLocal g(this, l->GetName(),
                                          destination.c_str(), false);
          g.Generate(os, config, configurationTypes);
        } break;
        case cmStateEnums::SHARED_LIBRARY: {
#if defined(_WIN32) || defined(__CYGWIN__)
          // Special code to handle DLL.  Install the import library
          // to the normal destination and the DLL to the runtime
          // destination.
          cmInstallTargetGeneratorLocal g1(this, l->GetName(),
                                           destination.c_str(), true);
          g1.Generate(os, config, configurationTypes);
          // We also skip over the leading slash given by the user.
          destination = l->Target->GetRuntimeInstallPath().substr(1);
          cmSystemTools::ConvertToUnixSlashes(destination);
          cmInstallTargetGeneratorLocal g2(this, l->GetName(),
                                           destination.c_str(), false);
          g2.Generate(os, config, configurationTypes);
#else
          // Use a target install generator.
          cmInstallTargetGeneratorLocal g(this, l->GetName(),
                                          destination.c_str(), false);
          g.Generate(os, config, configurationTypes);
#endif
        } break;
        default:
          break;
      }
    }

    // Include the user-specified post-install script for this target.
    if (const char* postinstall = l->GetProperty("POST_INSTALL_SCRIPT")) {
      cmInstallScriptGenerator g(postinstall, false, nullptr, false);
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
    std::string md5name = md5.HashString(objName.substr(0, pos));
    md5name += objName.substr(pos);
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
  std::map<std::string, std::string>::iterator it =
    this->UniqueObjectNamesMap.find(sin);

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
  if ((relSource && !relBinary) || (subSource && !subBinary)) {
    objectName = relFromSource;
  } else if ((relBinary && !relSource) || (subBinary && !subSource)) {
    objectName = relFromBinary;
  } else if (relFromBinary.length() < relFromSource.length()) {
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
        std::string repVar = "CMAKE_";
        repVar += lang;
        repVar += "_OUTPUT_EXTENSION_REPLACE";
        replaceExt = this->Makefile->IsOn(repVar);
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
    if (const char* value =
          this->Makefile->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY")) {
      switch (sscanf(value, "%u.%u.%u", &major, &minor, &patch)) {
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
      return false;
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
  if (const char* val = target->GetProperty(prop)) {
    mf->AddDefinition(prop, val);
  }
}

void cmLocalGenerator::GenerateAppleInfoPList(cmGeneratorTarget* target,
                                              const std::string& targetName,
                                              const std::string& fname)
{
  // Find the Info.plist template.
  const char* in = target->GetProperty("MACOSX_BUNDLE_INFO_PLIST");
  std::string inFile = (in && *in) ? in : "MacOSXBundleInfo.plist.in";
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
  mf->AddDefinition("MACOSX_BUNDLE_EXECUTABLE_NAME", targetName.c_str());
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
  const char* in = target->GetProperty("MACOSX_FRAMEWORK_INFO_PLIST");
  std::string inFile = (in && *in) ? in : "MacOSXFrameworkInfo.plist.in";
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
  mf->AddDefinition("MACOSX_FRAMEWORK_NAME", targetName.c_str());
  cmLGInfoProp(mf, target, "MACOSX_FRAMEWORK_ICON_FILE");
  cmLGInfoProp(mf, target, "MACOSX_FRAMEWORK_IDENTIFIER");
  cmLGInfoProp(mf, target, "MACOSX_FRAMEWORK_SHORT_VERSION_STRING");
  cmLGInfoProp(mf, target, "MACOSX_FRAMEWORK_BUNDLE_VERSION");
  mf->ConfigureFile(inFile, fname, false, false, false);
}
