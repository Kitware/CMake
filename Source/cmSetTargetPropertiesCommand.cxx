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
#include "cmSetTargetPropertiesCommand.h"

// cmSetTargetPropertiesCommand
bool cmSetTargetPropertiesCommand::InitialPass(
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
      this->SetError("called with illegal arguments, maybe missing a PROPERTIES specifier?");
      return false;
      }
    }
  if(propertyPairs.size() == 0)
    {
     this->SetError("called with illegal arguments, maybe missing a PROPERTIES specifier?");
     return false;
    }
  
  cmTargets& targets = m_Makefile->GetTargets();
  // now loop over all the targets
  int i;
  unsigned int k;
  for(i = 0; i < numFiles; ++i)
    {   
    // if the file is already in the makefile just set properites on it
    cmTargets::iterator t = targets.find(args[i]);
    if ( t != targets.end())
      {
      cmTarget& target = t->second;
      // now loop through all the props and set them
      for (k = 0; k < propertyPairs.size(); k = k + 2)
        {
        target.SetProperty(propertyPairs[k].c_str(),propertyPairs[k+1].c_str());
        }
      }
    // if file is not already in the makefile, then add it
    else
      { 
      std::string message = "Can not find target to add properties to: ";
      message += args[i];
      this->SetError(message.c_str());
      }
    }
  return true;
}

