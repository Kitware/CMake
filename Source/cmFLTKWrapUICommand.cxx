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
#include "cmFLTKWrapUICommand.h"

// cmFLTKWrapUICommand
bool cmFLTKWrapUICommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() != 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  const char* FLTK_WRAP_UI_value = m_Makefile->GetDefinition("FLTK_WRAP_UI");
  if (FLTK_WRAP_UI_value==0)
    {
    this->SetError("called with FLTK_WRAP_UI undefined");
    return false;
    }
  
  if(cmSystemTools::IsOff(FLTK_WRAP_UI_value))
    {
    this->SetError("called with FLTK_WRAP_UI off : ");
    return false;
    }


  // what is the current source dir
  std::string cdir = m_Makefile->GetCurrentDirectory();

  // get parameter for the command
  m_Target              = args[0];  // Target that will use the generated files
  m_GUISourceList       = args[1];  // Source List of the GUI source files 

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
      cmSourceFile source_file;
      std::string srcName = cmSystemTools::GetFilenameWithoutExtension(*i);
      const bool headerFileOnly = true;
      header_file.SetName(srcName.c_str(), 
                  outputDirectory.c_str(), "h",headerFileOnly);
      source_file.SetName(srcName.c_str(), 
                  outputDirectory.c_str(), "cxx",!headerFileOnly);
      std::string origname = cdir + "/" + *i;
      std::string hname   = header_file.GetFullPath();
      std::string cxxname = source_file.GetFullPath();
      m_WrapUserInterface.push_back(origname);
      // add starting depends
      source_file.GetDepends().push_back(hname);
      source_file.GetDepends().push_back(origname);
      header_file.GetDepends().push_back(origname);
      m_GeneratedHeadersClasses.push_back(header_file);
      m_GeneratedSourcesClasses.push_back(source_file);
      }
    }
  
  return true;
}

void cmFLTKWrapUICommand::FinalPass() 
{

  // first we add the rules for all the .fl to .h and .cxx files
  size_t lastHeadersClass = m_GeneratedHeadersClasses.size();
  std::string fluid_exe = "${FLTK_FLUID_EXE}";


  std::string outputGUIDirectory = m_Makefile->GetCurrentOutputDirectory();

  // Generate code for all the .fl files
  for(size_t classNum = 0; classNum < lastHeadersClass; classNum++)
    {
    // set up .fl to .h and .cxx command
    std::string hres = outputGUIDirectory;
    hres += "/";
    hres += m_GeneratedHeadersClasses[classNum].GetSourceName() + "." +
        m_GeneratedHeadersClasses[classNum].GetSourceExtension();

    std::string cxxres = outputGUIDirectory;
    cxxres += "/";
    cxxres += m_GeneratedSourcesClasses[classNum].GetSourceName() + "." +
        m_GeneratedSourcesClasses[classNum].GetSourceExtension();

    std::vector<std::string> cxxargs;
    cxxargs.push_back("-c"); // instructs Fluid to run in command line
    cxxargs.push_back("-h"); // optionally rename .h files
    cxxargs.push_back(hres);
    cxxargs.push_back("-o"); // optionally rename .cxx files
    cxxargs.push_back(cxxres);
    cxxargs.push_back(m_WrapUserInterface[classNum]);// name of the GUI fluid file

    std::vector<std::string> depends;

    std::vector<std::string> outputs;
    outputs.push_back( cxxres );
    outputs.push_back( hres   );
     
    // Add command for generating the .h and .cxx files
    m_Makefile->AddCustomCommand(m_WrapUserInterface[classNum].c_str(),
                                 fluid_exe.c_str(), cxxargs, depends, 
                                 outputs, m_Target.c_str() );
    cmSourceFile* sf = m_Makefile->AddSource(m_GeneratedSourcesClasses[classNum]);
    
    m_Makefile->GetTargets()[m_Target].GetSourceFiles().push_back( sf );
        
    }
}



