/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#include "cmConfigureFileNoAutoconf.h"

// cmConfigureFileNoAutoconf
bool cmConfigureFileNoAutoconf::Invoke(std::vector<std::string>& args)
{
  if(args.size() != 2 )
    {
    this->SetError("called with incorrect number of arguments, expected 2");
    return false;
    }
  m_InputFile = args[0];
  m_OuputFile = args[1];
  return true;
}

void cmConfigureFileNoAutoconf::FinalPass()
{
#ifdef CMAKE_HAS_AUTOCONF
  return;
#else  
  m_Makefile->ExpandVariablesInString(m_InputFile);
  m_Makefile->ExpandVariablesInString(m_OuputFile);
  std::ifstream fin(m_InputFile.c_str());
  if(!fin)
    {
    cmSystemTools::Error("Could not open file for read in copy operatation",
                         m_InputFile.c_str());
    return;
    }
  cmSystemTools::ConvertToUnixSlashes(m_OuputFile);
  std::string::size_type pos = m_OuputFile.rfind('/');
  if(pos != std::string::npos)
    {
    std::string path = m_OuputFile.substr(0, pos);
    cmSystemTools::MakeDirectory(path.c_str());
    }
  std::string tempOutputFile = m_OuputFile;
  tempOutputFile += ".tmp";
  std::ofstream fout(tempOutputFile.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Could not open file for write in copy operatation", 
                         tempOutputFile.c_str());
    return;
    }
  // now copy input to output and expand varibles in the
  // input file at the same time
  const int bufSize = 4096;
  char buffer[bufSize];
  std::string inLine;
  while(fin)
    {
    fin.getline(buffer, bufSize);
    if(fin)
      {
      inLine = buffer;
      m_Makefile->ExpandVariablesInString(inLine);
      fout << inLine << "\n";
      }
    }
  // close the files before attempting to copy
  fin.close();
  fout.close();
  cmSystemTools::CopyFileIfDifferent(tempOutputFile.c_str(),
                                     m_OuputFile.c_str());
  cmSystemTools::RemoveFile(tempOutputFile.c_str());
#endif
}

  
