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
#include "cmVTKWrapJavaCommand.h"

// cmVTKWrapJavaCommand
bool cmVTKWrapJavaCommand::InitialPass(std::vector<std::string> const& argsIn)
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
  if(!m_Makefile->IsOn("VTK_WRAP_JAVA"))
    {
    return true;
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
  for(std::vector<std::string>::const_iterator j = (args.begin() + 2);
      j != args.end(); ++j)
    {   
    cmSourceFile *curr = m_Makefile->GetSource(j->c_str());

    // if we should wrap the class
    if (!curr || !curr->GetWrapExclude())
      {
      cmSourceFile file;
      if (curr)
        {
        file.SetIsAnAbstractClass(curr->IsAnAbstractClass());
        }
      std::string srcName = cmSystemTools::GetFilenameWithoutExtension(*j);
      std::string newName = srcName + "Java";
      file.SetName(newName.c_str(), m_Makefile->GetCurrentOutputDirectory(),
                   "cxx",false);
      std::string hname = cdir + "/" + srcName + ".h";
      m_WrapHeaders.push_back(hname);
      // add starting depends
      file.GetDepends().push_back(hname);
      m_WrapClasses.push_back(file);
      m_OriginalNames.push_back(srcName);
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

void cmVTKWrapJavaCommand::FinalPass() 
{
  // first we add the rules for all the .h to Java.cxx files
  size_t lastClass = m_WrapClasses.size();
  std::vector<std::string> depends;
  std::vector<std::string> depends2;
  std::vector<std::string> alldepends;  
  std::vector<std::string> empty;
  std::string wjava = "${VTK_WRAP_JAVA_EXE}";
  std::string pjava = "${VTK_PARSE_JAVA_EXE}";
  std::string hints = "${VTK_WRAP_HINTS}";
  std::string resultDirectory = "${VTK_JAVA_HOME}";

  m_Makefile->ExpandVariablesInString(hints);

  // wrap all the .h files
  depends.push_back(wjava);
  depends2.push_back(pjava);
  if (strcmp("${VTK_WRAP_HINTS}",hints.c_str()))
    {
    depends.push_back(hints);
    depends2.push_back(hints);
    }
  for(size_t classNum = 0; classNum < lastClass; classNum++)
    {
    m_Makefile->AddSource(m_WrapClasses[classNum]);

    // wrap java
    std::string res = m_Makefile->GetCurrentOutputDirectory();
    res += "/";
    res += m_WrapClasses[classNum].GetSourceName() + ".cxx";
    std::string res2 = resultDirectory + "/" + 
      m_OriginalNames[classNum] + ".java";
    
    std::vector<std::string> args;
    args.push_back(m_WrapHeaders[classNum]);
    if (strcmp("${VTK_WRAP_HINTS}",hints.c_str()))
      {
      args.push_back(hints);
      }
    args.push_back((m_WrapClasses[classNum].IsAnAbstractClass() ? "0" : "1"));
    args.push_back(res);

    m_Makefile->AddCustomCommand(m_WrapHeaders[classNum].c_str(),
                                 wjava.c_str(), args, depends, 
                                 res.c_str(), m_LibraryName.c_str());

    std::vector<std::string> args2;
    args2.push_back(m_WrapHeaders[classNum]);
    if (strcmp("${VTK_WRAP_HINTS}",hints.c_str()))
      {
      args2.push_back(hints);
      }
    args2.push_back((m_WrapClasses[classNum].IsAnAbstractClass() ? "0" : "1"));
    args2.push_back(res2);

    m_Makefile->AddCustomCommand(m_WrapHeaders[classNum].c_str(),
                                 pjava.c_str(), args2, depends2, 
                                 res2.c_str(), m_LibraryName.c_str());
    alldepends.push_back(res2);
    }

  m_Makefile->AddUtilityCommand((m_LibraryName+"JavaClasses").c_str(),
                                "",
                                "",
                                true,
                                alldepends,
                                empty);
  
}



