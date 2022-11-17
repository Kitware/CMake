/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallExportGenerator.h"

#include <map>
#include <sstream>
#include <utility>

#include <cm/memory>

#ifndef CMAKE_BOOTSTRAP
#  include "cmExportInstallAndroidMKGenerator.h"
#endif
#include "cmExportInstallFileGenerator.h"
#include "cmExportSet.h"
#include "cmInstallType.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmInstallExportGenerator::cmInstallExportGenerator(
  cmExportSet* exportSet, std::string const& destination,
  std::string file_permissions, std::vector<std::string> const& configurations,
  std::string const& component, MessageLevel message, bool exclude_from_all,
  std::string filename, std::string name_space,
  std::string cxx_modules_directory, bool exportOld, bool android,
  cmListFileBacktrace backtrace)
  : cmInstallGenerator(destination, configurations, component, message,
                       exclude_from_all, false, std::move(backtrace))
  , ExportSet(exportSet)
  , FilePermissions(std::move(file_permissions))
  , FileName(std::move(filename))
  , Namespace(std::move(name_space))
  , CxxModulesDirectory(std::move(cxx_modules_directory))
  , ExportOld(exportOld)
{
  if (android) {
#ifndef CMAKE_BOOTSTRAP
    this->EFGen = cm::make_unique<cmExportInstallAndroidMKGenerator>(this);
#endif
  } else {
    this->EFGen = cm::make_unique<cmExportInstallFileGenerator>(this);
  }
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

#ifndef CMAKE_BOOTSTRAP
  path += '/';
  // Replace the destination path with a hash to keep it short.
  path += cmSystemTools::ComputeStringMD5(this->Destination);
#endif

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
    e << "INSTALL(EXPORT) given unknown export \""
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
  this->EFGen->SetExportOld(this->ExportOld);
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
  auto cxx_module_dest =
    cmStrCat(this->Destination, '/', this->CxxModulesDirectory);
  std::string config_file_example;
  for (auto const& i : this->EFGen->GetConfigCxxModuleFiles()) {
    config_file_example = i.second;
    break;
  }
  if (!config_file_example.empty()) {
    // Remove old per-configuration export files if the main changes.
    std::string installedDir = cmStrCat(
      "$ENV{DESTDIR}", ConvertToAbsoluteDestination(cxx_module_dest), '/');
    std::string installedFile = cmStrCat(installedDir, "/cxx-modules.cmake");
    std::string toInstallFile =
      cmStrCat(cmSystemTools::GetFilenamePath(config_file_example),
               "/cxx-modules.cmake");
    os << indent << "if(EXISTS \"" << installedFile << "\")\n";
    Indent indentN = indent.Next();
    Indent indentNN = indentN.Next();
    Indent indentNNN = indentNN.Next();
    /* clang-format off */
    os << indentN << "file(DIFFERENT _cmake_export_file_changed FILES\n"
       << indentN << "     \"" << installedFile << "\"\n"
       << indentN << "     \"" << toInstallFile << "\")\n";
    os << indentN << "if(_cmake_export_file_changed)\n";
    os << indentNN << "file(GLOB _cmake_old_config_files \"" << installedDir
       << this->EFGen->GetConfigImportFileGlob() << "\")\n";
    os << indentNN << "if(_cmake_old_config_files)\n";
    os << indentNNN << "string(REPLACE \";\" \", \" _cmake_old_config_files_text \"${_cmake_old_config_files}\")\n";
    os << indentNNN << R"(message(STATUS "Old C++ module export file \")" << installedFile
       << "\\\" will be replaced.  Removing files [${_cmake_old_config_files_text}].\")\n";
    os << indentNNN << "unset(_cmake_old_config_files_text)\n";
    os << indentNNN << "file(REMOVE ${_cmake_old_config_files})\n";
    os << indentNN << "endif()\n";
    os << indentNN << "unset(_cmake_old_config_files)\n";
    os << indentN << "endif()\n";
    os << indentN << "unset(_cmake_export_file_changed)\n";
    os << indent << "endif()\n";
    /* clang-format on */

    // All of these files are siblings; get its location to know where the
    // "anchor" file is.
    files.push_back(toInstallFile);
    this->AddInstallRule(os, cxx_module_dest, cmInstallType_FILES, files,
                         false, this->FilePermissions.c_str(), nullptr,
                         nullptr, nullptr, indent);
    files.clear();
  }
  for (auto const& i : this->EFGen->GetConfigCxxModuleFiles()) {
    files.push_back(i.second);
    std::string config_test = this->CreateConfigTest(i.first);
    os << indent << "if(" << config_test << ")\n";
    this->AddInstallRule(os, cxx_module_dest, cmInstallType_FILES, files,
                         false, this->FilePermissions.c_str(), nullptr,
                         nullptr, nullptr, indent.Next());
    os << indent << "endif()\n";
    files.clear();
  }
  for (auto const& i : this->EFGen->GetConfigCxxModuleTargetFiles()) {
    std::string config_test = this->CreateConfigTest(i.first);
    os << indent << "if(" << config_test << ")\n";
    this->AddInstallRule(os, cxx_module_dest, cmInstallType_FILES, i.second,
                         false, this->FilePermissions.c_str(), nullptr,
                         nullptr, nullptr, indent.Next());
    os << indent << "endif()\n";
    files.clear();
  }
}

void cmInstallExportGenerator::GenerateScriptActions(std::ostream& os,
                                                     Indent indent)
{
  // Remove old per-configuration export files if the main changes.
  std::string installedDir = cmStrCat(
    "$ENV{DESTDIR}", ConvertToAbsoluteDestination(this->Destination), '/');
  std::string installedFile = cmStrCat(installedDir, this->FileName);
  os << indent << "if(EXISTS \"" << installedFile << "\")\n";
  Indent indentN = indent.Next();
  Indent indentNN = indentN.Next();
  Indent indentNNN = indentNN.Next();
  /* clang-format off */
  os << indentN << "file(DIFFERENT _cmake_export_file_changed FILES\n"
     << indentN << "     \"" << installedFile << "\"\n"
     << indentN << "     \"" << this->MainImportFile << "\")\n";
  os << indentN << "if(_cmake_export_file_changed)\n";
  os << indentNN << "file(GLOB _cmake_old_config_files \"" << installedDir
     << this->EFGen->GetConfigImportFileGlob() << "\")\n";
  os << indentNN << "if(_cmake_old_config_files)\n";
  os << indentNNN << "string(REPLACE \";\" \", \" _cmake_old_config_files_text \"${_cmake_old_config_files}\")\n";
  os << indentNNN << R"(message(STATUS "Old export file \")" << installedFile
     << "\\\" will be replaced.  Removing files [${_cmake_old_config_files_text}].\")\n";
  os << indentNNN << "unset(_cmake_old_config_files_text)\n";
  os << indentNNN << "file(REMOVE ${_cmake_old_config_files})\n";
  os << indentNN << "endif()\n";
  os << indentNN << "unset(_cmake_old_config_files)\n";
  os << indentN << "endif()\n";
  os << indentN << "unset(_cmake_export_file_changed)\n";
  os << indent << "endif()\n";
  /* clang-format on */

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
