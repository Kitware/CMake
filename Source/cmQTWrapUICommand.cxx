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
#include "cmQTWrapUICommand.h"

// cmQTWrapUICommand
bool cmQTWrapUICommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 4 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  m_Makefile->ExpandSourceListArguments(argsIn, args, 3);

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
  std::string sourceListValue;
  std::string headerListValue;
  const char *def = m_Makefile->GetDefinition(m_SourceList.c_str());  
  if (def)
    {
    sourceListValue = def;
    }
 
  // get the list of classes for this library
  for(std::vector<std::string>::iterator j = (args.begin() + 3);
      j != args.end(); ++j)
    {   
    cmSourceFile *curr = m_Makefile->GetSource(j->c_str());
    
    // if we should wrap the class
    if (!curr || !curr->GetPropertyAsBool("WRAP_EXCLUDE"))
      {
      cmSourceFile header_file;
      cmSourceFile source_file;
      cmSourceFile moc_file;
      std::string srcName = cmSystemTools::GetFilenameWithoutExtension(*j);
      header_file.SetName(srcName.c_str(), 
                          m_Makefile->GetCurrentOutputDirectory(),
                          "h",false);
      source_file.SetName(srcName.c_str(), 
                          m_Makefile->GetCurrentOutputDirectory(),
                          "cxx",false);
      std::string moc_source_name("moc_");
      moc_source_name = moc_source_name + srcName;
      moc_file.SetName(moc_source_name.c_str(), 
                       m_Makefile->GetCurrentOutputDirectory(),
                       "cxx",false);
      std::string origname;
      if ( (*j)[0] == '/' )
        {
        origname = *j;
        }
      else
        {
        if ( curr && curr->GetPropertyAsBool("GENERATED") )
          {
          origname = std::string( m_Makefile->GetCurrentOutputDirectory() ) + "/" + *j;
          }
        else
          {
          origname = cdir + "/" + *j;
          }
        }
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
      m_Makefile->AddSource(header_file);
      m_Makefile->AddSource(source_file);
      m_Makefile->AddSource(moc_file);
      
      // create the list of sources
      if (headerListValue.size() > 0)
        {
        headerListValue += ";";
        }
      headerListValue += header_file.GetFullPath();
      
      // create the list of sources
      if (sourceListValue.size() > 0)
        {
        sourceListValue += ";";
        }
      sourceListValue += source_file.GetFullPath();
      sourceListValue += ";";
      sourceListValue += moc_file.GetFullPath();
      }
    }
  
  m_Makefile->AddDefinition(m_SourceList.c_str(), sourceListValue.c_str());  
  m_Makefile->AddDefinition(m_HeaderList.c_str(), headerListValue.c_str());  
  return true;
}

void cmQTWrapUICommand::FinalPass() 
{

  // first we add the rules for all the .ui to .h and .cxx files
  size_t lastHeadersClass = m_WrapHeadersClasses.size();
  std::vector<std::string> depends;
  std::string uic_exe = "${QT_UIC_EXECUTABLE}";
  std::string moc_exe = "${QT_MOC_EXECUTABLE}";


  // wrap all the .h files
  depends.push_back(uic_exe);

  const char * GENERATED_QT_FILES_value=
      m_Makefile->GetDefinition("GENERATED_QT_FILES");
  std::string ui_list("");
  if (GENERATED_QT_FILES_value!=0)
    {
    ui_list=ui_list+GENERATED_QT_FILES_value;
    } 

  for(size_t classNum = 0; classNum < lastHeadersClass; classNum++)
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



