/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmGeneratedFileStream.h"

#include "cmSystemTools.h"

// Includes needed for implementation of RenameFile.  This is not in
// system tools because it is not implemented robustly enough to move
// files across directories.
#ifdef _WIN32
# include <windows.h>
# include <sys/stat.h>
#endif

#if defined(CMAKE_BUILD_WITH_CMAKE)
# include <cm_zlib.h>
#endif

//----------------------------------------------------------------------------
cmGeneratedFileStream::cmGeneratedFileStream():
  cmGeneratedFileStreamBase(), Stream()
{
}

//----------------------------------------------------------------------------
cmGeneratedFileStream::cmGeneratedFileStream(const char* name, bool quiet):
  cmGeneratedFileStreamBase(name),
  Stream(TempName.c_str())
{
  // Check if the file opened.
  if(!*this && !quiet)
    {
    cmSystemTools::Error("Cannot open file for write: ", 
                         this->TempName.c_str());
    cmSystemTools::ReportLastSystemError("");
    }
}

//----------------------------------------------------------------------------
cmGeneratedFileStream::~cmGeneratedFileStream()
{
  // This is the first destructor called.  Check the status of the
  // stream and give the information to the private base.  Next the
  // stream will be destroyed which will close the temporary file.
  // Finally the base destructor will be called to replace the
  // destination file.
  this->Okay = (*this)?true:false;
}

//----------------------------------------------------------------------------
cmGeneratedFileStream&
cmGeneratedFileStream::Open(const char* name, bool quiet, bool binaryFlag)
{
  // Store the file name and construct the temporary file name.
  this->cmGeneratedFileStreamBase::Open(name);

  // Open the temporary output file.
  if ( binaryFlag )
    {
    this->Stream::open(this->TempName.c_str(), 
                       std::ios::out | std::ios::binary);
    }
  else
    {
    this->Stream::open(this->TempName.c_str(), std::ios::out);
    }

  // Check if the file opened.
  if(!*this && !quiet)
    {
    cmSystemTools::Error("Cannot open file for write: ", 
                         this->TempName.c_str());
    cmSystemTools::ReportLastSystemError("");
    }
  return *this;
}

//----------------------------------------------------------------------------
bool
cmGeneratedFileStream::Close()
{
  // Save whether the temporary output file is valid before closing.
  this->Okay = (*this)?true:false;

  // Close the temporary output file.
  this->Stream::close();

  // Remove the temporary file (possibly by renaming to the real file).
  return this->cmGeneratedFileStreamBase::Close();
}

//----------------------------------------------------------------------------
void cmGeneratedFileStream::SetCopyIfDifferent(bool copy_if_different)
{
  this->CopyIfDifferent = copy_if_different;
}

//----------------------------------------------------------------------------
void cmGeneratedFileStream::SetCompression(bool compression)
{
  this->Compress = compression;
}

//----------------------------------------------------------------------------
void cmGeneratedFileStream::SetCompressionExtraExtension(bool ext)
{
  this->CompressExtraExtension = ext;
}

//----------------------------------------------------------------------------
cmGeneratedFileStreamBase::cmGeneratedFileStreamBase():
  Name(),
  TempName(),
  CopyIfDifferent(false),
  Okay(false),
  Compress(false),
  CompressExtraExtension(true)
{
}

//----------------------------------------------------------------------------
cmGeneratedFileStreamBase::cmGeneratedFileStreamBase(const char* name):
  Name(),
  TempName(),
  CopyIfDifferent(false),
  Okay(false),
  Compress(false),
  CompressExtraExtension(true)
{
  this->Open(name);
}

//----------------------------------------------------------------------------
cmGeneratedFileStreamBase::~cmGeneratedFileStreamBase()
{
  this->Close();
}

//----------------------------------------------------------------------------
void cmGeneratedFileStreamBase::Open(const char* name)
{
  // Save the original name of the file.
  this->Name = name;

  // Create the name of the temporary file.
  this->TempName = name;
  this->TempName += ".tmp";

  // Make sure the temporary file that will be used is not present.
  cmSystemTools::RemoveFile(this->TempName.c_str());

  std::string dir = cmSystemTools::GetFilenamePath(this->TempName);
  cmSystemTools::MakeDirectory(dir.c_str());
}

//----------------------------------------------------------------------------
bool cmGeneratedFileStreamBase::Close()
{
  bool replaced = false;

  std::string resname = this->Name;
  if ( this->Compress && this->CompressExtraExtension )
    {
    resname += ".gz";
    }

  // Only consider replacing the destination file if no error
  // occurred.
  if(!this->Name.empty() &&
    this->Okay &&
    (!this->CopyIfDifferent ||
     cmSystemTools::FilesDiffer(this->TempName.c_str(), resname.c_str())))
    {
    // The destination is to be replaced.  Rename the temporary to the
    // destination atomically.
    if ( this->Compress )
      {
      std::string gzname = this->TempName + ".temp.gz";
      if ( this->CompressFile(this->TempName.c_str(), gzname.c_str()) )
        {
        this->RenameFile(gzname.c_str(), resname.c_str());
        }
      cmSystemTools::RemoveFile(gzname.c_str());
      }
    else
      {
      this->RenameFile(this->TempName.c_str(), resname.c_str());
      }

    replaced = true;
    }

  // Else, the destination was not replaced.
  //
  // Always delete the temporary file. We never want it to stay around.
  cmSystemTools::RemoveFile(this->TempName.c_str());

  return replaced;
}

//----------------------------------------------------------------------------
#ifdef CMAKE_BUILD_WITH_CMAKE
int cmGeneratedFileStreamBase::CompressFile(const char* oldname,
                                            const char* newname)
{
  gzFile gf = gzopen(newname, "w");
  if ( !gf )
    {
    return 0;
    }
  FILE* ifs = fopen(oldname, "r");
  if ( !ifs )
    {
    return 0;
    }
  size_t res;
  const size_t BUFFER_SIZE = 1024;
  char buffer[BUFFER_SIZE];
  while ( (res = fread(buffer, 1, BUFFER_SIZE, ifs)) > 0 )
    {
    if ( !gzwrite(gf, buffer, static_cast<int>(res)) )
      {
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
int cmGeneratedFileStreamBase::CompressFile(const char*, const char*)
{
  return 0;
}
#endif

//----------------------------------------------------------------------------
int cmGeneratedFileStreamBase::RenameFile(const char* oldname,
                                          const char* newname)
{
#ifdef _WIN32
  /* On Windows the move functions will not replace existing files.
     Check if the destination exists.  */
  struct stat newFile;
  if(stat(newname, &newFile) == 0)
    {
    /* The destination exists.  We have to replace it carefully.  The
       MoveFileEx function does what we need but is not available on
       Win9x.  */
    OSVERSIONINFO osv;
    DWORD attrs;

    /* Make sure the destination is not read only.  */
    attrs = GetFileAttributes(newname);
    if(attrs & FILE_ATTRIBUTE_READONLY)
      {
      SetFileAttributes(newname, attrs & ~FILE_ATTRIBUTE_READONLY);
      }

    /* Check the windows version number.  */
    osv.dwOSVersionInfoSize = sizeof(osv);
    GetVersionEx(&osv);
    if(osv.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
      {
      /* This is Win9x.  There is no MoveFileEx implementation.  We
         cannot quite rename the file atomically.  Just delete the
         destination and then move the file.  */
      DeleteFile(newname);
      return MoveFile(oldname, newname);
      }
    else
      {
      /* This is not Win9x.  Use the MoveFileEx implementation.  */
      return MoveFileEx(oldname, newname, MOVEFILE_REPLACE_EXISTING);
      }
    }
  else
    {
    /* The destination does not exist.  Just move the file.  */
    return MoveFile(oldname, newname);
    }
#else
  /* On UNIX we have an OS-provided call to do this atomically.  */
  return rename(oldname, newname) == 0;
#endif
}

//----------------------------------------------------------------------------
void cmGeneratedFileStream::SetName(const char* fname)
{
  if ( !fname )
    {
    this->Name = "";
    return;
    }
  this->Name = fname;
}
