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
      std::string srcName = cmSystemTools::GetFilenameWithoutLastExtension(*j);
      std::string newName = "moc_" + srcName;
      file.SetName(newName.c_str(), m_Makefile->GetCurrentOutputDirectory(),
                   "cxx",false);
      std::string hname;
      if ( (*j)[0] == '/' || (*j)[1] == ':' )
        {
        hname = *j;
        }
      else
        {
        if ( curr && curr->GetPropertyAsBool("GENERATED") )
          {
          hname = std::string( m_Makefile->GetCurrentOutputDirectory() ) + "/" + *j;
          }
        else
          {
          hname = cdir + "/" + *j;
          }
        }
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
  const char* moc_exe = m_Makefile->GetRequiredDefinition("QT_MOC_EXECUTABLE");

  // wrap all the .h files
  depends.push_back(moc_exe);

  for(size_t classNum = 0; classNum < lastClass; classNum++)
    {
    // Add output to build list
    m_Makefile->AddSource(m_WrapClasses[classNum]);

    // set up moc command
    std::string res = m_Makefile->GetCurrentOutputDirectory();
    res += "/";
    res += m_WrapClasses[classNum].GetSourceName() + ".cxx";

    cmCustomCommandLine commandLine;
    commandLine.push_back(moc_exe);
    commandLine.push_back("-o");
    commandLine.push_back(res);
    commandLine.push_back(m_WrapHeaders[classNum]);

    cmCustomCommandLines commandLines;
    commandLines.push_back(commandLine);

    std::vector<std::string> realdepends = depends;
    realdepends.push_back(m_WrapHeaders[classNum]);

    const char* no_main_dependency = 0;
    const char* no_working_dir = 0;
    m_Makefile->AddCustomCommandToOutput(res.c_str(),
                                         realdepends,
                                         no_main_dependency,
                                         commandLines,
                                         "QT Wrapped File",
                                         no_working_dir);
    }
}
