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
#include "cmAuxSourceDirectoryCommand.h"
#include "cmDirectory.h"

// cmAuxSourceDirectoryCommand
bool cmAuxSourceDirectoryCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2 || args.size() > 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  std::string sourceListValue;
  std::string templateDirectory = args[0];
  m_Makefile->AddExtraDirectory(templateDirectory.c_str());
  std::string tdir = m_Makefile->GetCurrentDirectory();
  tdir += "/";
  tdir += templateDirectory;
  // Load all the files in the directory
  cmDirectory dir;
  if(dir.Load(tdir.c_str()))
    {
    size_t numfiles = dir.GetNumberOfFiles();
    for(size_t i =0; i < numfiles; ++i)
      {
      std::string file = dir.GetFile(i);
      // Split the filename into base and extension
      std::string::size_type dotpos = file.rfind(".");
      if( dotpos != std::string::npos )
        {
        std::string ext = file.substr(dotpos+1);
        file = file.substr(0, dotpos);
        // Process only source files
        if( file.size() != 0
            && std::find( m_Makefile->GetSourceExtensions().begin(),
                          m_Makefile->GetSourceExtensions().end(), ext )
                 != m_Makefile->GetSourceExtensions().end() )
          {
          std::string fullname = templateDirectory;
          fullname += "/";
          fullname += file;
          // add the file as a class file so 
          // depends can be done
          cmSourceFile cmfile;
          cmfile.SetName(fullname.c_str(), m_Makefile->GetCurrentDirectory(),
                         m_Makefile->GetSourceExtensions(),
                         m_Makefile->GetHeaderExtensions());
          cmfile.SetIsAnAbstractClass(false);
          m_Makefile->AddSource(cmfile);
          if (sourceListValue.size() > 0)
            {
            sourceListValue += ";";
            }
          sourceListValue += cmfile.GetSourceName();
          }
        }
      }
    }
  m_Makefile->AddDefinition(args[1].c_str(), sourceListValue.c_str());  
  return true;
}

