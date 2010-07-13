/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackGenerator.h"

#include "cmMakefile.h"
#include "cmCPackLog.h"
#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackComponentGroup.h"
#include "cmXMLSafe.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/Glob.hxx>
#include <memory> // auto_ptr

#if defined(__HAIKU__)
#include <StorageKit.h>
#endif

//----------------------------------------------------------------------
cmCPackGenerator::cmCPackGenerator()
{
  this->GeneratorVerbose = false;
  this->MakefileMap = 0;
  this->Logger = 0;
}

//----------------------------------------------------------------------
cmCPackGenerator::~cmCPackGenerator()
{
  this->MakefileMap = 0;
}

//----------------------------------------------------------------------
void cmCPackGeneratorProgress(const char *msg, float prog, void* ptr)
{
  cmCPackGenerator* self = static_cast<cmCPackGenerator*>(ptr);
  self->DisplayVerboseOutput(msg, prog);
}

//----------------------------------------------------------------------
void cmCPackGenerator::DisplayVerboseOutput(const char* msg,
  float progress)
{
  (void)progress;
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "" << msg << std::endl);
}

//----------------------------------------------------------------------
int cmCPackGenerator::PrepareNames()
{  
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
    "Create temp directory." << std::endl);

  std::string tempDirectory = this->GetOption("CPACK_PACKAGE_DIRECTORY");
  tempDirectory += "/_CPack_Packages/";
  const char* toplevelTag = this->GetOption("CPACK_TOPLEVEL_TAG");
  if ( toplevelTag )
    {
    tempDirectory += toplevelTag;
    tempDirectory += "/";
    }
  tempDirectory += this->GetOption("CPACK_GENERATOR");
  std::string topDirectory = tempDirectory;
  this->GetOption("CPACK_PACKAGE_FILE_NAME");
  const char* pfname = this->GetOption("CPACK_PACKAGE_FILE_NAME");
  if(!pfname)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "CPACK_PACKAGE_FILE_NAME not specified" << std::endl);
    return 0;
    }
  std::string outName = pfname; 
  tempDirectory += "/" + outName;
  if(!this->GetOutputExtension())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "No output extension specified" << std::endl);
    return 0;
    }
  outName += this->GetOutputExtension();
  const char* pdir = this->GetOption("CPACK_PACKAGE_DIRECTORY");
  if(!pdir)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "CPACK_PACKAGE_DIRECTORY not specified" << std::endl);
    return 0;
    }

  std::string destFile = pdir;
  destFile += "/" + outName;
  std::string outFile = topDirectory + "/" + outName;
  this->SetOptionIfNotSet("CPACK_TOPLEVEL_DIRECTORY", topDirectory.c_str());
  this->SetOptionIfNotSet("CPACK_TEMPORARY_DIRECTORY", tempDirectory.c_str());
  this->SetOptionIfNotSet("CPACK_OUTPUT_FILE_NAME", outName.c_str());
  this->SetOptionIfNotSet("CPACK_OUTPUT_FILE_PATH", destFile.c_str());
  this->SetOptionIfNotSet("CPACK_TEMPORARY_PACKAGE_FILE_NAME",
                          outFile.c_str());
  this->SetOptionIfNotSet("CPACK_INSTALL_DIRECTORY", this->GetInstallPath());
  this->SetOptionIfNotSet("CPACK_NATIVE_INSTALL_DIRECTORY",
    cmsys::SystemTools::ConvertToOutputPath(this->GetInstallPath()).c_str());
  this->SetOptionIfNotSet("CPACK_TEMPORARY_INSTALL_DIRECTORY",
                          tempDirectory.c_str());

  cmCPackLogger(cmCPackLog::LOG_DEBUG,
    "Look for: CPACK_PACKAGE_DESCRIPTION_FILE" << std::endl);
  const char* descFileName
    = this->GetOption("CPACK_PACKAGE_DESCRIPTION_FILE");
  if ( descFileName )
    {
    cmCPackLogger(cmCPackLog::LOG_DEBUG,
                  "Look for: " << descFileName << std::endl);
    if ( !cmSystemTools::FileExists(descFileName) )
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Cannot find description file name: [" 
                    << descFileName << "]" << std::endl);
      return 0;
      }
    std::ifstream ifs(descFileName);
    if ( !ifs )
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Cannot open description file name: " << descFileName << std::endl);
      return 0;
      }
    cmOStringStream ostr;
    std::string line;

    cmCPackLogger(cmCPackLog::LOG_VERBOSE,
      "Read description file: " << descFileName << std::endl);
    while ( ifs && cmSystemTools::GetLineFromStream(ifs, line) )
      {
      ostr << cmXMLSafe(line) << std::endl;
      }
    this->SetOptionIfNotSet("CPACK_PACKAGE_DESCRIPTION", ostr.str().c_str());
    }
  if ( !this->GetOption("CPACK_PACKAGE_DESCRIPTION") )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Project description not specified. Please specify "
      "CPACK_PACKAGE_DESCRIPTION or CPACK_PACKAGE_DESCRIPTION_FILE."
      << std::endl);
    return 0;
    }

  this->SetOptionIfNotSet("CPACK_REMOVE_TOPLEVEL_DIRECTORY", "1");

  return 1;
}

//----------------------------------------------------------------------
int cmCPackGenerator::InstallProject()
{
  cmCPackLogger(cmCPackLog::LOG_OUTPUT, "Install projects" << std::endl);
  this->CleanTemporaryDirectory();

  std::string bareTempInstallDirectory
    = this->GetOption("CPACK_TEMPORARY_INSTALL_DIRECTORY");
  std::string tempInstallDirectoryStr = bareTempInstallDirectory;
  bool setDestDir = cmSystemTools::IsOn(this->GetOption("CPACK_SET_DESTDIR"));
  if (!setDestDir)
    {
    tempInstallDirectoryStr += this->GetPackagingInstallPrefix();
    }

  const char* tempInstallDirectory = tempInstallDirectoryStr.c_str();
  int res = 1;
  if ( !cmsys::SystemTools::MakeDirectory(bareTempInstallDirectory.c_str()))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Problem creating temporary directory: " 
                  << (tempInstallDirectory ? tempInstallDirectory : "(NULL}")
                  << std::endl);
    return 0;
    }

  if ( setDestDir )
    {
    std::string destDir = "DESTDIR=";
    destDir += tempInstallDirectory;
    cmSystemTools::PutEnv(destDir.c_str());
    }
  else
    {
    // Make sure there is no destdir
    cmSystemTools::PutEnv("DESTDIR=");
    }

  // If the CPackConfig file sets CPACK_INSTALL_COMMANDS then run them
  // as listed
  if ( !this->InstallProjectViaInstallCommands(
      setDestDir, tempInstallDirectory) )
    {
    return 0;
    }

  // If the CPackConfig file sets CPACK_INSTALL_SCRIPT then run them
  // as listed
  if ( !this->InstallProjectViaInstallScript(
      setDestDir, tempInstallDirectory) )
    {
    return 0;
    }

  // If the CPackConfig file sets CPACK_INSTALLED_DIRECTORIES
  // then glob it and copy it to CPACK_TEMPORARY_DIRECTORY
  // This is used in Source packageing
  if ( !this->InstallProjectViaInstalledDirectories(
      setDestDir, tempInstallDirectory) )
    {
    return 0;
    }


  // If the project is a CMAKE project then run pre-install
  // and then read the cmake_install script to run it
  if ( !this->InstallProjectViaInstallCMakeProjects(
         setDestDir, bareTempInstallDirectory.c_str()) )
    {
    return 0;
    }

  if ( setDestDir )
    {
    cmSystemTools::PutEnv("DESTDIR=");
    }

  return res;
}

//----------------------------------------------------------------------
int cmCPackGenerator::InstallProjectViaInstallCommands(
  bool setDestDir, const char* tempInstallDirectory)
{
  (void) setDestDir;
  const char* installCommands = this->GetOption("CPACK_INSTALL_COMMANDS");
  if ( installCommands && *installCommands )
    {
    std::string tempInstallDirectoryEnv = "CMAKE_INSTALL_PREFIX=";
    tempInstallDirectoryEnv += tempInstallDirectory;
    cmSystemTools::PutEnv(tempInstallDirectoryEnv.c_str());
    std::vector<std::string> installCommandsVector;
    cmSystemTools::ExpandListArgument(installCommands,installCommandsVector);
    std::vector<std::string>::iterator it;
    for ( it = installCommandsVector.begin();
      it != installCommandsVector.end();
      ++it )
      {
      cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Execute: " << it->c_str()
        << std::endl);
      std::string output;
      int retVal = 1;
      bool resB = cmSystemTools::RunSingleCommand(it->c_str(), &output,
        &retVal, 0, this->GeneratorVerbose, 0);
      if ( !resB || retVal )
        {
        std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
        tmpFile += "/InstallOutput.log";
        cmGeneratedFileStream ofs(tmpFile.c_str());
        ofs << "# Run command: " << it->c_str() << std::endl
          << "# Output:" << std::endl
          << output.c_str() << std::endl;
        cmCPackLogger(cmCPackLog::LOG_ERROR,
          "Problem running install command: " << it->c_str() << std::endl
          << "Please check " << tmpFile.c_str() << " for errors"
          << std::endl);
        return 0;
        }
      }
    }
  return 1;
}

//----------------------------------------------------------------------
int cmCPackGenerator::InstallProjectViaInstalledDirectories(
  bool setDestDir, const char* tempInstallDirectory)
{
  (void)setDestDir;
  (void)tempInstallDirectory;
  std::vector<cmsys::RegularExpression> ignoreFilesRegex;
  const char* cpackIgnoreFiles = this->GetOption("CPACK_IGNORE_FILES");
  if ( cpackIgnoreFiles )
    {
    std::vector<std::string> ignoreFilesRegexString;
    cmSystemTools::ExpandListArgument(cpackIgnoreFiles,
                                      ignoreFilesRegexString);
    std::vector<std::string>::iterator it;
    for ( it = ignoreFilesRegexString.begin();
      it != ignoreFilesRegexString.end();
      ++it )
      {
      cmCPackLogger(cmCPackLog::LOG_VERBOSE,
        "Create ignore files regex for: " << it->c_str() << std::endl);
      ignoreFilesRegex.push_back(it->c_str());
      }
    }
  const char* installDirectories
    = this->GetOption("CPACK_INSTALLED_DIRECTORIES");
  if ( installDirectories && *installDirectories )
    {
    std::vector<std::string> installDirectoriesVector;
    cmSystemTools::ExpandListArgument(installDirectories,
      installDirectoriesVector);
    if ( installDirectoriesVector.size() % 2 != 0 )
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "CPACK_INSTALLED_DIRECTORIES should contain pairs of <directory> and "
        "<subdirectory>. The <subdirectory> can be '.' to be installed in "
        "the toplevel directory of installation." << std::endl);
      return 0;
      }
    std::vector<std::string>::iterator it;
    const char* tempDir = tempInstallDirectory;
    for ( it = installDirectoriesVector.begin();
      it != installDirectoriesVector.end();
      ++it )
      {
      cmCPackLogger(cmCPackLog::LOG_DEBUG, "Find files" << std::endl);
      cmsys::Glob gl;
      std::string toplevel = it->c_str();
      it ++;
      std::string subdir = it->c_str();
      std::string findExpr = toplevel;
      findExpr += "/*";
      cmCPackLogger(cmCPackLog::LOG_OUTPUT,
        "- Install directory: " << toplevel << std::endl);
      gl.RecurseOn();
      if ( !gl.FindFiles(findExpr) )
        {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
          "Cannot find any files in the installed directory" << std::endl);
        return 0;
        }
      std::vector<std::string>& files = gl.GetFiles();
      std::vector<std::string>::iterator gfit;
      std::vector<cmsys::RegularExpression>::iterator regIt;
      for ( gfit = files.begin(); gfit != files.end(); ++ gfit )
        {
        bool skip = false;
        std::string &inFile = *gfit;
        for ( regIt= ignoreFilesRegex.begin();
          regIt!= ignoreFilesRegex.end();
          ++ regIt)
          {
          if ( regIt->find(inFile.c_str()) )
            {
            cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Ignore file: "
              << inFile.c_str() << std::endl);
            skip = true;
            }
          }
        if ( skip )
          {
          continue;
          }
        std::string filePath = tempDir;
        filePath += "/" + subdir + "/"
          + cmSystemTools::RelativePath(toplevel.c_str(), gfit->c_str());
        cmCPackLogger(cmCPackLog::LOG_DEBUG, "Copy file: "
          << inFile.c_str() << " -> " << filePath.c_str() << std::endl);
        if ( !cmSystemTools::CopyFileIfDifferent(inFile.c_str(),
            filePath.c_str()) )
          {
          cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem copying file: "
            << inFile.c_str() << " -> " << filePath.c_str() << std::endl);
          return 0;
          }
        }
      }
    }
  return 1;
}

//----------------------------------------------------------------------
int cmCPackGenerator::InstallProjectViaInstallScript(
  bool setDestDir, const char* tempInstallDirectory)
{
  const char* cmakeScripts
    = this->GetOption("CPACK_INSTALL_SCRIPT");
  if ( cmakeScripts && *cmakeScripts )
    {
    cmCPackLogger(cmCPackLog::LOG_OUTPUT,
                  "- Install scripts: " << cmakeScripts << std::endl);
    std::vector<std::string> cmakeScriptsVector;
    cmSystemTools::ExpandListArgument(cmakeScripts,
      cmakeScriptsVector);
    std::vector<std::string>::iterator it;
    for ( it = cmakeScriptsVector.begin();
      it != cmakeScriptsVector.end();
      ++it )
      {
      std::string installScript = it->c_str();

      cmCPackLogger(cmCPackLog::LOG_OUTPUT,
        "- Install script: " << installScript << std::endl);

      if ( setDestDir )
        {
        // For DESTDIR based packaging, use the *project* CMAKE_INSTALL_PREFIX
        // underneath the tempInstallDirectory. The value of the project's
        // CMAKE_INSTALL_PREFIX is sent in here as the value of the
        // CPACK_INSTALL_PREFIX variable.
       std::string dir;
        if (this->GetOption("CPACK_INSTALL_PREFIX"))
          {
          dir += this->GetOption("CPACK_INSTALL_PREFIX");
          }
        this->SetOption("CMAKE_INSTALL_PREFIX", dir.c_str());
        cmCPackLogger(cmCPackLog::LOG_DEBUG,
          "- Using DESTDIR + CPACK_INSTALL_PREFIX... (this->SetOption)"
          << std::endl);
        cmCPackLogger(cmCPackLog::LOG_DEBUG,
          "- Setting CMAKE_INSTALL_PREFIX to '" << dir << "'" << std::endl);
        }
      else
        {
        this->SetOption("CMAKE_INSTALL_PREFIX", tempInstallDirectory);

        cmCPackLogger(cmCPackLog::LOG_DEBUG,
          "- Using non-DESTDIR install... (this->SetOption)" << std::endl);
        cmCPackLogger(cmCPackLog::LOG_DEBUG,
          "- Setting CMAKE_INSTALL_PREFIX to '" << tempInstallDirectory
          << "'" << std::endl);
        }

      this->SetOptionIfNotSet("CMAKE_CURRENT_BINARY_DIR",
        tempInstallDirectory);
      this->SetOptionIfNotSet("CMAKE_CURRENT_SOURCE_DIR",
        tempInstallDirectory);
      int res = this->MakefileMap->ReadListFile(0, installScript.c_str());
      if ( cmSystemTools::GetErrorOccuredFlag() || !res )
        {
        return 0;
        }
      }
    }
  return 1;
}

//----------------------------------------------------------------------
int cmCPackGenerator::InstallProjectViaInstallCMakeProjects(
  bool setDestDir, const char* baseTempInstallDirectory)
{
  const char* cmakeProjects
    = this->GetOption("CPACK_INSTALL_CMAKE_PROJECTS");
  const char* cmakeGenerator
    = this->GetOption("CPACK_CMAKE_GENERATOR");
  if ( cmakeProjects && *cmakeProjects )
    {
    if ( !cmakeGenerator )
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "CPACK_INSTALL_CMAKE_PROJECTS is specified, but "
                    "CPACK_CMAKE_GENERATOR is not. CPACK_CMAKE_GENERATOR "
                    "is required to install the project."
                    << std::endl);
      return 0;
      }
    std::vector<std::string> cmakeProjectsVector;
    cmSystemTools::ExpandListArgument(cmakeProjects,
      cmakeProjectsVector);
    std::vector<std::string>::iterator it;
    for ( it = cmakeProjectsVector.begin();
      it != cmakeProjectsVector.end();
      ++it )
      {
      if ( it+1 == cmakeProjectsVector.end() ||
        it+2 == cmakeProjectsVector.end() ||
        it+3 == cmakeProjectsVector.end() )
        {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
          "Not enough items on list: CPACK_INSTALL_CMAKE_PROJECTS. "
          "CPACK_INSTALL_CMAKE_PROJECTS should hold quadruplet of install "
          "directory, install project name, install component, and install "
          "subdirectory."
          << std::endl);
        return 0;
        }
      std::string installDirectory = it->c_str();
      ++it;
      std::string installProjectName = it->c_str();
      ++it;
      std::string installComponent = it->c_str();
      ++it;
      std::string installSubDirectory = it->c_str();
      std::string installFile = installDirectory + "/cmake_install.cmake";

      std::vector<std::string> componentsVector;

      bool componentInstall = false;
      if (this->SupportsComponentInstallation())
        {
        // Determine the installation types for this project (if provided).
        std::string installTypesVar = "CPACK_" 
          + cmSystemTools::UpperCase(installComponent) + "_INSTALL_TYPES";
        const char *installTypes = this->GetOption(installTypesVar.c_str());
        if (installTypes && *installTypes)
          {
          std::vector<std::string> installTypesVector;
          cmSystemTools::ExpandListArgument(installTypes, installTypesVector);
          std::vector<std::string>::iterator installTypeIt;
          for (installTypeIt = installTypesVector.begin();
               installTypeIt != installTypesVector.end();
               ++installTypeIt)
            {
            this->GetInstallationType(installProjectName.c_str(), 
                                      installTypeIt->c_str());
            }
          }

        // Determine the set of components that will be used in this project
        std::string componentsVar 
          = "CPACK_COMPONENTS_" + cmSystemTools::UpperCase(installComponent);
        const char *components = this->GetOption(componentsVar.c_str());
        if (components && *components) 
          {
          cmSystemTools::ExpandListArgument(components, componentsVector);
          std::vector<std::string>::iterator compIt;
          for (compIt = componentsVector.begin();
               compIt != componentsVector.end();
               ++compIt)
            {
            GetComponent(installProjectName.c_str(), compIt->c_str());
            }
          componentInstall = true;
          }
        }
      if (componentsVector.empty())
        {
        componentsVector.push_back(installComponent);
        }

      const char* buildConfig = this->GetOption("CPACK_BUILD_CONFIG");
      cmGlobalGenerator* globalGenerator
        = this->MakefileMap->GetCMakeInstance()->CreateGlobalGenerator(
          cmakeGenerator);
      // set the global flag for unix style paths on cmSystemTools as
      // soon as the generator is set.  This allows gmake to be used
      // on windows.
      cmSystemTools::SetForceUnixPaths(globalGenerator->GetForceUnixPaths());

      // Does this generator require pre-install?
      if ( globalGenerator->GetPreinstallTargetName() )
        {
        globalGenerator->FindMakeProgram(this->MakefileMap);
        const char* cmakeMakeProgram
          = this->MakefileMap->GetDefinition("CMAKE_MAKE_PROGRAM");
        std::string buildCommand
          = globalGenerator->GenerateBuildCommand(cmakeMakeProgram,
            installProjectName.c_str(), 0,
            globalGenerator->GetPreinstallTargetName(),
            buildConfig, false, false);
        cmCPackLogger(cmCPackLog::LOG_DEBUG,
          "- Install command: " << buildCommand << std::endl);
        cmCPackLogger(cmCPackLog::LOG_OUTPUT,
          "- Run preinstall target for: " << installProjectName << std::endl);
        std::string output;
        int retVal = 1;
        bool resB =
          cmSystemTools::RunSingleCommand(buildCommand.c_str(),
                                          &output,
                                          &retVal,
                                          installDirectory.c_str(),
                                          this->GeneratorVerbose, 0);
        if ( !resB || retVal )
          {
          std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
          tmpFile += "/PreinstallOutput.log";
          cmGeneratedFileStream ofs(tmpFile.c_str());
          ofs << "# Run command: " << buildCommand.c_str() << std::endl
            << "# Directory: " << installDirectory.c_str() << std::endl
            << "# Output:" << std::endl
            << output.c_str() << std::endl;
          cmCPackLogger(cmCPackLog::LOG_ERROR,
            "Problem running install command: " << buildCommand.c_str()
            << std::endl
            << "Please check " << tmpFile.c_str() << " for errors"
            << std::endl);
          return 0;
          }
        }
      delete globalGenerator;

      cmCPackLogger(cmCPackLog::LOG_OUTPUT,
        "- Install project: " << installProjectName << std::endl);

      // Run the installation for each component
      std::vector<std::string>::iterator componentIt;
      for (componentIt = componentsVector.begin();
           componentIt != componentsVector.end();
           ++componentIt)
        {
        std::string tempInstallDirectory = baseTempInstallDirectory;
        installComponent = *componentIt;
        if (componentInstall)
          {
          cmCPackLogger(cmCPackLog::LOG_OUTPUT,
                        "-   Install component: " << installComponent 
                        << std::endl);
          }
        
        cmake cm;
        cm.AddCMakePaths();
        cm.SetProgressCallback(cmCPackGeneratorProgress, this);
        cmGlobalGenerator gg;
        gg.SetCMakeInstance(&cm);
        std::auto_ptr<cmLocalGenerator> lg(gg.CreateLocalGenerator());
        cmMakefile *mf = lg->GetMakefile();
        std::string realInstallDirectory = tempInstallDirectory;
        if ( !installSubDirectory.empty() && installSubDirectory != "/" )
          {
          realInstallDirectory += installSubDirectory;
          }
        if (componentInstall)
          {
          tempInstallDirectory += "/";
          tempInstallDirectory += installComponent;
          }

        if (!setDestDir)
          {
          tempInstallDirectory += this->GetPackagingInstallPrefix();
          }

        if ( setDestDir )
          {
          // For DESTDIR based packaging, use the *project*
          // CMAKE_INSTALL_PREFIX underneath the tempInstallDirectory. The
          // value of the project's CMAKE_INSTALL_PREFIX is sent in here as
          // the value of the CPACK_INSTALL_PREFIX variable.
          //
          std::string dir;
          if (this->GetOption("CPACK_INSTALL_PREFIX"))
            {
            dir += this->GetOption("CPACK_INSTALL_PREFIX");
            }
          mf->AddDefinition("CMAKE_INSTALL_PREFIX", dir.c_str());

          cmCPackLogger(
            cmCPackLog::LOG_DEBUG,
            "- Using DESTDIR + CPACK_INSTALL_PREFIX... (mf->AddDefinition)"
            << std::endl);
          cmCPackLogger(cmCPackLog::LOG_DEBUG,
                        "- Setting CMAKE_INSTALL_PREFIX to '" << dir << "'" 
                        << std::endl);

          // Make sure that DESTDIR + CPACK_INSTALL_PREFIX directory
          // exists:
          //
          if (cmSystemTools::StringStartsWith(dir.c_str(), "/"))
            {
            dir = tempInstallDirectory + dir;
            }
          else
            {
            dir = tempInstallDirectory + "/" + dir;
            }

          cmCPackLogger(cmCPackLog::LOG_DEBUG,
                        "- Creating directory: '" << dir << "'" << std::endl);

          if ( !cmsys::SystemTools::MakeDirectory(dir.c_str()))
            {
            cmCPackLogger(cmCPackLog::LOG_ERROR,
                          "Problem creating temporary directory: " 
                          << dir << std::endl);
            return 0;
            }
          }
        else
          {
          mf->AddDefinition("CMAKE_INSTALL_PREFIX",
                            tempInstallDirectory.c_str());

          if ( !cmsys::SystemTools::MakeDirectory(
                 tempInstallDirectory.c_str()))
            {
            cmCPackLogger(cmCPackLog::LOG_ERROR,
                          "Problem creating temporary directory: " 
                          << tempInstallDirectory << std::endl);
            return 0;
            }

          cmCPackLogger(cmCPackLog::LOG_DEBUG,
                        "- Using non-DESTDIR install... (mf->AddDefinition)"
                        << std::endl);
          cmCPackLogger(cmCPackLog::LOG_DEBUG,
                        "- Setting CMAKE_INSTALL_PREFIX to '"
                        << tempInstallDirectory
                        << "'" << std::endl);
          }

        if ( buildConfig && *buildConfig )
          {
          mf->AddDefinition("BUILD_TYPE", buildConfig);
          }
        std::string installComponentLowerCase
          = cmSystemTools::LowerCase(installComponent);
        if ( installComponentLowerCase != "all" )
          {
          mf->AddDefinition("CMAKE_INSTALL_COMPONENT",
                            installComponent.c_str());
          }

        // strip on TRUE, ON, 1, one or several file names, but not on 
        // FALSE, OFF, 0 and an empty string
        if (!cmSystemTools::IsOff(this->GetOption("CPACK_STRIP_FILES")))
          {
          mf->AddDefinition("CMAKE_INSTALL_DO_STRIP", "1");
          }
        int res = mf->ReadListFile(0, installFile.c_str());
        if ( cmSystemTools::GetErrorOccuredFlag() || !res )
          {
          return 0;
          }
        }
      }
    }
  return 1;
}

//----------------------------------------------------------------------
bool cmCPackGenerator::ReadListFile(const char* moduleName)
{
  std::string fullPath = this->MakefileMap->GetModulesFile(moduleName);
  return this->MakefileMap->ReadListFile(0, fullPath.c_str());
}

//----------------------------------------------------------------------
void cmCPackGenerator::SetOptionIfNotSet(const char* op,
  const char* value)
{
  const char* def = this->MakefileMap->GetDefinition(op);
  if ( def && *def )
    {
    return;
    }
  this->SetOption(op, value);
}

//----------------------------------------------------------------------
void cmCPackGenerator::SetOption(const char* op, const char* value)
{
  if ( !op )
    {
    return;
    }
  if ( !value )
    {
    this->MakefileMap->RemoveDefinition(op);
    return;
    }
  cmCPackLogger(cmCPackLog::LOG_DEBUG, this->GetNameOfClass()
    << "::SetOption(" << op << ", " << value << ")" << std::endl);
  this->MakefileMap->AddDefinition(op, value);
}

//----------------------------------------------------------------------
int cmCPackGenerator::DoPackage()
{
  cmCPackLogger(cmCPackLog::LOG_OUTPUT,
    "Create package using " << this->Name.c_str() << std::endl);

  if ( !this->PrepareNames() )
    {
    return 0;
    }

  if ( cmSystemTools::IsOn(
      this->GetOption("CPACK_REMOVE_TOPLEVEL_DIRECTORY")) )
    {
    const char* toplevelDirectory
      = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    if ( cmSystemTools::FileExists(toplevelDirectory) )
      {
      cmCPackLogger(cmCPackLog::LOG_VERBOSE, 
                    "Remove toplevel directory: "
        << toplevelDirectory << std::endl);
      if ( !cmSystemTools::RepeatedRemoveDirectory(toplevelDirectory) )
        {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
          "Problem removing toplevel directory: "
          << toplevelDirectory
          << std::endl);
        return 0;
        }
      }
    }
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
                "About to install project " << std::endl);

  if ( !this->InstallProject() )
    {
    return 0;
    }
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
                "Done install project " << std::endl);


  const char* tempPackageFileName = this->GetOption(
    "CPACK_TEMPORARY_PACKAGE_FILE_NAME");
  const char* packageFileName = this->GetOption("CPACK_OUTPUT_FILE_PATH");
  const char* tempDirectory = this->GetOption("CPACK_TEMPORARY_DIRECTORY");

  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Find files" << std::endl);
  cmsys::Glob gl;
  std::string findExpr = tempDirectory;
  findExpr += "/*";
  gl.RecurseOn();
  if ( !gl.FindFiles(findExpr) )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Cannot find any files in the packaging tree" << std::endl);
    return 0;
    }

  cmCPackLogger(cmCPackLog::LOG_OUTPUT, "Compress package" << std::endl);
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Compress files to: "
    << (tempPackageFileName ? tempPackageFileName : "(NULL)") << std::endl);
  if ( cmSystemTools::FileExists(tempPackageFileName) )
    {
    cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Remove old package file"
      << std::endl);
    cmSystemTools::RemoveFile(tempPackageFileName);
    }
  if ( cmSystemTools::IsOn(this->GetOption(
        "CPACK_INCLUDE_TOPLEVEL_DIRECTORY")) )
    {
    tempDirectory = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
    }

  // The files to be installed
  std::vector<std::string> files = gl.GetFiles();

  // For component installations, determine which files go into which
  // components.
  if (!this->Components.empty())
    {
    std::vector<std::string>::const_iterator it;
    for ( it = files.begin(); it != files.end(); ++ it )
      {
      std::string fileN = cmSystemTools::RelativePath(tempDirectory,
                                                      it->c_str());

      // Determine which component we are in.
      std::string componentName = fileN.substr(0, fileN.find('/'));

      // Strip off the component part of the path.
      fileN = fileN.substr(fileN.find('/')+1, std::string::npos);

      // Add this file to the list of files for the component.
      this->Components[componentName].Files.push_back(fileN);
      }
    }

  if ( !this->CompressFiles(tempPackageFileName,
      tempDirectory, files) || cmSystemTools::GetErrorOccuredFlag())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem compressing the directory"
      << std::endl);
    return 0;
    }

  cmCPackLogger(cmCPackLog::LOG_OUTPUT, "Finalize package" << std::endl);
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Copy final package: "
                << (tempPackageFileName ? tempPackageFileName : "(NULL)" )
                << " to " 
                << (packageFileName ? packageFileName : "(NULL)")
                << std::endl);
  if ( !cmSystemTools::CopyFileIfDifferent(tempPackageFileName,
      packageFileName) )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem copying the package: "
                  << (tempPackageFileName ? tempPackageFileName : "(NULL)" )
                  << " to " 
                  << (packageFileName ? packageFileName : "(NULL)")
                  << std::endl);
    return 0;
    }

  cmCPackLogger(cmCPackLog::LOG_OUTPUT, "Package " 
                << (packageFileName ? packageFileName : "(NULL)")
                << " generated." << std::endl);
  return 1;
}

//----------------------------------------------------------------------
int cmCPackGenerator::Initialize(const char* name, cmMakefile* mf)
{
  this->MakefileMap = mf;
  this->Name = name;
  if ( !this->SetCMakeRoot() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Cannot initialize the generator" << std::endl);
    return 0;
    }
  // set the running generator name
  this->SetOption("CPACK_GENERATOR", this->Name.c_str());
  // Load the project specific config file
  const char* config = 
    this->GetOption("CPACK_PROJECT_CONFIG_FILE");
  if(config)
    { 
    mf->ReadListFile(config);
    }
  int result = this->InitializeInternal();
  if (cmSystemTools::GetErrorOccuredFlag())
    {
    return 0;
    }

  // If a generator subclass did not already set this option in its
  // InitializeInternal implementation, and the project did not already set
  // it, the default value should be:
  this->SetOptionIfNotSet("CPACK_PACKAGING_INSTALL_PREFIX", "/");

  return result;
}

//----------------------------------------------------------------------
int cmCPackGenerator::InitializeInternal()
{
  return 1;
}

//----------------------------------------------------------------------
bool cmCPackGenerator::IsSet(const char* name) const
{
  return this->MakefileMap->IsSet(name);
}

//----------------------------------------------------------------------
const char* cmCPackGenerator::GetOption(const char* op)
{ 
  const char* ret = this->MakefileMap->GetDefinition(op);
  if(!ret)
    { 
    cmCPackLogger(cmCPackLog::LOG_DEBUG, 
                  "Warning, GetOption return NULL for: "
                  << op 
                  << std::endl);
    }
  return ret;
}

//----------------------------------------------------------------------
int cmCPackGenerator::SetCMakeRoot()
{
  // use the CMAKE_ROOT from cmake which should have been
  // found by now
  const char* root=
    this->MakefileMap->GetDefinition("CMAKE_ROOT");

  if(root)
    {
      this->CMakeRoot = root;
      cmCPackLogger(cmCPackLog::LOG_DEBUG, "Looking for CMAKE_ROOT: "
                    << this->CMakeRoot.c_str() << std::endl);
      this->SetOption("CMAKE_ROOT", this->CMakeRoot.c_str());
      return 1;
    }
  cmCPackLogger(cmCPackLog::LOG_ERROR,
                "Could not find CMAKE_ROOT !!!" 
                << std::endl
                << "CMake has most likely not been installed correctly."
                << std::endl
                <<"Modules directory not found in"
                << std::endl);
  return 0;
}

//----------------------------------------------------------------------
int cmCPackGenerator::CompressFiles(const char* outFileName,
  const char* toplevel, const std::vector<std::string>& files)
{
  (void)outFileName;
  (void)toplevel;
  (void)files;
  return 0;
}

//----------------------------------------------------------------------
const char* cmCPackGenerator::GetInstallPath()
{
  if ( !this->InstallPath.empty() )
    {
    return this->InstallPath.c_str();
    }
#if defined(_WIN32) && !defined(__CYGWIN__)
  const char* prgfiles = cmsys::SystemTools::GetEnv("ProgramFiles");
  const char* sysDrive = cmsys::SystemTools::GetEnv("SystemDrive");
  if ( prgfiles )
    {
    this->InstallPath = prgfiles;
    }
  else if ( sysDrive )
    {
    this->InstallPath = sysDrive;
    this->InstallPath += "/Program Files";
    }
  else
    {
    this->InstallPath = "c:/Program Files";
    }
  this->InstallPath += "/";
  this->InstallPath += this->GetOption("CPACK_PACKAGE_NAME");
  this->InstallPath += "-";
  this->InstallPath += this->GetOption("CPACK_PACKAGE_VERSION");
#elif defined(__HAIKU__)
  BPath dir;
  if (find_directory(B_COMMON_DIRECTORY, &dir) == B_OK)
    {
    this->InstallPath = dir.Path();
    }
  else
    {
    this->InstallPath = "/boot/common";
    }
#else
  this->InstallPath = "/usr/local/";
#endif
  return this->InstallPath.c_str();
}

//----------------------------------------------------------------------
const char* cmCPackGenerator::GetPackagingInstallPrefix()
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "GetPackagingInstallPrefix: '"
    << this->GetOption("CPACK_PACKAGING_INSTALL_PREFIX") << "'" << std::endl);

  return this->GetOption("CPACK_PACKAGING_INSTALL_PREFIX");
}

//----------------------------------------------------------------------
std::string cmCPackGenerator::FindTemplate(const char* name)
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Look for template: "
    << (name ? name : "(NULL)") << std::endl);
  std::string ffile = this->MakefileMap->GetModulesFile(name);
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Found template: "
    << ffile.c_str() << std::endl);
  return ffile;
}

//----------------------------------------------------------------------
bool cmCPackGenerator::ConfigureString(const std::string& inString,
  std::string& outString)
{
  this->MakefileMap->ConfigureString(inString,
    outString, true, false);
  return true;
}

//----------------------------------------------------------------------
bool cmCPackGenerator::ConfigureFile(const char* inName,
  const char* outName, bool copyOnly /* = false */)
{
  return this->MakefileMap->ConfigureFile(inName, outName,
    copyOnly, true, false) == 1;
}

//----------------------------------------------------------------------
int cmCPackGenerator::CleanTemporaryDirectory()
{
  std::string tempInstallDirectoryWithPostfix
    = this->GetOption("CPACK_TEMPORARY_INSTALL_DIRECTORY");
  const char* tempInstallDirectory = tempInstallDirectoryWithPostfix.c_str();
  if(cmsys::SystemTools::FileExists(tempInstallDirectory))
    {
    cmCPackLogger(cmCPackLog::LOG_OUTPUT,
                  "- Clean temporary : "
                  << tempInstallDirectory << std::endl);
    if(!cmSystemTools::RepeatedRemoveDirectory(tempInstallDirectory))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Problem removing temporary directory: " <<
                    tempInstallDirectory
                    << std::endl);
      return 0;
      }
    }
  return 1;
}

//----------------------------------------------------------------------
bool cmCPackGenerator::SupportsComponentInstallation() const
{
  return false;
}

//----------------------------------------------------------------------
cmCPackInstallationType*
cmCPackGenerator::GetInstallationType(const char *projectName,
                                      const char *name)
{
  (void) projectName;
  bool hasInstallationType = this->InstallationTypes.count(name) != 0;
  cmCPackInstallationType *installType = &this->InstallationTypes[name];
  if (!hasInstallationType) 
    {
    // Define the installation type
    std::string macroPrefix = "CPACK_INSTALL_TYPE_" 
      + cmsys::SystemTools::UpperCase(name);
    installType->Name = name;

    const char* displayName 
      = this->GetOption((macroPrefix + "_DISPLAY_NAME").c_str());
    if (displayName && *displayName)
      {
      installType->DisplayName = displayName;
      }
    else
      {
      installType->DisplayName = installType->Name;
      }

    installType->Index = static_cast<unsigned>(
      this->InstallationTypes.size());
    }
  return installType;
}

//----------------------------------------------------------------------
cmCPackComponent*
cmCPackGenerator::GetComponent(const char *projectName, const char *name)
{
  bool hasComponent = this->Components.count(name) != 0;
  cmCPackComponent *component = &this->Components[name];
  if (!hasComponent) 
    {
    // Define the component
    std::string macroPrefix = "CPACK_COMPONENT_" 
      + cmsys::SystemTools::UpperCase(name);
    component->Name = name;
    const char* displayName 
      = this->GetOption((macroPrefix + "_DISPLAY_NAME").c_str());
    if (displayName && *displayName)
      {
      component->DisplayName = displayName;
      }
    else
      {
      component->DisplayName = component->Name;
      }
    component->IsHidden 
      = this->IsSet((macroPrefix + "_HIDDEN").c_str());
    component->IsRequired
      = this->IsSet((macroPrefix + "_REQUIRED").c_str());
    component->IsDisabledByDefault
      = this->IsSet((macroPrefix + "_DISABLED").c_str());
    component->IsDownloaded
      = this->IsSet((macroPrefix + "_DOWNLOADED").c_str())
        || cmSystemTools::IsOn(this->GetOption("CPACK_DOWNLOAD_ALL"));

    const char* archiveFile = this->GetOption((macroPrefix + 
                                               "_ARCHIVE_FILE").c_str());
    if (archiveFile && *archiveFile)
      {
      component->ArchiveFile = archiveFile;
      }

    const char* groupName = this->GetOption((macroPrefix + "_GROUP").c_str());
    if (groupName && *groupName) 
      {
      component->Group = GetComponentGroup(projectName, groupName);
      component->Group->Components.push_back(component);
      }
    else
      {
      component->Group = 0;
      }

    const char* description
      = this->GetOption((macroPrefix + "_DESCRIPTION").c_str());
    if (description && *description)
      {
      component->Description = description;
      }

    // Determine the installation types.
    const char *installTypes 
      = this->GetOption((macroPrefix + "_INSTALL_TYPES").c_str());
    if (installTypes && *installTypes) 
      {
      std::vector<std::string> installTypesVector;
      cmSystemTools::ExpandListArgument(installTypes, installTypesVector);
      std::vector<std::string>::iterator installTypesIt;
      for (installTypesIt = installTypesVector.begin(); 
           installTypesIt != installTypesVector.end(); 
           ++installTypesIt)
        {
        component->InstallationTypes.push_back(
          this->GetInstallationType(projectName, installTypesIt->c_str()));
        }
      }

    // Determine the component dependencies.
    const char *depends = this->GetOption((macroPrefix + "_DEPENDS").c_str());
    if (depends && *depends) 
      {
      std::vector<std::string> dependsVector;
      cmSystemTools::ExpandListArgument(depends, dependsVector);
      std::vector<std::string>::iterator dependIt;
      for (dependIt = dependsVector.begin(); 
           dependIt != dependsVector.end(); 
           ++dependIt)
        {
        cmCPackComponent *child = GetComponent(projectName, 
                                               dependIt->c_str());
        component->Dependencies.push_back(child);
        child->ReverseDependencies.push_back(component);
        }
      }
    }
  return component;
}

//----------------------------------------------------------------------
cmCPackComponentGroup*
cmCPackGenerator::GetComponentGroup(const char *projectName, const char *name)
{
  (void) projectName;
  std::string macroPrefix = "CPACK_COMPONENT_GROUP_" 
        + cmsys::SystemTools::UpperCase(name);
  bool hasGroup = this->ComponentGroups.count(name) != 0;
  cmCPackComponentGroup *group = &this->ComponentGroups[name];
  if (!hasGroup) 
    {
    // Define the group
    group->Name = name;
    const char* displayName 
      = this->GetOption((macroPrefix + "_DISPLAY_NAME").c_str());
    if (displayName && *displayName)
      {
      group->DisplayName = displayName;
      }
    else
      {
      group->DisplayName = group->Name;
      }

    const char* description
      = this->GetOption((macroPrefix + "_DESCRIPTION").c_str());
    if (description && *description)
      {
      group->Description = description;
      }
    group->IsBold
      = this->IsSet((macroPrefix + "_BOLD_TITLE").c_str());
    group->IsExpandedByDefault
      = this->IsSet((macroPrefix + "_EXPANDED").c_str());
    const char* parentGroupName 
      = this->GetOption((macroPrefix + "_PARENT_GROUP").c_str());
    if (parentGroupName && *parentGroupName)
      {
      group->ParentGroup = GetComponentGroup(projectName, parentGroupName);
      group->ParentGroup->Subgroups.push_back(group);
      }
    else
      {
      group->ParentGroup = 0;
      }
    }
  return group;
}
