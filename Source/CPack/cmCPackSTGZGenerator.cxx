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


//----------------------------------------------------------------------
cmCPackSTGZGenerator::cmCPackSTGZGenerator()
{
}

//----------------------------------------------------------------------
cmCPackSTGZGenerator::~cmCPackSTGZGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackSTGZGenerator::GenerateHeader(std::ostream* os)
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Writing header" << std::endl);
  *os
    << "#!/bin/sh" << std::endl
    << "echo \"" << this->GetOption("CPACK_PACKAGE_NAME")
    << " - self-extracting archive.\"" << std::endl
    << "echo \"If you want to stop extracting, please press <ctrl-C>.\""
    << std::endl
    << "read line" << std::endl
    << "echo \"Extracting... Please wait...\"" << std::endl
    << "echo \"\"" << std::endl
    << "" << std::endl
    << "# take the archive portion of this file and pipe it to tar"
    << std::endl
    << "# the NUMERIC parameter in this command should be one more"
    << std::endl
    << "# than the number of lines in this header file" << std::endl
    << "tail +18 \"$0\" | gunzip | tar xf -" << std::endl
    << "" << std::endl
    << "exit 0" << std::endl
    << "echo \"\"" << std::endl
    << "#-----------------------------------------------------------"
    << std::endl
    << "#      Start of TAR.GZ file" << std::endl
    << "#-----------------------------------------------------------"
    << std::endl;
  return this->Superclass::GenerateHeader(os);
}
