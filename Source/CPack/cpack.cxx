/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cmext/algorithm>

#include "cmsys/Encoding.hxx"

#include "cmCMakePresetsGraph.h"
#include "cmCPackGenerator.h"
#include "cmCPackGeneratorFactory.h"
#include "cmCPackLog.h"
#include "cmCommandLineArgument.h"
#include "cmConsoleBuf.h"
#include "cmDocumentation.h"
#include "cmDocumentationEntry.h"
#include "cmGlobalGenerator.h"
#include "cmJSONState.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStateSnapshot.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmake.h"

namespace {
const cmDocumentationEntry cmDocumentationName = {
  {},
  "  cpack - Packaging driver provided by CMake."
};

const cmDocumentationEntry cmDocumentationUsage = { {}, "  cpack [options]" };

const cmDocumentationEntry cmDocumentationOptions[14] = {
  { "-G <generators>", "Override/define CPACK_GENERATOR" },
  { "-C <Configuration>", "Specify the project configuration" },
  { "-D <var>=<value>", "Set a CPack variable." },
  { "--config <configFile>", "Specify the config file." },
  { "-V,--verbose", "Enable verbose output" },
  { "--trace", "Put underlying cmake scripts in trace mode." },
  { "--trace-expand", "Put underlying cmake scripts in expanded trace mode." },
  { "--debug", "Enable debug output (for CPack developers)" },
  { "-P <packageName>", "Override/define CPACK_PACKAGE_NAME" },
  { "-R <packageVersion>", "Override/define CPACK_PACKAGE_VERSION" },
  { "-B <packageDirectory>", "Override/define CPACK_PACKAGE_DIRECTORY" },
  { "--vendor <vendorName>", "Override/define CPACK_PACKAGE_VENDOR" },
  { "--preset", "Read arguments from a package preset" },
  { "--list-presets", "List available package presets" }
};

void cpackProgressCallback(const std::string& message, float /*unused*/)
{
  std::cout << "-- " << message << '\n';
}

std::vector<cmDocumentationEntry> makeGeneratorDocs(
  const cmCPackGeneratorFactory& gf)
{
  const auto& generators = gf.GetGeneratorsList();

  std::vector<cmDocumentationEntry> docs;
  docs.reserve(generators.size());

  std::transform(
    generators.cbegin(), generators.cend(), std::back_inserter(docs),
    [](const std::decay<decltype(generators)>::type::value_type& gen) {
      return cmDocumentationEntry{ gen.first, gen.second };
    });
  return docs;
}

} // namespace

// this is CPack.
int main(int argc, char const* const* argv)
{
  cmSystemTools::EnsureStdPipes();

  // Replace streambuf so we can output Unicode to console
  cmConsoleBuf consoleBuf;
  consoleBuf.SetUTF8Pipes();

  cmsys::Encoding::CommandLineArguments args =
    cmsys::Encoding::CommandLineArguments::Main(argc, argv);
  argc = args.argc();
  argv = args.argv();

  std::vector<std::string> inputArgs;
  inputArgs.reserve(argc - 1);
  cm::append(inputArgs, argv + 1, argv + argc);

  cmSystemTools::InitializeLibUV();
  cmSystemTools::FindCMakeResources(argv[0]);
  cmCPackLog log;

  log.SetErrorPrefix("CPack Error: ");
  log.SetWarningPrefix("CPack Warning: ");
  log.SetOutputPrefix("CPack: ");
  log.SetVerbosePrefix("CPack Verbose: ");

  if (cmSystemTools::GetCurrentWorkingDirectory().empty()) {
    cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                "Current working directory cannot be established.\n");
    return 1;
  }

  std::string generator;
  bool help = false;
  bool helpVersion = false;
  std::string helpFull;
  std::string helpMAN;
  std::string helpHTML;

  std::string cpackProjectName;
  std::string cpackProjectDirectory;
  std::string cpackBuildConfig;
  std::string cpackProjectVersion;
  std::string cpackProjectPatch;
  std::string cpackProjectVendor;
  std::string cpackConfigFile;

  std::string preset;
  bool listPresets = false;

  std::map<std::string, std::string> definitions;

  auto const verboseLambda = [&log](const std::string&, cmake*,
                                    cmMakefile*) -> bool {
    log.SetVerbose(true);
    cmCPack_Log(&log, cmCPackLog::LOG_OUTPUT, "Enable Verbose\n");
    return true;
  };

  auto const debugLambda = [&log](const std::string&, cmake*,
                                  cmMakefile*) -> bool {
    log.SetDebug(true);
    cmCPack_Log(&log, cmCPackLog::LOG_OUTPUT, "Enable Debug\n");
    return true;
  };

  auto const traceLambda = [](const std::string&, cmake* state,
                              cmMakefile*) -> bool {
    state->SetTrace(true);
    return true;
  };

  auto const traceExpandLambda = [](const std::string&, cmake* state,
                                    cmMakefile*) -> bool {
    state->SetTrace(true);
    state->SetTraceExpand(true);
    return true;
  };

  using CommandArgument =
    cmCommandLineArgument<bool(std::string const&, cmake*, cmMakefile*)>;

  std::vector<CommandArgument> arguments = {
    CommandArgument{ "--help", CommandArgument::Values::Zero,
                     CommandArgument::setToTrue(help) },
    CommandArgument{ "--help-full", CommandArgument::Values::Zero,
                     CommandArgument::setToValue(helpFull) },
    CommandArgument{ "--help-html", CommandArgument::Values::Zero,
                     CommandArgument::setToValue(helpHTML) },
    CommandArgument{ "--help-man", CommandArgument::Values::Zero,
                     CommandArgument::setToValue(helpMAN) },
    CommandArgument{ "--version", CommandArgument::Values::Zero,
                     CommandArgument::setToTrue(helpVersion) },
    CommandArgument{ "-V", CommandArgument::Values::Zero, verboseLambda },
    CommandArgument{ "--verbose", CommandArgument::Values::Zero,
                     verboseLambda },
    CommandArgument{ "--debug", CommandArgument::Values::Zero, debugLambda },
    CommandArgument{ "--config", CommandArgument::Values::One,
                     CommandArgument::setToValue(cpackConfigFile) },
    CommandArgument{ "--trace", CommandArgument::Values::Zero, traceLambda },
    CommandArgument{ "--trace-expand", CommandArgument::Values::Zero,
                     traceExpandLambda },
    CommandArgument{ "-C", CommandArgument::Values::One,
                     CommandArgument::setToValue(cpackBuildConfig) },
    CommandArgument{ "-G", CommandArgument::Values::One,
                     CommandArgument::setToValue(generator) },
    CommandArgument{ "-P", CommandArgument::Values::One,
                     CommandArgument::setToValue(cpackProjectName) },
    CommandArgument{ "-R", CommandArgument::Values::One,
                     CommandArgument::setToValue(cpackProjectVersion) },
    CommandArgument{ "-B", CommandArgument::Values::One,
                     CommandArgument::setToValue(cpackProjectDirectory) },
    CommandArgument{ "--patch", CommandArgument::Values::One,
                     CommandArgument::setToValue(cpackProjectPatch) },
    CommandArgument{ "--vendor", CommandArgument::Values::One,
                     CommandArgument::setToValue(cpackProjectVendor) },
    CommandArgument{ "--preset", CommandArgument::Values::One,
                     CommandArgument::setToValue(preset) },
    CommandArgument{ "--list-presets", CommandArgument::Values::Zero,
                     CommandArgument::setToTrue(listPresets) },
    CommandArgument{ "-D", CommandArgument::Values::One,
                     [&log, &definitions](const std::string& arg, cmake*,
                                          cmMakefile*) -> bool {
                       std::string value = arg;
                       size_t pos = value.find_first_of('=');
                       if (pos == std::string::npos) {
                         cmCPack_Log(
                           &log, cmCPackLog::LOG_ERROR,
                           "Please specify CPack definitions as: KEY=VALUE\n");
                         return false;
                       }
                       std::string key = value.substr(0, pos);
                       value.erase(0, pos + 1);
                       definitions[key] = value;
                       cmCPack_Log(&log, cmCPackLog::LOG_DEBUG,
                                   "Set CPack variable: " << key << " to \""
                                                          << value << "\"\n");
                       return true;
                     } },
  };

  cmake cminst(cmake::RoleScript, cmState::CPack);
  cminst.SetHomeDirectory("");
  cminst.SetHomeOutputDirectory("");
  cminst.SetProgressCallback(cpackProgressCallback);
  cminst.GetCurrentSnapshot().SetDefaultDefinitions();
  cmGlobalGenerator cmgg(&cminst);
  cmMakefile globalMF(&cmgg, cminst.GetCurrentSnapshot());

  bool parsed = true;
  for (std::size_t i = 0; i < inputArgs.size(); i++) {
    auto const& arg = inputArgs[i];
    for (auto const& m : arguments) {
      if (m.matches(arg)) {
        if (!m.parse(arg, i, inputArgs, &cminst, &globalMF)) {
          parsed = false;
        }
        break;
      }
    }
  }

  cmCPackGeneratorFactory generators;
  generators.SetLogger(&log);

  // Set up presets
  if (!preset.empty() || listPresets) {
    const auto workingDirectory = cmSystemTools::GetCurrentWorkingDirectory();

    auto const presetGeneratorsPresent =
      [&generators](const cmCMakePresetsGraph::PackagePreset& p) {
        return std::all_of(p.Generators.begin(), p.Generators.end(),
                           [&generators](const std::string& gen) {
                             return generators.GetGeneratorsList().count(
                                      gen) != 0;
                           });
      };

    cmCMakePresetsGraph presetsGraph;
    auto result = presetsGraph.ReadProjectPresets(workingDirectory);
    if (result != true) {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                  "Could not read presets from "
                    << workingDirectory << ":"
                    << presetsGraph.parseState.GetErrorMessage() << '\n');
      return 1;
    }

    if (listPresets) {
      presetsGraph.PrintPackagePresetList(presetGeneratorsPresent);
      return 0;
    }

    auto presetPair = presetsGraph.PackagePresets.find(preset);
    if (presetPair == presetsGraph.PackagePresets.end()) {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                  "No such package preset in " << workingDirectory << ": \""
                                               << preset << "\"\n");
      presetsGraph.PrintPackagePresetList(presetGeneratorsPresent);
      return 1;
    }

    if (presetPair->second.Unexpanded.Hidden) {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                  "Cannot use hidden package preset in "
                    << workingDirectory << ": \"" << preset << "\"\n");
      presetsGraph.PrintPackagePresetList(presetGeneratorsPresent);
      return 1;
    }

    auto const& expandedPreset = presetPair->second.Expanded;
    if (!expandedPreset) {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                  "Could not evaluate package preset \""
                    << preset << "\": Invalid macro expansion\n");
      presetsGraph.PrintPackagePresetList(presetGeneratorsPresent);
      return 1;
    }

    if (!expandedPreset->ConditionResult) {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                  "Cannot use disabled package preset in "
                    << workingDirectory << ": \"" << preset << "\"\n");
      presetsGraph.PrintPackagePresetList(presetGeneratorsPresent);
      return 1;
    }

    if (!presetGeneratorsPresent(presetPair->second.Unexpanded)) {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR, "Cannot use preset");
      presetsGraph.PrintPackagePresetList(presetGeneratorsPresent);
      return 1;
    }

    auto configurePresetPair =
      presetsGraph.ConfigurePresets.find(expandedPreset->ConfigurePreset);
    if (configurePresetPair == presetsGraph.ConfigurePresets.end()) {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                  "No such configure preset in "
                    << workingDirectory << ": \""
                    << expandedPreset->ConfigurePreset << "\"\n");
      presetsGraph.PrintConfigurePresetList();
      return 1;
    }

    if (configurePresetPair->second.Unexpanded.Hidden) {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                  "Cannot use hidden configure preset in "
                    << workingDirectory << ": \""
                    << expandedPreset->ConfigurePreset << "\"\n");
      presetsGraph.PrintConfigurePresetList();
      return 1;
    }

    auto const& expandedConfigurePreset = configurePresetPair->second.Expanded;
    if (!expandedConfigurePreset) {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                  "Could not evaluate configure preset \""
                    << expandedPreset->ConfigurePreset
                    << "\": Invalid macro expansion\n");
      return 1;
    }

    cmSystemTools::ChangeDirectory(expandedConfigurePreset->BinaryDir);

    auto presetEnvironment = expandedPreset->Environment;
    for (auto const& var : presetEnvironment) {
      if (var.second) {
        cmSystemTools::PutEnv(cmStrCat(var.first, '=', *var.second));
      }
    }

    if (!expandedPreset->ConfigFile.empty() && cpackConfigFile.empty()) {
      cpackConfigFile = expandedPreset->ConfigFile;
    }

    if (!expandedPreset->Generators.empty() && generator.empty()) {
      generator = cmJoin(expandedPreset->Generators, ";");
    }

    if (!expandedPreset->Configurations.empty() && cpackBuildConfig.empty()) {
      cpackBuildConfig = cmJoin(expandedPreset->Configurations, ";");
    }

    definitions.insert(expandedPreset->Variables.begin(),
                       expandedPreset->Variables.end());

    if (expandedPreset->DebugOutput == true) {
      debugLambda("", &cminst, &globalMF);
    }

    if (expandedPreset->VerboseOutput == true) {
      verboseLambda("", &cminst, &globalMF);
    }

    if (!expandedPreset->PackageName.empty() && cpackProjectName.empty()) {
      cpackProjectName = expandedPreset->PackageName;
    }

    if (!expandedPreset->PackageVersion.empty() &&
        cpackProjectVersion.empty()) {
      cpackProjectVersion = expandedPreset->PackageVersion;
    }

    if (!expandedPreset->PackageDirectory.empty() &&
        cpackProjectDirectory.empty()) {
      cpackProjectDirectory = expandedPreset->PackageDirectory;
    }

    if (!expandedPreset->VendorName.empty() && cpackProjectVendor.empty()) {
      cpackProjectVendor = expandedPreset->VendorName;
    }
  }

  cmCPack_Log(&log, cmCPackLog::LOG_VERBOSE,
              "Read CPack config file: " << cpackConfigFile << '\n');

  bool cpackConfigFileSpecified = true;
  if (cpackConfigFile.empty()) {
    cpackConfigFile = cmStrCat(cmSystemTools::GetCurrentWorkingDirectory(),
                               "/CPackConfig.cmake");
    cpackConfigFileSpecified = false;
  }

  cmDocumentation doc;
  doc.addCPackStandardDocSections();
  /* Were we invoked to display doc or to do some work ?
   * Unlike cmake launching cpack with zero argument
   * should launch cpack using "cpackConfigFile" if it exists
   * in the current directory.
   */
  help = doc.CheckOptions(argc, argv, "-G") && argc != 1;

  // This part is used for cpack documentation lookup as well.
  cminst.AddCMakePaths();

  if (parsed && !help) {
    // find out which system cpack is running on, so it can setup the search
    // paths, so FIND_XXX() commands can be used in scripts
    std::string systemFile =
      globalMF.GetModulesFile("CMakeDetermineSystem.cmake");
    if (!globalMF.ReadListFile(systemFile)) {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                  "Error reading CMakeDetermineSystem.cmake\n");
      return 1;
    }

    systemFile =
      globalMF.GetModulesFile("CMakeSystemSpecificInformation.cmake");
    if (!globalMF.ReadListFile(systemFile)) {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                  "Error reading CMakeSystemSpecificInformation.cmake\n");
      return 1;
    }

    if (!cpackBuildConfig.empty()) {
      globalMF.AddDefinition("CPACK_BUILD_CONFIG", cpackBuildConfig);
    }

    if (cmSystemTools::FileExists(cpackConfigFile)) {
      cpackConfigFile = cmSystemTools::CollapseFullPath(cpackConfigFile);
      cmCPack_Log(&log, cmCPackLog::LOG_VERBOSE,
                  "Read CPack configuration file: " << cpackConfigFile
                                                    << '\n');
      if (!globalMF.ReadListFile(cpackConfigFile)) {
        cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                    "Problem reading CPack config file: \"" << cpackConfigFile
                                                            << "\"\n");
        return 1;
      }
    } else if (cpackConfigFileSpecified) {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                  "Cannot find CPack config file: \"" << cpackConfigFile
                                                      << "\"\n");
      return 1;
    }

    if (!generator.empty()) {
      globalMF.AddDefinition("CPACK_GENERATOR", generator);
    }
    if (!cpackProjectName.empty()) {
      globalMF.AddDefinition("CPACK_PACKAGE_NAME", cpackProjectName);
    }
    if (!cpackProjectVersion.empty()) {
      globalMF.AddDefinition("CPACK_PACKAGE_VERSION", cpackProjectVersion);
    }
    if (!cpackProjectVendor.empty()) {
      globalMF.AddDefinition("CPACK_PACKAGE_VENDOR", cpackProjectVendor);
    }
    // if this is not empty it has been set on the command line
    // go for it. Command line override values set in config file.
    if (!cpackProjectDirectory.empty()) {
      globalMF.AddDefinition("CPACK_PACKAGE_DIRECTORY", cpackProjectDirectory);
    }
    // The value has not been set on the command line
    else {
      // get a default value (current working directory)
      cpackProjectDirectory = cmSystemTools::GetCurrentWorkingDirectory();
      // use default value if no value has been provided by the config file
      if (!globalMF.IsSet("CPACK_PACKAGE_DIRECTORY")) {
        globalMF.AddDefinition("CPACK_PACKAGE_DIRECTORY",
                               cpackProjectDirectory);
      }
    }
    for (auto const& cd : definitions) {
      globalMF.AddDefinition(cd.first, cd.second);
    }

    // Force CPACK_PACKAGE_DIRECTORY as absolute path
    cpackProjectDirectory =
      globalMF.GetSafeDefinition("CPACK_PACKAGE_DIRECTORY");
    cpackProjectDirectory =
      cmSystemTools::CollapseFullPath(cpackProjectDirectory);
    globalMF.AddDefinition("CPACK_PACKAGE_DIRECTORY", cpackProjectDirectory);

    cmValue cpackModulesPath = globalMF.GetDefinition("CPACK_MODULE_PATH");
    if (cpackModulesPath) {
      globalMF.AddDefinition("CMAKE_MODULE_PATH", *cpackModulesPath);
    }
    cmValue genList = globalMF.GetDefinition("CPACK_GENERATOR");
    if (!genList) {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                  "CPack generator not specified\n");
    } else {
      cmList generatorsList{ *genList };
      for (std::string const& gen : generatorsList) {
        cmMakefile::ScopePushPop raii(&globalMF);
        cmMakefile* mf = &globalMF;
        cmCPack_Log(&log, cmCPackLog::LOG_VERBOSE,
                    "Specified generator: " << gen << '\n');
        if (!mf->GetDefinition("CPACK_PACKAGE_NAME")) {
          cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                      "CPack project name not specified" << '\n');
          parsed = false;
        }
        if (parsed &&
            !(mf->GetDefinition("CPACK_PACKAGE_VERSION") ||
              (mf->GetDefinition("CPACK_PACKAGE_VERSION_MAJOR") &&
               mf->GetDefinition("CPACK_PACKAGE_VERSION_MINOR") &&
               mf->GetDefinition("CPACK_PACKAGE_VERSION_PATCH")))) {
          cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                      "CPack project version not specified\n"
                      "Specify CPACK_PACKAGE_VERSION, or "
                      "CPACK_PACKAGE_VERSION_MAJOR, "
                      "CPACK_PACKAGE_VERSION_MINOR, and "
                      "CPACK_PACKAGE_VERSION_PATCH.\n");
          parsed = false;
        }
        if (parsed) {
          std::unique_ptr<cmCPackGenerator> cpackGenerator =
            generators.NewGenerator(gen);
          if (cpackGenerator) {
            cpackGenerator->SetTrace(cminst.GetTrace());
            cpackGenerator->SetTraceExpand(cminst.GetTraceExpand());
          } else {
            cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                        "Could not create CPack generator: " << gen << '\n');
            // Print out all the valid generators
            cmDocumentation generatorDocs;
            generatorDocs.SetSection("Generators",
                                     makeGeneratorDocs(generators));
            std::cerr << '\n';
            generatorDocs.PrintDocumentation(cmDocumentation::ListGenerators,
                                             std::cerr);
            parsed = false;
          }

          if (parsed && !cpackGenerator->Initialize(gen, mf)) {
            cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                        "Cannot initialize the generator " << gen << '\n');
            parsed = false;
          }

          if (!mf->GetDefinition("CPACK_INSTALL_COMMANDS") &&
              !mf->GetDefinition("CPACK_INSTALL_SCRIPT") &&
              !mf->GetDefinition("CPACK_INSTALLED_DIRECTORIES") &&
              !mf->GetDefinition("CPACK_INSTALL_CMAKE_PROJECTS")) {
            cmCPack_Log(
              &log, cmCPackLog::LOG_ERROR,
              "Please specify build tree of the project that uses CMake "
              "using CPACK_INSTALL_CMAKE_PROJECTS, specify "
              "CPACK_INSTALL_COMMANDS, CPACK_INSTALL_SCRIPT, or "
              "CPACK_INSTALLED_DIRECTORIES.\n");
            parsed = false;
          }
          if (parsed) {
            cmValue projName = mf->GetDefinition("CPACK_PACKAGE_NAME");
            cmCPack_Log(&log, cmCPackLog::LOG_VERBOSE,
                        "Use generator: " << cpackGenerator->GetNameOfClass()
                                          << '\n');
            cmCPack_Log(&log, cmCPackLog::LOG_VERBOSE,
                        "For project: " << *projName << '\n');

            cmValue projVersion = mf->GetDefinition("CPACK_PACKAGE_VERSION");
            if (!projVersion) {
              cmValue projVersionMajor =
                mf->GetDefinition("CPACK_PACKAGE_VERSION_MAJOR");
              cmValue projVersionMinor =
                mf->GetDefinition("CPACK_PACKAGE_VERSION_MINOR");
              cmValue projVersionPatch =
                mf->GetDefinition("CPACK_PACKAGE_VERSION_PATCH");
              std::ostringstream ostr;
              ostr << *projVersionMajor << "." << *projVersionMinor << '.'
                   << *projVersionPatch;
              mf->AddDefinition("CPACK_PACKAGE_VERSION", ostr.str());
            }

            int res = cpackGenerator->DoPackage();
            if (!res) {
              cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                          "Error when generating package: " << *projName
                                                            << '\n');
              return 1;
            }
          }
        }
      }
    }
  }

  /* In this case we are building the documentation object
   * instance in order to create appropriate structure
   * in order to satisfy the appropriate --help-xxx request
   */
  if (help) {
    // Construct and print requested documentation.
    doc.SetName("cpack");
    doc.SetSection("Name", cmDocumentationName);
    doc.SetSection("Usage", cmDocumentationUsage);
    doc.PrependSection("Options", cmDocumentationOptions);
    doc.SetSection("Generators", makeGeneratorDocs(generators));
    return !doc.PrintRequestedDocumentation(std::cout);
  }

  return int(cmSystemTools::GetErrorOccurredFlag());
}
