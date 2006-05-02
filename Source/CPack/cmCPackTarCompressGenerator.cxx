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

#include "cmCPackTarCompressGenerator.h"

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"

#include <cmsys/SystemTools.hxx>

//----------------------------------------------------------------------
cmCPackTarCompressGenerator::cmCPackTarCompressGenerator()
{
}

//----------------------------------------------------------------------
cmCPackTarCompressGenerator::~cmCPackTarCompressGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackTarCompressGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", "1");
  std::vector<std::string> path;
  std::vector<std::string> names;
  names.push_back("tar");
  names.push_back("gtar");
  std::string pkgPath = cmSystemTools::FindProgram(names, path, false);
  if ( pkgPath.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find Tar" << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_INSTALLER_PROGRAM_TAR", pkgPath.c_str());
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Found Tar program: "
    << pkgPath.c_str()
    << std::endl);
  pkgPath = cmSystemTools::FindProgram("compress", path, false);
  if ( pkgPath.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find Compress" << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_INSTALLER_PROGRAM", pkgPath.c_str());
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Found Compress program: "
    << pkgPath.c_str()
    << std::endl);

  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
int cmCPackTarCompressGenerator::CompressFiles(const char* outFileName,
  const char* toplevel, const std::vector<std::string>& files)
{
  std::string packageDirFileName
    = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
  packageDirFileName += ".tar";
  cmOStringStream dmgCmd;
  dmgCmd << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM_TAR")
    << "\" cvf \"" << packageDirFileName
    << "\"";
  std::vector<std::string>::const_iterator fileIt;
  for ( fileIt = files.begin(); fileIt != files.end(); ++ fileIt )
    {
    dmgCmd << " \""
      << cmSystemTools::RelativePath(toplevel, fileIt->c_str())
      << "\"";
    }
  std::string output;
  int retVal = -1;
  int res = cmSystemTools::RunSingleCommand(dmgCmd.str().c_str(), &output,
    &retVal, toplevel, this->GeneratorVerbose, 0);
  if ( !res || retVal )
    {
    std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    tmpFile += "/CompressTar.log";
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << dmgCmd.str().c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running Tar command: "
      << dmgCmd.str().c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }

  cmOStringStream dmgCmd1;
  dmgCmd1 << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM")
    << "\" \"" << packageDirFileName
    << "\"";
  retVal = -1;
  res = cmSystemTools::RunSingleCommand(dmgCmd1.str().c_str(), &output,
    &retVal, toplevel, this->GeneratorVerbose, 0);
  if ( !res || retVal )
    {
    std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    tmpFile += "/CompressCompress.log";
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << dmgCmd1.str().c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running Compress command: "
      << dmgCmd1.str().c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }

  std::string compressOutFile = packageDirFileName + ".Z";
  if ( !cmSystemTools::SameFile(compressOutFile.c_str(), outFileName ) )
    {
    if ( !this->RenameFile(compressOutFile.c_str(), outFileName) )
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem renaming: \""
        << compressOutFile.c_str() << "\" to \""
        << outFileName << std::endl);
      return 0;
      }
    }

  return 1;
}

//----------------------------------------------------------------------------
int cmCPackTarCompressGenerator::RenameFile(const char* oldname,
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

