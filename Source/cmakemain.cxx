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

int main(int ac, char** av)
{
  bool wiz = false;
  std::vector<std::string> args;
  for(int i =0; i < ac; ++i)
    {
    if(strcmp(av[i], "-i") == 0)
      {
      wiz = true;
      }
    else
      {
      args.push_back(av[i]);
      }
    }
  if(!wiz)
    {
    cmake cm;
    int ret = cm.Generate(args);
    cmMakefileGenerator::UnRegisterGenerators();
    return ret;
    }
  cmakewizard wizard;
  wizard.RunWizard(args); 
  cmMakefileGenerator::UnRegisterGenerators();
  return 0;
}
