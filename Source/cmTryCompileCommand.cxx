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
#include "cmTryCompileCommand.h"
#include "cmCacheManager.h"

// cmExecutableCommand
bool cmTryCompileCommand::InitialPass(std::vector<std::string> const& argv)
{
  if(argv.size() < 4)
    {
    return false;
    }
  
  const char* sourceDirectory = argv[1].c_str();
  const char* binaryDirectory = argv[2].c_str();
  const char* projectName = argv[3].c_str();
  const char* targetName = 0;
  
  if (argv.size() == 5)
    {
    targetName = argv[4].c_str();
    }
  
  int res = m_Makefile->TryCompile(sourceDirectory, binaryDirectory,
                                   projectName, targetName);
  
  // set the result var to the return value to indicate success or failure
  m_Makefile->AddDefinition(argv[0].c_str(), (res == 0 ? "TRUE" : "FALSE"));
  return true;
}


      
