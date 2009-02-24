/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmCTestSVN.h"

//----------------------------------------------------------------------------
cmCTestSVN::cmCTestSVN(cmCTest* ct, std::ostream& log): cmCTestVC(ct, log)
{
}

//----------------------------------------------------------------------------
cmCTestSVN::~cmCTestSVN()
{
}

//----------------------------------------------------------------------------
void cmCTestSVN::CleanupImpl()
{
  const char* svn = this->CommandLineTool.c_str();
  const char* svn_cleanup[] = {svn, "cleanup", 0};
  OutputLogger out(this->Log, "cleanup-out> ");
  OutputLogger err(this->Log, "cleanup-err> ");
  this->RunChild(svn_cleanup, &out, &err);
}
