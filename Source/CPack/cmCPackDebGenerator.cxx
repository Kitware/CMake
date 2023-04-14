/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCPackDebGenerator.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <ostream>
#include <set>
#include <stdexcept>
#include <utility>

#include "cmsys/Glob.hxx"

#include "cm_sys_stat.h"

#include "cmArchiveWrite.h"
#include "cmCPackComponentGroup.h"
#include "cmCPackGenerator.h"
#include "cmCPackLog.h"
#include "cmCryptoHash.h"
#include "cmGeneratedFileStream.h"
#include "cmList.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

namespace {

class DebGenerator
{
public:
  DebGenerator(cmCPackLog* logger, std::string outputName, std::string workDir,
               std::string topLevelDir, std::string temporaryDir,
               cmValue debianCompressionType, cmValue numThreads,
               cmValue debianArchiveType,
               std::map<std::string, std::string> controlValues,
               bool genShLibs, std::string shLibsFilename, bool genPostInst,
               std::string postInst, bool genPostRm, std::string postRm,
               cmValue controlExtra, bool permissionStrctPolicy,
               std::vector<std::string> packageFiles);

  bool generate() const;

private:
  void generateDebianBinaryFile() const;
  void generateControlFile() const;
  bool generateDataTar() const;
  std::string generateMD5File() const;
  bool generateControlTar(std::string const& md5Filename) const;
  bool generateDeb() const;

  cmCPackLog* Logger;
  const std::string OutputName;
  const std::string WorkDir;
  std::string CompressionSuffix;
  const std::string TopLevelDir;
  const std::string TemporaryDir;
  const std::string DebianArchiveType;
  long NumThreads;
  const std::map<std::string, std::string> ControlValues;
  const bool GenShLibs;
  const std::string ShLibsFilename;
  const bool GenPostInst;
  const std::string PostInst;
  const bool GenPostRm;
  const std::string PostRm;
  cmValue ControlExtra;
  const bool PermissionStrictPolicy;
  const std::vector<std::string> PackageFiles;
  cmArchiveWrite::Compress TarCompressionType;
};

DebGenerator::DebGenerator(
  cmCPackLog* logger, std::string outputName, std::string workDir,
  std::string topLevelDir, std::string temporaryDir,
  cmValue debCompressionType, cmValue numThreads, cmValue debianArchiveType,
  std::map<std::string, std::string> controlValues, bool genShLibs,
  std::string shLibsFilename, bool genPostInst, std::string postInst,
  bool genPostRm, std::string postRm, cmValue controlExtra,
  bool permissionStrictPolicy, std::vector<std::string> packageFiles)
  : Logger(logger)
  , OutputName(std::move(outputName))
  , WorkDir(std::move(workDir))
  , TopLevelDir(std::move(topLevelDir))
  , TemporaryDir(std::move(temporaryDir))
  , DebianArchiveType(debianArchiveType ? *debianArchiveType : "gnutar")
  , ControlValues(std::move(controlValues))
  , GenShLibs(genShLibs)
  , ShLibsFilename(std::move(shLibsFilename))
  , GenPostInst(genPostInst)
  , PostInst(std::move(postInst))
  , GenPostRm(genPostRm)
  , PostRm(std::move(postRm))
  , ControlExtra(controlExtra)
  , PermissionStrictPolicy(permissionStrictPolicy)
  , PackageFiles(std::move(packageFiles))
{
  std::string debianCompressionType = "gzip";
  if (debCompressionType) {
    debianCompressionType = *debCompressionType;
  }

  if (debianCompressionType == "lzma") {
    this->CompressionSuffix = ".lzma";
    this->TarCompressionType = cmArchiveWrite::CompressLZMA;
  } else if (debianCompressionType == "xz") {
    this->CompressionSuffix = ".xz";
    this->TarCompressionType = cmArchiveWrite::CompressXZ;
  } else if (debianCompressionType == "bzip2") {
    this->CompressionSuffix = ".bz2";
    this->TarCompressionType = cmArchiveWrite::CompressBZip2;
  } else if (debianCompressionType == "gzip") {
    this->CompressionSuffix = ".gz";
    this->TarCompressionType = cmArchiveWrite::CompressGZip;
  } else if (debianCompressionType == "zstd") {
    this->CompressionSuffix = ".zst";
    this->TarCompressionType = cmArchiveWrite::CompressZstd;
  } else if (debianCompressionType == "none") {
    this->CompressionSuffix.clear();
    this->TarCompressionType = cmArchiveWrite::CompressNone;
  } else {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Error unrecognized compression type: "
                    << debianCompressionType << std::endl);
  }

  if (numThreads) {
    if (!cmStrToLong(*numThreads, &this->NumThreads)) {
      this->NumThreads = 1;
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Unrecognized number of threads: " << numThreads
                                                       << std::endl);
    }
  } else {
    this->NumThreads = 1;
  }
}

bool DebGenerator::generate() const
{
  this->generateDebianBinaryFile();
  this->generateControlFile();
  if (!this->generateDataTar()) {
    return false;
  }
  std::string md5Filename = this->generateMD5File();
  if (!this->generateControlTar(md5Filename)) {
    return false;
  }
  return this->generateDeb();
}

void DebGenerator::generateDebianBinaryFile() const
{
  // debian-binary file
  const std::string dbfilename = this->WorkDir + "/debian-binary";
  cmGeneratedFileStream out;
  out.Open(dbfilename, false, true);
  out << "2.0\n"; // required for valid debian package
}

void DebGenerator::generateControlFile() const
{
  std::string ctlfilename = this->WorkDir + "/control";

  cmGeneratedFileStream out;
  out.Open(ctlfilename, false, true);
  for (auto const& kv : this->ControlValues) {
    out << kv.first << ": " << kv.second << "\n";
  }

  unsigned long totalSize = 0;
  {
    for (std::string const& file : this->PackageFiles) {
      totalSize += cmSystemTools::FileLength(file);
    }
  }
  out << "Installed-Size: " << (totalSize + 1023) / 1024 << "\n\n";
}

bool DebGenerator::generateDataTar() const
{
  std::string filename_data_tar =
    this->WorkDir + "/data.tar" + this->CompressionSuffix;
  cmGeneratedFileStream fileStream_data_tar;
  fileStream_data_tar.Open(filename_data_tar, false, true);
  if (!fileStream_data_tar) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Error opening the file \""
                    << filename_data_tar << "\" for writing" << std::endl);
    return false;
  }
  cmArchiveWrite data_tar(fileStream_data_tar, this->TarCompressionType,
                          this->DebianArchiveType, 0,
                          static_cast<int>(this->NumThreads));
  if (!data_tar.Open()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Error opening the archive \""
                    << filename_data_tar
                    << "\", ERROR = " << data_tar.GetError() << std::endl);
    return false;
  }

  // uid/gid should be the one of the root user, and this root user has
  // always uid/gid equal to 0.
  data_tar.SetUIDAndGID(0U, 0U);
  data_tar.SetUNAMEAndGNAME("root", "root");

  // now add all directories which have to be compressed
  // collect all top level install dirs for that
  // e.g. /opt/bin/foo, /usr/bin/bar and /usr/bin/baz would
  // give /usr and /opt
  size_t topLevelLength = this->WorkDir.length();
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
                "WDIR: \"" << this->WorkDir
                           << "\", length = " << topLevelLength << std::endl);
  std::set<std::string> orderedFiles;

  // we have to reconstruct the parent folders as well

  for (std::string currentPath : this->PackageFiles) {
    while (currentPath != this->WorkDir) {
      // the last one IS WorkDir, but we do not want this one:
      // XXX/application/usr/bin/myprogram with GEN_WDIR=XXX/application
      // should not add XXX/application
      orderedFiles.insert(currentPath);
      currentPath = cmSystemTools::CollapseFullPath("..", currentPath);
    }
  }

  for (std::string const& file : orderedFiles) {
    cmCPackLogger(cmCPackLog::LOG_DEBUG,
                  "FILEIT: \"" << file << "\"" << std::endl);
    std::string::size_type slashPos = file.find('/', topLevelLength + 1);
    std::string relativeDir =
      file.substr(topLevelLength, slashPos - topLevelLength);
    cmCPackLogger(cmCPackLog::LOG_DEBUG,
                  "RELATIVEDIR: \"" << relativeDir << "\"" << std::endl);

#ifdef _WIN32
    std::string mode_t_adt_filename = file + ":cmake_mode_t";
    cmsys::ifstream permissionStream(mode_t_adt_filename.c_str());

    mode_t permissions = 0;

    if (permissionStream) {
      permissionStream >> std::oct >> permissions;
    }

    if (permissions != 0) {
      data_tar.SetPermissions(permissions);
    } else if (cmSystemTools::FileIsDirectory(file)) {
      data_tar.SetPermissions(0755);
    } else {
      data_tar.ClearPermissions();
    }
#endif

    // do not recurse because the loop will do it
    if (!data_tar.Add(file, topLevelLength, ".", false)) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Problem adding file to tar:\n"
                    "#top level directory: "
                      << this->WorkDir
                      << "\n"
                         "#file: "
                      << file
                      << "\n"
                         "#error:"
                      << data_tar.GetError() << std::endl);
      return false;
    }
  }
  return true;
}

std::string DebGenerator::generateMD5File() const
{
  std::string md5filename = this->WorkDir + "/md5sums";

  cmGeneratedFileStream out;
  out.Open(md5filename, false, true);

  std::string topLevelWithTrailingSlash = cmStrCat(this->TemporaryDir, '/');
  for (std::string const& file : this->PackageFiles) {
    // hash only regular files
    if (cmSystemTools::FileIsDirectory(file) ||
        cmSystemTools::FileIsSymlink(file)) {
      continue;
    }

    std::string output =
      cmSystemTools::ComputeFileHash(file, cmCryptoHash::AlgoMD5);
    if (output.empty()) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Problem computing the md5 of " << file << std::endl);
    }

    output += "  " + file + "\n";
    // debian md5sums entries are like this:
    // 014f3604694729f3bf19263bac599765  usr/bin/ccmake
    // thus strip the full path (with the trailing slash)
    cmSystemTools::ReplaceString(output, topLevelWithTrailingSlash.c_str(),
                                 "");
    out << output;
  }
  // each line contains a eol.
  // Do not end the md5sum file with yet another (invalid)
  return md5filename;
}

bool DebGenerator::generateControlTar(std::string const& md5Filename) const
{
  std::string filename_control_tar = this->WorkDir + "/control.tar.gz";

  cmGeneratedFileStream fileStream_control_tar;
  fileStream_control_tar.Open(filename_control_tar, false, true);
  if (!fileStream_control_tar) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Error opening the file \""
                    << filename_control_tar << "\" for writing" << std::endl);
    return false;
  }
  cmArchiveWrite control_tar(fileStream_control_tar,
                             cmArchiveWrite::CompressGZip,
                             this->DebianArchiveType);
  if (!control_tar.Open()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Error opening the archive \""
                    << filename_control_tar
                    << "\", ERROR = " << control_tar.GetError() << std::endl);
    return false;
  }

  // sets permissions and uid/gid for the files
  control_tar.SetUIDAndGID(0u, 0u);
  control_tar.SetUNAMEAndGNAME("root", "root");

  /* permissions are set according to
  https://www.debian.org/doc/debian-policy/ch-files.html#s-permissions-owners
  and
  https://lintian.debian.org/tags/control-file-has-bad-permissions.html
  */
  const mode_t permission644 = 0644;
  const mode_t permissionExecute = 0111;
  const mode_t permission755 = permission644 | permissionExecute;

  // for md5sum and control (that we have generated here), we use 644
  // (RW-R--R--)
  // so that deb lintian doesn't warn about it
  control_tar.SetPermissions(permission644);

  // adds control and md5sums
  if (!control_tar.Add(md5Filename, this->WorkDir.length(), ".") ||
      !control_tar.Add(this->WorkDir + "/control", this->WorkDir.length(),
                       ".")) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Error adding file to tar:\n"
                  "#top level directory: "
                    << this->WorkDir
                    << "\n"
                       "#file: \"control\" or \"md5sums\"\n"
                       "#error:"
                    << control_tar.GetError() << std::endl);
    return false;
  }

  // adds generated shlibs file
  if (this->GenShLibs) {
    if (!control_tar.Add(this->ShLibsFilename, this->WorkDir.length(), ".")) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error adding file to tar:\n"
                    "#top level directory: "
                      << this->WorkDir
                      << "\n"
                         "#file: \"shlibs\"\n"
                         "#error:"
                      << control_tar.GetError() << std::endl);
      return false;
    }
  }

  // adds LDCONFIG related files
  if (this->GenPostInst) {
    control_tar.SetPermissions(permission755);
    if (!control_tar.Add(this->PostInst, this->WorkDir.length(), ".")) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error adding file to tar:\n"
                    "#top level directory: "
                      << this->WorkDir
                      << "\n"
                         "#file: \"postinst\"\n"
                         "#error:"
                      << control_tar.GetError() << std::endl);
      return false;
    }
    control_tar.SetPermissions(permission644);
  }

  if (this->GenPostRm) {
    control_tar.SetPermissions(permission755);
    if (!control_tar.Add(this->PostRm, this->WorkDir.length(), ".")) {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error adding file to tar:\n"
                    "#top level directory: "
                      << this->WorkDir
                      << "\n"
                         "#file: \"postinst\"\n"
                         "#error:"
                      << control_tar.GetError() << std::endl);
      return false;
    }
    control_tar.SetPermissions(permission644);
  }

  // for the other files, we use
  // -either the original permission on the files
  // -either a permission strictly defined by the Debian policies
  if (this->ControlExtra) {
    // permissions are now controlled by the original file permissions

    static const char* strictFiles[] = { "config", "postinst", "postrm",
                                         "preinst", "prerm" };
    std::set<std::string> setStrictFiles(
      strictFiles, strictFiles + sizeof(strictFiles) / sizeof(strictFiles[0]));

    // default
    control_tar.ClearPermissions();

    cmList controlExtraList{ this->ControlExtra };
    for (std::string const& i : controlExtraList) {
      std::string filenamename = cmsys::SystemTools::GetFilenameName(i);
      std::string localcopy = this->WorkDir + "/" + filenamename;

      if (this->PermissionStrictPolicy) {
        control_tar.SetPermissions(
          setStrictFiles.count(filenamename) ? permission755 : permission644);
      }

      // if we can copy the file, it means it does exist, let's add it:
      if (!cmsys::SystemTools::FileExists(i)) {
        cmCPackLogger(cmCPackLog::LOG_WARNING,
                      "Adding file to tar:\n"
                      "#top level directory: "
                        << this->WorkDir
                        << "\n"
                           "#missing file: "
                        << i << std::endl);
      }

      if (cmsys::SystemTools::CopyFileIfDifferent(i, localcopy)) {
        control_tar.Add(localcopy, this->WorkDir.length(), ".");
      }
    }
  }

  return true;
}

bool DebGenerator::generateDeb() const
{
  // ar -r your-package-name.deb debian-binary control.tar.* data.tar.*
  // A debian package .deb is simply an 'ar' archive. The only subtle
  // difference is that debian uses the BSD ar style archive whereas most
  // Linux distro have a GNU ar.
  // See http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=161593 for more info
  std::string const outputPath = this->TopLevelDir + "/" + this->OutputName;
  std::string const tlDir = this->WorkDir + "/";
  cmGeneratedFileStream debStream;
  debStream.Open(outputPath, false, true);
  cmArchiveWrite deb(debStream, cmArchiveWrite::CompressNone, "arbsd");
  if (!deb.Open()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Error opening the archive \""
                    << outputPath << "\", ERROR = " << deb.GetError()
                    << std::endl);
    return false;
  }

  // uid/gid should be the one of the root user, and this root user has
  // always uid/gid equal to 0.
  deb.SetUIDAndGID(0u, 0u);
  deb.SetUNAMEAndGNAME("root", "root");

  if (!deb.Add(tlDir + "debian-binary", tlDir.length()) ||
      !deb.Add(tlDir + "control.tar.gz", tlDir.length()) ||
      !deb.Add(tlDir + "data.tar" + this->CompressionSuffix, tlDir.length())) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Error creating debian package:\n"
                  "#top level directory: "
                    << this->TopLevelDir
                    << "\n"
                       "#file: "
                    << this->OutputName
                    << "\n"
                       "#error:"
                    << deb.GetError() << std::endl);
    return false;
  }
  return true;
}

std::vector<std::string> findFilesIn(const std::string& path)
{
  cmsys::Glob gl;
  std::string findExpr = path + "/*";
  gl.RecurseOn();
  gl.SetRecurseListDirs(true);
  gl.SetRecurseThroughSymlinks(false);
  if (!gl.FindFiles(findExpr)) {
    throw std::runtime_error(
      "Cannot find any files in the installed directory");
  }
  std::vector<std::string> files{ gl.GetFiles() };
  // Sort files so that they have a reproducible order
  std::sort(files.begin(), files.end());
  return files;
}

} // end anonymous namespace

cmCPackDebGenerator::cmCPackDebGenerator() = default;

cmCPackDebGenerator::~cmCPackDebGenerator() = default;

int cmCPackDebGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_PACKAGING_INSTALL_PREFIX", "/usr");
  if (cmIsOff(this->GetOption("CPACK_SET_DESTDIR"))) {
    this->SetOption("CPACK_SET_DESTDIR", "I_ON");
  }
  return this->Superclass::InitializeInternal();
}

int cmCPackDebGenerator::PackageOnePack(std::string const& initialTopLevel,
                                        std::string const& packageName)
{
  // Begin the archive for this pack
  std::string localToplevel(initialTopLevel);
  std::string packageFileName(
    cmSystemTools::GetParentDirectory(this->toplevel));
  std::string outputFileName(*this->GetOption("CPACK_PACKAGE_FILE_NAME") +
                             "-" + packageName + this->GetOutputExtension());

  localToplevel += "/" + packageName;
  /* replace the TEMP DIRECTORY with the component one */
  this->SetOption("CPACK_TEMPORARY_DIRECTORY", localToplevel);
  packageFileName += "/" + outputFileName;
  /* replace proposed CPACK_OUTPUT_FILE_NAME */
  this->SetOption("CPACK_OUTPUT_FILE_NAME", outputFileName);
  /* replace the TEMPORARY package file name */
  this->SetOption("CPACK_TEMPORARY_PACKAGE_FILE_NAME", packageFileName);
  // Tell CPackDeb.cmake the name of the component GROUP.
  this->SetOption("CPACK_DEB_PACKAGE_COMPONENT", packageName);
  // Tell CPackDeb.cmake the path where the component is.
  std::string component_path = cmStrCat('/', packageName);
  this->SetOption("CPACK_DEB_PACKAGE_COMPONENT_PART_PATH", component_path);
  if (!this->ReadListFile("Internal/CPack/CPackDeb.cmake")) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Error while execution CPackDeb.cmake" << std::endl);
    return 0;
  }

  return this->createDebPackages();
}

int cmCPackDebGenerator::PackageComponents(bool ignoreGroup)
{
  // Reset package file name list it will be populated during the
  // component packaging run
  this->packageFileNames.clear();
  std::string initialTopLevel(this->GetOption("CPACK_TEMPORARY_DIRECTORY"));

  int retval = 1;
  // The default behavior is to have one package by component group
  // unless CPACK_COMPONENTS_IGNORE_GROUP is specified.
  if (ignoreGroup) {
    // CPACK_COMPONENTS_IGNORE_GROUPS is set
    // We build 1 package per component
    for (auto const& comp : this->Components) {
      retval &= this->PackageOnePack(initialTopLevel, comp.first);
    }
    return retval;
  }

  for (auto const& compG : this->ComponentGroups) {
    cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                  "Packaging component group: " << compG.first << std::endl);
    // Begin the archive for this group
    retval &= this->PackageOnePack(initialTopLevel, compG.first);
  }
  // Handle Orphan components (components not belonging to any groups)
  for (auto const& comp : this->Components) {
    // Does the component belong to a group?
    if (comp.second.Group == nullptr) {
      cmCPackLogger(
        cmCPackLog::LOG_VERBOSE,
        "Component <"
          << comp.second.Name
          << "> does not belong to any group, package it separately."
          << std::endl);
      // Begin the archive for this orphan component
      retval &= this->PackageOnePack(initialTopLevel, comp.first);
    }
  }
  return retval;
}

//----------------------------------------------------------------------
int cmCPackDebGenerator::PackageComponentsAllInOne(
  const std::string& compInstDirName)
{
  /* Reset package file name list it will be populated during the
   * component packaging run*/
  this->packageFileNames.clear();
  std::string initialTopLevel(this->GetOption("CPACK_TEMPORARY_DIRECTORY"));

  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "Packaging all groups in one package..."
                "(CPACK_COMPONENTS_ALL_[GROUPS_]IN_ONE_PACKAGE is set)"
                  << std::endl);

  // The ALL GROUPS in ONE package case
  std::string localToplevel(initialTopLevel);
  std::string packageFileName(
    cmSystemTools::GetParentDirectory(this->toplevel));
  std::string outputFileName(*this->GetOption("CPACK_PACKAGE_FILE_NAME") +
                             this->GetOutputExtension());
  // all GROUP in one vs all COMPONENT in one
  // if must be here otherwise non component paths have a trailing / while
  // components don't
  if (!compInstDirName.empty()) {
    localToplevel += "/" + compInstDirName;
  }

  /* replace the TEMP DIRECTORY with the component one */
  this->SetOption("CPACK_TEMPORARY_DIRECTORY", localToplevel);
  packageFileName += "/" + outputFileName;
  /* replace proposed CPACK_OUTPUT_FILE_NAME */
  this->SetOption("CPACK_OUTPUT_FILE_NAME", outputFileName);
  /* replace the TEMPORARY package file name */
  this->SetOption("CPACK_TEMPORARY_PACKAGE_FILE_NAME", packageFileName);

  if (!compInstDirName.empty()) {
    // Tell CPackDeb.cmake the path where the component is.
    std::string component_path = cmStrCat('/', compInstDirName);
    this->SetOption("CPACK_DEB_PACKAGE_COMPONENT_PART_PATH", component_path);
  }
  if (!this->ReadListFile("Internal/CPack/CPackDeb.cmake")) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Error while execution CPackDeb.cmake" << std::endl);
    return 0;
  }

  return this->createDebPackages();
}

int cmCPackDebGenerator::PackageFiles()
{
  /* Are we in the component packaging case */
  if (this->WantsComponentInstallation()) {
    // CASE 1 : COMPONENT ALL-IN-ONE package
    // If ALL GROUPS or ALL COMPONENTS in ONE package has been requested
    // then the package file is unique and should be open here.
    if (this->componentPackageMethod == ONE_PACKAGE) {
      return this->PackageComponentsAllInOne("ALL_COMPONENTS_IN_ONE");
    }
    // CASE 2 : COMPONENT CLASSICAL package(s) (i.e. not all-in-one)
    // There will be 1 package for each component group
    // however one may require to ignore component group and
    // in this case you'll get 1 package for each component.
    return this->PackageComponents(this->componentPackageMethod ==
                                   ONE_PACKAGE_PER_COMPONENT);
  }
  // CASE 3 : NON COMPONENT package.
  return this->PackageComponentsAllInOne("");
}

bool cmCPackDebGenerator::createDebPackages()
{
  auto make_package = [this](const std::string& path,
                             const char* const output_var,
                             bool (cmCPackDebGenerator::*creator)()) -> bool {
    try {
      this->packageFiles = findFilesIn(path);
    } catch (const std::runtime_error& ex) {
      cmCPackLogger(cmCPackLog::LOG_ERROR, ex.what() << std::endl);
      return false;
    }

    if ((this->*creator)()) {
      // add the generated package to package file names list
      this->packageFileNames.emplace_back(
        cmStrCat(this->GetOption("CPACK_TOPLEVEL_DIRECTORY"), '/',
                 this->GetOption(output_var)));
      return true;
    }
    return false;
  };
  bool retval =
    make_package(this->GetOption("GEN_WDIR"), "GEN_CPACK_OUTPUT_FILE_NAME",
                 &cmCPackDebGenerator::createDeb);
  cmValue dbgsymdir_path = this->GetOption("GEN_DBGSYMDIR");
  if (this->IsOn("GEN_CPACK_DEBIAN_DEBUGINFO_PACKAGE") && dbgsymdir_path) {
    retval = make_package(*dbgsymdir_path, "GEN_CPACK_DBGSYM_OUTPUT_FILE_NAME",
                          &cmCPackDebGenerator::createDbgsymDDeb) &&
      retval;
  }
  return static_cast<int>(retval);
}

bool cmCPackDebGenerator::createDeb()
{
  std::map<std::string, std::string> controlValues;

  // debian policy enforce lower case for package name
  controlValues["Package"] = cmsys::SystemTools::LowerCase(
    this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_NAME"));
  controlValues["Version"] =
    *this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_VERSION");
  controlValues["Section"] =
    *this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_SECTION");
  controlValues["Priority"] =
    *this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_PRIORITY");
  controlValues["Architecture"] =
    *this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_ARCHITECTURE");
  controlValues["Maintainer"] =
    *this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_MAINTAINER");
  controlValues["Description"] =
    *this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_DESCRIPTION");

  cmValue debian_pkg_source =
    this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_SOURCE");
  if (cmNonempty(debian_pkg_source)) {
    controlValues["Source"] = *debian_pkg_source;
  }
  cmValue debian_pkg_dep = this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_DEPENDS");
  if (cmNonempty(debian_pkg_dep)) {
    controlValues["Depends"] = *debian_pkg_dep;
  }
  cmValue debian_pkg_rec =
    this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_RECOMMENDS");
  if (cmNonempty(debian_pkg_rec)) {
    controlValues["Recommends"] = *debian_pkg_rec;
  }
  cmValue debian_pkg_sug =
    this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_SUGGESTS");
  if (cmNonempty(debian_pkg_sug)) {
    controlValues["Suggests"] = *debian_pkg_sug;
  }
  cmValue debian_pkg_url =
    this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_HOMEPAGE");
  if (cmNonempty(debian_pkg_url)) {
    controlValues["Homepage"] = *debian_pkg_url;
  }
  cmValue debian_pkg_predep =
    this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_PREDEPENDS");
  if (cmNonempty(debian_pkg_predep)) {
    controlValues["Pre-Depends"] = *debian_pkg_predep;
  }
  cmValue debian_pkg_enhances =
    this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_ENHANCES");
  if (cmNonempty(debian_pkg_enhances)) {
    controlValues["Enhances"] = *debian_pkg_enhances;
  }
  cmValue debian_pkg_breaks =
    this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_BREAKS");
  if (cmNonempty(debian_pkg_breaks)) {
    controlValues["Breaks"] = *debian_pkg_breaks;
  }
  cmValue debian_pkg_conflicts =
    this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_CONFLICTS");
  if (cmNonempty(debian_pkg_conflicts)) {
    controlValues["Conflicts"] = *debian_pkg_conflicts;
  }
  cmValue debian_pkg_provides =
    this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_PROVIDES");
  if (cmNonempty(debian_pkg_provides)) {
    controlValues["Provides"] = *debian_pkg_provides;
  }
  cmValue debian_pkg_replaces =
    this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_REPLACES");
  if (cmNonempty(debian_pkg_replaces)) {
    controlValues["Replaces"] = *debian_pkg_replaces;
  }

  const std::string strGenWDIR(this->GetOption("GEN_WDIR"));
  const std::string shlibsfilename = strGenWDIR + "/shlibs";

  cmValue debian_pkg_shlibs =
    this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_SHLIBS");
  const bool gen_shibs = this->IsOn("CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS") &&
    cmNonempty(debian_pkg_shlibs);
  if (gen_shibs) {
    cmGeneratedFileStream out;
    out.Open(shlibsfilename, false, true);
    out << debian_pkg_shlibs;
    out << '\n';
  }

  const std::string postinst = strGenWDIR + "/postinst";
  const std::string postrm = strGenWDIR + "/postrm";
  if (this->IsOn("GEN_CPACK_DEBIAN_GENERATE_POSTINST")) {
    cmGeneratedFileStream out;
    out.Open(postinst, false, true);
    out << "#!/bin/sh\n\n"
           "set -e\n\n"
           "if [ \"$1\" = \"configure\" ]; then\n"
           "\tldconfig\n"
           "fi\n";
  }
  if (this->IsOn("GEN_CPACK_DEBIAN_GENERATE_POSTRM")) {
    cmGeneratedFileStream out;
    out.Open(postrm, false, true);
    out << "#!/bin/sh\n\n"
           "set -e\n\n"
           "if [ \"$1\" = \"remove\" ]; then\n"
           "\tldconfig\n"
           "fi\n";
  }

  DebGenerator gen(
    this->Logger, this->GetOption("GEN_CPACK_OUTPUT_FILE_NAME"), strGenWDIR,
    this->GetOption("CPACK_TOPLEVEL_DIRECTORY"),
    this->GetOption("CPACK_TEMPORARY_DIRECTORY"),
    this->GetOption("GEN_CPACK_DEBIAN_COMPRESSION_TYPE"),
    this->GetOption("CPACK_THREADS"),
    this->GetOption("GEN_CPACK_DEBIAN_ARCHIVE_TYPE"), controlValues, gen_shibs,
    shlibsfilename, this->IsOn("GEN_CPACK_DEBIAN_GENERATE_POSTINST"), postinst,
    this->IsOn("GEN_CPACK_DEBIAN_GENERATE_POSTRM"), postrm,
    this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA"),
    this->IsSet("GEN_CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION"),
    this->packageFiles);

  return gen.generate();
}

bool cmCPackDebGenerator::createDbgsymDDeb()
{
  // Packages containing debug symbols follow the same structure as .debs
  // but have different metadata and content.

  std::map<std::string, std::string> controlValues;
  // debian policy enforce lower case for package name
  std::string packageNameLower = cmsys::SystemTools::LowerCase(
    this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_NAME"));
  cmValue debian_pkg_version =
    this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_VERSION");

  controlValues["Package"] = packageNameLower + "-dbgsym";
  controlValues["Package-Type"] = "ddeb";
  controlValues["Version"] = *debian_pkg_version;
  controlValues["Auto-Built-Package"] = "debug-symbols";
  controlValues["Depends"] =
    *this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_NAME") + std::string(" (= ") +
    *debian_pkg_version + ")";
  controlValues["Section"] = "debug";
  controlValues["Priority"] = "optional";
  controlValues["Architecture"] =
    *this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_ARCHITECTURE");
  controlValues["Maintainer"] =
    *this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_MAINTAINER");
  controlValues["Description"] =
    std::string("debug symbols for ") + packageNameLower;

  cmValue debian_pkg_source =
    this->GetOption("GEN_CPACK_DEBIAN_PACKAGE_SOURCE");
  if (cmNonempty(debian_pkg_source)) {
    controlValues["Source"] = *debian_pkg_source;
  }
  cmValue debian_build_ids = this->GetOption("GEN_BUILD_IDS");
  if (cmNonempty(debian_build_ids)) {
    controlValues["Build-Ids"] = *debian_build_ids;
  }

  DebGenerator gen(
    this->Logger, this->GetOption("GEN_CPACK_DBGSYM_OUTPUT_FILE_NAME"),
    this->GetOption("GEN_DBGSYMDIR"),
    this->GetOption("CPACK_TOPLEVEL_DIRECTORY"),
    this->GetOption("CPACK_TEMPORARY_DIRECTORY"),
    this->GetOption("GEN_CPACK_DEBIAN_COMPRESSION_TYPE"),
    this->GetOption("CPACK_THREADS"),
    this->GetOption("GEN_CPACK_DEBIAN_ARCHIVE_TYPE"), controlValues, false, "",
    false, "", false, "", nullptr,
    this->IsSet("GEN_CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION"),
    this->packageFiles);

  return gen.generate();
}

bool cmCPackDebGenerator::SupportsComponentInstallation() const
{
  return this->IsOn("CPACK_DEB_COMPONENT_INSTALL");
}

std::string cmCPackDebGenerator::GetComponentInstallDirNameSuffix(
  const std::string& componentName)
{
  if (this->componentPackageMethod == ONE_PACKAGE_PER_COMPONENT) {
    return componentName;
  }

  if (this->componentPackageMethod == ONE_PACKAGE) {
    return { "ALL_COMPONENTS_IN_ONE" };
  }
  // We have to find the name of the COMPONENT GROUP
  // the current COMPONENT belongs to.
  std::string groupVar =
    "CPACK_COMPONENT_" + cmSystemTools::UpperCase(componentName) + "_GROUP";
  if (nullptr != this->GetOption(groupVar)) {
    return *this->GetOption(groupVar);
  }
  return componentName;
}
