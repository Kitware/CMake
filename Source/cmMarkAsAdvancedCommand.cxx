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
#include "cmMarkAsAdvancedCommand.h"

// cmMarkAsAdvancedCommand
bool cmMarkAsAdvancedCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);

  unsigned int i =0;
  const char* value = "1";
  bool overwrite = false;
  if(args[0] == "CLEAR" || args[0] == "FORCE")
    {
    overwrite = true;
    if(args[0] == "CLEAR")
      {
      value = "0";
      }
    i = 1;
    }
  for(; i < args.size(); ++i)
    {
    std::string variable = args[i];
    cmCacheManager* manager = m_Makefile->GetCacheManager();
    cmCacheManager::CacheIterator it = manager->GetCacheIterator(variable.c_str());
    if ( it.IsAtEnd() )
      {
      m_Makefile->AddCacheDefinition(variable.c_str(), 0, 0,
                                     cmCacheManager::UNINITIALIZED);
      }
    it.Find(variable.c_str());
    if ( it.IsAtEnd() )
      {
      cmSystemTools::Error("This should never happen...");
      return false;
      }
    it.SetProperty("ADVANCED", value);
    }
  return true;
}

