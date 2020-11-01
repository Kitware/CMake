/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExtraKateGenerator.h"

#include <cstring>
#include <memory>
#include <ostream>
#include <set>
#include <vector>

#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmProperty.h"
#include "cmSourceFile.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmExtraKateGenerator::cmExtraKateGenerator() = default;

cmExternalMakefileProjectGeneratorFactory* cmExtraKateGenerator::GetFactory()
{
  static cmExternalMakefileProjectGeneratorSimpleFactory<cmExtraKateGenerator>
    factory("Kate", "Generates Kate project files.");

  if (factory.GetSupportedGlobalGenerators().empty()) {
#if defined(_WIN32)
    factory.AddSupportedGlobalGenerator("MinGW Makefiles");
    factory.AddSupportedGlobalGenerator("NMake Makefiles");
// disable until somebody actually tests it:
// factory.AddSupportedGlobalGenerator("MSYS Makefiles");
#endif
    factory.AddSupportedGlobalGenerator("Ninja");
    factory.AddSupportedGlobalGenerator("Unix Makefiles");
  }

  return &factory;
}

void cmExtraKateGenerator::Generate()
{
  const auto& lg = this->GlobalGenerator->GetLocalGenerators()[0];
  const cmMakefile* mf = lg->GetMakefile();
  this->ProjectName = this->GenerateProjectName(
    lg->GetProjectName(), mf->GetSafeDefinition("CMAKE_BUILD_TYPE"),
    this->GetPathBasename(lg->GetBinaryDirectory()));
  this->UseNinja = (this->GlobalGenerator->GetName() == "Ninja");

  this->CreateKateProjectFile(*lg);
  this->CreateDummyKateProjectFile(*lg);
}

void cmExtraKateGenerator::CreateKateProjectFile(
  const cmLocalGenerator& lg) const
{
  std::string filename = cmStrCat(lg.GetBinaryDirectory(), "/.kateproject");
  cmGeneratedFileStream fout(filename);
  if (!fout) {
    return;
  }

  /* clang-format off */
  fout <<
    "{\n"
    "\t\"name\": \"" << this->ProjectName << "\",\n"
    "\t\"directory\": \"" << lg.GetSourceDirectory() << "\",\n"
    "\t\"files\": [ { " << this->GenerateFilesString(lg) << "} ],\n";
  /* clang-format on */
  this->WriteTargets(lg, fout);
  fout << "}\n";
}

void cmExtraKateGenerator::WriteTargets(const cmLocalGenerator& lg,
                                        cmGeneratedFileStream& fout) const
{
  cmMakefile const* mf = lg.GetMakefile();
  const std::string& make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  const std::string& makeArgs =
    mf->GetSafeDefinition("CMAKE_KATE_MAKE_ARGUMENTS");
  std::string const& homeOutputDir = lg.GetBinaryDirectory();

  /* clang-format off */
  fout <<
  "\t\"build\": {\n"
  "\t\t\"directory\": \"" << homeOutputDir << "\",\n"
  "\t\t\"default_target\": \"all\",\n"
  "\t\t\"clean_target\": \"clean\",\n";
  /* clang-format on */

  // build, clean and quick are for the build plugin kate <= 4.12:
  fout << "\t\t\"build\": \"" << make << " -C \\\"" << homeOutputDir << "\\\" "
       << makeArgs << " "
       << "all\",\n";
  fout << "\t\t\"clean\": \"" << make << " -C \\\"" << homeOutputDir << "\\\" "
       << makeArgs << " "
       << "clean\",\n";
  fout << "\t\t\"quick\": \"" << make << " -C \\\"" << homeOutputDir << "\\\" "
       << makeArgs << " "
       << "install\",\n";

  // this is for kate >= 4.13:
  fout << "\t\t\"targets\":[\n";

  this->AppendTarget(fout, "all", make, makeArgs, homeOutputDir,
                     homeOutputDir);
  this->AppendTarget(fout, "clean", make, makeArgs, homeOutputDir,
                     homeOutputDir);

  // add all executable and library targets and some of the GLOBAL
  // and UTILITY targets
  for (const auto& localGen : this->GlobalGenerator->GetLocalGenerators()) {
    const auto& targets = localGen->GetGeneratorTargets();
    std::string currentDir = localGen->GetCurrentBinaryDirectory();
    bool topLevel = (currentDir == localGen->GetBinaryDirectory());

    for (const auto& target : targets) {
      std::string const& targetName = target->GetName();
      switch (target->GetType()) {
        case cmStateEnums::GLOBAL_TARGET: {
          bool insertTarget = false;
          // Only add the global targets from CMAKE_BINARY_DIR,
          // not from the subdirs
          if (topLevel) {
            insertTarget = true;
            // only add the "edit_cache" target if it's not ccmake, because
            // this will not work within the IDE
            if (targetName == "edit_cache") {
              cmProp editCommand =
                localGen->GetMakefile()->GetDefinition("CMAKE_EDIT_COMMAND");
              if (editCommand == nullptr ||
                  strstr(editCommand->c_str(), "ccmake") != nullptr) {
                insertTarget = false;
              }
            }
          }
          if (insertTarget) {
            this->AppendTarget(fout, targetName, make, makeArgs, currentDir,
                               homeOutputDir);
          }
        } break;
        case cmStateEnums::UTILITY:
          // Add all utility targets, except the Nightly/Continuous/
          // Experimental-"sub"targets as e.g. NightlyStart
          if ((cmHasLiteralPrefix(targetName, "Nightly") &&
               (targetName != "Nightly")) ||
              (cmHasLiteralPrefix(targetName, "Continuous") &&
               (targetName != "Continuous")) ||
              (cmHasLiteralPrefix(targetName, "Experimental") &&
               (targetName != "Experimental"))) {
            break;
          }

          this->AppendTarget(fout, targetName, make, makeArgs, currentDir,
                             homeOutputDir);
          break;
        case cmStateEnums::EXECUTABLE:
        case cmStateEnums::STATIC_LIBRARY:
        case cmStateEnums::SHARED_LIBRARY:
        case cmStateEnums::MODULE_LIBRARY:
        case cmStateEnums::OBJECT_LIBRARY: {
          this->AppendTarget(fout, targetName, make, makeArgs, currentDir,
                             homeOutputDir);
          std::string fastTarget = cmStrCat(targetName, "/fast");
          this->AppendTarget(fout, fastTarget, make, makeArgs, currentDir,
                             homeOutputDir);

        } break;
        default:
          break;
      }
    }

    // insert rules for compiling, preprocessing and assembling individual
    // files
    std::vector<std::string> objectFileTargets;
    localGen->GetIndividualFileTargets(objectFileTargets);
    for (std::string const& f : objectFileTargets) {
      this->AppendTarget(fout, f, make, makeArgs, currentDir, homeOutputDir);
    }
  }

  fout << "\t] }\n";
}

void cmExtraKateGenerator::AppendTarget(cmGeneratedFileStream& fout,
                                        const std::string& target,
                                        const std::string& make,
                                        const std::string& makeArgs,
                                        const std::string& path,
                                        const std::string& homeOutputDir) const
{
  static char JsonSep = ' ';

  fout << "\t\t\t" << JsonSep << R"({"name":")" << target
       << "\", "
          "\"build_cmd\":\""
       << make << " -C \\\"" << (this->UseNinja ? homeOutputDir : path)
       << "\\\" " << makeArgs << " " << target << "\"}\n";

  JsonSep = ',';
}

void cmExtraKateGenerator::CreateDummyKateProjectFile(
  const cmLocalGenerator& lg) const
{
  std::string filename =
    cmStrCat(lg.GetBinaryDirectory(), '/', this->ProjectName, ".kateproject");
  cmGeneratedFileStream fout(filename);
  if (!fout) {
    return;
  }

  fout << "#Generated by " << cmSystemTools::GetCMakeCommand()
       << ", do not edit.\n";
}

std::string cmExtraKateGenerator::GenerateFilesString(
  const cmLocalGenerator& lg) const
{
  std::string s = cmStrCat(lg.GetSourceDirectory(), "/.git");
  if (cmSystemTools::FileExists(s)) {
    return "\"git\": 1 ";
  }

  s = cmStrCat(lg.GetSourceDirectory(), "/.svn");
  if (cmSystemTools::FileExists(s)) {
    return "\"svn\": 1 ";
  }

  s = cmStrCat(lg.GetSourceDirectory(), '/');

  std::set<std::string> files;
  std::string tmp;
  const auto& lgs = this->GlobalGenerator->GetLocalGenerators();

  for (const auto& lgen : lgs) {
    cmMakefile* makefile = lgen->GetMakefile();
    const std::vector<std::string>& listFiles = makefile->GetListFiles();
    for (std::string const& listFile : listFiles) {
      tmp = listFile;
      {
        files.insert(tmp);
      }
    }

    for (const auto& sf : makefile->GetSourceFiles()) {
      if (sf->GetIsGenerated()) {
        continue;
      }

      tmp = sf->ResolveFullPath();
      files.insert(tmp);
    }
  }

  const char* sep = "";
  tmp = "\"list\": [";
  for (std::string const& f : files) {
    tmp += sep;
    tmp += " \"";
    tmp += f;
    tmp += "\"";
    sep = ",";
  }
  tmp += "] ";

  return tmp;
}

std::string cmExtraKateGenerator::GenerateProjectName(
  const std::string& name, const std::string& type,
  const std::string& path) const
{
  return name + (type.empty() ? "" : "-") + type + '@' + path;
}

std::string cmExtraKateGenerator::GetPathBasename(
  const std::string& path) const
{
  std::string outputBasename = path;
  while (!outputBasename.empty() &&
         (outputBasename.back() == '/' || outputBasename.back() == '\\')) {
    outputBasename.resize(outputBasename.size() - 1);
  }
  std::string::size_type loc = outputBasename.find_last_of("/\\");
  if (loc != std::string::npos) {
    outputBasename = outputBasename.substr(loc + 1);
  }

  return outputBasename;
}
