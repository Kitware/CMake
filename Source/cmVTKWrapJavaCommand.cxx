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
#include "cmVTKWrapJavaCommand.h"

// cmVTKWrapJavaCommand
bool cmVTKWrapJavaCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  if(!cmCacheManager::GetInstance()->IsOn("VTK_WRAP_JAVA"))
    {
    return true;
    }

  // add in a depend in the vtkVTKWrapJava executable
  m_Makefile->AddUtility("vtkWrapJava");
  m_Makefile->AddUtility("vtkParseJava");
  
  // what is the current source dir
  std::string cdir = m_Makefile->GetCurrentDirectory();

  // keep the library name
  m_LibraryName = args[0];
  m_SourceList = args[1];
  
  // get the list of classes for this library
  cmMakefile::SourceMap &Classes = m_Makefile->GetSources();
  for(std::vector<std::string>::iterator j = (args.begin() + 2);
      j != args.end(); ++j)
    {   
    cmMakefile::SourceMap::iterator l = Classes.find(*j);
    for(std::vector<cmSourceFile>::iterator i = l->second.begin(); 
        i != l->second.end(); i++)
      {
      cmSourceFile &curr = *i;
      // if we should wrap the class
      if (!curr.GetWrapExclude())
        {
        cmSourceFile file;
        file.SetIsAnAbstractClass(curr.IsAnAbstractClass());
        std::string newName = curr.GetSourceName() + "Java";
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

void cmVTKWrapJavaCommand::FinalPass() 
{
  // first we add the rules for all the .h to Java.cxx files
  int lastClass = m_WrapClasses.size();
  std::vector<std::string> depends;
  std::string wjava = "${VTK_WRAP_JAVA_EXE}";
  std::string pjava = "${VTK_PARSE_JAVA_EXE}";
  std::string hints = "${VTK_WRAP_HINTS}";
  
  // wrap all the .h files
  depends.push_back(wjava);
  depends.push_back(pjava);
  for(int classNum = 0; classNum < lastClass; classNum++)
    {
    m_Makefile->AddSource(m_WrapClasses[classNum],m_SourceList.c_str());

    // wrap java
    std::string res = m_WrapClasses[classNum].GetSourceName() + ".cxx";
    std::string res2 = m_OriginalNames[classNum] + ".java";
    std::vector<std::string> resvec;
    resvec.push_back(res);
    resvec.push_back(res2);
    
    std::string cmd = wjava + " " + m_WrapHeaders[classNum] + " "
      + hints + (m_WrapClasses[classNum].IsAnAbstractClass() ? " 0 " : " 1 ") + " > " + m_WrapClasses[classNum].GetSourceName() + ".cxx\\\n\t" + 
      pjava + " " + m_WrapHeaders[classNum] + " "
      + hints + (m_WrapClasses[classNum].IsAnAbstractClass() ? " 0 " : " 1 ") + " > " + m_OriginalNames[classNum] + ".java";
    m_Makefile->AddCustomCommand(m_WrapHeaders[classNum].c_str(),
                                 cmd.c_str(), depends, 
                                 resvec, m_LibraryName.c_str());
    }
}



