/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmakewizard.h"
#include "cmake.h"
#include "cmMakefileGenerator.h"
#include "cmCacheManager.h"

int main(int ac, char** av)
{
  bool wiz = false;
  bool command = false;
  std::vector<std::string> args;
  for(int i =0; i < ac; ++i)
    {
    if(strcmp(av[i], "-i") == 0)
      {
      wiz = true;
      }
    else
      {
      if (strcmp(av[i], "-E") == 0)
        {
        command = true;
        }
      else 
        {
        args.push_back(av[i]);
        }
      }
    }

  if(command)
    {
    int ret = cmake::CMakeCommand(args);
    return ret;
    }
  if (wiz)
    {
    cmakewizard wizard;
    wizard.RunWizard(args); 
    cmMakefileGenerator::UnRegisterGenerators();
    cmCacheManager::DeleteInstance();
    return 0;
    }
  cmake cm;
  int ret = cm.Generate(args);
  cmMakefileGenerator::UnRegisterGenerators();
  cmCacheManager::DeleteInstance();
  return ret;  
}
