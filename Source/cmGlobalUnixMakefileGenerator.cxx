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

#include "cmGlobalUnixMakefileGenerator.h"
#include "cmLocalUnixMakefileGenerator.h"
#include "cmMakefile.h"
#include "cmake.h"

cmGlobalUnixMakefileGenerator::cmGlobalUnixMakefileGenerator()
{
  // This type of makefile always requires unix style paths
  m_ForceUnixPaths = true;
  m_FindMakeProgramFile = "CMakeUnixFindMake.cmake";
}

void cmGlobalUnixMakefileGenerator::EnableLanguage(std::vector<std::string>const& languages,
                                                   cmMakefile *mf)
{
  mf->AddDefinition("CMAKE_CFG_INTDIR",".");
  this->cmGlobalGenerator::EnableLanguage(languages, mf);
  std::string path;
  for(std::vector<std::string>::const_iterator l = languages.begin();
      l != languages.end(); ++l)
    {
    const char* lang = l->c_str();
    std::string langComp = "CMAKE_";
    langComp += lang;
    langComp += "_COMPILER";
    
    if(!mf->GetDefinition(langComp.c_str()))
      {
      cmSystemTools::Error(langComp.c_str(), " not set, after EnableLanguage");
      continue;
      }
    const char* cc = mf->GetRequiredDefinition(langComp.c_str());
    path = cmSystemTools::FindProgram(cc);
    if(path.size() == 0)
      {
      std::string message = "your ";
      message += lang;
      message += " compiler: ";
      if(cc)
        {
        message +=  cc;
        }
      else
        {
        message += "(NULL)";
        }
      message += " was not found in your path.   "
        "For CMake to correctly use try compile commands, the compiler must "
        "be in your path.   Please add the compiler to your PATH environment,"
        " and re-run CMake.";
        cmSystemTools::Error(message.c_str());
      }
    }
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalUnixMakefileGenerator::CreateLocalGenerator()
{
  cmLocalGenerator *lg = new cmLocalUnixMakefileGenerator;
  lg->SetGlobalGenerator(this);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalUnixMakefileGenerator::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.name = this->GetName();
  entry.brief = "Generates standard UNIX makefiles.";
  entry.full =
    "A hierarchy of UNIX makefiles is generated into the build tree.  Any "
    "standard UNIX-style make program can build the project through the "
    "default make target.  A \"make install\" target is also provided.";
}
