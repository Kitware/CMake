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
#include "cmAuxSourceDirectoryCommand.h"
#include "cmSourceFile.h"

#include <cmsys/Directory.hxx>

// cmAuxSourceDirectoryCommand
bool cmAuxSourceDirectoryCommand::InitialPass
(std::vector<std::string> const& args)
{
  if(args.size() < 2 || args.size() > 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::string sourceListValue;
  std::string templateDirectory = args[0];
  this->Makefile->AddExtraDirectory(templateDirectory.c_str());
  std::string tdir = this->Makefile->GetCurrentDirectory();
  tdir += "/";
  tdir += templateDirectory;

  // was the list already populated
  const char *def = this->Makefile->GetDefinition(args[1].c_str());  
  if (def)
    {
    sourceListValue = def;
    }
  
  // Load all the files in the directory
  cmsys::Directory dir;
  if(dir.Load(tdir.c_str()))
    {
    size_t numfiles = dir.GetNumberOfFiles();
    for(size_t i =0; i < numfiles; ++i)
      {
      std::string file = dir.GetFile(static_cast<unsigned long>(i));
      // Split the filename into base and extension
      std::string::size_type dotpos = file.rfind(".");
      if( dotpos != std::string::npos )
        {
        std::string ext = file.substr(dotpos+1);
        file = file.substr(0, dotpos);
        // Process only source files
        if( file.size() != 0
            && std::find( this->Makefile->GetSourceExtensions().begin(),
                          this->Makefile->GetSourceExtensions().end(), ext )
                 != this->Makefile->GetSourceExtensions().end() )
          {
          std::string fullname = templateDirectory;
          fullname += "/";
          fullname += file;
          // add the file as a class file so 
          // depends can be done
          cmSourceFile cmfile;
          cmfile.SetName(fullname.c_str(), 
                         this->Makefile->GetCurrentDirectory(),
                         this->Makefile->GetSourceExtensions(),
                         this->Makefile->GetHeaderExtensions());
          cmfile.SetProperty("ABSTRACT","0");
          this->Makefile->AddSource(cmfile);
          if (sourceListValue.size() > 0)
            {
            sourceListValue += ";";
            }
          sourceListValue += cmfile.GetSourceName();
          sourceListValue += ".";
          sourceListValue += cmfile.GetSourceExtension();
          }
        }
      }
    }
  this->Makefile->AddDefinition(args[1].c_str(), sourceListValue.c_str());  
  return true;
}

