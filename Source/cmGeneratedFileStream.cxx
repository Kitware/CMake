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

//----------------------------------------------------------------------------
cmGeneratedFileStream::cmGeneratedFileStream(const char* name, bool quiet):
  cmGeneratedFileStreamBase(name),
  Stream(m_TempName.c_str())
{
  // Check if the file opened.
  if(!*this && !quiet)
    {
    cmSystemTools::Error("Cannot open file for write: ", m_TempName.c_str());
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
  m_Okay = (*this)?true:false;
}

//----------------------------------------------------------------------------
void cmGeneratedFileStream::SetCopyIfDifferent(bool copy_if_different)
{
  m_CopyIfDifferent = copy_if_different;
}

//----------------------------------------------------------------------------
cmGeneratedFileStreamBase::cmGeneratedFileStreamBase(const char* name):
  m_Name(name),
  m_TempName(name),
  m_CopyIfDifferent(false),
  m_Okay(false)
{
  // Create the name of the temporary file.
  m_TempName += ".tmp";

  // Make sure the temporary file that will be used is not present.
  cmSystemTools::RemoveFile(m_TempName.c_str());
}

//----------------------------------------------------------------------------
cmGeneratedFileStreamBase::~cmGeneratedFileStreamBase()
{
  // Only consider replacing the destination file if no error
  // occurred.
  if(m_Okay &&
     (!m_CopyIfDifferent ||
      cmSystemTools::FilesDiffer(m_TempName.c_str(), m_Name.c_str())))
    {
    // The destination is to be replaced.  Rename the temporary to the
    // destination atomically.
    this->RenameFile(m_TempName.c_str(), m_Name.c_str());
    }
  else
    {
    // The destination was not replaced.  Just delete the temporary
    // file.
    cmSystemTools::RemoveFile(m_TempName.c_str());
    }
}

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
