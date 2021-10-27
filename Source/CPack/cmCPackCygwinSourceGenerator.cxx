/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCPackCygwinSourceGenerator.h"

#include "cmsys/SystemTools.hxx"

#include "cmCPackLog.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmake.h"

// Includes needed for implementation of RenameFile.  This is not in
// system tools because it is not implemented robustly enough to move
// files across directories.
#ifdef _WIN32
#  include <windows.h>

#  include "cm_sys_stat.h"
#endif

cmCPackCygwinSourceGenerator::cmCPackCygwinSourceGenerator()
  : cmCPackArchiveGenerator(cmArchiveWrite::CompressBZip2, "paxr", ".tar.bz2")
{
}

cmCPackCygwinSourceGenerator::~cmCPackCygwinSourceGenerator() = default;

int cmCPackCygwinSourceGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", "0");
  return this->Superclass::InitializeInternal();
}

int cmCPackCygwinSourceGenerator::PackageFiles()
{
  // Create a tar file of the sources
  std::string packageDirFileName =
    cmStrCat(this->GetOption("CPACK_TEMPORARY_DIRECTORY"), ".tar.bz2");
  packageFileNames[0] = packageDirFileName;
  std::string output;
  // create tar.bz2 file with the list of source files
  if (!this->cmCPackArchiveGenerator::PackageFiles()) {
    return 0;
  }
  // Now create a tar file that contains the above .tar.bz2 file
  // and the CPACK_CYGWIN_PATCH_FILE and CPACK_TOPLEVEL_DIRECTORY
  // files
  const std::string& compressOutFile = packageDirFileName;
  // at this point compressOutFile is the full path to
  // _CPack_Package/.../package-2.5.0.tar.bz2
  // we want to create a tar _CPack_Package/.../package-2.5.0-1-src.tar.bz2
  // with these
  //   _CPack_Package/.../package-2.5.0-1.patch
  //   _CPack_Package/.../package-2.5.0-1.sh
  //   _CPack_Package/.../package-2.5.0.tar.bz2
  // the -1 is CPACK_CYGWIN_PATCH_NUMBER

  // first copy the patch file and the .sh file
  // to the toplevel cpack temp dir

  // copy the patch file into place
  if (!this->GetOption("CPACK_CYGWIN_PATCH_FILE")) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "No patch file specified for cygwin sources.");
    return 0;
  }
  if (!cmSystemTools::CopyFileAlways(
        this->GetOption("CPACK_CYGWIN_PATCH_FILE"),
        this->GetOption("CPACK_TOPLEVEL_DIRECTORY"))) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "problem copying: ["
                    << this->GetOption("CPACK_CYGWIN_PATCH_FILE") << "]\nto\n["
                    << this->GetOption("CPACK_TOPLEVEL_DIRECTORY") << "]\n");
    return 0;
  }
  if (!this->GetOption("CPACK_CYGWIN_BUILD_SCRIPT")) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "No build script specified for cygwin sources.");
    return 0;
  }
  // copy the build script into place
  if (!cmSystemTools::CopyFileAlways(
        this->GetOption("CPACK_CYGWIN_BUILD_SCRIPT"),
        this->GetOption("CPACK_TOPLEVEL_DIRECTORY"))) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "problem copying: "
                    << this->GetOption("CPACK_CYGWIN_BUILD_SCRIPT") << "\nto\n"
                    << this->GetOption("CPACK_TOPLEVEL_DIRECTORY") << "]\n");
    return 0;
  }
  std::string outerTarFile =
    cmStrCat(this->GetOption("CPACK_TEMPORARY_DIRECTORY"), '-');
  cmValue patch = this->GetOption("CPACK_CYGWIN_PATCH_NUMBER");
  if (!patch) {
    cmCPackLogger(cmCPackLog::LOG_WARNING,
                  "CPACK_CYGWIN_PATCH_NUMBER"
                    << " not specified, defaulting to 1\n");
    outerTarFile += "1";
  } else {
    outerTarFile += patch;
  }
  outerTarFile += "-src.tar.bz2";
  std::string tmpDir = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  std::string buildScript =
    cmStrCat(tmpDir, '/',
             cmSystemTools::GetFilenameName(
               this->GetOption("CPACK_CYGWIN_BUILD_SCRIPT")));
  std::string patchFile =
    cmStrCat(tmpDir, '/',
             cmSystemTools::GetFilenameName(
               this->GetOption("CPACK_CYGWIN_PATCH_FILE")));

  std::string file = cmSystemTools::GetFilenameName(compressOutFile);
  std::string sourceTar =
    cmStrCat(cmSystemTools::GetFilenamePath(compressOutFile), '/', file);
  /* reset list of file to be packaged */
  files.clear();
  // a source release in cygwin should have the build script used
  // to build the package, the patch file that is different from the
  // regular upstream version of the sources, and a bziped tar file
  // of the original sources
  files.push_back(buildScript);
  files.push_back(patchFile);
  files.push_back(sourceTar);
  /* update the name of the produced package */
  packageFileNames[0] = outerTarFile;
  /* update the toplevel dir */
  toplevel = tmpDir;
  if (!this->cmCPackArchiveGenerator::PackageFiles()) {
    return 0;
  }
  return 1;
}

const char* cmCPackCygwinSourceGenerator::GetPackagingInstallPrefix()
{
  this->InstallPrefix =
    cmStrCat('/', this->GetOption("CPACK_PACKAGE_FILE_NAME"));
  return this->InstallPrefix.c_str();
}

const char* cmCPackCygwinSourceGenerator::GetOutputExtension()
{
  this->OutputExtension = "-";
  cmValue patch = this->GetOption("CPACK_CYGWIN_PATCH_NUMBER");
  if (!patch) {
    cmCPackLogger(cmCPackLog::LOG_WARNING,
                  "CPACK_CYGWIN_PATCH_NUMBER"
                    << " not specified, defaulting to 1\n");
    this->OutputExtension += "1";
  } else {
    this->OutputExtension += patch;
  }
  this->OutputExtension += "-src.tar.bz2";
  return this->OutputExtension.c_str();
}
