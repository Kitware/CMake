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

  // where will the binaries be stored
  const char* binaryDirectory = argv[2].c_str();
  const char* sourceDirectory = argv[1].c_str();
  const char* projectName = 0;
  const char* targetName = 0;
  std::string tmpString;

  // compute the binary dir when TRY_COMPILE is called with a src file
  // signature
  if (argv.size() == 3)
    {
    tmpString = argv[2] + "/CMakeTmp";
    binaryDirectory = tmpString.c_str();
    }
  
  // do not allow recursive try Compiles
  if (!strcmp(binaryDirectory,m_Makefile->GetHomeOutputDirectory()))
    {
    cmSystemTools::Error("Attempt at a recursive or nested TRY_COMPILE", 
                         binaryDirectory);
    return false;
    }
      
  // which signature are we using? If we are using var srcfile bindir
  if (argv.size() == 3)
    {
    // remove any CMakeCache.txt files so we will have a clean test
    std::string ccFile = tmpString + "/CMakeCache.txt";
    cmSystemTools::RemoveFile(ccFile.c_str());
    
    // we need to create a directory and CMakeList file etc...
    // first create the directories
    sourceDirectory = binaryDirectory;
    cmSystemTools::MakeDirectory(binaryDirectory);

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
    fprintf(fout,"ADD_EXECUTABLE(cmTryCompileExec \"%s\")\n",argv[1].c_str());
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
                                   projectName, targetName);
  
  // set the result var to the return value to indicate success or failure
  m_Makefile->AddDefinition(argv[0].c_str(), (res == 0 ? "TRUE" : "FALSE"));
      
  // if we created a directory etc, then cleanup after ourselves  
  cmDirectory dir;
  dir.Load(binaryDirectory);
  size_t fileNum;
  for (fileNum = 0; fileNum <  dir.GetNumberOfFiles(); ++fileNum)
    {
    cmSystemTools::RemoveFile(dir.GetFile(fileNum));
    }

  return true;
}


      
