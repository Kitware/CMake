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

#include "cmGlobalKdevelopGenerator.h"
#include "cmLocalKdevelopGenerator.h"
#include "cmMakefile.h"
#include "cmake.h"

cmGlobalKdevelopGenerator::cmGlobalKdevelopGenerator()
{
  // This type of makefile always requires unix style paths
  m_ForceUnixPaths = true;
  m_FindMakeProgramFile = "CMakeUnixFindMake.cmake";
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalKdevelopGenerator::CreateLocalGenerator()
{
  cmLocalGenerator *lg = new cmLocalKdevelopGenerator;
  lg->SetGlobalGenerator(this);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalKdevelopGenerator::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.name = this->GetName();
  entry.brief = "Generates KDevelop 3 project files.";
  entry.full =
    "A hierarchy of UNIX makefiles is generated into the build tree.  Any "
    "standard UNIX-style make program can build the project through the "
     "default make target.  A \"make install\" target is also provided."
     "Additionally in each directory which features executable or library"
     " targets a KDevelop 3 project file is generated.\n"
     "If you change the settings using KDevelop your cmake will try its best"
     "to keep your changes when regenerating the project files.";
}
