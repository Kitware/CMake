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
#include "cmQTWrapCPPCommand.h"

// cmQTWrapCPPCommand
bool cmQTWrapCPPCommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  m_Makefile->ExpandSourceListArguments(argsIn, args, 2);

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
    this->SetError("called with QT_WRAP_CPP off : ");
    return false;
    }

  // what is the current source dir
  std::string cdir = m_Makefile->GetCurrentDirectory();

  // keep the library name
  m_LibraryName = args[0];
  m_SourceList = args[1];
  
  std::string sourceListValue;

  // was the list already populated
  const char *def = m_Makefile->GetDefinition(m_SourceList.c_str());  
  if (def)
    {
    sourceListValue = def;
    }

  // get the list of classes for this library
  for(std::vector<std::string>::iterator j = (args.begin() + 2);
      j != args.end(); ++j)
    {   
    cmSourceFile *curr = m_Makefile->GetSource(j->c_str());
    
    // if we should wrap the class
    if (!curr || !curr->GetPropertyAsBool("WRAP_EXCLUDE"))
      {
      cmSourceFile file;
      if (curr)
        {
        file.SetProperty("ABSTRACT",curr->GetProperty("ABSTRACT"));
        }
      std::string srcName = cmSystemTools::GetFilenameWithoutExtension(*j);
      std::string newName = "moc_" + srcName;
      file.SetName(newName.c_str(), m_Makefile->GetCurrentOutputDirectory(),
                   "cxx",false);
      std::string hname = cdir + "/" + *j;
      m_WrapHeaders.push_back(hname);
      // add starting depends
      file.GetDepends().push_back(hname);
      m_WrapClasses.push_back(file);
      if (sourceListValue.size() > 0)
        {
        sourceListValue += ";";
        }
      sourceListValue += newName + ".cxx";
      }
    }
  
  m_Makefile->AddDefinition(m_SourceList.c_str(), sourceListValue.c_str());
  return true;
}

void cmQTWrapCPPCommand::FinalPass() 
{

  // first we add the rules for all the .h to Moc files
  size_t lastClass = m_WrapClasses.size();
  std::vector<std::string> depends;
  std::string moc_exe = "${QT_MOC_EXECUTABLE}";

  // wrap all the .h files
  depends.push_back(moc_exe);

  const char * GENERATED_QT_FILES_value=
      m_Makefile->GetDefinition("GENERATED_QT_FILES"); 
  std::string moc_list("");
  if (GENERATED_QT_FILES_value!=0)
    {
    moc_list=moc_list+GENERATED_QT_FILES_value;
    }

  for(size_t classNum = 0; classNum < lastClass; classNum++)
    {
    // Add output to build list
    m_Makefile->AddSource(m_WrapClasses[classNum]);

    // set up moc command
    std::string res = m_Makefile->GetCurrentOutputDirectory();
    res += "/";
    res += m_WrapClasses[classNum].GetSourceName() + ".cxx";

    moc_list = moc_list + " " + res;
    
    std::vector<std::string> args;
    args.push_back("-o");
    args.push_back(res);
    args.push_back(m_WrapHeaders[classNum]);

    m_Makefile->AddCustomCommand(m_WrapHeaders[classNum].c_str(),
                                 moc_exe.c_str(), args, depends, 
                                 res.c_str(), m_LibraryName.c_str());

    }

  m_Makefile->AddDefinition("GENERATED_QT_FILES",moc_list.c_str());
}



