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
    this->SetError("called with QT_WRAP_CPP off : ");
    return false;
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
        std::string hname = cdir + "/" + curr.GetSourceName() + "." +
            curr.GetSourceExtension();
        m_WrapHeaders.push_back(hname);
        // add starting depends
        file.GetDepends().push_back(hname);
        m_WrapClasses.push_back(file);
        }
      }
    }
  
  return true;
}

void cmQTWrapCPPCommand::FinalPass() 
{

  // first we add the rules for all the .h to Moc files
  int lastClass = m_WrapClasses.size();
  std::vector<std::string> depends;
  std::string moc_exe = "${QT_MOC_EXE}";


  // wrap all the .h files
  depends.push_back(moc_exe);

  const char * GENERATED_QT_FILES_value=
      m_Makefile->GetDefinition("GENERATED_QT_FILES"); 
  std::string moc_list("");
  if (GENERATED_QT_FILES_value!=0)
    {
    moc_list=moc_list+GENERATED_QT_FILES_value;
    }

  for(int classNum = 0; classNum < lastClass; classNum++)
    {
    // Add output to build list
    m_Makefile->AddSource(m_WrapClasses[classNum],m_SourceList.c_str());

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



