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
#include "cmTryCompileCommand.h"
#include "cmCacheManager.h"

// cmExecutableCommand
bool cmTryCompileCommand::InitialPass(std::vector<std::string> const& argv)
{
  if(argv.size() < 3)
    {
    return false;
    }

  // which signature were we called with ?
  bool srcFileSignature = true;
  
  // look for CMAKE_FLAGS and store them
  std::vector<std::string> cmakeFlags;
  int i;
  for (i = 3; i < argv.size(); ++i)
    {
    if (argv[i] == "CMAKE_FLAGS")
      {
      for (; i < argv.size(); ++i)
        {
        cmakeFlags.push_back(argv[i]);
        }
      }
    else
      {
      srcFileSignature = false;
      }
    }
  
  // where will the binaries be stored
  const char* binaryDirectory = argv[1].c_str();
  const char* sourceDirectory = argv[2].c_str();
  const char* projectName = 0;
  const char* targetName = 0;
  std::string tmpString;

  // compute the binary dir when TRY_COMPILE is called with a src file
  // signature
  if (srcFileSignature)
    {
    tmpString = argv[1] + "/CMakeTmp";
    binaryDirectory = tmpString.c_str();
    }
  // make sure the binary directory exists
  cmSystemTools::MakeDirectory(binaryDirectory);
  
  // do not allow recursive try Compiles
  if (!strcmp(binaryDirectory,m_Makefile->GetHomeOutputDirectory()))
    {
    cmSystemTools::Error("Attempt at a recursive or nested TRY_COMPILE", 
                         binaryDirectory);
    return false;
    }
      
  // which signature are we using? If we are using var srcfile bindir
  if (srcFileSignature)
    {
    // remove any CMakeCache.txt files so we will have a clean test
    std::string ccFile = tmpString + "/CMakeCache.txt";
    cmSystemTools::RemoveFile(ccFile.c_str());
    
    // we need to create a directory and CMakeList file etc...
    // first create the directories
    sourceDirectory = binaryDirectory;

    // now create a CMakeList.txt file in that directory
    std::string outFileName = tmpString + "/CMakeLists.txt";
    FILE *fout = fopen(outFileName.c_str(),"w");
    if (!fout)
      {
      cmSystemTools::Error("Failed to create CMakeList file for ", 
                           outFileName.c_str());
      return false;
      }
    fprintf(fout,"PROJECT(CMAKE_TRY_COMPILE)\n");
    fprintf(fout, "IF (CMAKE_ANSI_CXXFLAGS)\n");
    fprintf(fout, "  SET(CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} ${CMAKE_ANSI_CXXFLAGS}\")\n");
    fprintf(fout, "ENDIF (CMAKE_ANSI_CXXFLAGS)\n");
    fprintf(fout,"ADD_EXECUTABLE(cmTryCompileExec \"%s\")\n",argv[2].c_str());
    fclose(fout);
    projectName = "CMAKE_TRY_COMPILE";
    targetName = "cmTryCompileExec";
    }
  // else the srcdir bindir project target signature
  else
    {
    projectName = argv[3].c_str();
    
    if (argv.size() == 5)
      {
      targetName = argv[4].c_str();
      }
    }
  
  // actually do the try compile now that everything is setup
  int res = m_Makefile->TryCompile(sourceDirectory, binaryDirectory,
                                   projectName, targetName, &cmakeFlags);
  
  // set the result var to the return value to indicate success or failure
  m_Makefile->AddDefinition(argv[0].c_str(), (res == 0 ? "TRUE" : "FALSE"));
      
  // if we created a directory etc, then cleanup after ourselves  
  if (srcFileSignature)
    {
    cmDirectory dir;
    dir.Load(binaryDirectory);
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
    }
  
  return true;
}


      
