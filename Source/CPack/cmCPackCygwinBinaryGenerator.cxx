/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCPackCygwinBinaryGenerator.h"

#include "cmsys/SystemTools.hxx"

#include "cmCPackLog.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmake.h"

cmCPackCygwinBinaryGenerator::cmCPackCygwinBinaryGenerator()
  : cmCPackArchiveGenerator(cmArchiveWrite::CompressBZip2, "paxr", ".tar.bz2")
{
}

cmCPackCygwinBinaryGenerator::~cmCPackCygwinBinaryGenerator() = default;

int cmCPackCygwinBinaryGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_PACKAGING_INSTALL_PREFIX", "/usr");
  this->SetOptionIfNotSet("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", "0");
  return this->Superclass::InitializeInternal();
}

int cmCPackCygwinBinaryGenerator::PackageFiles()
{
  std::string packageName =
    cmStrCat(this->GetOption("CPACK_PACKAGE_NAME"), '-',
             this->GetOption("CPACK_PACKAGE_VERSION"));
  packageName = cmsys::SystemTools::LowerCase(packageName);
  std::string manifest = cmStrCat("/usr/share/doc/", packageName, "/MANIFEST");
  std::string manifestFile = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
  // Create a MANIFEST file that contains all of the files in
  // the tar file
  std::string tempdir = manifestFile;
  manifestFile += manifest;
  // create an extra scope to force the stream
  // to create the file before the super class is called
  {
    cmGeneratedFileStream ofs(manifestFile);
    for (std::string const& file : files) {
      // remove the temp dir and replace with /usr
      ofs << file.substr(tempdir.size()) << '\n';
    }
    ofs << manifest << '\n';
  }
  // add the manifest file to the list of all files
  files.push_back(manifestFile);

  // create the bzip2 tar file
  return this->Superclass::PackageFiles();
}

const char* cmCPackCygwinBinaryGenerator::GetOutputExtension()
{
  this->OutputExtension = "-";
  cmValue patchNumber = this->GetOption("CPACK_CYGWIN_PATCH_NUMBER");
  if (!patchNumber) {
    this->OutputExtension += '1';
    cmCPackLogger(cmCPackLog::LOG_WARNING,
                  "CPACK_CYGWIN_PATCH_NUMBER not specified using 1"
                    << std::endl);
  } else {
    this->OutputExtension += patchNumber;
  }
  this->OutputExtension += ".tar.bz2";
  return this->OutputExtension.c_str();
}
