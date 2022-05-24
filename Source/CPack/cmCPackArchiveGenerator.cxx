/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCPackArchiveGenerator.h"

#include <cstring>
#include <map>
#include <ostream>
#include <utility>
#include <vector>

#include "cmCPackComponentGroup.h"
#include "cmCPackGenerator.h"
#include "cmCPackLog.h"
#include "cmGeneratedFileStream.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmWorkingDirectory.h"

cmCPackGenerator* cmCPackArchiveGenerator::Create7ZGenerator()
{
  return new cmCPackArchiveGenerator(cmArchiveWrite::CompressNone, "7zip",
                                     ".7z");
}

cmCPackGenerator* cmCPackArchiveGenerator::CreateTBZ2Generator()
{
  return new cmCPackArchiveGenerator(cmArchiveWrite::CompressBZip2, "paxr",
                                     ".tar.bz2");
}

cmCPackGenerator* cmCPackArchiveGenerator::CreateTGZGenerator()
{
  return new cmCPackArchiveGenerator(cmArchiveWrite::CompressGZip, "paxr",
                                     ".tar.gz");
}

cmCPackGenerator* cmCPackArchiveGenerator::CreateTXZGenerator()
{
  return new cmCPackArchiveGenerator(cmArchiveWrite::CompressXZ, "paxr",
                                     ".tar.xz");
}

cmCPackGenerator* cmCPackArchiveGenerator::CreateTZGenerator()
{
  return new cmCPackArchiveGenerator(cmArchiveWrite::CompressCompress, "paxr",
                                     ".tar.Z");
}

cmCPackGenerator* cmCPackArchiveGenerator::CreateTZSTGenerator()
{
  return new cmCPackArchiveGenerator(cmArchiveWrite::CompressZstd, "paxr",
                                     ".tar.zst");
}

cmCPackGenerator* cmCPackArchiveGenerator::CreateZIPGenerator()
{
  return new cmCPackArchiveGenerator(cmArchiveWrite::CompressNone, "zip",
                                     ".zip");
}

cmCPackArchiveGenerator::cmCPackArchiveGenerator(
  cmArchiveWrite::Compress compress, std::string format, std::string extension)
  : Compress(compress)
  , ArchiveFormat(std::move(format))
  , OutputExtension(std::move(extension))
{
}

cmCPackArchiveGenerator::~cmCPackArchiveGenerator() = default;

std::string cmCPackArchiveGenerator::GetArchiveComponentFileName(
  const std::string& component, bool isGroupName)
{
  std::string componentUpper(cmSystemTools::UpperCase(component));
  std::string packageFileName;

  if (this->IsSet("CPACK_ARCHIVE_" + componentUpper + "_FILE_NAME")) {
    packageFileName +=
      *this->GetOption("CPACK_ARCHIVE_" + componentUpper + "_FILE_NAME");
  } else if (this->IsSet("CPACK_ARCHIVE_FILE_NAME")) {
    packageFileName += this->GetComponentPackageFileName(
      this->GetOption("CPACK_ARCHIVE_FILE_NAME"), component, isGroupName);
  } else {
    packageFileName += this->GetComponentPackageFileName(
      this->GetOption("CPACK_PACKAGE_FILE_NAME"), component, isGroupName);
  }

  packageFileName += this->GetOutputExtension();

  return packageFileName;
}

int cmCPackArchiveGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", "1");
  return this->Superclass::InitializeInternal();
}

int cmCPackArchiveGenerator::addOneComponentToArchive(
  cmArchiveWrite& archive, cmCPackComponent* component)
{
  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "   - packaging component: " << component->Name << std::endl);
  // Add the files of this component to the archive
  std::string localToplevel(this->GetOption("CPACK_TEMPORARY_DIRECTORY"));
  localToplevel += "/" + component->Name;
  // Change to local toplevel
  cmWorkingDirectory workdir(localToplevel);
  if (workdir.Failed()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Failed to change working directory to "
                    << localToplevel << " : "
                    << std::strerror(workdir.GetLastResult()) << std::endl);
    return 0;
  }
  std::string filePrefix;
  if (this->IsOn("CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY")) {
    filePrefix = cmStrCat(this->GetOption("CPACK_PACKAGE_FILE_NAME"), '/');
  }
  cmValue installPrefix = this->GetOption("CPACK_PACKAGING_INSTALL_PREFIX");
  if (installPrefix && installPrefix->size() > 1 &&
      (*installPrefix)[0] == '/') {
    // add to file prefix and remove the leading '/'
    filePrefix += installPrefix->substr(1);
    filePrefix += "/";
  }
  for (std::string const& file : component->Files) {
    std::string rp = filePrefix + file;
    cmCPackLogger(cmCPackLog::LOG_DEBUG, "Adding file: " << rp << std::endl);
    archive.Add(rp, 0, nullptr, false);
    if (!archive) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "ERROR while packaging files: " << archive.GetError()
                                                    << std::endl);
      return 0;
    }
  }
  return 1;
}

/*
 * The macro will open/create a file 'filename'
 * an declare and open the associated
 * cmArchiveWrite 'archive' object.
 */
#define DECLARE_AND_OPEN_ARCHIVE(filename, archive)                           \
  cmGeneratedFileStream gf;                                                   \
  gf.Open((filename), false, true);                                           \
  if (!GenerateHeader(&gf)) {                                                 \
    cmCPackLogger(cmCPackLog::LOG_ERROR,                                      \
                  "Problem to generate Header for archive <"                  \
                    << (filename) << ">." << std::endl);                      \
    return 0;                                                                 \
  }                                                                           \
  cmArchiveWrite archive(gf, this->Compress, this->ArchiveFormat, 0,          \
                         this->GetThreadCount());                             \
  do {                                                                        \
    if (!archive.Open()) {                                                    \
      cmCPackLogger(cmCPackLog::LOG_ERROR,                                    \
                    "Problem to open archive <"                               \
                      << (filename) << ">, ERROR = " << (archive).GetError()  \
                      << std::endl);                                          \
      return 0;                                                               \
    }                                                                         \
    if (!(archive)) {                                                         \
      cmCPackLogger(cmCPackLog::LOG_ERROR,                                    \
                    "Problem to create archive <"                             \
                      << (filename) << ">, ERROR = " << (archive).GetError()  \
                      << std::endl);                                          \
      return 0;                                                               \
    }                                                                         \
  } while (false)

int cmCPackArchiveGenerator::PackageComponents(bool ignoreGroup)
{
  this->packageFileNames.clear();
  // The default behavior is to have one package by component group
  // unless CPACK_COMPONENTS_IGNORE_GROUP is specified.
  if (!ignoreGroup) {
    for (auto const& compG : this->ComponentGroups) {
      cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                    "Packaging component group: " << compG.first << std::endl);
      // Begin the archive for this group
      std::string packageFileName = std::string(this->toplevel) + "/" +
        this->GetArchiveComponentFileName(compG.first, true);

      // open a block in order to automatically close archive
      // at the end of the block
      {
        DECLARE_AND_OPEN_ARCHIVE(packageFileName, archive);
        // now iterate over the component of this group
        for (cmCPackComponent* comp : (compG.second).Components) {
          // Add the files of this component to the archive
          this->addOneComponentToArchive(archive, comp);
        }
      }
      // add the generated package to package file names list
      this->packageFileNames.push_back(std::move(packageFileName));
    }
    // Handle Orphan components (components not belonging to any groups)
    for (auto& comp : this->Components) {
      // Does the component belong to a group?
      if (comp.second.Group == nullptr) {
        cmCPackLogger(
          cmCPackLog::LOG_VERBOSE,
          "Component <"
            << comp.second.Name
            << "> does not belong to any group, package it separately."
            << std::endl);
        std::string localToplevel(
          this->GetOption("CPACK_TEMPORARY_DIRECTORY"));
        std::string packageFileName = std::string(this->toplevel);

        localToplevel += "/" + comp.first;
        packageFileName +=
          "/" + this->GetArchiveComponentFileName(comp.first, false);

        {
          DECLARE_AND_OPEN_ARCHIVE(packageFileName, archive);
          // Add the files of this component to the archive
          this->addOneComponentToArchive(archive, &(comp.second));
        }
        // add the generated package to package file names list
        this->packageFileNames.push_back(std::move(packageFileName));
      }
    }
  }
  // CPACK_COMPONENTS_IGNORE_GROUPS is set
  // We build 1 package per component
  else {
    for (auto& comp : this->Components) {
      std::string localToplevel(this->GetOption("CPACK_TEMPORARY_DIRECTORY"));
      std::string packageFileName = std::string(this->toplevel);

      localToplevel += "/" + comp.first;
      packageFileName +=
        "/" + this->GetArchiveComponentFileName(comp.first, false);

      {
        DECLARE_AND_OPEN_ARCHIVE(packageFileName, archive);
        // Add the files of this component to the archive
        this->addOneComponentToArchive(archive, &(comp.second));
      }
      // add the generated package to package file names list
      this->packageFileNames.push_back(std::move(packageFileName));
    }
  }
  return 1;
}

int cmCPackArchiveGenerator::PackageComponentsAllInOne()
{
  // reset the package file names
  this->packageFileNames.clear();
  this->packageFileNames.emplace_back(this->toplevel);
  this->packageFileNames[0] += "/";

  if (this->IsSet("CPACK_ARCHIVE_FILE_NAME")) {
    this->packageFileNames[0] += *this->GetOption("CPACK_ARCHIVE_FILE_NAME");
  } else {
    this->packageFileNames[0] += *this->GetOption("CPACK_PACKAGE_FILE_NAME");
  }

  this->packageFileNames[0] += this->GetOutputExtension();

  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "Packaging all groups in one package..."
                "(CPACK_COMPONENTS_ALL_GROUPS_IN_ONE_PACKAGE is set)"
                  << std::endl);
  DECLARE_AND_OPEN_ARCHIVE(packageFileNames[0], archive);

  // The ALL COMPONENTS in ONE package case
  for (auto& comp : this->Components) {
    // Add the files of this component to the archive
    this->addOneComponentToArchive(archive, &(comp.second));
  }

  // archive goes out of scope so it will finalized and closed.
  return 1;
}

int cmCPackArchiveGenerator::PackageFiles()
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
                "Toplevel: " << this->toplevel << std::endl);

  if (this->WantsComponentInstallation()) {
    // CASE 1 : COMPONENT ALL-IN-ONE package
    // If ALL COMPONENTS in ONE package has been requested
    // then the package file is unique and should be open here.
    if (this->componentPackageMethod == ONE_PACKAGE) {
      return this->PackageComponentsAllInOne();
    }
    // CASE 2 : COMPONENT CLASSICAL package(s) (i.e. not all-in-one)
    // There will be 1 package for each component group
    // however one may require to ignore component group and
    // in this case you'll get 1 package for each component.
    return this->PackageComponents(this->componentPackageMethod ==
                                   ONE_PACKAGE_PER_COMPONENT);
  }

  // CASE 3 : NON COMPONENT package.
  DECLARE_AND_OPEN_ARCHIVE(packageFileNames[0], archive);
  cmWorkingDirectory workdir(this->toplevel);
  if (workdir.Failed()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Failed to change working directory to "
                    << this->toplevel << " : "
                    << std::strerror(workdir.GetLastResult()) << std::endl);
    return 0;
  }
  for (std::string const& file : this->files) {
    // Get the relative path to the file
    std::string rp = cmSystemTools::RelativePath(this->toplevel, file);
    archive.Add(rp, 0, nullptr, false);
    if (!archive) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Problem while adding file <"
                      << file << "> to archive <" << this->packageFileNames[0]
                      << ">, ERROR = " << archive.GetError() << std::endl);
      return 0;
    }
  }
  // The destructor of cmArchiveWrite will close and finish the write
  return 1;
}

int cmCPackArchiveGenerator::GenerateHeader(std::ostream* /*unused*/)
{
  return 1;
}

bool cmCPackArchiveGenerator::SupportsComponentInstallation() const
{
  // The Component installation support should only
  // be activated if explicitly requested by the user
  // (for backward compatibility reason)
  return this->IsOn("CPACK_ARCHIVE_COMPONENT_INSTALL");
}

int cmCPackArchiveGenerator::GetThreadCount() const
{
  int threads = 1;

  // CPACK_ARCHIVE_THREADS overrides CPACK_THREADS
  if (this->IsSet("CPACK_ARCHIVE_THREADS")) {
    threads = std::stoi(this->GetOption("CPACK_ARCHIVE_THREADS"));
  } else if (this->IsSet("CPACK_THREADS")) {
    threads = std::stoi(this->GetOption("CPACK_THREADS"));
  }

  return threads;
}
