/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCoreTryCompile.h"
#include "cmake.h"
#include "cmCacheManager.h"
#include "cmGlobalGenerator.h"
#include <cmsys/Directory.hxx>

int cmCoreTryCompile::TryCompileCode(std::vector<std::string> const& argv)
{
  this->BinaryDirectory = argv[1].c_str();
  this->OutputFile = "";
  // which signature were we called with ?
  this->SrcFileSignature = false;
  unsigned int i;
  
  const char* sourceDirectory = argv[2].c_str();
  const char* projectName = 0;
  const char* targetName = 0;
  int extraArgs = 0;
  
  // look for CMAKE_FLAGS and store them
  std::vector<std::string> cmakeFlags;
  for (i = 3; i < argv.size(); ++i)
    {
    if (argv[i] == "CMAKE_FLAGS")
      {
     // CMAKE_FLAGS is the first argument because we need an argv[0] that
     // is not used, so it matches regular command line parsing which has
     // the program name as arg 0
      for (; i < argv.size() && argv[i] != "COMPILE_DEFINITIONS" && 
             argv[i] != "OUTPUT_VARIABLE"; 
           ++i)
        {
        extraArgs++;
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
      extraArgs += 2;
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
      extraArgs++;
      for (i = i + 1; i < argv.size() && argv[i] != "CMAKE_FLAGS" && 
             argv[i] != "OUTPUT_VARIABLE"; 
           ++i)
        {
        extraArgs++;
        compileFlags.push_back(argv[i]);
        }
      break;
      }
    }

  // look for COPY_FILE
  std::string copyFile;
  for (i = 3; i < argv.size(); ++i)
    {
    if (argv[i] == "COPY_FILE")
      {
      if ( argv.size() <= (i+1) )
        {
        cmSystemTools::Error(
          "COPY_FILE specified but there is no variable");
        return -1;
        }
      extraArgs += 2;
      copyFile = argv[i+1];
      break;
      }
    }
    
  // do we have a srcfile signature
  if (argv.size() - extraArgs == 3)
    {
    this->SrcFileSignature = true;
    }

  // compute the binary dir when TRY_COMPILE is called with a src file
  // signature
  if (this->SrcFileSignature)
    {
    this->BinaryDirectory += cmake::GetCMakeFilesDirectory();
    this->BinaryDirectory += "/CMakeTmp";
    }
  else
    {
    // only valid for srcfile signatures
    if (compileFlags.size())
      {
      cmSystemTools::Error(
        "COMPILE_FLAGS specified on a srcdir type TRY_COMPILE");
      return -1;
      }
    if (copyFile.size())
      {
      cmSystemTools::Error("COPY_FILE specified on a srcdir type TRY_COMPILE");
      return -1;
      }
    }
  // make sure the binary directory exists
  cmSystemTools::MakeDirectory(this->BinaryDirectory.c_str());
  
  // do not allow recursive try Compiles
  if (this->BinaryDirectory == this->Makefile->GetHomeOutputDirectory())
    {
    cmSystemTools::Error(
      "Attempt at a recursive or nested TRY_COMPILE in directory ",
      this->BinaryDirectory.c_str());
    return -1;
    }
  
  std::string outFileName = this->BinaryDirectory + "/CMakeLists.txt";
  // which signature are we using? If we are using var srcfile bindir
  if (this->SrcFileSignature)
    {
    // remove any CMakeCache.txt files so we will have a clean test
    std::string ccFile = this->BinaryDirectory + "/CMakeCache.txt";
    cmSystemTools::RemoveFile(ccFile.c_str());
    
    // we need to create a directory and CMakeList file etc...
    // first create the directories
    sourceDirectory = this->BinaryDirectory.c_str();

    // now create a CMakeList.txt file in that directory
    FILE *fout = fopen(outFileName.c_str(),"w");
    if (!fout)
      {
      cmSystemTools::Error("Failed to create CMakeList file for ", 
                           outFileName.c_str());
      cmSystemTools::ReportLastSystemError("");
      return -1;
      }

    std::string source = argv[2];
    std::string ext = cmSystemTools::GetFilenameExtension(source);
    const char* lang =(this->Makefile->GetCMakeInstance()->GetGlobalGenerator()
                        ->GetLanguageFromExtension(ext.c_str()));
    const char* def = this->Makefile->GetDefinition("CMAKE_MODULE_PATH");
    fprintf(fout, "cmake_minimum_required(VERSION %u.%u)\n",
            cmVersion::GetMajorVersion(), cmVersion::GetMinorVersion());
    if(def)
      {
      fprintf(fout, "SET(CMAKE_MODULE_PATH %s)\n", def);
      }
    if(lang)
      {
      fprintf(fout, "PROJECT(CMAKE_TRY_COMPILE %s)\n", lang);
      }
    else
      {
      cmOStringStream err;
      err << "Unknown extension \"" << ext << "\" for file \""
          << source << "\".  TRY_COMPILE only works for enabled languages.\n"
          << "Currently enabled languages are:";
      std::vector<std::string> langs;
      this->Makefile->GetCMakeInstance()->GetGlobalGenerator()->
        GetEnabledLanguages(langs);
      for(std::vector<std::string>::iterator l = langs.begin();
          l != langs.end(); ++l)
        {
        err << " " << *l;
        }
      err << "\nSee PROJECT command for help enabling other languages.";
      cmSystemTools::Error(err.str().c_str());
      fclose(fout);
      return -1;
      }
    std::string langFlags = "CMAKE_";
    langFlags +=  lang;
    langFlags += "_FLAGS";
    fprintf(fout, "SET(CMAKE_VERBOSE_MAKEFILE 1)\n");
    fprintf(fout, "SET(CMAKE_%s_FLAGS \"", lang);
    const char* flags = this->Makefile->GetDefinition(langFlags.c_str()); 
    if(flags)
      {
      fprintf(fout, " %s ", flags);
      }
    fprintf(fout, " ${COMPILE_DEFINITIONS}\")\n");
    fprintf(fout, "INCLUDE_DIRECTORIES(${INCLUDE_DIRECTORIES})\n");
    fprintf(fout, "SET(CMAKE_SUPPRESS_REGENERATION 1)\n");
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

    /* for the TRY_COMPILEs we want to be able to specify the architecture.
      So the user can set CMAKE_OSX_ARCHITECTURE to i386;ppc and then set 
      CMAKE_TRY_COMPILE_OSX_ARCHITECTURE first to i386 and then to ppc to
      have the tests run for each specific architecture. Since 
      cmLocalGenerator doesn't allow building for "the other" 
      architecture only via CMAKE_OSX_ARCHITECTURES,use to CMAKE_DO_TRY_COMPILE
      to enforce it for this case here.
      */
    cmakeFlags.push_back("-DCMAKE_DO_TRY_COMPILE=TRUE");
    if(this->Makefile->GetDefinition("CMAKE_TRY_COMPILE_OSX_ARCHITECTURES")!=0)
      {
      std::string flag="-DCMAKE_OSX_ARCHITECTURES=";
      flag += this->Makefile->GetSafeDefinition(
                                        "CMAKE_TRY_COMPILE_OSX_ARCHITECTURES");
      cmakeFlags.push_back(flag);
      }
    else if (this->Makefile->GetDefinition("CMAKE_OSX_ARCHITECTURES")!=0)
      {
      std::string flag="-DCMAKE_OSX_ARCHITECTURES=";
      flag += this->Makefile->GetSafeDefinition("CMAKE_OSX_ARCHITECTURES");
      cmakeFlags.push_back(flag);
      }
    /* on APPLE also pass CMAKE_OSX_SYSROOT to the try_compile */
    if(this->Makefile->GetDefinition("CMAKE_OSX_SYSROOT")!=0)
      {
      std::string flag="-DCMAKE_OSX_SYSROOT=";
      flag += this->Makefile->GetSafeDefinition("CMAKE_OSX_SYSROOT");
      cmakeFlags.push_back(flag);
      }
    /* on APPLE also pass CMAKE_OSX_DEPLOYMENT_TARGET to the try_compile */
    if(this->Makefile->GetDefinition("CMAKE_OSX_DEPLOYMENT_TARGET")!=0)
      {
      std::string flag="-DCMAKE_OSX_DEPLOYMENT_TARGET=";
      flag += this->Makefile->GetSafeDefinition("CMAKE_OSX_DEPLOYMENT_TARGET");
      cmakeFlags.push_back(flag);
      }

    fprintf(fout, "ADD_EXECUTABLE(cmTryCompileExec \"%s\")\n",source.c_str());
    fprintf(fout, 
            "TARGET_LINK_LIBRARIES(cmTryCompileExec ${LINK_LIBRARIES})\n");
    fclose(fout);
    projectName = "CMAKE_TRY_COMPILE";
    targetName = "cmTryCompileExec";
    // if the source is not in CMakeTmp 
    if(source.find("CMakeTmp") == source.npos)
      {
      this->Makefile->AddCMakeDependFile(source.c_str());
      }
    
    }
  // else the srcdir bindir project target signature
  else
    {
    projectName = argv[3].c_str();
    
    if (argv.size() - extraArgs == 5)
      {
      targetName = argv[4].c_str();
      }
    }
  
  bool erroroc = cmSystemTools::GetErrorOccuredFlag();
  cmSystemTools::ResetErrorOccuredFlag();
  std::string output;
  // actually do the try compile now that everything is setup
  int res = this->Makefile->TryCompile(sourceDirectory, 
                                       this->BinaryDirectory.c_str(),
                                       projectName, 
                                       targetName, 
                                       this->SrcFileSignature,
                                       &cmakeFlags, 
                                       &output);
  if ( erroroc )
    {
    cmSystemTools::SetErrorOccured();
    }
  
  // set the result var to the return value to indicate success or failure
  this->Makefile->AddCacheDefinition(argv[0].c_str(),
                                     (res == 0 ? "TRUE" : "FALSE"),
                                     "Result of TRY_COMPILE",
                                     cmCacheManager::INTERNAL);

  if ( outputVariable.size() > 0 )
    {
    this->Makefile->AddDefinition(outputVariable.c_str(), output.c_str());
    }
  
  if (this->SrcFileSignature)
    {
    this->FindOutputFile(targetName);

    if ((res==0) && (copyFile.size()))
      {
      if(this->OutputFile.empty() ||
         !cmSystemTools::CopyFileAlways(this->OutputFile.c_str(),
                                        copyFile.c_str()))
        {
        cmOStringStream emsg;
        emsg << "Could not COPY_FILE.\n"
          << "  OutputFile: '" << this->OutputFile.c_str() << "'\n"
          << "    copyFile: '" << copyFile.c_str() << "'\n";

        if (this->FindErrorMessage.size())
          {
          emsg << "\n";
          emsg << this->FindErrorMessage.c_str() << "\n";
          }

        cmSystemTools::Error(emsg.str().c_str());
        return -1;
        }
      }
    }
  return res;
}

void cmCoreTryCompile::CleanupFiles(const char* binDir)
{
  if ( !binDir )
    {
    return;
    }
  
  std::string bdir = binDir;
  if(bdir.find("CMakeTmp") == std::string::npos)
    {
    cmSystemTools::Error(
      "TRY_COMPILE attempt to remove -rf directory that does not contain "
      "CMakeTmp:", binDir);
    return;
    }
  
  cmsys::Directory dir;
  dir.Load(binDir);
  size_t fileNum;
  std::set<cmStdString> deletedFiles;
  for (fileNum = 0; fileNum <  dir.GetNumberOfFiles(); ++fileNum)
    {
    if (strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)),".") &&
        strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)),".."))
      {
      
      if(deletedFiles.find( dir.GetFile(static_cast<unsigned long>(fileNum))) 
         == deletedFiles.end())
        {
        deletedFiles.insert(dir.GetFile(static_cast<unsigned long>(fileNum)));
        std::string fullPath = binDir;
        fullPath += "/";
        fullPath += dir.GetFile(static_cast<unsigned long>(fileNum));
        if(cmSystemTools::FileIsDirectory(fullPath.c_str()))
          {
          this->CleanupFiles(fullPath.c_str());
          }
        else
          {
          if(!cmSystemTools::RemoveFile(fullPath.c_str()))
            {
            bool removed = false;
            int numAttempts = 0;
            // sometimes anti-virus software hangs on to
            // new files and we can not delete them, so try
            // 5 times with .5 second delay between tries.
            while(!removed && numAttempts < 5)
              {
              cmSystemTools::Delay(500);
              if(cmSystemTools::RemoveFile(fullPath.c_str()))
                {
                removed = true;
                }
              numAttempts++;
              }
            if(!removed)
              {
              std::string m = "Remove failed on file: ";
              m += fullPath;
              cmSystemTools::ReportLastSystemError(m.c_str());
              }
            }
          }
        }
      }
    }
}

void cmCoreTryCompile::FindOutputFile(const char* targetName)
{
  this->FindErrorMessage = "";
  this->OutputFile = "";
  std::string tmpOutputFile = "/";
  tmpOutputFile += targetName;
  tmpOutputFile +=this->Makefile->GetSafeDefinition("CMAKE_EXECUTABLE_SUFFIX");

  // a list of directories where to search for the compilation result
  // at first directly in the binary dir
  std::vector<std::string> searchDirs;
  searchDirs.push_back("");

  const char* config = this->Makefile->GetDefinition(
                                            "CMAKE_TRY_COMPILE_CONFIGURATION");
  // if a config was specified try that first
  if (config && config[0])
    {
    std::string tmp = "/";
    tmp += config;
    searchDirs.push_back(tmp);
    }
  searchDirs.push_back("/Debug");
  searchDirs.push_back("/Development");

  for(std::vector<std::string>::const_iterator it = searchDirs.begin();
      it != searchDirs.end();
      ++it)
    {
    std::string command = this->BinaryDirectory;
    command += *it;
    command += tmpOutputFile;
    if(cmSystemTools::FileExists(command.c_str()))
      {
      tmpOutputFile = cmSystemTools::CollapseFullPath(command.c_str());
      this->OutputFile = tmpOutputFile;
      return;
      }
    }

  cmOStringStream emsg;
  emsg << "Unable to find executable for " << this->GetName() << ": tried \"";
  for (unsigned int i = 0; i < searchDirs.size(); ++i)
    {
    emsg << this->BinaryDirectory << searchDirs[i] << tmpOutputFile;
    if (i < searchDirs.size() - 1)
      {
      emsg << "\" and \"";
      }
    else
      {
      emsg << "\".";
      }
    }
  this->FindErrorMessage = emsg.str();
  return;
}
