/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Author:    Franck Bettinger.

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "cmFLTKWrapUICommand.h"

// cmFLTKWrapUICommand
bool cmFLTKWrapUICommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2 )
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

  std::string outputGUIDirectory = m_Makefile->GetCurrentOutputDirectory();

  if(!cmSystemTools::MakeDirectory( outputGUIDirectory.c_str() ) )
    {
    cmSystemTools::Error("Error failed create GUI directory:",
                                          outputGUIDirectory.c_str() );
                             
    }

  // what is the current source dir
  std::string cdir = m_Makefile->GetCurrentDirectory();

  // get parameter for the command
  m_Target              = args[0];  // Target that will use the generated files
  m_GUISourceList       = args[1];  // Source List of the GUI source files 

  cmMakefile::SourceMap &GUISources = m_Makefile->GetSources();
 
  // get the list of GUI files from which .cxx and .h will be generated 
  cmMakefile::SourceMap::iterator l = GUISources.find( m_GUISourceList );
  if (l == GUISources.end())
    {
    this->SetError("bad source list passed to FLTKWrapUICommand");
    return false;
    }

  // Some of the generated files are *.h so the directory "GUI" 
  // where they are created have to be added to the include path
  m_Makefile->AddIncludeDirectory( outputGUIDirectory.c_str() );

  for(std::vector<cmSourceFile>::iterator i = l->second.begin(); 
      i != l->second.end(); i++)
    {
    cmSourceFile &curr = *i;
    // if we should use the source GUI 
    // to generate .cxx and .h files
    if (!curr.GetWrapExclude())
      {
      cmSourceFile header_file;
      cmSourceFile source_file;
      const bool headerFileOnly = true;
      header_file.SetName(curr.GetSourceName().c_str(), 
                  outputGUIDirectory.c_str(), "h",headerFileOnly);
      source_file.SetName(curr.GetSourceName().c_str(), 
                  outputGUIDirectory.c_str(), "cxx",!headerFileOnly);
      std::string origname = cdir + "/" + curr.GetSourceName() + "." +
          curr.GetSourceExtension();
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
  int lastHeadersClass = m_GeneratedHeadersClasses.size();
  std::string fluid_exe = "${FLTK_FLUID_EXE}";


  std::string outputGUIDirectory = m_Makefile->GetCurrentOutputDirectory();

  // Generate code for all the .fl files
  for(int classNum = 0; classNum < lastHeadersClass; classNum++)
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
    
    m_Makefile->GetTargets()[m_Target].GetSourceFiles().push_back( m_GeneratedSourcesClasses[classNum] );
        
    }
}



