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
#include "cmConfigureFileCommand.h"

// cmConfigureFileCommand
bool cmConfigureFileCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments, expected 2");
    return false;
    }
  m_InputFile = args[0];
  m_OuputFile = args[1];
  m_CopyOnly = false;
  m_EscapeQuotes = false;
  m_Immediate = false;
  m_AtOnly = false;
  for(unsigned int i=2;i < args.size();++i)
    {
    if(args[i] == "COPYONLY")
      {
      m_CopyOnly = true;
      }
    else if(args[i] == "ESCAPE_QUOTES")
      {
      m_EscapeQuotes = true;
      }
    else if(args[i] == "@ONLY")
      {
      m_AtOnly = true;
      }
    else if(args[i] == "IMMEDIATE")
      {
      m_Immediate = true;
      }
    }
  
  // If we were told to copy the file immediately, then do it on the
  // first pass (now).
  if(m_Immediate)
    {
    this->ConfigureFile();
    }
  
  return true;
}

void cmConfigureFileCommand::FinalPass()
{
  if(!m_Immediate)
    {
    this->ConfigureFile();
    }
}

void cmConfigureFileCommand::ConfigureFile()
{
  m_Makefile->AddCMakeDependFile(m_InputFile.c_str());
  cmSystemTools::ConvertToUnixSlashes(m_OuputFile);
  std::string::size_type pos = m_OuputFile.rfind('/');
  if(pos != std::string::npos)
    {
    std::string path = m_OuputFile.substr(0, pos);
    cmSystemTools::MakeDirectory(path.c_str());
    }
  
  if(m_CopyOnly)
    {
    cmSystemTools::CopyFileIfDifferent(m_InputFile.c_str(),
                                       m_OuputFile.c_str());
    }
  else
    {
    std::string tempOutputFile = m_OuputFile;
    tempOutputFile += ".tmp";
    std::ofstream fout(tempOutputFile.c_str());
    if(!fout)
      {
      cmSystemTools::Error("Could not open file for write in copy operatation ", 
                           tempOutputFile.c_str());
      return;
      }
    std::ifstream fin(m_InputFile.c_str());
    if(!fin)
      {
      cmSystemTools::Error("Could not open file for read in copy operatation ",
                           m_InputFile.c_str());
      return;
      }

    // now copy input to output and expand varibles in the
    // input file at the same time
    const int bufSize = 4096;
    char buffer[bufSize];
    std::string inLine;
    cmRegularExpression cmdefine("#cmakedefine[ \t]*([A-Za-z_0-9]*)");
    while(fin)
      {
      fin.getline(buffer, bufSize);
      if(fin)
        {
        inLine = buffer;
        m_Makefile->ExpandVariablesInString(inLine, m_EscapeQuotes, m_AtOnly);
        m_Makefile->RemoveVariablesInString(inLine, m_AtOnly);
        // look for special cmakedefine symbol and handle it
        // is the symbol defined
        if (cmdefine.find(inLine))
          {
          const char *def = m_Makefile->GetDefinition(cmdefine.match(1).c_str());
          if(!cmSystemTools::IsOff(def))
            {
            cmSystemTools::ReplaceString(inLine,
                                         "#cmakedefine", "#define");
            fout << inLine << "\n";
            }
          else
            {
            cmSystemTools::ReplaceString(inLine,
                                         "#cmakedefine", "#undef");
            fout << "/* " << inLine << " */\n";
            }
          }
        else
          {
          fout << inLine << "\n";
          }
        }
      }
    // close the files before attempting to copy
    fin.close();
    fout.close();
    cmSystemTools::CopyFileIfDifferent(tempOutputFile.c_str(),
                                       m_OuputFile.c_str());
    cmSystemTools::RemoveFile(tempOutputFile.c_str());
    }
}

  
