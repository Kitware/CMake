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
#include "cmAuxSourceDirectoryCommand.h"
#include "cmDirectory.h"

// cmAuxSourceDirectoryCommand
bool cmAuxSourceDirectoryCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 1 || args.size() > 1)
    {
    this->SetError("PROJECT called with incorrect number of arguments");
    return false;
    }
  
  std::string templateDirectory = args[0];
  m_Makefile->AddExtraDirectory(templateDirectory.c_str());
  std::string tdir = m_Makefile->GetCurrentDirectory();
  tdir += "/";
  tdir += templateDirectory;
  // Load all the files in the directory
  cmDirectory dir;
  if(dir.Load(tdir.c_str()))
    {
    int numfiles = dir.GetNumberOfFiles();
    for(int i =0; i < numfiles; ++i)
      {
      std::string file = dir.GetFile(i);
      // ignore files less than f.cxx in length
      if(file.size() > 4)
        {
        // Remove the extension
        std::string::size_type dotpos = file.rfind(".");
        file = file.substr(0, dotpos);
        std::string fullname = templateDirectory;
        fullname += "/";
        fullname += file;
        // add the file as a class file so 
        // depends can be done
        cmClassFile cmfile;
        cmfile.SetName(fullname.c_str(), m_Makefile->GetCurrentDirectory());
        cmfile.m_AbstractClass = false;
        m_Makefile->AddClass(cmfile);
        }
      }
    }
  return true;
}

