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
#include "cmQTWrapUICommand.h"

// cmQTWrapUICommand
bool cmQTWrapUICommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 4 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  const char* QT_WRAP_UI_value = m_Makefile->GetDefinition("QT_WRAP_UI");
  if (QT_WRAP_UI_value==0)
    {
    this->SetError("called with QT_WRAP_UI undefined");
    return false;
    }
  
  if(cmSystemTools::IsOff(QT_WRAP_UI_value))
    {
    this->SetError("called with QT_WRAP_UI off : ");
    return false;
    }

  // what is the current source dir
  std::string cdir = m_Makefile->GetCurrentDirectory();

  // keep the library name
  m_LibraryName = args[0];
  m_HeaderList = args[1];
  m_SourceList = args[2];
 
  // get the list of classes for this library
  cmMakefile::SourceMap &Classes = m_Makefile->GetSources();


  for(std::vector<std::string>::const_iterator j = (args.begin() + 3);
      j != args.end(); ++j)
    {  
    cmMakefile::SourceMap::iterator l = Classes.find(*j);
    if (l == Classes.end())
      {
      this->SetError("bad source list passed to QTWrapUICommand");
      return false;
      }
    for(std::vector<cmSourceFile>::iterator i = l->second.begin(); 
        i != l->second.end(); i++)
      {
      cmSourceFile &curr = *i;
      // if we should wrap the class
      if (!curr.GetWrapExclude())
        {
        cmSourceFile header_file;
        cmSourceFile source_file;
        cmSourceFile moc_file;
        header_file.SetName(curr.GetSourceName().c_str(), 
                    m_Makefile->GetCurrentOutputDirectory(),
                     "h",false);
        source_file.SetName(curr.GetSourceName().c_str(), 
                    m_Makefile->GetCurrentOutputDirectory(),
                     "cxx",false);
        std::string moc_source_name("moc_");
        moc_source_name = moc_source_name + curr.GetSourceName().c_str();
        moc_file.SetName(moc_source_name.c_str(), 
                    m_Makefile->GetCurrentOutputDirectory(),
                     "cxx",false);
        std::string origname = cdir + "/" + curr.GetSourceName() + "." +
            curr.GetSourceExtension();
        std::string hname = header_file.GetFullPath();
        m_WrapUserInterface.push_back(origname);
        // add starting depends
        moc_file.GetDepends().push_back(hname);
        source_file.GetDepends().push_back(hname);
        source_file.GetDepends().push_back(origname);
        header_file.GetDepends().push_back(origname);
        m_WrapHeadersClasses.push_back(header_file);
        m_WrapSourcesClasses.push_back(source_file);
        m_WrapMocClasses.push_back(moc_file);
        m_Makefile->AddSource(header_file,
            m_HeaderList.c_str());
        m_Makefile->AddSource(source_file,
            m_SourceList.c_str());
        m_Makefile->AddSource(moc_file,
            m_SourceList.c_str());
        }
      }
    }
  
  return true;
}

void cmQTWrapUICommand::FinalPass() 
{

  // first we add the rules for all the .ui to .h and .cxx files
  int lastHeadersClass = m_WrapHeadersClasses.size();
  std::vector<std::string> depends;
  std::string uic_exe = "${QT_UIC_EXE}";
  std::string moc_exe = "${QT_MOC_EXE}";


  // wrap all the .h files
  depends.push_back(uic_exe);

  const char * GENERATED_QT_FILES_value=
      m_Makefile->GetDefinition("GENERATED_QT_FILES");
  std::string ui_list("");
  if (GENERATED_QT_FILES_value!=0)
    {
    ui_list=ui_list+GENERATED_QT_FILES_value;
    } 

  for(int classNum = 0; classNum < lastHeadersClass; classNum++)
    {
    // set up .ui to .h and .cxx command

    std::string hres = m_Makefile->GetCurrentOutputDirectory();
    hres += "/";
    hres += m_WrapHeadersClasses[classNum].GetSourceName() + "." +
        m_WrapHeadersClasses[classNum].GetSourceExtension();

    std::string cxxres = m_Makefile->GetCurrentOutputDirectory();
    cxxres += "/";
    cxxres += m_WrapSourcesClasses[classNum].GetSourceName() + "." +
        m_WrapSourcesClasses[classNum].GetSourceExtension();

    std::string mocres = m_Makefile->GetCurrentOutputDirectory();
    mocres += "/";
    mocres += m_WrapMocClasses[classNum].GetSourceName() + "." +
        m_WrapMocClasses[classNum].GetSourceExtension();

    ui_list = ui_list + " " + hres + " " + cxxres + " " + mocres;
    
    std::vector<std::string> hargs;
    hargs.push_back("-o");
    hargs.push_back(hres);
    hargs.push_back(m_WrapUserInterface[classNum]);

    std::vector<std::string> cxxargs;
    cxxargs.push_back("-impl");
    cxxargs.push_back(hres);
    cxxargs.push_back("-o");
    cxxargs.push_back(cxxres);
    cxxargs.push_back(m_WrapUserInterface[classNum]);

    std::vector<std::string> mocargs;
    mocargs.push_back("-o");
    mocargs.push_back(mocres);
    mocargs.push_back(hres);

    m_Makefile->AddCustomCommand(m_WrapUserInterface[classNum].c_str(),
                                 uic_exe.c_str(), hargs, depends, 
                                 hres.c_str(), m_LibraryName.c_str());

    depends.push_back(hres);

    m_Makefile->AddCustomCommand(m_WrapUserInterface[classNum].c_str(),
                                 uic_exe.c_str(), cxxargs, depends, 
                                 cxxres.c_str(), m_LibraryName.c_str());

    depends.clear();
    depends.push_back(moc_exe);

    m_Makefile->AddCustomCommand(hres.c_str(),
                                 moc_exe.c_str(), mocargs, depends, 
                                 mocres.c_str(), m_LibraryName.c_str());

    }

  m_Makefile->AddDefinition("GENERATED_QT_FILES",ui_list.c_str());

}



