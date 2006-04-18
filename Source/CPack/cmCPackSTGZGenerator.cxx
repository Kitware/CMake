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

#include "cmCPackSTGZGenerator.h"

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmCPackLog.h"

#include <cmsys/ios/sstream>
#include <sys/types.h>
#include <sys/stat.h>

//----------------------------------------------------------------------
cmCPackSTGZGenerator::cmCPackSTGZGenerator()
{
}

//----------------------------------------------------------------------
cmCPackSTGZGenerator::~cmCPackSTGZGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackSTGZGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", "0");

  std::string inFile = this->FindTemplate("CPack.STGZ_Header.sh.in");
  if ( inFile.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find template file: "
      << inFile.c_str() << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_STGZ_HEADER_FILE", inFile.c_str());
  this->SetOption("CPACK_AT_SIGN", "@");

  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
int cmCPackSTGZGenerator::CompressFiles(const char* outFileName,
  const char* toplevel, const std::vector<std::string>& files)
{
  if ( !this->Superclass::CompressFiles(outFileName, toplevel, files) )
    {
    return 0;
    }
  return cmSystemTools::SetPermissions(outFileName,
#if defined( _MSC_VER ) || defined( __MINGW32__ )
    S_IREAD | S_IWRITE | S_IEXEC
#elif defined( __BORLANDC__ )
    S_IRUSR | S_IWUSR | S_IXUSR
#else
    S_IRUSR | S_IWUSR | S_IXUSR |
    S_IRGRP | S_IWGRP | S_IXGRP |
    S_IROTH | S_IWOTH | S_IXOTH
#endif
  );
}

//----------------------------------------------------------------------
int cmCPackSTGZGenerator::GenerateHeader(std::ostream* os)
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Writing header" << std::endl);
  cmsys_ios::ostringstream str;
  int counter = 0;

  const char headerLengthTag[] = "###CPACK_HEADER_LENGTH###";

  // Create the header
  std::string inFile = this->GetOption("CPACK_STGZ_HEADER_FILE");
  std::string line;
  std::ifstream ifs(inFile.c_str());
  std::string packageHeaderText;
  while ( cmSystemTools::GetLineFromStream(ifs, line) )
    {
    packageHeaderText += line + "\n";
    }

  // Configure in the values
  std::string res;
  this->ConfigureString(packageHeaderText, res);

  // Count the lines
  const char* ptr = res.c_str();
  while ( *ptr )
    {
    if ( *ptr == '\n' )
      {
      counter ++;
      }
    ++ptr;
    }
  counter ++;
  cmCPackLogger(cmCPackLog::LOG_ERROR, "Counter: " << counter << std::endl);
  char buffer[1024];
  sprintf(buffer, "%d", counter);
  cmSystemTools::ReplaceString(res, headerLengthTag, buffer);

  // Write in file
  *os << res.c_str();
  return this->Superclass::GenerateHeader(os);
}
