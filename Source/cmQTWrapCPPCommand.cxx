/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Author:    Ian Scott.

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
#include "cmQTWrapCPPCommand.h"

// cmQTWrapCPPCommand
bool cmQTWrapCPPCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  const char* QT_WRAP_CPP_value = m_Makefile->GetDefinition("QT_WRAP_CPP");
  if (QT_WRAP_CPP_value==0)
    {
    this->SetError("called with QT_WRAP_CPP undefined");
    return false;
    }
  
  if(cmSystemTools::IsOff(QT_WRAP_CPP_value))
    {
    return true;
    }

  // what is the current source dir
  std::string cdir = m_Makefile->GetCurrentDirectory();

  // keep the library name
  m_LibraryName = args[0];
  m_SourceList = args[1];
  
  // get the list of classes for this library
  cmMakefile::SourceMap &Classes = m_Makefile->GetSources();


  for(std::vector<std::string>::const_iterator j = (args.begin() + 2);
      j != args.end(); ++j)
    {   
    cmMakefile::SourceMap::iterator l = Classes.find(*j);
    if (l == Classes.end())
      {
      this->SetError("bad source list passed to QTWrapCPPCommand");
      return false;
      }
    for(std::vector<cmSourceFile>::iterator i = l->second.begin(); 
        i != l->second.end(); i++)
      {
      cmSourceFile &curr = *i;
      // if we should wrap the class
      if (!curr.GetWrapExclude())
        {
        cmSourceFile file;
        file.SetIsAnAbstractClass(curr.IsAnAbstractClass());
        std::string newName = "moc_" + curr.GetSourceName();
        file.SetName(newName.c_str(), m_Makefile->GetCurrentOutputDirectory(),
                     "cxx",false);
        std::string hname = cdir + "/" + curr.GetSourceName() + ".h";
        m_WrapHeaders.push_back(hname);
        // add starting depends
        file.GetDepends().push_back(hname);
        m_WrapClasses.push_back(file);
        m_OriginalNames.push_back(curr.GetSourceName());
        }
      }
    }
  
  return true;
}

void cmQTWrapCPPCommand::FinalPass() 
{
  // first we add the rules for all the .h to Java.cxx files
  int lastClass = m_WrapClasses.size();
  std::vector<std::string> depends;
  std::string moc_exe = "${QT_MOC_EXE}";


  // wrap all the .h files
  depends.push_back(moc_exe);

  for(int classNum = 0; classNum < lastClass; classNum++)
    {
    // Add output to build list
    m_Makefile->AddSource(m_WrapClasses[classNum],m_SourceList.c_str());

    // set up moc command
    std::string res = m_WrapClasses[classNum].GetSourceName() + ".cxx";
    
    std::vector<std::string> args;
    args.push_back("-o");
    args.push_back(res);
    args.push_back(m_WrapHeaders[classNum]);

    m_Makefile->AddCustomCommand(m_WrapHeaders[classNum].c_str(),
                                 moc_exe.c_str(), args, depends, 
                                 res.c_str(), m_LibraryName.c_str());

    }

  
}



