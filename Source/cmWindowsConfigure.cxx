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
#include "cmWindowsConfigure.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"
#include <windows.h>

bool cmWindowsConfigure::Configure(const char* file)
{
  std::ifstream fin(file);
  if(!fin)
    {
    return false;
    }
  char inbuf[5001];
  while(fin.getline(inbuf, 5000) )
    {
    std::string inputLine = inbuf;
    if(inputLine[0] != '#')
      {
      std::string destDir;
      std::string fromFile;
      std::string toFile;
      std::string::size_type pos = inputLine.find(':');
      if(pos != std::string::npos)
        {
        destDir = inputLine.substr(0, pos);
        std::string::size_type nextPos = inputLine.find(':', pos+1);
        if(nextPos != std::string::npos)
          {
          std::string toFileName = inputLine.substr(pos+1, nextPos-pos-1);
          fromFile = inputLine.substr(nextPos+1);
          toFile = destDir;
          toFile += "/";
          toFile += toFileName;
          cmSystemTools::ReplaceString(toFile, "${CMAKE_BINARY_DIR}",
                                       m_WhereBuild.c_str() );
          cmSystemTools::ReplaceString(toFile, "${CMAKE_SOURCE_DIR}",
                                       m_WhereSource.c_str() );
          cmSystemTools::ReplaceString(fromFile, "${CMAKE_BINARY_DIR}",
                                       m_WhereBuild.c_str() );
          cmSystemTools::ReplaceString(fromFile, "${CMAKE_SOURCE_DIR}",
                                       m_WhereSource.c_str() );
          cmSystemTools::ReplaceString(destDir, "${CMAKE_BINARY_DIR}",
                                       m_WhereBuild.c_str() );
          cmSystemTools::ReplaceString(destDir, "${CMAKE_SOURCE_DIR}",
                                       m_WhereSource.c_str() );
          }
        }
      if(destDir != "" && fromFile != "" && toFile != "")
        {
        if(!cmSystemTools::MakeDirectory(destDir.c_str()) )
          {
          std::string error = "Error: can not create directory: ";
          error += destDir;
          MessageBox(0, error.c_str(), "config ERROR", MB_OK);
          return false;
          }
        if(!CopyFile(fromFile.c_str(), toFile.c_str(), FALSE))
          {
          std::string error = "Error: can not copy : ";
          error += fromFile;
          error += " to ";
          error += toFile;
          MessageBox(0, error.c_str(), "config ERROR", MB_OK);
          return false;
          }
        }
      else if (inputLine != "")
        {
        std::string error = "Error in parsing : ";
        error += file;
        error += " in line:\n ";
        error += inputLine;
        MessageBox(0, error.c_str(), "config ERROR", MB_OK);
        return false;
        }
      }
    }
  return true;
}

