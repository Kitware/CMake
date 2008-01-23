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
#include "cmSetTestsPropertiesCommand.h"

#include "cmake.h"
#include "cmTest.h"

// cmSetTestsPropertiesCommand
bool cmSetTestsPropertiesCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
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
      this->SetError("called with illegal arguments, maybe "
                     "missing a PROPERTIES specifier?");
      return false;
      }
    }
  if(propertyPairs.size() == 0)
    {
    this->SetError("called with illegal arguments, maybe "
                   "missing a PROPERTIES specifier?");
    return false;
    }


  // now loop over all the targets
  int i;
  for(i = 0; i < numFiles; ++i)
    {   
    std::string errors;
    bool ret = 
      cmSetTestsPropertiesCommand::SetOneTest(args[i].c_str(), 
                                              propertyPairs,
                                              this->Makefile, errors);
    if (!ret)
      {
      this->SetError(errors.c_str());
      return ret;
      }
    }

  return true;
}


bool cmSetTestsPropertiesCommand
::SetOneTest(const char *tname, 
             std::vector<std::string> &propertyPairs,
             cmMakefile *mf, std::string &errors)
{
  std::vector<cmTest*> &tests = *mf->GetTests();
  // now loop over all the targets
  unsigned int k;
  bool found = false;
  // if the file is already in the makefile just set properites on it
  std::vector<cmTest*>::iterator it;
  for ( it = tests.begin(); it != tests.end(); ++ it )
    {
    cmTest* test = *it;
    if ( !strcmp(test->GetName(),tname ))
      {
      // now loop through all the props and set them
      for (k = 0; k < propertyPairs.size(); k = k + 2)
        {
        test->SetProperty(propertyPairs[k].c_str(),
                          propertyPairs[k+1].c_str());
        }
      found = true;
      break;
      }
    }
  
  // if file is not already in the makefile, then add it
  if ( ! found )
    { 
    errors = "Can not find test to add properties to: ";
    errors += tname;
    return false;
    } 

  return true;
}

