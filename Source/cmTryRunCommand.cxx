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
#include "cmTryRunCommand.h"
#include "cmCacheManager.h"
#include "cmTryCompileCommand.h"

// cmExecutableCommand
bool cmTryRunCommand::InitialPass(std::vector<std::string> const& argv)
{
  if(argv.size() < 4)
    {
    return false;
    }

  if ( m_Makefile->GetLocal() )
    {
    return true;
    }

  // build an arg list for TryCompile and extract the runArgs
  std::vector<std::string> tryCompile;
  std::string runArgs;
  unsigned int i;
  for (i = 1; i < argv.size(); ++i)
    {
    if (argv[i] == "ARGS")
      {
      ++i;
      while (i < argv.size() && argv[i] != "COMPILE_DEFINITIONS" &&
             argv[i] != "CMAKE_FLAGS")
        {
        runArgs += " ";
        runArgs += argv[i];
        ++i;
        }
      if (i < argv.size())
        {
        tryCompile.push_back(argv[i]);
        }
      }
    else
      {
      tryCompile.push_back(argv[i]);
      }
    }
  
  // do the try compile
  int res = cmTryCompileCommand::CoreTryCompileCode(m_Makefile, tryCompile, false);
  
  // now try running the command if it compiled
  std::string binaryDirectory = argv[2] + "/CMakeTmp";
  if (!res)
    {
    int retVal;
    std::string output;
    std::string command;
    command = binaryDirectory;
    command += "/cmTryCompileExec";
    command += cmSystemTools::GetExecutableExtension();
    std::string fullPath;
    if(cmSystemTools::FileExists(command.c_str()))
      {
      fullPath = cmSystemTools::CollapseFullPath(command.c_str());
      }
    else
      {
      command = binaryDirectory;
      command += "/Debug/cmTryCompileExec";
      command += cmSystemTools::GetExecutableExtension();
      if(cmSystemTools::FileExists(command.c_str()))
        {
        fullPath = cmSystemTools::CollapseFullPath(command.c_str());
        }
      else
        {
        cmSystemTools::Error("Unable to find executable for TRY_RUN",
                             command.c_str());
        }
      }
    if (fullPath.size() > 1)
      {
      std::string finalCommand = fullPath;
      if(runArgs.size())
        {
        finalCommand = cmSystemTools::ConvertToOutputPath(fullPath.c_str());
        finalCommand += runArgs;
        }
      cmSystemTools::RunCommand(finalCommand.c_str(), output, retVal, 0, false);
      // set the run var
      char retChar[1000];
      sprintf(retChar,"%i",retVal);
      m_Makefile->AddCacheDefinition(argv[0].c_str(), retChar,
                                      "Result of TRY_RUN", cmCacheManager::INTERNAL);
      }
    }
  
  // if we created a directory etc, then cleanup after ourselves  
  cmDirectory dir;
  dir.Load(binaryDirectory.c_str());
  size_t fileNum;
  for (fileNum = 0; fileNum <  dir.GetNumberOfFiles(); ++fileNum)
    {
    if (strcmp(dir.GetFile(fileNum),".") &&
        strcmp(dir.GetFile(fileNum),".."))
      {
      std::string fullPath = binaryDirectory;
      fullPath += "/";
      fullPath += dir.GetFile(fileNum);
      cmSystemTools::RemoveFile(fullPath.c_str());
      }
    }
  std::string cacheFile = binaryDirectory;
  cacheFile += "/CMakeLists.txt";
  cmListFileCache::GetInstance()->FlushCache(cacheFile.c_str());
  
  return true;
}


      
