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
bool cmSetSourceFilesPropertiesCommand::InitialPass(std::vector<std::string> const& 
                                                 args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::vector<std::string>::const_iterator j;
  // first collect up all the flags that need to be set on the file
  bool abstract = false;
  bool wrap_exclude = false;
  bool generated = false;
  std::string flags;
  for(j= args.begin(); j != args.end();++j)
    {
    if(*j == "ABSTRACT")
      {
      abstract = true;
      }
    else if(*j == "WRAP_EXCLUDE")
      {
      wrap_exclude = true;
      }
    else if(*j == "GENERATED")
      {
      generated = true;
      }
    else if(*j == "FLAGS")
      {
      ++j;
      if(j == args.end())
        {
         this->SetError("called with incorrect number of arguments FLAGS with no flags");
         return false;
        }
      flags = *j;
      }
    }
  // now loop over all the files
  for(j = args.begin(); j != args.end(); ++j)
    {   
    // at the sign of the first property exit the loop
    if(*j == "ABSTRACT" || *j == "WRAP_EXCLUDE" || *j == "FLAGS")
      {
      break;
      }
    // if the file is already in the makefile just set properites on it
    cmSourceFile* sf = m_Makefile->GetSource(j->c_str());
    if(sf)
      {
      if(flags.size())
        {
        sf->SetCompileFlags(flags.c_str());
        }
      sf->SetIsAnAbstractClass(abstract);
      sf->SetWrapExclude(wrap_exclude);
      }
    // if file is not already in the makefile, then add it
    else
      { 
      std::string newfile = *j;
      cmSourceFile file; 
      std::string path = cmSystemTools::GetFilenamePath(newfile);
      // set the flags
      file.SetIsAnAbstractClass(abstract);
      file.SetWrapExclude(wrap_exclude);
      if(flags.size())
        {
        file.SetCompileFlags(flags.c_str());
        }
      if(generated)
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

