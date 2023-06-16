/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGeneratedFileStream.h"

#include <cstdio>

#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

#if !defined(CMAKE_BOOTSTRAP)
#  include <cm3p/zlib.h>

#  include "cm_codecvt.hxx"
#endif

cmGeneratedFileStream::cmGeneratedFileStream(Encoding encoding)
{
#ifndef CMAKE_BOOTSTRAP
  if (encoding != codecvt_Encoding::None) {
    this->imbue(std::locale(this->getloc(), new codecvt(encoding)));
  }
#else
  static_cast<void>(encoding);
#endif
}

cmGeneratedFileStream::cmGeneratedFileStream(std::string const& name,
                                             bool quiet, Encoding encoding)
  : cmGeneratedFileStreamBase(name)
  , Stream(this->TempName.c_str()) // NOLINT(cmake-use-cmsys-fstream)
{
  // Check if the file opened.
  if (!*this && !quiet) {
    cmSystemTools::Error("Cannot open file for write: " + this->TempName);
    cmSystemTools::ReportLastSystemError("");
  }
#ifndef CMAKE_BOOTSTRAP
  if (encoding != codecvt_Encoding::None) {
    this->imbue(std::locale(this->getloc(), new codecvt(encoding)));
  }
#else
  static_cast<void>(encoding);
#endif
  if (encoding == codecvt_Encoding::UTF8_WITH_BOM) {
    // Write the BOM encoding header into the file
    char magic[] = { static_cast<char>(0xEF), static_cast<char>(0xBB),
                     static_cast<char>(0xBF) };
    this->write(magic, 3);
  }
}

cmGeneratedFileStream::~cmGeneratedFileStream()
{
  // This is the first destructor called.  Check the status of the
  // stream and give the information to the private base.  Next the
  // stream will be destroyed which will close the temporary file.
  // Finally the base destructor will be called to replace the
  // destination file.
  this->Okay = !this->fail();
}

cmGeneratedFileStream& cmGeneratedFileStream::Open(std::string const& name,
                                                   bool quiet, bool binaryFlag)
{
  // Store the file name and construct the temporary file name.
  this->cmGeneratedFileStreamBase::Open(name);

  // Open the temporary output file.
  if (binaryFlag) {
    this->Stream::open( // NOLINT(cmake-use-cmsys-fstream)
      this->TempName.c_str(), std::ios::out | std::ios::binary);
  } else {
    this->Stream::open( // NOLINT(cmake-use-cmsys-fstream)
      this->TempName.c_str());
  }

  // Check if the file opened.
  if (!*this && !quiet) {
    cmSystemTools::Error("Cannot open file for write: " + this->TempName);
    cmSystemTools::ReportLastSystemError("");
  }
  return *this;
}

bool cmGeneratedFileStream::Close()
{
  // Save whether the temporary output file is valid before closing.
  this->Okay = !this->fail();

  // Close the temporary output file.
  this->Stream::close(); // NOLINT(cmake-use-cmsys-fstream)

  // Remove the temporary file (possibly by renaming to the real file).
  return this->cmGeneratedFileStreamBase::Close();
}

void cmGeneratedFileStream::SetCopyIfDifferent(bool copy_if_different)
{
  this->CopyIfDifferent = copy_if_different;
}

void cmGeneratedFileStream::SetCompression(bool compression)
{
  this->Compress = compression;
}

void cmGeneratedFileStream::SetCompressionExtraExtension(bool ext)
{
  this->CompressExtraExtension = ext;
}

cmGeneratedFileStreamBase::cmGeneratedFileStreamBase() = default;

cmGeneratedFileStreamBase::cmGeneratedFileStreamBase(std::string const& name)
{
  this->Open(name);
}

cmGeneratedFileStreamBase::~cmGeneratedFileStreamBase()
{
  this->Close();
}

void cmGeneratedFileStreamBase::Open(std::string const& name)
{
  // Save the original name of the file.
  this->Name = cmSystemTools::CollapseFullPath(name);

  // Create the name of the temporary file.
  this->TempName = this->Name;
#if defined(__VMS)
  this->TempName += "_";
#else
  this->TempName += ".";
#endif
  if (!this->TempExt.empty()) {
    this->TempName += this->TempExt;
  } else {
    char buf[64];
    snprintf(buf, sizeof(buf), "tmp%05x",
             cmSystemTools::RandomSeed() & 0xFFFFF);
    this->TempName += buf;
  }

  // Make sure the temporary file that will be used is not present.
  cmSystemTools::RemoveFile(this->TempName);

  std::string dir = cmSystemTools::GetFilenamePath(this->TempName);
  cmSystemTools::MakeDirectory(dir);
}

bool cmGeneratedFileStreamBase::Close()
{
  bool replaced = false;

  std::string resname = this->Name;
  if (this->Compress && this->CompressExtraExtension) {
    resname += ".gz";
  }

  // Only consider replacing the destination file if no error
  // occurred.
  if (!this->Name.empty() && this->Okay &&
      (!this->CopyIfDifferent ||
       cmSystemTools::FilesDiffer(this->TempName, resname))) {
    // The destination is to be replaced.  Rename the temporary to the
    // destination atomically.
    if (this->Compress) {
      std::string gzname = cmStrCat(this->TempName, ".temp.gz");
      if (this->CompressFile(this->TempName, gzname)) {
        this->RenameFile(gzname, resname);
      }
      cmSystemTools::RemoveFile(gzname);
    } else {
      this->RenameFile(this->TempName, resname);
    }

    replaced = true;
  }

  // Else, the destination was not replaced.
  //
  // Always delete the temporary file. We never want it to stay around.
  if (!this->TempName.empty()) {
    cmSystemTools::RemoveFile(this->TempName);
  }

  return replaced;
}

#ifndef CMAKE_BOOTSTRAP
int cmGeneratedFileStreamBase::CompressFile(std::string const& oldname,
                                            std::string const& newname)
{
  gzFile gf = gzopen(newname.c_str(), "w");
  if (!gf) {
    return 0;
  }
  FILE* ifs = cmsys::SystemTools::Fopen(oldname, "r");
  if (!ifs) {
    gzclose(gf);
    return 0;
  }
  size_t res;
  const size_t BUFFER_SIZE = 1024;
  char buffer[BUFFER_SIZE];
  while ((res = fread(buffer, 1, BUFFER_SIZE, ifs)) > 0) {
    if (!gzwrite(gf, buffer, static_cast<int>(res))) {
      fclose(ifs);
      gzclose(gf);
      return 0;
    }
  }
  fclose(ifs);
  gzclose(gf);
  return 1;
}
#else
int cmGeneratedFileStreamBase::CompressFile(std::string const&,
                                            std::string const&)
{
  return 0;
}
#endif

int cmGeneratedFileStreamBase::RenameFile(std::string const& oldname,
                                          std::string const& newname)
{
  return cmSystemTools::RenameFile(oldname, newname);
}

void cmGeneratedFileStream::SetName(const std::string& fname)
{
  this->Name = cmSystemTools::CollapseFullPath(fname);
}

void cmGeneratedFileStream::SetTempExt(std::string const& ext)
{
  this->TempExt = ext;
}

void cmGeneratedFileStream::WriteAltEncoding(std::string const& data,
                                             Encoding encoding)
{
#ifndef CMAKE_BOOTSTRAP
  std::locale prevLocale =
    this->imbue(std::locale(this->getloc(), new codecvt(encoding)));
  this->write(data.data(), data.size());
  this->imbue(prevLocale);
#else
  static_cast<void>(encoding);
  this->write(data.data(), data.size());
#endif
}
