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
#include "cmSetSourceFilesPropertiesCommand.h"

// cmSetSourceFilesPropertiesCommand
bool cmSetSourceFilesPropertiesCommand::InitialPass(
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
  bool generated = false;
  int numFiles = 0;
  std::vector<std::string>::const_iterator j;
  for(j= args.begin(); j != args.end();++j)
    {
    // old style allows for specifier before PROPERTIES keyword
    if(*j == "ABSTRACT")
      {
      doingFiles = false;
      propertyPairs.push_back("ABSTRACT");
      propertyPairs.push_back("1");
      }
    else if(*j == "WRAP_EXCLUDE")
      {
      doingFiles = false;
      propertyPairs.push_back("WRAP_EXCLUDE");
      propertyPairs.push_back("1");
      }
    else if(*j == "GENERATED")
      {
      doingFiles = false;
      generated = true;
      propertyPairs.push_back("GENERATED");
      propertyPairs.push_back("1");
      }
    else if(*j == "COMPILE_FLAGS")
      {
      doingFiles = false;
      propertyPairs.push_back("COMPILE_FLAGS");
      ++j;
      if(j == args.end())
        {
        this->SetError("called with incorrect number of arguments COMPILE_FLAGS with no flags");
        return false;
        }
      propertyPairs.push_back(*j);
      }
    else if(*j == "OBJECT_DEPENDS")
      {
      doingFiles = false;
      propertyPairs.push_back("OBJECT_DEPENDS");
      ++j;
      if(j == args.end())
        {
        this->SetError("called with incorrect number of arguments "
                       "OBJECT_DEPENDS with no dependencies");
        return false;
        }
      propertyPairs.push_back(*j);
      }
    else if(*j == "PROPERTIES")
      {
      // now loop through the rest of the arguments, new style
      ++j;
      while (j != args.end())
        {
        propertyPairs.push_back(*j);
        if(*j == "GENERATED")
          {
          ++j;
          if(j != args.end() && cmSystemTools::IsOn(j->c_str()))
            {
            generated = true;
            }
          }
        else
          {
          ++j;
          }
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
  
  // now loop over all the files
  int i;
  unsigned int k;
  for(i = 0; i < numFiles; ++i)
    {   
    // get the source file
    cmSourceFile* sf =
      m_Makefile->GetOrCreateSource(args[i].c_str(), generated);
    // now loop through all the props and set them
    for (k = 0; k < propertyPairs.size(); k = k + 2)
      {
      sf->SetProperty(propertyPairs[k].c_str(),propertyPairs[k+1].c_str());
      }
    }
  return true;
}

