/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  Copyright (c) 2004 Alexander Neundorf, neundorf@kde.org. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmGlobalGenerator.h"
#include "cmLocalKdevelopGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmSourceFile.h"
#include "cmMakeDepend.h"
#include "cmCacheManager.h"
#include "cmGeneratedFileStream.h"
#include "cmake.h"
#include <cmsys/RegularExpression.hxx>


cmLocalKdevelopGenerator::cmLocalKdevelopGenerator()
  :cmLocalUnixMakefileGenerator2()
{
}

cmLocalKdevelopGenerator::~cmLocalKdevelopGenerator()
{
}


void cmLocalKdevelopGenerator::Generate(bool fromTheTop)
{
  cmLocalUnixMakefileGenerator2::Generate(fromTheTop); 
  return;
}
