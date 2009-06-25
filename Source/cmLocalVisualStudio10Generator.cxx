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
#include "cmLocalVisualStudio10Generator.h"
#include "cmTarget.h"
#include "cmMakefile.h"
#include "cmVisualStudio10TargetGenerator.h"
#include "cmGlobalVisualStudio7Generator.h"

//----------------------------------------------------------------------------
cmLocalVisualStudio10Generator::cmLocalVisualStudio10Generator()
{

}

cmLocalVisualStudio10Generator::~cmLocalVisualStudio10Generator()
{
}

void cmLocalVisualStudio10Generator::Generate()
{
  
  cmTargets &tgts = this->Makefile->GetTargets();
  // Create the regeneration custom rule.
  if(!this->Makefile->IsOn("CMAKE_SUPPRESS_REGENERATION"))
    {
    // Create a rule to regenerate the build system when the target
    // specification source changes.
    if(cmSourceFile* sf = this->CreateVCProjBuildRule())
      {
      // Add the rule to targets that need it.
      for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); ++l)
        {
        if(l->first != CMAKE_CHECK_BUILD_SYSTEM_TARGET)
          {
          l->second.AddSourceFile(sf);
          }
        }
      }
    }

  for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); ++l)
    {
    cmVisualStudio10TargetGenerator tg(&l->second, 
                                       (cmGlobalVisualStudio7Generator*)
                                       this->GetGlobalGenerator());
    tg.Generate();
    }
  this->WriteStampFiles();
}

