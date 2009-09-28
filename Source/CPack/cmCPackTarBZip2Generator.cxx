/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackTarBZip2Generator.h"

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"

#include <cmsys/SystemTools.hxx>

// Includes needed for implementation of RenameFile.  This is not in
// system tools because it is not implemented robustly enough to move
// files across directories.
#ifdef _WIN32
# include <windows.h>
# include <sys/stat.h>
#endif

//----------------------------------------------------------------------
cmCPackTarBZip2Generator::cmCPackTarBZip2Generator()
{
  this->Compress = false;
}

//----------------------------------------------------------------------
cmCPackTarBZip2Generator::~cmCPackTarBZip2Generator()
{
}

//----------------------------------------------------------------------
int cmCPackTarBZip2Generator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", "1");
  std::vector<std::string> path;
  std::string pkgPath = cmSystemTools::FindProgram("bzip2", path, false);
  if ( pkgPath.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find BZip2" << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_INSTALLER_PROGRAM", pkgPath.c_str());
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Found Compress program: "
    << pkgPath.c_str()
    << std::endl);

  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
int cmCPackTarBZip2Generator::BZip2File(const char* packageDirFileName)
{
  int retVal = 0;
  cmOStringStream dmgCmd1;
  dmgCmd1 << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM")
    << "\" \"" << packageDirFileName
    << "\"";
  retVal = -1;
  std::string output;
  int res = cmSystemTools::RunSingleCommand(dmgCmd1.str().c_str(), &output,
    &retVal, 0, this->GeneratorVerbose, 0);
  if ( !res || retVal )
    {
    std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    tmpFile += "/CompressBZip2.log";
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << dmgCmd1.str().c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running BZip2 command: "
      << dmgCmd1.str().c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------
int cmCPackTarBZip2Generator::CompressFiles(const char* outFileName,
  const char* toplevel, const std::vector<std::string>& files)
{
  std::string packageDirFileName
    = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
  packageDirFileName += ".tar";
  std::string output;
  if ( !this->Superclass::CompressFiles(packageDirFileName.c_str(),
      toplevel, files) )
    {
    return 0;
    }

  if(!this->BZip2File(packageDirFileName.c_str()))
    {
    return 0;
    }
  
  std::string compressOutFile = packageDirFileName + ".bz2";
  if ( !cmSystemTools::SameFile(compressOutFile.c_str(), outFileName ) )
    {
    if ( !this->RenameFile(compressOutFile.c_str(), outFileName) )
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem renaming: \""
        << compressOutFile.c_str() << "\" to \""
        << (outFileName ? outFileName : "(NULL)") << std::endl);
      return 0;
      }
    }

  return 1;
}

//----------------------------------------------------------------------------
int cmCPackTarBZip2Generator::RenameFile(const char* oldname,
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

