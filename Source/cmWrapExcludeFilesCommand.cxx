/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#include "cmWrapExcludeFilesCommand.h"

// cmWrapExcludeFilesCommand
bool cmWrapExcludeFilesCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  cmMakefile::SourceMap &Classes = m_Makefile->GetSources();
  for(std::vector<std::string>::iterator j = args.begin();
      j != args.end(); ++j)
    {   
    for(cmMakefile::SourceMap::iterator l = Classes.begin(); 
        l != Classes.end(); l++)
      {
      for(std::vector<cmSourceFile>::iterator i = l->second.begin(); 
          i != l->second.end(); i++)
        {
        if(i->GetSourceName() == (*j))
          {
          i->SetWrapExclude(true);
          }
        }
      }
    }
  return true;
}

