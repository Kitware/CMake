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
#include "cmEnableTestingCommand.h"

// we do this in the final pass so that we now the subdirs have all 
// been defined
bool cmEnableTestingCommand::InitialPass(std::vector<std::string> const&)
{
  m_Makefile->AddDefinition("CMAKE_TESTING_ENABLED","1");
  return true;
}

void cmEnableTestingCommand::FinalPass()
{
  // Create a full path filename for output Testfile
  std::string fname;
  fname = m_Makefile->GetStartOutputDirectory();
  fname += "/";
  fname += "DartTestfile.txt";
  
  cmSystemTools::MakeDirectory(m_Makefile->GetStartOutputDirectory());

  // Open the output Testfile
  std::ofstream fout(fname.c_str());
  if (!fout)
    {
    cmSystemTools::Error("Error Writing ", fname.c_str());
    cmSystemTools::ReportLastSystemError("");
    return;
    }
  
  fout << "# CMake generated Testfile for " << std::endl
       << "#\tSource directory: "
       << m_Makefile->GetStartDirectory()
       << std::endl
       << "#\tBuild directory: " << m_Makefile->GetStartOutputDirectory()
       << std::endl
       << "# " << std::endl
       << "# This file replicates the SUBDIRS() and ADD_TEST() commands from the source"
       << std::endl
       << "# tree CMakeLists.txt file, skipping any SUBDIRS() or ADD_TEST() commands"
       << std::endl
       << "# that are excluded by CMake control structures, i.e. IF() commands."
       << std::endl
       << "#" << std::endl 
       << "# The next line is critical for Dart to work" << std::endl 
       << "# Duh :-)" << std::endl << std::endl;

  // write out the subdirs for the current directory
  if (!m_Makefile->GetSubDirectories().empty())
    {
    fout << "SUBDIRS(";
    const std::vector<std::pair<cmStdString, bool> >& subdirs = m_Makefile->GetSubDirectories();
    std::vector<std::pair<cmStdString, bool> >::const_iterator i = subdirs.begin();
    fout << (*i).first.c_str();
    ++i;
    for(; i != subdirs.end(); ++i)
      {
      fout << " " << i->first.c_str();
      }
    fout << ")" << std::endl << std::endl;;
    }
  fout.close();  

  return;
}

