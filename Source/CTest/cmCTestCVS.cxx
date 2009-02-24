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
#include "cmCTestCVS.h"

//----------------------------------------------------------------------------
cmCTestCVS::cmCTestCVS(cmCTest* ct, std::ostream& log): cmCTestVC(ct, log)
{
}

//----------------------------------------------------------------------------
cmCTestCVS::~cmCTestCVS()
{
}
