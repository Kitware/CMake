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
#include "cmEnableLanguageCommand.h"

// cmEnableLanguageCommand
bool cmEnableLanguageCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  bool optional = false;
  std::vector<std::string> languages;
  if(args.size() < 1 )
    {
    this->SetError
      ("called with incorrect number of arguments");
    return false;
    } 
  if(this->Makefile->GetCMakeInstance()->GetIsInTryCompile())
    {
    this->SetError
      ("called from a try_compile, "
       "all languages must be enabled before trying them.");
    return false;
    }
  for (std::vector<std::string>::const_iterator it = args.begin();
       it != args.end();
       ++it)
    {
    if ((*it) == "OPTIONAL")
      {
      optional = true;
      }
    else
      {
      languages.push_back(*it);
      }
    }

  this->Makefile->EnableLanguage(languages, optional);
  return true;
}

