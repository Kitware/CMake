/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

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
#include "cmCablePackageCommand.h"
#include "cmCacheManager.h"
#include "cmTarget.h"


cmCablePackageCommand::~cmCablePackageCommand()
{
  // If we are the owner of the cmCableData, we must delete it here.
  // For most cmCableCommands, the cmCableCommand destructor will take
  // care of this.  If this package happens to be the last one, and is
  // the owner, then the destructor of cmCableData will call back to a method
  // in this class after the package part of it has been freed!
  if(m_CableData && m_CableData->OwnerIs(this))
    {
    delete m_CableData;
    // Make sure our superclass's destructor doesn't try to delete the
    // cmCableData too.
    m_CableData = NULL;
    }
}

// cmCablePackageCommand
bool cmCablePackageCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() != 2)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // setup this once. Really this should probably be moved somewhere else
  // at some point. 
  {  
  // We must add a custom rule to cause the cable_config.xml to be re-built
  // when it is removed.  Rebuilding it means re-running CMake.
  std::string cMakeLists = m_Makefile->GetStartDirectory();
  cMakeLists += "/";
  cMakeLists += "CMakeLists.txt";

  std::string command;
#if defined(_WIN32) && !defined(__CYGWIN__)
  command = "\"";
  command += m_Makefile->GetHomeDirectory();
  command += "/CMake/Source/CMakeSetupCMD\" \"";
  command += cMakeLists;
  command += "\" -DSP";
#else
  command = "\"";
  command += m_Makefile->GetHomeOutputDirectory();  
  command += "/CMake/Source/CMakeBuildTargets\" \"";
  command += cMakeLists;
  command += "\"";
#endif
  command += " -H\"";
  command += m_Makefile->GetHomeDirectory();
  command += "\" -S\"";
  command += m_Makefile->GetStartDirectory();
  command += "\" -O\"";
  command += m_Makefile->GetStartOutputDirectory();
  command += "\" -B\"";
  command += m_Makefile->GetHomeOutputDirectory();
  command += "\"";

  std::vector<std::string> depends;
  m_Makefile->AddCustomCommand(cMakeLists.c_str(), 
                               command.c_str(),
                               depends,
                               "cable_config.xml", args[1].c_str());
  }

  // This command needs to access the Cable data.
  this->SetupCableData();
  
  // The argument is the package name.
  m_PackageName = args[0];
  m_TargetName = args[1];

  // Ask the cable data to begin the package.  This may call another
  // cmCablePackageCommand's WritePackageFooter().  This will call
  // this cmCablePackageCommand's WritePackageHeader().
  m_CableData->BeginPackage(this);

  // Tell the makefile that it needs the "cable" utility.  
  m_Makefile->AddUtility("cable");

  // Add custom rules to the makefile to generate this package's source
  // files.
  {
  std::string command = "${CABLE}";
  m_Makefile->ExpandVariablesInString(command);
  std::vector<std::string> depends;
  depends.push_back(command);
  command = "\""+command+"\" cable_config.xml";
  
  std::vector<std::string> outputs;
  outputs.push_back("Cxx/"+m_PackageName+"_cxx.cxx");
  outputs.push_back("Cxx/"+m_PackageName+"_cxx.h");
  
  // A rule for the package's source files.
  m_Makefile->AddCustomCommand("cable_config.xml",
                               command.c_str(),
                               depends,
                               outputs, m_TargetName.c_str());
  }

  // Add custom rules to the makefile to generate this package's xml files.
  {
  std::string command = "${GCCXML}";
  m_Makefile->ExpandVariablesInString(command);
  std::vector<std::string> depends;
  depends.push_back(command);
  std::string input = "Cxx/"+m_PackageName+"_cxx.cxx";
  std::string output = "Cxx/"+m_PackageName+"_cxx.xml";
  command = "\""+command+"\" ${CXX_FLAGS} -fsyntax-only -fxml=" + output + " -c " + input;
  
  std::vector<std::string> outputs;
  outputs.push_back("Cxx/"+m_PackageName+"_cxx.xml");
  
  // A rule for the package's source files.
  m_Makefile->AddCustomCommand(input.c_str(),
                               command.c_str(),
                               depends,
                               outputs, m_TargetName.c_str());
  }  
  
  // add the source list to the target
  m_Makefile->GetTargets()[m_TargetName.c_str()].GetSourceLists().push_back(m_PackageName);

  return true;
}


void cmCablePackageCommand::FinalPass()
{
  // Add a rule to build the generated package.
  std::string fileName = "Cxx/"+m_PackageName+"_cxx";
  std::string filePath = m_Makefile->GetStartOutputDirectory();
  cmSourceFile file;
  file.SetIsAnAbstractClass(false);
  file.SetIsAHeaderFileOnly(false);
  file.SetName(fileName.c_str(), filePath.c_str(), "cxx", false);
  m_Makefile->AddSource(file, m_PackageName.c_str());
}


/**
 * Write a CABLE package header.
 */
void cmCablePackageCommand::WritePackageHeader() const
{
  std::ostream& os = m_CableData->GetOutputStream();
  cmCableData::Indentation indent = m_CableData->GetIndentation();
  os << indent << "<Package name=\"" << m_PackageName.c_str() << "\">"
     << std::endl;
  m_CableData->Indent();
}


/**
 * Write a CABLE package footer.
 */
void cmCablePackageCommand::WritePackageFooter() const
{
  m_CableData->Unindent();
  std::ostream& os = m_CableData->GetOutputStream();
  cmCableData::Indentation indent = m_CableData->GetIndentation();
  os << indent << "</Package> <!-- \"" << m_PackageName.c_str() << "\" -->"
     << std::endl;
}

