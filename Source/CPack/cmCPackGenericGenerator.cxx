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

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/Glob.hxx>
#include <memory> // auto_ptr

//----------------------------------------------------------------------
cmCPackGenericGenerator::cmCPackGenericGenerator()
{
  m_GeneratorVerbose = false;
  m_GlobalGenerator = 0;
  m_LocalGenerator = 0;
  m_MakefileMap = 0;
  m_CMakeInstance = 0;
}

//----------------------------------------------------------------------
cmCPackGenericGenerator::~cmCPackGenericGenerator()
{
  if ( m_GlobalGenerator )
    {
    delete m_GlobalGenerator;
    m_GlobalGenerator = 0;
    }
  if ( m_LocalGenerator )
    {
    delete m_LocalGenerator;
    m_LocalGenerator = 0;
    }
  if ( m_CMakeInstance )
    {
    delete m_CMakeInstance;
    m_CMakeInstance = 0;
    }
  m_MakefileMap = 0;
}

//----------------------------------------------------------------------
int cmCPackGenericGenerator::PrepareNames()
{
  std::string tempDirectory = this->GetOption("CPACK_PROJECT_DIRECTORY");
  tempDirectory += "/_CPack_Packages/";
  tempDirectory += this->GetOption("CPACK_GENERATOR");
  std::string topDirectory = tempDirectory;

  std::string outName = this->GetOption("CPACK_PROJECT_NAME");
  outName += "-";
  outName += this->GetOption("CPACK_PROJECT_VERSION");
  const char* patch = this->GetOption("CPACK_PROJECT_VERSION_PATCH");
  if ( patch && *patch )
    {
    outName += "-";
    outName += patch;
    }
  const char* postfix = this->GetOutputPostfix();
  if ( postfix && *postfix )
    {
    outName += "-";
    outName += postfix;
    }
  tempDirectory += "/" + outName;

  outName += ".";
  outName += this->GetOutputExtension();


  std::string installFile = this->GetOption("CPACK_PROJECT_DIRECTORY");
  installFile += "/cmake_install.cmake";

  std::string destFile = this->GetOption("CPACK_PROJECT_DIRECTORY");
  destFile += "/" + outName;

  std::string outFile = topDirectory + "/" + outName;
  std::string installPrefix = tempDirectory + this->GetInstallPrefix();

  this->SetOption("CPACK_TOPLEVEL_DIRECTORY", topDirectory.c_str());
  this->SetOption("CPACK_TEMPORARY_DIRECTORY", tempDirectory.c_str());
  this->SetOption("CPACK_INSTALL_FILE_NAME", installFile.c_str());
  this->SetOption("CPACK_OUTPUT_FILE_NAME", outName.c_str());
  this->SetOption("CPACK_PACKAGE_FILE_NAME", destFile.c_str());
  this->SetOption("CPACK_TEMPORARY_PACKAGE_FILE_NAME", outFile.c_str());
  this->SetOption("CPACK_INSTALL_DIRECTORY", this->GetInstallPath());
  this->SetOption("CPACK_NATIVE_INSTALL_DIRECTORY",
    cmsys::SystemTools::ConvertToOutputPath(this->GetInstallPath()).c_str());
  this->SetOption("CPACK_TEMPORARY_INSTALL_DIRECTORY", installPrefix.c_str());

  std::cout << "Look for: CPACK_PROJECT_DESCRIPTION_FILE_NAME" << std::endl;
  const char* descFileName = this->GetOption("CPACK_PROJECT_DESCRIPTION_FILE_NAME");
  std::cout << "Look for: " << descFileName << std::endl;
  if ( descFileName )
    {
    if ( !cmSystemTools::FileExists(descFileName) )
      {
      std::cout << "Cannot find description file name: " << descFileName << std::endl;
      return 0;
      }
    std::ifstream ifs(descFileName);
    if ( !ifs )
      {
      std::cout << "Cannot open description file name: " << descFileName << std::endl;
      return 0;
      }
    cmOStringStream ostr;
    std::string line;
    while ( ifs && cmSystemTools::GetLineFromStream(ifs, line) )
      {
      ostr << cmSystemTools::MakeXMLSafe(line.c_str()) << std::endl;
      }
    this->SetOption("CPACK_PROJECT_DESCRIPTION", ostr.str().c_str());
    }
  if ( !this->GetOption("CPACK_PROJECT_DESCRIPTION") )
    {
    std::cout << "Project description not specified. Please specify CPACK_PROJECT_DESCRIPTION or CPACK_PROJECT_DESCRIPTION_FILE_NAME." << std::endl;
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------
int cmCPackGenericGenerator::InstallProject()
{
  std::cout << "Install project" << std::endl;
  const char* tempInstallDirectory = this->GetOption("CPACK_TEMPORARY_INSTALL_DIRECTORY");
  const char* installFile = this->GetOption("CPACK_INSTALL_FILE_NAME");
  if ( !cmsys::SystemTools::MakeDirectory(tempInstallDirectory))
    {
    std::cerr << "Problem creating temporary directory: " << tempInstallDirectory << std::endl;
    return 0;
    }
  cmake cm;
  cmGlobalGenerator gg;
  gg.SetCMakeInstance(&cm);
  std::auto_ptr<cmLocalGenerator> lg(gg.CreateLocalGenerator());
  lg->SetGlobalGenerator(&gg);
  cmMakefile *mf = lg->GetMakefile();
  bool movable = true;
  if ( movable )
    {
    mf->AddDefinition("CMAKE_INSTALL_PREFIX", tempInstallDirectory);
    }
  const char* buildConfig = this->GetOption("CPACK_BUILD_CONFIG");
  if ( buildConfig && *buildConfig )
    {
    mf->AddDefinition("BUILD_TYPE", buildConfig);
    }

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
  int res = mf->ReadListFile(0, installFile);
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
    m_MakefileMap->RemoveDefinition(op);
    return;
    }
  std::cout << this->GetNameOfClass() << "::SetOption(" << op << ", " << value << ")" << std::endl;
  m_MakefileMap->AddDefinition(op, value);
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
  const char* packageFileName = this->GetOption("CPACK_PACKAGE_FILE_NAME");
  const char* tempDirectory = this->GetOption("CPACK_TEMPORARY_DIRECTORY");


  std::cout << "Find files" << std::endl;
  cmsys::Glob gl;
  std::string findExpr = tempDirectory;
  findExpr += "/*";
  gl.RecurseOn();
  if ( !gl.FindFiles(findExpr) )
    {
    std::cerr << "CPack error: cannot find any files in the packaging tree" << std::endl;
    return 0;
    }

  std::cout << "Compress files to: " << tempPackageFileName << std::endl;
  if ( !this->CompressFiles(tempPackageFileName,
      tempDirectory, gl.GetFiles()) )
    {
    std::cerr << "CPack error: problem compressing the directory" << std::endl;
    return 0;
    }

  std::cout << "Finalize package" << std::endl;
  std::cout << "Copy final package: " << tempPackageFileName << " to " << packageFileName << std::endl;
  if ( !cmSystemTools::CopyFileIfDifferent(tempPackageFileName, packageFileName) )
    {
    std::cerr << "CPack error: problem copying the package: " << tempPackageFileName << " to " << packageFileName << std::endl;
    return 0;
    }

  std::cout << "All done" << std::endl;
  return 1;
}

//----------------------------------------------------------------------
int cmCPackGenericGenerator::Initialize(const char* name)
{
  m_CMakeInstance = new cmake;
  m_CMakeInstance->AddCMakePaths(m_CMakeRoot.c_str());
  m_GlobalGenerator = new cmGlobalGenerator;
  m_GlobalGenerator->SetCMakeInstance(m_CMakeInstance);
  m_LocalGenerator = m_GlobalGenerator->CreateLocalGenerator();
  m_MakefileMap = m_LocalGenerator->GetMakefile();
  m_Name = name;
  this->SetOption("CPACK_GENERATOR", name);
  return 1;
}

//----------------------------------------------------------------------
const char* cmCPackGenericGenerator::GetOption(const char* op)
{
  return m_MakefileMap->GetDefinition(op);
}

//----------------------------------------------------------------------
int cmCPackGenericGenerator::GenerateHeader(std::ostream* os)
{
  (void)os;
  return 1;
}

//----------------------------------------------------------------------
int cmCPackGenericGenerator::FindRunningCMake(const char* arg0)
{
  int found = 0;
  // Find our own executable.
  std::vector<cmStdString> failures;
  m_CPackSelf = arg0;
  cmSystemTools::ConvertToUnixSlashes(m_CPackSelf);
  failures.push_back(m_CPackSelf);
  m_CPackSelf = cmSystemTools::FindProgram(m_CPackSelf.c_str());
  if(!cmSystemTools::FileExists(m_CPackSelf.c_str()))
    {
    failures.push_back(m_CPackSelf);
    m_CPackSelf =  "/usr/local/bin/ctest";
    }
  if(!cmSystemTools::FileExists(m_CPackSelf.c_str()))
    {
    failures.push_back(m_CPackSelf);
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
  if(cmSystemTools::SplitProgramPath(m_CPackSelf.c_str(),
                                     dir, file, true))
    {
    m_CMakeSelf = dir += "/cmake";
    m_CMakeSelf += cmSystemTools::GetExecutableExtension();
    if(cmSystemTools::FileExists(m_CMakeSelf.c_str()))
      {
      found = 1;
      }
    }
  if ( !found )
    {
    failures.push_back(m_CMakeSelf);
#ifdef CMAKE_BUILD_DIR
    std::string intdir = ".";
#ifdef  CMAKE_INTDIR
    intdir = CMAKE_INTDIR;
#endif
    m_CMakeSelf = CMAKE_BUILD_DIR;
    m_CMakeSelf += "/bin/";
    m_CMakeSelf += intdir;
    m_CMakeSelf += "/cmake";
    m_CMakeSelf += cmSystemTools::GetExecutableExtension();
#endif
    if(!cmSystemTools::FileExists(m_CMakeSelf.c_str()))
      {
      failures.push_back(m_CMakeSelf);
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
  if (getenv("CMAKE_ROOT"))
    {
    cMakeRoot = getenv("CMAKE_ROOT");
    modules = cMakeRoot + "/Modules/CMake.cmake";
    }
  if(!cmSystemTools::FileExists(modules.c_str()))
    {
    // next try exe/..
    cMakeRoot  = cmSystemTools::GetProgramPath(m_CMakeSelf.c_str());
    std::string::size_type slashPos = cMakeRoot.rfind("/");
    if(slashPos != std::string::npos)      
      {
      cMakeRoot = cMakeRoot.substr(0, slashPos);
      }
    // is there no Modules direcory there?
    modules = cMakeRoot + "/Modules/CMake.cmake"; 
    }
  
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try exe/../share/cmake
    cMakeRoot += CMAKE_DATA_DIR;
    modules = cMakeRoot + "/Modules/CMake.cmake";
    }
#ifdef CMAKE_ROOT_DIR
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try compiled in root directory
    cMakeRoot = CMAKE_ROOT_DIR;
    modules = cMakeRoot + "/Modules/CMake.cmake";
    }
#endif
#ifdef CMAKE_PREFIX
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try compiled in install prefix
    cMakeRoot = CMAKE_PREFIX CMAKE_DATA_DIR;
    modules = cMakeRoot + "/Modules/CMake.cmake";
    }
#endif
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try 
    cMakeRoot  = cmSystemTools::GetProgramPath(m_CMakeSelf.c_str());
    cMakeRoot += CMAKE_DATA_DIR;
    modules = cMakeRoot +  "/Modules/CMake.cmake";
    }
  if(!cmSystemTools::FileExists(modules.c_str()))
    {
    // next try exe
    cMakeRoot  = cmSystemTools::GetProgramPath(m_CMakeSelf.c_str());
    // is there no Modules direcory there?
    modules = cMakeRoot + "/Modules/CMake.cmake"; 
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
  m_CMakeRoot = cMakeRoot;
  return 1;
}

//----------------------------------------------------------------------
int cmCPackGenericGenerator::CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files)
{
  (void)outFileName;
  (void)toplevel;
  (void)files;
  return 0;
}

//----------------------------------------------------------------------
const char* cmCPackGenericGenerator::GetInstallPath()
{
  if ( !m_InstallPath.empty() )
    {
    return m_InstallPath.c_str();
    }
#if defined(_WIN32) && !defined(__CYGWIN__)
  const char* prgfiles = cmsys::SystemTools::GetEnv("ProgramFiles");
  const char* sysDrive = cmsys::SystemTools::GetEnv("SystemDrive");
  if ( prgfiles )
    {
    m_InstallPath = prgfiles;
    }
  else if ( sysDrive )
    {
    m_InstallPath = sysDrive;
    m_InstallPath += "/Program Files";
    }
  else 
    {
    m_InstallPath = "c:/Program Files";
    }
  m_InstallPath += "/";
  m_InstallPath += this->GetOption("CPACK_PROJECT_NAME");
  m_InstallPath += "-";
  m_InstallPath += this->GetOption("CPACK_PROJECT_VERSION");
#else
  m_InstallPath = "/usr/local/";
#endif
  return m_InstallPath.c_str();
}

//----------------------------------------------------------------------
std::string cmCPackGenericGenerator::FindTemplate(const char* name)
{
  return m_MakefileMap->GetModulesFile(name);
}

//----------------------------------------------------------------------
bool cmCPackGenericGenerator::ConfigureFile(const char* inName, const char* outName)
{
  return m_MakefileMap->ConfigureFile(inName, outName, false, true, false);
}

