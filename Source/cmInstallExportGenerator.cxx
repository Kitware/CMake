/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallExportGenerator.h"

#include <algorithm>
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
#include "cmLocalGenerator.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmInstallExportGenerator::cmInstallExportGenerator(
  cmExportSet* exportSet, std::string const& destination,
  std::string file_permissions, std::vector<std::string> const& configurations,
  std::string const& component, MessageLevel message, bool exclude_from_all,
  std::string filename, std::string name_space, bool exportOld, bool android)
  : cmInstallGenerator(destination, configurations, component, message,
                       exclude_from_all)
  , ExportSet(exportSet)
  , FilePermissions(std::move(file_permissions))
  , FileName(std::move(filename))
  , Namespace(std::move(name_space))
  , ExportOld(exportOld)
  , LocalGenerator(nullptr)
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
  this->ExportSet->Compute(lg);
  return true;
}

void cmInstallExportGenerator::ComputeTempDir()
{
  // Choose a temporary directory in which to generate the import
  // files to be installed.
  this->TempDir = cmStrCat(this->LocalGenerator->GetCurrentBinaryDirectory(),
                           "/CMakeFiles/Export");
  if (this->Destination.empty()) {
    return;
  }
  this->TempDir += "/";

  // Enforce a maximum length.
  bool useMD5 = false;
#if defined(_WIN32) || defined(__CYGWIN__)
  std::string::size_type const max_total_len = 250;
#else
  std::string::size_type const max_total_len = 1000;
#endif
  // Will generate files of the form "<temp-dir>/<base>-<config>.<ext>".
  std::string::size_type const len = this->TempDir.size() + 1 +
    this->FileName.size() + 1 + this->GetMaxConfigLength();
  if (len < max_total_len) {
    // Keep the total path length below the limit.
    std::string::size_type const max_len = max_total_len - len;
    if (this->Destination.size() > max_len) {
      useMD5 = true;
    }
  } else {
    useMD5 = true;
  }
  if (useMD5) {
    // Replace the destination path with a hash to keep it short.
    this->TempDir += cmSystemTools::ComputeStringMD5(this->Destination);
  } else {
    std::string dest = this->Destination;
    // Avoid unix full paths.
    if (dest[0] == '/') {
      dest[0] = '_';
    }
    // Avoid windows full paths by removing colons.
    std::replace(dest.begin(), dest.end(), ':', '_');
    // Avoid relative paths that go up the tree.
    cmSystemTools::ReplaceString(dest, "../", "__/");
    // Avoid spaces.
    std::replace(dest.begin(), dest.end(), ' ', '_');
    this->TempDir += dest;
  }
}

size_t cmInstallExportGenerator::GetMaxConfigLength() const
{
  // Always use at least 8 for "noconfig".
  size_t len = 8;
  if (this->ConfigurationTypes->empty()) {
    if (this->ConfigurationName.size() > 8) {
      len = this->ConfigurationName.size();
    }
  } else {
    for (std::string const& c : *this->ConfigurationTypes) {
      if (c.size() > len) {
        len = c.size();
      }
    }
  }
  return len;
}

void cmInstallExportGenerator::GenerateScript(std::ostream& os)
{
  // Skip empty sets.
  if (ExportSet->GetTargetExports().empty()) {
    std::ostringstream e;
    e << "INSTALL(EXPORT) given unknown export \"" << ExportSet->GetName()
      << "\"";
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
}

void cmInstallExportGenerator::GenerateScriptActions(std::ostream& os,
                                                     Indent indent)
{
  // Remove old per-configuration export files if the main changes.
  std::string installedDir =
    cmStrCat("$ENV{DESTDIR}",
             this->ConvertToAbsoluteDestination(this->Destination), '/');
  std::string installedFile = cmStrCat(installedDir, this->FileName);
  os << indent << "if(EXISTS \"" << installedFile << "\")\n";
  Indent indentN = indent.Next();
  Indent indentNN = indentN.Next();
  Indent indentNNN = indentNN.Next();
  /* clang-format off */
  os << indentN << "file(DIFFERENT EXPORT_FILE_CHANGED FILES\n"
     << indentN << "     \"" << installedFile << "\"\n"
     << indentN << "     \"" << this->MainImportFile << "\")\n";
  os << indentN << "if(EXPORT_FILE_CHANGED)\n";
  os << indentNN << "file(GLOB OLD_CONFIG_FILES \"" << installedDir
     << this->EFGen->GetConfigImportFileGlob() << "\")\n";
  os << indentNN << "if(OLD_CONFIG_FILES)\n";
  os << indentNNN << R"(message(STATUS "Old export file \")" << installedFile
     << "\\\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].\")\n";
  os << indentNNN << "file(REMOVE ${OLD_CONFIG_FILES})\n";
  os << indentNN << "endif()\n";
  os << indentN << "endif()\n";
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
