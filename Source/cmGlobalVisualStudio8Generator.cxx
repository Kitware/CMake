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
#include "windows.h" // this must be first to define GetCurrentDirectory
#include "cmGlobalVisualStudio8Generator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"
#include "cmake.h"



cmGlobalVisualStudio8Generator::cmGlobalVisualStudio8Generator()
{
  m_FindMakeProgramFile = "CMakeVS8FindMake.cmake";
}



///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalVisualStudio8Generator::CreateLocalGenerator()
{
  cmLocalVisualStudio7Generator *lg = new cmLocalVisualStudio7Generator;
  lg->SetVersion8();
  lg->SetGlobalGenerator(this);
  return lg;
}

  
// ouput standard header for dsw file
void cmGlobalVisualStudio8Generator::WriteSLNHeader(std::ostream& fout)
{
  fout << "Microsoft Visual Studio Solution File, Format Version 9.00\n";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio8Generator::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.name = this->GetName();
  entry.brief = "Generates Visual Studio .NET 2005 project files.";
  entry.full = "";
}
