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

#include "cmCPackGenericGenerator.h"

#include "cmMakefile.h"
#include "cmCPackLog.h"
#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmGeneratedFileStream.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/Glob.hxx>
#include <memory> // auto_ptr

//----------------------------------------------------------------------
cmCPackGenericGenerator::cmCPackGenericGenerator()
{
  this->GeneratorVerbose = false;
  this->MakefileMap = 0;
  this->Logger = 0;
}

//----------------------------------------------------------------------
cmCPackGenericGenerator::~cmCPackGenericGenerator()
{
  this->MakefileMap = 0;
}

//----------------------------------------------------------------------
int cmCPackGenericGenerator::PrepareNames()
{
  this->SetOption("CPACK_GENERATOR", this->Name.c_str());
  std::string tempDirectory = this->GetOption("CPACK_PACKAGE_DIRECTORY");
  tempDirectory += "/_CPack_Packages/";
  tempDirectory += this->GetOption("CPACK_GENERATOR");
  std::string topDirectory = tempDirectory;

  std::string outName = this->GetOption("CPACK_PACKAGE_FILE_NAME");
  tempDirectory += "/" + outName;
  outName += ".";
  outName += this->GetOutputExtension();

  std::string destFile = this->GetOption("CPACK_PACKAGE_DIRECTORY");
  destFile += "/" + outName;

  std::string outFile = topDirectory + "/" + outName;
  std::string installPrefix = tempDirectory + this->GetInstallPrefix();

  this->SetOption("CPACK_TOPLEVEL_DIRECTORY", topDirectory.c_str());
  this->SetOption("CPACK_TEMPORARY_DIRECTORY", tempDirectory.c_str());
  this->SetOption("CPACK_OUTPUT_FILE_NAME", outName.c_str());
  this->SetOption("CPACK_OUTPUT_FILE_PATH", destFile.c_str());
  this->SetOption("CPACK_TEMPORARY_PACKAGE_FILE_NAME", outFile.c_str());
  this->SetOption("CPACK_INSTALL_DIRECTORY", this->GetInstallPath());
  this->SetOption("CPACK_NATIVE_INSTALL_DIRECTORY",
    cmsys::SystemTools::ConvertToOutputPath(this->GetInstallPath()).c_str());
  this->SetOption("CPACK_TEMPORARY_INSTALL_DIRECTORY", installPrefix.c_str());

  cmCPackLogger(cmCPackLog::LOG_DEBUG,
    "Look for: CPACK_PACKAGE_DESCRIPTION_FILE" << std::endl);
  const char* descFileName
    = this->GetOption("CPACK_PACKAGE_DESCRIPTION_FILE");
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
    "Look for: " << descFileName << std::endl);
  if ( descFileName )
    {
    if ( !cmSystemTools::FileExists(descFileName) )
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Cannot find description file name: " << descFileName << std::endl);
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
      ostr << cmSystemTools::MakeXMLSafe(line.c_str()) << std::endl;
      }
    this->SetOption("CPACK_PACKAGE_DESCRIPTION", ostr.str().c_str());
    }
  if ( !this->GetOption("CPACK_PACKAGE_DESCRIPTION") )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Project description not specified. Please specify "
      "CPACK_PACKAGE_DESCRIPTION or CPACK_PACKAGE_DESCRIPTION_FILE."
      << std::endl);
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------
int cmCPackGenericGenerator::InstallProject()
{
  cmCPackLogger(cmCPackLog::LOG_OUTPUT, "Install project" << std::endl);
  const char* tempInstallDirectory
    = this->GetOption("CPACK_TEMPORARY_INSTALL_DIRECTORY");
  int res = 1;
  if ( !cmsys::SystemTools::MakeDirectory(tempInstallDirectory))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Problem creating temporary directory: " << tempInstallDirectory
      << std::endl);
    return 0;
    }
  bool movable = true;
  if ( movable )
    {
    // Make sure there is no destdir
    cmSystemTools::PutEnv("DESTDIR=");
    }
  else
    {
    std::string destDir = "DESTDIR=";
    destDir += tempInstallDirectory;
    cmSystemTools::PutEnv(destDir.c_str());
    }
  const char* installCommands = this->GetOption("CPACK_INSTALL_COMMANDS");
  if ( installCommands )
    {
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
        res = 0;
        break;
        }
      }
    }
  const char* installDirectories
    = this->GetOption("CPACK_INSTALLED_DIRECTORIES");
  if ( installDirectories )
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
    const char* tempDir = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
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
      gl.RecurseOn();
      if ( !gl.FindFiles(findExpr) )
        {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
          "Cannot find any files in the installed directory" << std::endl);
        return 0;
        }
      std::vector<std::string>& files = gl.GetFiles();
      std::vector<std::string>::iterator gfit;
      for ( gfit = files.begin(); gfit != files.end(); ++ gfit )
        {
        std::string filePath = tempDir;
        filePath += "/" + subdir + "/"
          + cmSystemTools::RelativePath(toplevel.c_str(), gfit->c_str());
        std::string &inFile = *gfit;
        cmCPackLogger(cmCPackLog::LOG_DEBUG, "Copy file: "
          << inFile.c_str() << " -> " << filePath.c_str() << std::endl);
        if ( !cmSystemTools::CopyFileIfDifferent(inFile.c_str(),
            filePath.c_str()) )
          {
          cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem copying file: "
            << inFile.c_str() << " -> " << filePath.c_str() << std::endl);
          }
        }
      }
    }
  const char* binaryDirectories = this->GetOption("CPACK_BINARY_DIR");
  if ( binaryDirectories )
    {
    std::vector<std::string> binaryDirectoriesVector;
    cmSystemTools::ExpandListArgument(binaryDirectories,
      binaryDirectoriesVector);
    std::vector<std::string>::iterator it;
    for ( it = binaryDirectoriesVector.begin();
      it != binaryDirectoriesVector.end();
      ++it )
      {
      std::string installFile = it->c_str();
      installFile += "/cmake_install.cmake";
      cmake cm;
      cmGlobalGenerator gg;
      gg.SetCMakeInstance(&cm);
      std::auto_ptr<cmLocalGenerator> lg(gg.CreateLocalGenerator());
      lg->SetGlobalGenerator(&gg);
      cmMakefile *mf = lg->GetMakefile();
      if ( movable )
        {
        mf->AddDefinition("CMAKE_INSTALL_PREFIX", tempInstallDirectory);
        }
      const char* buildConfig = this->GetOption("CPACK_BUILD_CONFIG");
      if ( buildConfig && *buildConfig )
        {
        mf->AddDefinition("BUILD_TYPE", buildConfig);
        }

      res = mf->ReadListFile(0, installFile.c_str());
      if ( cmSystemTools::GetErrorOccuredFlag() )
        {
        res = 0;
        }
      }
    }
  if ( !movable )
    {
    cmSystemTools::PutEnv("DESTDIR=");
    }
  return res;
}

//----------------------------------------------------------------------
void cmCPackGenericGenerator::SetOption(const char* op, const char* value)
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
int cmCPackGenericGenerator::ProcessGenerator()
{
  if ( !this->PrepareNames() )
    {
    return 0;
    }
  if ( !this->InstallProject() )
    {
    return 0;
    }

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
    << tempPackageFileName << std::endl);
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
  if ( !this->CompressFiles(tempPackageFileName,
      tempDirectory, gl.GetFiles()) )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem compressing the directory"
      << std::endl);
    return 0;
    }

  cmCPackLogger(cmCPackLog::LOG_OUTPUT, "Finalize package" << std::endl);
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Copy final package: "
    << tempPackageFileName << " to " << packageFileName << std::endl);
  if ( !cmSystemTools::CopyFileIfDifferent(tempPackageFileName,
      packageFileName) )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem copying the package: "
      << tempPackageFileName << " to " << packageFileName << std::endl);
    return 0;
    }

  cmCPackLogger(cmCPackLog::LOG_OUTPUT, "Package " << packageFileName
    << " generated." << std::endl);
  return 1;
}

//----------------------------------------------------------------------
int cmCPackGenericGenerator::Initialize(const char* name, cmMakefile* mf)
{
  this->MakefileMap = mf;
  this->Name = name;
  return 1;
}

//----------------------------------------------------------------------
const char* cmCPackGenericGenerator::GetOption(const char* op)
{
  return this->MakefileMap->GetDefinition(op);
}

//----------------------------------------------------------------------
int cmCPackGenericGenerator::FindRunningCMake(const char* arg0)
{
  int found = 0;
  // Find our own executable.
  std::vector<cmStdString> failures;
  this->CPackSelf = arg0;
  cmSystemTools::ConvertToUnixSlashes(this->CPackSelf);
  failures.push_back(this->CPackSelf);
  this->CPackSelf = cmSystemTools::FindProgram(this->CPackSelf.c_str());
  if(!cmSystemTools::FileExists(this->CPackSelf.c_str()))
    {
    failures.push_back(this->CPackSelf);
    this->CPackSelf =  "/usr/local/bin/ctest";
    }
  if(!cmSystemTools::FileExists(this->CPackSelf.c_str()))
    {
    failures.push_back(this->CPackSelf);
    cmOStringStream msg;
    msg << "CTEST can not find the command line program ctest.\n";
    msg << "  argv[0] = \"" << arg0 << "\"\n";
    msg << "  Attempted paths:\n";
    std::vector<cmStdString>::iterator i;
    for(i=failures.begin(); i != failures.end(); ++i)
      {
      msg << "    \"" << i->c_str() << "\"\n";
      }
    cmSystemTools::Error(msg.str().c_str());
    }
  std::string dir;
  std::string file;
  if(cmSystemTools::SplitProgramPath(this->CPackSelf.c_str(),
      dir, file, true))
    {
    this->CMakeSelf = dir += "/cmake";
    this->CMakeSelf += cmSystemTools::GetExecutableExtension();
    if(cmSystemTools::FileExists(this->CMakeSelf.c_str()))
      {
      found = 1;
      }
    }
  if ( !found )
    {
    failures.push_back(this->CMakeSelf);
#ifdef CMAKE_BUILD_DIR
    std::string intdir = ".";
#ifdef  CMAKE_INTDIR
    intdir = CMAKE_INTDIR;
#endif
    this->CMakeSelf = CMAKE_BUILD_DIR;
    this->CMakeSelf += "/bin/";
    this->CMakeSelf += intdir;
    this->CMakeSelf += "/cmake";
    this->CMakeSelf += cmSystemTools::GetExecutableExtension();
#endif
    if(!cmSystemTools::FileExists(this->CMakeSelf.c_str()))
      {
      failures.push_back(this->CMakeSelf);
      cmOStringStream msg;
      msg << "CTEST can not find the command line program cmake.\n";
      msg << "  argv[0] = \"" << arg0 << "\"\n";
      msg << "  Attempted paths:\n";
      std::vector<cmStdString>::iterator i;
      for(i=failures.begin(); i != failures.end(); ++i)
        {
        msg << "    \"" << i->c_str() << "\"\n";
        }
      cmSystemTools::Error(msg.str().c_str());
      }
    }
  // do CMAKE_ROOT, look for the environment variable first
  std::string cMakeRoot;
  std::string modules;
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Looking for CMAKE_ROOT" << std::endl);
  if (getenv("CMAKE_ROOT"))
    {
    cMakeRoot = getenv("CMAKE_ROOT");
    modules = cMakeRoot + "/Modules/CMake.cmake";
    }
  if(modules.empty() || !cmSystemTools::FileExists(modules.c_str()))
    {
    // next try exe/..
    cMakeRoot  = cmSystemTools::GetProgramPath(this->CMakeSelf.c_str());
    std::string::size_type slashPos = cMakeRoot.rfind("/");
    if(slashPos != std::string::npos)
      {
      cMakeRoot = cMakeRoot.substr(0, slashPos);
      }
    // is there no Modules direcory there?
    modules = cMakeRoot + "/Modules/CMake.cmake";
    cmCPackLogger(cmCPackLog::LOG_DEBUG, "Looking for CMAKE_ROOT: "
      << modules.c_str() << std::endl);
    }

  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try exe/../share/cmake
    cMakeRoot += CMAKE_DATA_DIR;
    modules = cMakeRoot + "/Modules/CMake.cmake";
    cmCPackLogger(cmCPackLog::LOG_DEBUG, "Looking for CMAKE_ROOT: "
      << modules.c_str() << std::endl);
    }
#ifdef CMAKE_ROOT_DIR
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try compiled in root directory
    cMakeRoot = CMAKE_ROOT_DIR;
    modules = cMakeRoot + "/Modules/CMake.cmake";
    cmCPackLogger(cmCPackLog::LOG_DEBUG, "Looking for CMAKE_ROOT: "
      << modules.c_str() << std::endl);
    }
#endif
#ifdef CMAKE_PREFIX
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try compiled in install prefix
    cMakeRoot = CMAKE_PREFIX CMAKE_DATA_DIR;
    modules = cMakeRoot + "/Modules/CMake.cmake";
    cmCPackLogger(cmCPackLog::LOG_DEBUG, "Looking for CMAKE_ROOT: "
      << modules.c_str() << std::endl);
    }
#endif
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try
    cMakeRoot  = cmSystemTools::GetProgramPath(this->CMakeSelf.c_str());
    cMakeRoot += CMAKE_DATA_DIR;
    modules = cMakeRoot +  "/Modules/CMake.cmake";
    cmCPackLogger(cmCPackLog::LOG_DEBUG, "Looking for CMAKE_ROOT: "
      << modules.c_str() << std::endl);
    }
  if(!cmSystemTools::FileExists(modules.c_str()))
    {
    // next try exe
    cMakeRoot  = cmSystemTools::GetProgramPath(this->CMakeSelf.c_str());
    // is there no Modules direcory there?
    modules = cMakeRoot + "/Modules/CMake.cmake";
    cmCPackLogger(cmCPackLog::LOG_DEBUG, "Looking for CMAKE_ROOT: "
      << modules.c_str() << std::endl);
    }
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // couldn't find modules
    cmSystemTools::Error("Could not find CMAKE_ROOT !!!\n"
      "CMake has most likely not been installed correctly.\n"
      "Modules directory not found in\n",
      cMakeRoot.c_str());
    return 0;
    }
  this->CMakeRoot = cMakeRoot;
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Looking for CMAKE_ROOT: "
    << this->CMakeRoot.c_str() << std::endl);
  this->SetOption("CMAKE_ROOT", this->CMakeRoot.c_str());
  return 1;
}

//----------------------------------------------------------------------
int cmCPackGenericGenerator::CompressFiles(const char* outFileName,
  const char* toplevel, const std::vector<std::string>& files)
{
  (void)outFileName;
  (void)toplevel;
  (void)files;
  return 0;
}

//----------------------------------------------------------------------
const char* cmCPackGenericGenerator::GetInstallPath()
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
#else
  this->InstallPath = "/usr/local/";
#endif
  return this->InstallPath.c_str();
}

//----------------------------------------------------------------------
std::string cmCPackGenericGenerator::FindTemplate(const char* name)
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Look for template: "
    << name << std::endl);
  std::string ffile = this->MakefileMap->GetModulesFile(name);
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Found template: "
    << ffile.c_str() << std::endl);
  return ffile;
}

//----------------------------------------------------------------------
bool cmCPackGenericGenerator::ConfigureFile(const char* inName,
  const char* outName)
{
  return this->MakefileMap->ConfigureFile(inName, outName,
    false, true, false) == 1;
}
