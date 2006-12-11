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
#include "cmSetPropertiesCommand.h"
#include "cmSetTargetPropertiesCommand.h"
#include "cmSetTestsPropertiesCommand.h"
#include "cmSetSourceFilesPropertiesCommand.h"

// cmSetPropertiesCommand
bool cmSetPropertiesCommand::InitialPass(
  std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // first collect up the list of files
  std::vector<std::string> propertyPairs;
  bool doingFiles = true;
  int numFiles = 0;
  std::vector<std::string>::const_iterator j;
  for(j= args.begin(); j != args.end();++j)
    {
    if(*j == "PROPERTIES")
      {
      doingFiles = false;
      // now loop through the rest of the arguments, new style
      ++j;
      while (j != args.end())
        {
        propertyPairs.push_back(*j);
        ++j;
        if(j == args.end())
          {
          this->SetError("called with incorrect number of arguments.");
          return false;
          }
        propertyPairs.push_back(*j);
        ++j;
        }
      // break out of the loop because j is already == end
      break;
      }
    else if (doingFiles)
      {
      numFiles++;
      }
    else
      {
      this->SetError("called with illegal arguments, maybe missing "
                     "a PROPERTIES specifier?");
      return false;
      }
    }
  if(propertyPairs.size() == 0)
    {
     this->SetError("called with illegal arguments, maybe missing "
                    "a PROPERTIES specifier?");
     return false;
    }
  
  cmProperty::ScopeType scope;
  const char *scopeName = 0;
  if (args[0] == "GLOBAL" && numFiles == 1)
    {
    scope = cmProperty::GLOBAL;
    }
  else if (args[0] == "DIRECTORY" && numFiles == 1)
    {
    scope = cmProperty::DIRECTORY;
    }
  else if (args[0] == "TARGET" && numFiles == 2)
    {
    scope = cmProperty::TARGET;
    scopeName = args[1].c_str();
    }
  else if (args[0] == "TEST" && numFiles == 2)
    {
    scope = cmProperty::TEST;
    scopeName = args[1].c_str();
    }
  else if (args[0] == "SOURCE_FILE" && numFiles == 2)
    {
    scope = cmProperty::SOURCE_FILE;
    scopeName = args[1].c_str();
    }
  else
    {
    this->SetError("called with illegal arguments.");
    return false;
    }

  switch (scope) 
    {
    case cmProperty::TARGET:
      {
      bool ret = cmSetTargetPropertiesCommand::
        SetOneTarget(scopeName,propertyPairs, this->Makefile);
      if (!ret)
        {
        std::string message = "Can not find target to add properties to: ";
        message += scopeName;
        this->SetError(message.c_str());
        return ret;
        }
      }
      break;
    case cmProperty::DIRECTORY:
      {
      std::string errors;
      bool ret = 
        cmSetDirectoryPropertiesCommand::RunCommand(this->Makefile,
                                                    args.begin() + 2,
                                                    args.end(),
                                                    errors);
      if (!ret)
        {
        this->SetError(errors.c_str());
        return ret;
        }
      }
      break;
    case cmProperty::GLOBAL:
      {
      for(j= propertyPairs.begin(); j != propertyPairs.end(); ++j)
        {
        this->Makefile->GetCMakeInstance()->SetProperty(j->c_str(),
                                                        (++j)->c_str());
        }
      }
      break;
    case cmProperty::TEST:
      {
      std::string errors;
      bool ret = cmSetTestsPropertiesCommand::
        SetOneTest(scopeName,propertyPairs, this->Makefile, errors);
      if (!ret)
        {
        this->SetError(errors.c_str());
        return ret;
        }
      }
      break;
    case cmProperty::SOURCE_FILE:
      {
      std::string errors;
      bool ret = cmSetSourceFilesPropertiesCommand::
        RunCommand(this->Makefile,
                   args.begin()+1, args.begin()+2,
                   args.begin() + 2, args.end(),
                   errors);
      if (!ret)
        {
        this->SetError(errors.c_str());
        }
      return ret;
      }
      break;
    }

  return true;
}

