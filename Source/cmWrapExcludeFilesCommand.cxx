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
#include "cmWrapExcludeFilesCommand.h"

// cmWrapExcludeFilesCommand
bool cmWrapExcludeFilesCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  const char* versionValue
    = m_Makefile->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION");
  if (versionValue && atof(versionValue) > 1.2)
    {
    this->SetError("The WRAP_EXCLUDE_FILES command has been deprecated in CMake version 1.4. You should use the SET_SOURCE_FILES_PROPERTIES command instead.\n");
    return false;
    }

  if(argsIn.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args; 
  cmSystemTools::ExpandListArguments(argsIn, args);
  cmMakefile::SourceMap &Classes = m_Makefile->GetSources();
  for(std::vector<std::string>::const_iterator j = args.begin();
      j != args.end(); ++j)
    {   
    for(cmMakefile::SourceMap::iterator l = Classes.begin(); 
        l != Classes.end(); l++)
      {
      for(std::vector<cmSourceFile*>::iterator i = l->second.begin(); 
          i != l->second.end(); i++)
        {
        if((*i)->GetSourceName() == (*j))
          {
          (*i)->SetWrapExclude(true);
          }
        }
      }
    }
  return true;
}

