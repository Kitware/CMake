/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2011 Kitware, Inc., Insight Software Consortium

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
#include "cmExportTryCompileFileGenerator.h"
#include <cmsys/Directory.hxx>

#include <assert.h>

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
  char targetNameBuf[64];
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
             argv[i] != "OUTPUT_VARIABLE" &&
             argv[i] != "LINK_LIBRARIES";
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
        this->Makefile->IssueMessage(cmake::FATAL_ERROR,
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
             argv[i] != "OUTPUT_VARIABLE" &&
             argv[i] != "LINK_LIBRARIES";
           ++i)
        {
        extraArgs++;
        compileFlags.push_back(argv[i]);
        }
      break;
      }
    }

  std::vector<cmTarget*> targets;
  std::string libsToLink = " ";
  for (i = 3; i < argv.size(); ++i)
    {
    if (argv[i] == "LINK_LIBRARIES")
      {
      if ( argv.size() <= (i+1) )
        {
        this->Makefile->IssueMessage(cmake::FATAL_ERROR,
          "LINK_LIBRARIES specified but there is no content");
        return -1;
        }
      extraArgs++;
      ++i;
      for ( ; i < argv.size() && argv[i] != "CMAKE_FLAGS"
          && argv[i] != "COMPILE_DEFINITIONS" && argv[i] != "OUTPUT_VARIABLE";
          ++i)
        {
        extraArgs++;
        libsToLink += argv[i] + " ";
        cmTarget *tgt = this->Makefile->FindTargetToUse(argv[i].c_str());
        if (!tgt)
          {
          continue;
          }
        switch(tgt->GetType())
        {
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::UNKNOWN_LIBRARY:
          break;
        case cmTarget::EXECUTABLE:
          if (tgt->IsExecutableWithExports())
            {
            break;
            }
        default:
          this->Makefile->IssueMessage(cmake::FATAL_ERROR,
            "Only libraries may be used as try_compile IMPORTED "
            "LINK_LIBRARIES.  Got " + std::string(tgt->GetName()) + " of "
            "type " + tgt->GetTargetTypeName(tgt->GetType()) + ".");
          return -1;
        }
        if (!tgt->IsImported())
          {
          continue;
          }
        targets.push_back(tgt);
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
        this->Makefile->IssueMessage(cmake::FATAL_ERROR,
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
      this->Makefile->IssueMessage(cmake::FATAL_ERROR,
        "COMPILE_FLAGS specified on a srcdir type TRY_COMPILE");
      return -1;
      }
    if (copyFile.size())
      {
      this->Makefile->IssueMessage(cmake::FATAL_ERROR,
        "COPY_FILE specified on a srcdir type TRY_COMPILE");
      return -1;
      }
    }
  // make sure the binary directory exists
  cmSystemTools::MakeDirectory(this->BinaryDirectory.c_str());

  // do not allow recursive try Compiles
  if (this->BinaryDirectory == this->Makefile->GetHomeOutputDirectory())
    {
    cmOStringStream e;
    e << "Attempt at a recursive or nested TRY_COMPILE in directory\n"
      << "  " << this->BinaryDirectory << "\n";
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
    return -1;
    }

  std::string outFileName = this->BinaryDirectory + "/CMakeLists.txt";
  // which signature are we using? If we are using var srcfile bindir
  if (this->SrcFileSignature)
    {
    // remove any CMakeCache.txt files so we will have a clean test
    std::string ccFile = this->BinaryDirectory + "/CMakeCache.txt";
    cmSystemTools::RemoveFile(ccFile.c_str());

    // we need to create a directory and CMakeLists file etc...
    // first create the directories
    sourceDirectory = this->BinaryDirectory.c_str();

    // now create a CMakeLists.txt file in that directory
    FILE *fout = fopen(outFileName.c_str(),"w");
    if (!fout)
      {
      cmOStringStream e;
      e << "Failed to open\n"
        << "  " << outFileName.c_str() << "\n"
        << cmSystemTools::GetLastSystemError();
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
      return -1;
      }

    std::string source = argv[2];
    std::string ext = cmSystemTools::GetFilenameLastExtension(source);
    const char* lang =(this->Makefile->GetCMakeInstance()->GetGlobalGenerator()
                        ->GetLanguageFromExtension(ext.c_str()));
    const char* def = this->Makefile->GetDefinition("CMAKE_MODULE_PATH");
    fprintf(fout, "cmake_minimum_required(VERSION %u.%u.%u.%u)\n",
            cmVersion::GetMajorVersion(), cmVersion::GetMinorVersion(),
            cmVersion::GetPatchVersion(), cmVersion::GetTweakVersion());
    if(def)
      {
      fprintf(fout, "SET(CMAKE_MODULE_PATH %s)\n", def);
      }

    const char* rulesOverrideBase = "CMAKE_USER_MAKE_RULES_OVERRIDE";
    std::string rulesOverrideLang =
      rulesOverrideBase + (lang ? std::string("_") + lang : std::string(""));
    if(const char* rulesOverridePath =
       this->Makefile->GetDefinition(rulesOverrideLang.c_str()))
      {
      fprintf(fout, "SET(%s \"%s\")\n",
              rulesOverrideLang.c_str(), rulesOverridePath);
      }
    else if(const char* rulesOverridePath2 =
            this->Makefile->GetDefinition(rulesOverrideBase))
      {
      fprintf(fout, "SET(%s \"%s\")\n",
              rulesOverrideBase, rulesOverridePath2);
      }

    if(lang)
      {
      fprintf(fout, "PROJECT(CMAKE_TRY_COMPILE %s)\n", lang);
      }
    else
      {
      fclose(fout);
      cmOStringStream err;
      err << "Unknown extension \"" << ext << "\" for file\n"
          << "  " << source << "\n"
          << "try_compile() works only for enabled languages.  "
          << "Currently these are:\n ";
      std::vector<std::string> langs;
      this->Makefile->GetCMakeInstance()->GetGlobalGenerator()->
        GetEnabledLanguages(langs);
      for(std::vector<std::string>::iterator l = langs.begin();
          l != langs.end(); ++l)
        {
        err << " " << *l;
        }
      err << "\nSee project() command to enable other languages.";
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, err.str());
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

    /* Use a random file name to avoid rapid creation and deletion
       of the same executable name (some filesystems fail on that).  */
    sprintf(targetNameBuf, "cmTryCompileExec%u",
            cmSystemTools::RandomSeed());
    targetName = targetNameBuf;

    if (!targets.empty())
      {
      std::string fname = "/" + std::string(targetName) + "Targets.cmake";
      cmExportTryCompileFileGenerator tcfg;
      tcfg.SetExportFile((this->BinaryDirectory + fname).c_str());
      tcfg.SetExports(targets);
      tcfg.SetConfig(this->Makefile->GetDefinition(
                                          "CMAKE_TRY_COMPILE_CONFIGURATION"));

      if(!tcfg.GenerateImportFile())
        {
        this->Makefile->IssueMessage(cmake::FATAL_ERROR,
                                     "could not write export file.");
        return -1;
        }
      fprintf(fout,
              "\ninclude(\"${CMAKE_CURRENT_LIST_DIR}/%s\")\n\n",
              fname.c_str());
      }

    /* for the TRY_COMPILEs we want to be able to specify the architecture.
      So the user can set CMAKE_OSX_ARCHITECTURE to i386;ppc and then set
      CMAKE_TRY_COMPILE_OSX_ARCHITECTURE first to i386 and then to ppc to
      have the tests run for each specific architecture. Since
      cmLocalGenerator doesn't allow building for "the other"
      architecture only via CMAKE_OSX_ARCHITECTURES.
      */
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
    if(this->Makefile->GetDefinition("CMAKE_POSITION_INDEPENDENT_CODE")!=0)
      {
      fprintf(fout, "SET(CMAKE_POSITION_INDEPENDENT_CODE \"ON\")\n");
      }

    /* Put the executable at a known location (for COPY_FILE).  */
    fprintf(fout, "SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY \"%s\")\n",
            this->BinaryDirectory.c_str());
    /* Create the actual executable.  */
    fprintf(fout, "ADD_EXECUTABLE(%s \"%s\")\n", targetName, source.c_str());
    if (libsToLink.empty())
      {
      fprintf(fout,
              "TARGET_LINK_LIBRARIES(%s ${LINK_LIBRARIES})\n",targetName);
      }
    else
      {
      fprintf(fout, "TARGET_LINK_LIBRARIES(%s %s)\n",
              targetName,
              libsToLink.c_str());
      }
    fclose(fout);
    projectName = "CMAKE_TRY_COMPILE";
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
        emsg << "Cannot copy output executable\n"
             << "  '" << this->OutputFile.c_str() << "'\n"
             << "to destination specified by COPY_FILE:\n"
             << "  '" << copyFile.c_str() << "'\n";
        if(!this->FindErrorMessage.empty())
          {
          emsg << this->FindErrorMessage.c_str();
          }
        this->Makefile->IssueMessage(cmake::FATAL_ERROR, emsg.str());
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
          cmSystemTools::RemoveADirectory(fullPath.c_str());
          }
        else
          {
          // Sometimes anti-virus software hangs on to new files so we
          // cannot delete them immediately.  Try a few times.
          int tries = 5;
          while(!cmSystemTools::RemoveFile(fullPath.c_str()) &&
                --tries && cmSystemTools::FileExists(fullPath.c_str()))
            {
            cmSystemTools::Delay(500);
            }
          if(tries == 0)
            {
            std::string m = "Remove failed on file: " + fullPath;
            cmSystemTools::ReportLastSystemError(m.c_str());
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
  emsg << "Unable to find the executable at any of:\n";
  for (unsigned int i = 0; i < searchDirs.size(); ++i)
    {
    emsg << "  " << this->BinaryDirectory << searchDirs[i]
         << tmpOutputFile << "\n";
    }
  this->FindErrorMessage = emsg.str();
  return;
}
