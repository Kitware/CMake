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
#include "cmFLTKWrapUICommand.h"

#include "cmSourceFile.h"

// cmFLTKWrapUICommand
bool cmFLTKWrapUICommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // what is the current source dir
  std::string cdir = m_Makefile->GetCurrentDirectory();
  const char* fluid_exe =
    m_Makefile->GetRequiredDefinition("FLTK_FLUID_EXECUTABLE");

  // get parameter for the command
  m_Target              = args[0];  // Target that will use the generated files

  std::vector<std::string> newArgs;
  m_Makefile->ExpandSourceListArguments(args,newArgs, 1);
  
  // get the list of GUI files from which .cxx and .h will be generated 
  std::string outputDirectory = m_Makefile->GetCurrentOutputDirectory();

  // Some of the generated files are *.h so the directory "GUI" 
  // where they are created have to be added to the include path
  m_Makefile->AddIncludeDirectory( outputDirectory.c_str() );

  for(std::vector<std::string>::iterator i = (newArgs.begin() + 1); 
      i != newArgs.end(); i++)
    {
    cmSourceFile *curr = m_Makefile->GetSource(i->c_str());
    // if we should use the source GUI 
    // to generate .cxx and .h files
    if (!curr || !curr->GetPropertyAsBool("WRAP_EXCLUDE"))
      {
      cmSourceFile header_file;
      std::string srcName = cmSystemTools::GetFilenameWithoutExtension(*i);
      const bool headerFileOnly = true;
      header_file.SetName(srcName.c_str(), 
                  outputDirectory.c_str(), "h",headerFileOnly);
      std::string origname = cdir + "/" + *i;
      std::string hname   = header_file.GetFullPath();
      // add starting depends
      std::vector<std::string> depends;
      depends.push_back(origname);
      depends.push_back("fluid");
      std::string cxxres = outputDirectory.c_str();
      cxxres += "/" + srcName;
      cxxres += ".cxx";

      cmCustomCommandLine commandLine;
      commandLine.push_back(fluid_exe);
      commandLine.push_back("-c"); // instructs Fluid to run in command line
      commandLine.push_back("-h"); // optionally rename .h files
      commandLine.push_back(hname);
      commandLine.push_back("-o"); // optionally rename .cxx files
      commandLine.push_back(cxxres);
      commandLine.push_back(origname);// name of the GUI fluid file
      cmCustomCommandLines commandLines;
      commandLines.push_back(commandLine);

      // Add command for generating the .h and .cxx files
      const char* no_main_dependency = 0;
      const char* no_comment = 0;
      m_Makefile->AddCustomCommandToOutput(cxxres.c_str(),
                                           depends, no_main_dependency,
                                           commandLines, no_comment);
      m_Makefile->AddCustomCommandToOutput(hname.c_str(),
                                           depends, no_main_dependency,
                                           commandLines, no_comment);

      cmSourceFile *sf = m_Makefile->GetSource(cxxres.c_str());
      sf->GetDepends().push_back(hname);
      sf->GetDepends().push_back(origname);
      m_GeneratedSourcesClasses.push_back(sf);
      }
    }

  // create the variable with the list of sources in it
  size_t lastHeadersClass = m_GeneratedSourcesClasses.size();
  std::string sourceListValue;
  for(size_t classNum = 0; classNum < lastHeadersClass; classNum++)
    {
    if (classNum)
      {
      sourceListValue += ";";
      }
    sourceListValue += m_GeneratedSourcesClasses[classNum]->GetFullPath();
    }
  std::string varName = m_Target;
  varName += "_FLTK_UI_SRCS";
  m_Makefile->AddDefinition(varName.c_str(), sourceListValue.c_str());
  
  return true;
}

void cmFLTKWrapUICommand::FinalPass() 
{
  // people should add the srcs to the target themselves, but the old command
  // didn't support that, so check and see if they added the files in and if
  // they didn;t then print a warning and add then anyhow
  std::vector<std::string> srcs = 
    m_Makefile->GetTargets()[m_Target].GetSourceLists();
  bool found = false;
  for (unsigned int i = 0; i < srcs.size(); ++i)
    {
    if (srcs[i] == 
        m_GeneratedSourcesClasses[0]->GetFullPath())
      {
      found = true;
      break;
      }
    }
  if (!found)
    {
    cmSystemTools::Message("In CMake 2.2 the FLT_WRAP_UI command sets a variable to the list of source files that should be added to your executable or library. It appears that you have not added these source files to your target. You should change your CMakeLists.txt file to directly add the generated files to the target. For example FTLK_WRAP_UI(foo src1 src2 src3) will create a variable named foo_FLTK_UI_SRCS that contains the list of sources to add to your target when you call ADD_LIBRARY or ADD_EXECUTABLE. For now CMake 2.2 will add the sources to your target for you as was done in CMake 2.0 and earlier. In the future this may become an error.","Warning");
    // first we add the rules for all the .fl to .h and .cxx files
    size_t lastHeadersClass = m_GeneratedSourcesClasses.size();
    
    // Generate code for all the .fl files
    for(size_t classNum = 0; classNum < lastHeadersClass; classNum++)
      {
      m_Makefile->GetTargets()[m_Target].GetSourceFiles().push_back( 
                                                                    m_GeneratedSourcesClasses[classNum]);
      }
    }
}



