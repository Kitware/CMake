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

#include <cmsys/RegularExpression.hxx>

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
  mode_t perm = 0;
  cmSystemTools::GetPermissions(m_InputFile.c_str(), perm);
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

    // now copy input to output and expand variables in the
    // input file at the same time
    std::string inLine;
    std::string outLine;
    while( cmSystemTools::GetLineFromStream(fin, inLine) )
      {
      outLine = "";
      m_Makefile->ConfigureString(inLine, outLine, m_AtOnly, m_EscapeQuotes);
      fout << outLine.c_str() << "\n";
      }
    // close the files before attempting to copy
    fin.close();
    fout.close();
    cmSystemTools::CopyFileIfDifferent(tempOutputFile.c_str(),
      m_OuputFile.c_str());
    cmSystemTools::RemoveFile(tempOutputFile.c_str());
    cmSystemTools::SetPermissions(m_OuputFile.c_str(), perm);
    }
}


