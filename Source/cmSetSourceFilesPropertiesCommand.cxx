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
#include "cmSetSourceFilesPropertiesCommand.h"

// cmSetSourceFilesPropertiesCommand
bool cmSetSourceFilesPropertiesCommand::InitialPass(
  std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  cmSystemTools::ExpandListArguments(argsIn, args);

  // first collect up the list of files
  std::vector<std::string> propertyPairs;
  bool doingFiles = true;
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
    else if(*j == "PROPERTIES")
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
  int i, k;
  for(i = 0; i < numFiles; ++i)
    {   
    // if the file is already in the makefile just set properites on it
    cmSourceFile* sf = m_Makefile->GetSource(args[i].c_str());
    if(sf)
      {
      // now loop through all the props and set them
      for (k = 0; k < propertyPairs.size(); k = k + 2)
        {
        sf->SetProperty(propertyPairs[k].c_str(),propertyPairs[k+1].c_str());
        }
      }
    // if file is not already in the makefile, then add it
    else
      { 
      std::string newfile = args[i];
      cmSourceFile file; 
      std::string path = cmSystemTools::GetFilenamePath(newfile);
      // now loop through all the props and set them
      for (k = 0; k < propertyPairs.size(); k = k + 2)
        {
        file.SetProperty(propertyPairs[k].c_str(),propertyPairs[k+1].c_str());
        }
      if(file.GetPropertyAsBool("GENERATED"))
        {
        std::string ext = cmSystemTools::GetFilenameExtension(newfile);
        std::string name_no_ext = cmSystemTools::GetFilenameName(newfile.c_str());
        name_no_ext = name_no_ext.substr(0, name_no_ext.length()-ext.length());
        if ( ext.length() && ext[0] == '.' )
          {
          ext = ext.substr(1);
          }
        if((path.size() && path[0] == '/') ||
           (path.size() > 1 && path[1] == ':'))
          {
          file.SetName(name_no_ext.c_str(), path.c_str(), ext.c_str(), false);
          }
        else
          {
          file.SetName(name_no_ext.c_str(), m_Makefile->GetCurrentOutputDirectory(), 
                       ext.c_str(), false);
          }
        }
      else
        {
        // if this is a full path then 
        if((path.size() && path[0] == '/') ||
           (path.size() > 1 && path[1] == ':'))
          {
          file.SetName(cmSystemTools::GetFilenameName(newfile.c_str()).c_str(), 
                       path.c_str(),
                       m_Makefile->GetSourceExtensions(),
                       m_Makefile->GetHeaderExtensions());
          }
        else
          {
          file.SetName(newfile.c_str(), m_Makefile->GetCurrentDirectory(),
                       m_Makefile->GetSourceExtensions(),
                       m_Makefile->GetHeaderExtensions());
          }    
        }
      // add the source file to the makefile
      m_Makefile->AddSource(file);
      }
    }
  return true;
}

