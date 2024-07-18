/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallExportGenerator.h"

#include <map>
#include <sstream>
#include <utility>

#include "cmCryptoHash.h"
#include "cmExportInstallFileGenerator.h"
#include "cmExportSet.h"
#include "cmInstallType.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmScriptGenerator.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmInstallExportGenerator::cmInstallExportGenerator(
  cmExportSet* exportSet, std::string destination, std::string filePermissions,
  std::vector<std::string> const& configurations, std::string component,
  MessageLevel message, bool excludeFromAll, std::string filename,
  std::string targetNamespace, std::string cxxModulesDirectory,
  cmListFileBacktrace backtrace)
  : cmInstallGenerator(std::move(destination), configurations,
                       std::move(component), message, excludeFromAll, false,
                       std::move(backtrace))
  , ExportSet(exportSet)
  , FilePermissions(std::move(filePermissions))
  , FileName(std::move(filename))
  , Namespace(std::move(targetNamespace))
  , CxxModulesDirectory(std::move(cxxModulesDirectory))
{
  exportSet->AddInstallation(this);
}

cmInstallExportGenerator::~cmInstallExportGenerator() = default;

bool cmInstallExportGenerator::Compute(cmLocalGenerator* lg)
{
  this->LocalGenerator = lg;
  return this->ExportSet->Compute(lg);
}

std::string cmInstallExportGenerator::TempDirCalculate() const
{
  // Choose a temporary directory in which to generate the import
  // files to be installed.
  std::string path = cmStrCat(
    this->LocalGenerator->GetCurrentBinaryDirectory(), "/CMakeFiles/Export");
  if (this->Destination.empty()) {
    return path;
  }

  cmCryptoHash hasher(cmCryptoHash::AlgoMD5);
  path += '/';
  // Replace the destination path with a hash to keep it short.
  path += hasher.HashString(this->Destination);

  return path;
}

void cmInstallExportGenerator::ComputeTempDir()
{
  this->TempDir = this->TempDirCalculate();
}

std::string cmInstallExportGenerator::GetTempDir() const
{
  if (this->TempDir.empty()) {
    return this->TempDirCalculate();
  }
  return this->TempDir;
}

void cmInstallExportGenerator::GenerateScript(std::ostream& os)
{
  // Skip empty sets.
  if (this->ExportSet->GetTargetExports().empty()) {
    std::ostringstream e;
    e << "INSTALL(" << this->InstallSubcommand() << ") given unknown export \""
      << this->ExportSet->GetName() << "\"";
    cmSystemTools::Error(e.str());
    return;
  }

  // Create the temporary directory in which to store the files.
  this->ComputeTempDir();
  cmSystemTools::MakeDirectory(this->TempDir);

  // Construct a temporary location for the file.
  this->MainImportFile = cmStrCat(this->TempDir, '/', this->FileName);

  // Generate the import file for this export set.
  this->EFGen->SetExportFile(this->MainImportFile.c_str());
  this->EFGen->SetNamespace(this->Namespace);
  if (this->ConfigurationTypes->empty()) {
    if (!this->ConfigurationName.empty()) {
      this->EFGen->AddConfiguration(this->ConfigurationName);
    } else {
      this->EFGen->AddConfiguration("");
    }
  } else {
    for (std::string const& c : *this->ConfigurationTypes) {
      this->EFGen->AddConfiguration(c);
    }
  }
  this->EFGen->GenerateImportFile();

  // Perform the main install script generation.
  this->cmInstallGenerator::GenerateScript(os);
}

void cmInstallExportGenerator::GenerateScriptConfigs(std::ostream& os,
                                                     Indent indent)
{
  // Create the main install rules first.
  this->cmInstallGenerator::GenerateScriptConfigs(os, indent);

  // Now create a configuration-specific install rule for the import
  // file of each configuration.
  std::vector<std::string> files;
  for (auto const& i : this->EFGen->GetConfigImportFiles()) {
    files.push_back(i.second);
    std::string config_test = this->CreateConfigTest(i.first);
    os << indent << "if(" << config_test << ")\n";
    this->AddInstallRule(os, this->Destination, cmInstallType_FILES, files,
                         false, this->FilePermissions.c_str(), nullptr,
                         nullptr, nullptr, indent.Next());
    os << indent << "endif()\n";
    files.clear();
  }

  // Now create a configuration-specific install rule for the C++ module import
  // property file of each configuration.
  auto const cxxModuleDestination =
    cmStrCat(this->Destination, '/', this->CxxModulesDirectory);
  auto const cxxModuleInstallFilePath = this->EFGen->GetCxxModuleFile();
  auto const configImportFilesGlob = this->EFGen->GetConfigImportFileGlob();
  if (!cxxModuleInstallFilePath.empty() && !configImportFilesGlob.empty()) {
    auto const cxxModuleFilename =
      cmSystemTools::GetFilenameName(cxxModuleInstallFilePath);

    // Remove old per-configuration export files if the main changes.
    std::string installedDir =
      cmStrCat("$ENV{DESTDIR}",
               ConvertToAbsoluteDestination(cxxModuleDestination), '/');
    std::string installedFile = cmStrCat(installedDir, cxxModuleFilename);
    os << indent << "if(EXISTS \"" << installedFile << "\")\n";
    Indent indentN = indent.Next();
    Indent indentNN = indentN.Next();
    Indent indentNNN = indentNN.Next();
    os << indentN << "file(DIFFERENT _cmake_export_file_changed FILES\n"
       << indentN << "     \"" << installedFile << "\"\n"
       << indentN << "     \"" << cxxModuleInstallFilePath << "\")\n";
    os << indentN << "if(_cmake_export_file_changed)\n";
    os << indentNN << "file(GLOB _cmake_old_config_files \"" << installedDir
       << configImportFilesGlob << "\")\n";
    os << indentNN << "if(_cmake_old_config_files)\n";
    os << indentNNN
       << "string(REPLACE \";\" \", \" _cmake_old_config_files_text "
          "\"${_cmake_old_config_files}\")\n";
    os << indentNNN << R"(message(STATUS "Old C++ module export file \")"
       << installedFile
       << "\\\" will be replaced.  "
          "Removing files [${_cmake_old_config_files_text}].\")\n";
    os << indentNNN << "unset(_cmake_old_config_files_text)\n";
    os << indentNNN << "file(REMOVE ${_cmake_old_config_files})\n";
    os << indentNN << "endif()\n";
    os << indentNN << "unset(_cmake_old_config_files)\n";
    os << indentN << "endif()\n";
    os << indentN << "unset(_cmake_export_file_changed)\n";
    os << indent << "endif()\n";

    // All of these files are siblings; get its location to know where the
    // "anchor" file is.
    files.push_back(cxxModuleInstallFilePath);
    this->AddInstallRule(os, cxxModuleDestination, cmInstallType_FILES, files,
                         false, this->FilePermissions.c_str(), nullptr,
                         nullptr, nullptr, indent);
    files.clear();
  }
  for (auto const& i : this->EFGen->GetConfigCxxModuleFiles()) {
    files.push_back(i.second);
    std::string config_test = this->CreateConfigTest(i.first);
    os << indent << "if(" << config_test << ")\n";
    this->AddInstallRule(os, cxxModuleDestination, cmInstallType_FILES, files,
                         false, this->FilePermissions.c_str(), nullptr,
                         nullptr, nullptr, indent.Next());
    os << indent << "endif()\n";
    files.clear();
  }
  for (auto const& i : this->EFGen->GetConfigCxxModuleTargetFiles()) {
    std::string config_test = this->CreateConfigTest(i.first);
    os << indent << "if(" << config_test << ")\n";
    this->AddInstallRule(os, cxxModuleDestination, cmInstallType_FILES,
                         i.second, false, this->FilePermissions.c_str(),
                         nullptr, nullptr, nullptr, indent.Next());
    os << indent << "endif()\n";
    files.clear();
  }
}

void cmInstallExportGenerator::GenerateScriptActions(std::ostream& os,
                                                     Indent indent)
{
  auto const configImportFilesGlob = this->EFGen->GetConfigImportFileGlob();
  if (!configImportFilesGlob.empty()) {
    // Remove old per-configuration export files if the main changes.
    std::string installedDir = cmStrCat(
      "$ENV{DESTDIR}", ConvertToAbsoluteDestination(this->Destination), '/');
    std::string installedFile = cmStrCat(installedDir, this->FileName);
    os << indent << "if(EXISTS \"" << installedFile << "\")\n";
    Indent indentN = indent.Next();
    Indent indentNN = indentN.Next();
    Indent indentNNN = indentNN.Next();
    os << indentN << "file(DIFFERENT _cmake_export_file_changed FILES\n"
       << indentN << "     \"" << installedFile << "\"\n"
       << indentN << "     \"" << this->MainImportFile << "\")\n";
    os << indentN << "if(_cmake_export_file_changed)\n";
    os << indentNN << "file(GLOB _cmake_old_config_files \"" << installedDir
       << configImportFilesGlob << "\")\n";
    os << indentNN << "if(_cmake_old_config_files)\n";
    os << indentNNN
       << "string(REPLACE \";\" \", \" _cmake_old_config_files_text "
          "\"${_cmake_old_config_files}\")\n";
    os << indentNNN << R"(message(STATUS "Old export file \")" << installedFile
       << "\\\" will be replaced.  "
          "Removing files [${_cmake_old_config_files_text}].\")\n";
    os << indentNNN << "unset(_cmake_old_config_files_text)\n";
    os << indentNNN << "file(REMOVE ${_cmake_old_config_files})\n";
    os << indentNN << "endif()\n";
    os << indentNN << "unset(_cmake_old_config_files)\n";
    os << indentN << "endif()\n";
    os << indentN << "unset(_cmake_export_file_changed)\n";
    os << indent << "endif()\n";
  }

  // Install the main export file.
  std::vector<std::string> files;
  files.push_back(this->MainImportFile);
  this->AddInstallRule(os, this->Destination, cmInstallType_FILES, files,
                       false, this->FilePermissions.c_str(), nullptr, nullptr,
                       nullptr, indent);
}

std::string cmInstallExportGenerator::GetDestinationFile() const
{
  return this->Destination + '/' + this->FileName;
}
