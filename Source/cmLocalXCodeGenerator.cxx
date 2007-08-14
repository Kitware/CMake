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
#include "cmLocalXCodeGenerator.h"
#include "cmGlobalXCodeGenerator.h"
#include "cmSourceFile.h"

//----------------------------------------------------------------------------
cmLocalXCodeGenerator::cmLocalXCodeGenerator()
{
  // the global generator does this, so do not
  // put these flags into the language flags
  this->EmitUniversalBinaryFlags = false;
}

//----------------------------------------------------------------------------
cmLocalXCodeGenerator::~cmLocalXCodeGenerator()
{
}

//----------------------------------------------------------------------------
std::string
cmLocalXCodeGenerator::GetTargetDirectory(cmTarget const&) const
{
  // No per-target directory for this generator (yet).
  return "";
}

//----------------------------------------------------------------------------
void cmLocalXCodeGenerator::
GetTargetObjectFileDirectories(cmTarget* target,
                               std::vector<std::string>& 
                               dirs)
{
  cmGlobalXCodeGenerator* g = 
    (cmGlobalXCodeGenerator*)this->GetGlobalGenerator();
  g->SetCurrentLocalGenerator(this);
  g->GetTargetObjectFileDirectories(target,
                                    dirs);
}
