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
#include "cmTryCompileCommand.h"
#include "cmCacheManager.h"
#include "cmListFileCache.h"

int cmTryCompileCommand::CoreTryCompileCode(
  cmMakefile *mf, std::vector<std::string> const& argv, bool clean)
{
  // which signature were we called with ?
  bool srcFileSignature = false;
  unsigned int i;
  
  // where will the binaries be stored
  const char* binaryDirectory = argv[1].c_str();
  const char* sourceDirectory = argv[2].c_str();
  const char* projectName = 0;
  const char* targetName = 0;
  std::string tmpString;

  // do we have a srcfile signature
  if (argv.size() == 3 || argv[3] == "CMAKE_FLAGS" || argv[3] == "COMPILE_DEFINITIONS" ||
      argv[3] == "OUTPUT_VARIABLE")
    {
    srcFileSignature = true;
    }

  // look for CMAKE_FLAGS and store them
  std::vector<std::string> cmakeFlags;
  for (i = 3; i < argv.size(); ++i)
    {
    if (argv[i] == "CMAKE_FLAGS")
      {
      for (; i < argv.size() && argv[i] != "COMPILE_DEFINITIONS" && 
             argv[i] != "OUTPUT_VARIABLE"; 
           ++i)
        {
        cmakeFlags.push_back(argv[i]);
        }
      break;
      }
    }

  // look for OUTPUT_VARIABLE and store them
  std::string outputVariable;
  for (i = 3; i < argv.size(); ++i)
    {
    if (argv[i] == "OUTPUT_VARIABLE")
      {
      if ( argv.size() <= (i+1) )
        {
        cmSystemTools::Error(
          "OUTPUT_VARIABLE specified but there is no variable");
        return -1;
        }
      outputVariable = argv[i+1];
      break;
      }
    }

  // look for COMPILE_DEFINITIONS and store them
  std::vector<std::string> compileFlags;
  for (i = 3; i < argv.size(); ++i)
    {
    if (argv[i] == "COMPILE_DEFINITIONS")
      {
      // only valid for srcfile signatures
      if (!srcFileSignature)
        {
        cmSystemTools::Error(
          "COMPILE_FLAGS specified on a srcdir type TRY_COMPILE");
        return -1;
        }
      for (i = i + 1; i < argv.size() && argv[i] != "CMAKE_FLAGS" && 
             argv[i] != "OUTPUT_VARIABLE"; 
           ++i)
        {
        compileFlags.push_back(argv[i]);
        }
      break;
      }
    }

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
  if (!strcmp(binaryDirectory,mf->GetHomeOutputDirectory()))
    {
    cmSystemTools::Error("Attempt at a recursive or nested TRY_COMPILE", 
                         binaryDirectory);
    return -1;
    }
  
  std::string outFileName = tmpString + "/CMakeLists.txt";
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
    FILE *fout = fopen(outFileName.c_str(),"w");
    if (!fout)
      {
      cmSystemTools::Error("Failed to create CMakeList file for ", 
                           outFileName.c_str());
      return -1;
      }
    
    std::string source = argv[2];
    cmSystemTools::FileFormat format = 
      cmSystemTools::GetFileFormat( 
        cmSystemTools::GetFilenameExtension(source).c_str());
    if ( format == cmSystemTools::C_FILE_FORMAT )
      {
      fprintf(fout, "PROJECT(CMAKE_TRY_COMPILE C)\n");      
      }
    else if ( format == cmSystemTools::CXX_FILE_FORMAT )
      {
      fprintf(fout, "PROJECT(CMAKE_TRY_COMPILE CXX)\n");      
      }
    else
      {
      cmSystemTools::Error("Unknown file format for file: ", source.c_str(), 
                           "; TRY_COMPILE only works for C and CXX files");
      return -1;
      }

    fprintf(fout, "SET(CMAKE_C_FLAGS \"${CMAKE_C_FLAGS} ${COMPILE_DEFINITIONS}\"\")\n");
    fprintf(fout, "SET(CMAKE_CXX_FLAGS \"${CMAKE_CXX_FLAGS} ${COMPILE_DEFINITIONS}\")\n");
    fprintf(fout, "INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES})\n");
    fprintf(fout, "LINK_DIRECTORIES(${LINK_DIRECTORIES})\n");
    // handle any compile flags we need to pass on
    if (compileFlags.size())
      {
      fprintf(fout, "ADD_DEFINITIONS( ");
      for (i = 0; i < compileFlags.size(); ++i)
        {
        fprintf(fout,"%s ",compileFlags[i].c_str());
        }
      fprintf(fout, ")\n");
      }
    
    fprintf(fout, "ADD_EXECUTABLE(cmTryCompileExec \"%s\")\n",source.c_str());
    fprintf(fout, "TARGET_LINK_LIBRARIES(cmTryCompileExec ${LINK_LIBRARIES})\n");
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
  
  std::string output;
  // actually do the try compile now that everything is setup
  int res = mf->TryCompile(sourceDirectory, binaryDirectory,
                           projectName, targetName, &cmakeFlags, &output);
  
  // set the result var to the return value to indicate success or failure
  mf->AddCacheDefinition(argv[0].c_str(), (res == 0 ? "TRUE" : "FALSE"),
                         "Result of TRY_COMPILE",
                         cmCacheManager::INTERNAL);

  if ( outputVariable.size() > 0 )
    {
    mf->AddDefinition(outputVariable.c_str(), output.c_str());
    }
  
  // if They specified clean then we clean up what we can
  if (srcFileSignature && clean)
    {    
    cmListFileCache::GetInstance()->FlushCache(outFileName.c_str());
    cmTryCompileCommand::CleanupFiles(binaryDirectory);
    }
  
  return res;
}

// cmExecutableCommand
bool cmTryCompileCommand::InitialPass(std::vector<std::string> const& argv)
{
  if(argv.size() < 3)
    {
    return false;
    }

  if ( m_Makefile->GetLocal() )
    {
    return true;
    }

  cmTryCompileCommand::CoreTryCompileCode(m_Makefile,argv,true);
  
  return true;
}

static void cmTryCompileCommandNotUsed(bool){}

void cmTryCompileCommand::CleanupFiles(const char* binDir)
{
  if ( !binDir )
    {
    return;
    }
  std::string bdir = binDir;
  if(bdir.find("CMakeTmp") == std::string::npos)
    {
    cmSystemTools::Error("TRY_COMPILE attempt to remove -rf directory that does not contain CMakeTmp:", binDir);
    return;
    }
  
  cmDirectory dir;
  dir.Load(binDir);
  size_t fileNum;
  for (fileNum = 0; fileNum <  dir.GetNumberOfFiles(); ++fileNum)
    {
    if (strcmp(dir.GetFile(fileNum),".") &&
        strcmp(dir.GetFile(fileNum),".."))
      {
      std::string fullPath = binDir;
      fullPath += "/";
      fullPath += dir.GetFile(fileNum);
      if(cmSystemTools::FileIsDirectory(fullPath.c_str()))
        {
        cmTryCompileCommand::CleanupFiles(fullPath.c_str());
        }
      else
        {
        cmSystemTools::RemoveFile(fullPath.c_str());
        }
      }
    }
}
