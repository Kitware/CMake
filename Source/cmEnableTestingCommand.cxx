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
#include "cmLocalGenerator.h"

// we do this in the final pass so that we now the subdirs have all 
// been defined
bool cmEnableTestingCommand::InitialPass(std::vector<std::string> const&)
{
  m_Makefile->AddDefinition("CMAKE_TESTING_ENABLED","1");
  return true;
}

void cmEnableTestingCommand::FinalPass()
{
  // initialize the DartTestfile files for the tree
  this->CreateDartTestfileForMakefile(m_Makefile);
}

void cmEnableTestingCommand::CreateDartTestfileForMakefile(cmMakefile *mf)
{
  // Create a full path filename for output Testfile
  std::string fname;
  fname = mf->GetStartOutputDirectory();
  fname += "/";
  if ( m_Makefile->IsSet("DART_ROOT") )
    {
    fname += "DartTestfile.txt";
    }
  else
    {
    fname += "CTestTestfile.cmake";
    }
  
  cmSystemTools::MakeDirectory(mf->GetStartOutputDirectory());
  
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
       << mf->GetStartDirectory()
       << std::endl
       << "#\tBuild directory: " << mf->GetStartOutputDirectory()
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

  // get our output directory
  std::string outDir = mf->GetStartOutputDirectory();
  outDir += "/";
  
  // write out the subdirs for the current directory
  std::vector<cmLocalGenerator *>& children = 
    mf->GetLocalGenerator()->GetChildren();
  
  unsigned int i;
  if (children.size())
    {
    fout << "SUBDIRS(";
    std::string binP = children[0]->GetMakefile()->GetStartOutputDirectory();
    cmSystemTools::ReplaceString(binP, outDir.c_str(), "");
    fout << binP.c_str();
    for(i = 1; i < children.size(); ++i)
      {
      binP = children[i]->GetMakefile()->GetStartOutputDirectory();
      cmSystemTools::ReplaceString(binP, outDir.c_str(), "");
      fout << " " << binP.c_str();
      }
    fout << ")" << std::endl << std::endl;;
    }
  fout.close();  
  
  // then recurse
  if (children.size())
    {
    for(i = 0; i < children.size(); ++i)
      {
      this->CreateDartTestfileForMakefile(children[i]->GetMakefile());
      }
    }
  
  
  return;
}

