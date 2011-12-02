/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmSystemTools.h"

// Need these for documentation support.
#include "cmake.h"
#include "cmDocumentation.h"
#include "cmCPackGeneratorFactory.h"
#include "cmCPackGenerator.h"
#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"

#include "cmCPackLog.h"

#include <cmsys/CommandLineArguments.hxx>
#include <memory> // auto_ptr

//----------------------------------------------------------------------------
static const char * cmDocumentationName[][3] =
{
  {0,
   "  cpack - Packaging driver provided by CMake.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationUsage[][3] =
{
  {0,
   "  cpack -G <generator> [options]",
   0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationDescription[][3] =
{
  {0,
   "The \"cpack\" executable is the CMake packaging program.  "
   "CMake-generated build trees created for projects that use "
   "the INSTALL_* commands have packaging support.  "
   "This program will generate the package.", 0},
  CMAKE_STANDARD_INTRODUCTION,
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationOptions[][3] =
{
    {"-G <generator>", "Use the specified generator to generate package.",
    "CPack may support multiple native packaging systems on certain "
      "platforms. A generator is responsible for generating input files for "
      "particular system and invoking that systems. Possible generator names "
      "are specified in the Generators section." },
    {"-C <Configuration>", "Specify the project configuration",
    "This option specifies the configuration that the project was build "
      "with, for example 'Debug', 'Release'." },
    {"-D <var>=<value>", "Set a CPack variable.", \
    "Set a variable that can be used by the generator."}, \
    {"--config <config file>", "Specify the config file.",
    "Specify the config file to use to create the package. By default "
      "CPackConfig.cmake in the current directory will be used." },
    {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationSeeAlso[][3] =
{
    {0, "cmake", 0},
    {0, "ccmake", 0},
    {0, 0, 0}
};

//----------------------------------------------------------------------------
int cpackUnknownArgument(const char*, void*)
{
  return 1;
}

//----------------------------------------------------------------------------
struct cpackDefinitions
{
  typedef std::map<cmStdString, cmStdString> MapType;
  MapType Map;
  cmCPackLog *Log;
};

//----------------------------------------------------------------------------
int cpackDefinitionArgument(const char* argument, const char* cValue,
  void* call_data)
{
  (void)argument;
  cpackDefinitions* def = static_cast<cpackDefinitions*>(call_data);
  std::string value = cValue;
  size_t pos = value.find_first_of("=");
  if ( pos == std::string::npos )
    {
    cmCPack_Log(def->Log, cmCPackLog::LOG_ERROR,
      "Please specify CPack definitions as: KEY=VALUE" << std::endl);
    return 0;
    }
  std::string key = value.substr(0, pos);
  value = value.c_str() + pos + 1;
  def->Map[key] = value;
  cmCPack_Log(def->Log, cmCPackLog::LOG_DEBUG, "Set CPack variable: "
    << key.c_str() << " to \"" << value.c_str() << "\"" << std::endl);
  return 1;
}

//----------------------------------------------------------------------------
// this is CPack.
int main (int argc, char *argv[])
{
  cmSystemTools::FindExecutableDirectory(argv[0]);
  cmCPackLog log;
  log.SetErrorPrefix("CPack Error: ");
  log.SetWarningPrefix("CPack Warning: ");
  log.SetOutputPrefix("CPack: ");
  log.SetVerbosePrefix("CPack Verbose: ");

  cmSystemTools::EnableMSVCDebugHook();

  if ( cmSystemTools::GetCurrentWorkingDirectory().size() == 0 )
    {
    cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
      "Current working directory cannot be established." << std::endl);
    }

  std::string generator;
  bool help = false;
  bool helpVersion = false;
  bool verbose = false;
  bool debug = false;
  std::string helpFull;
  std::string helpMAN;
  std::string helpHTML;

  std::string cpackProjectName;
  std::string cpackProjectDirectory
    = cmsys::SystemTools::GetCurrentWorkingDirectory();
  std::string cpackBuildConfig;
  std::string cpackProjectVersion;
  std::string cpackProjectPatch;
  std::string cpackProjectVendor;
  std::string cpackConfigFile;

  cpackDefinitions definitions;
  definitions.Log = &log;

  cpackConfigFile = "";

  cmDocumentation doc;
  cmsys::CommandLineArguments arg;
  arg.Initialize(argc, argv);
  typedef cmsys::CommandLineArguments argT;
  // Help arguments
  arg.AddArgument("--help", argT::NO_ARGUMENT, &help, "CPack help");
  arg.AddArgument("--help-full", argT::SPACE_ARGUMENT, &helpFull,
    "CPack help");
  arg.AddArgument("--help-html", argT::SPACE_ARGUMENT, &helpHTML,
    "CPack help");
  arg.AddArgument("--help-man", argT::SPACE_ARGUMENT, &helpMAN, "CPack help");
  arg.AddArgument("--version", argT::NO_ARGUMENT, &helpVersion, "CPack help");

  arg.AddArgument("-V", argT::NO_ARGUMENT, &verbose, "CPack verbose");
  arg.AddArgument("--verbose", argT::NO_ARGUMENT, &verbose, "-V");
  arg.AddArgument("--debug", argT::NO_ARGUMENT, &debug, "-V");
  arg.AddArgument("--config", argT::SPACE_ARGUMENT, &cpackConfigFile,
    "CPack configuration file");
  arg.AddArgument("-C", argT::SPACE_ARGUMENT, &cpackBuildConfig,
    "CPack build configuration");
  arg.AddArgument("-G", argT::SPACE_ARGUMENT,
    &generator, "CPack generator");
  arg.AddArgument("-P", argT::SPACE_ARGUMENT,
    &cpackProjectName, "CPack project name");
  arg.AddArgument("-R", argT::SPACE_ARGUMENT,
    &cpackProjectVersion, "CPack project version");
  arg.AddArgument("-B", argT::SPACE_ARGUMENT,
    &cpackProjectDirectory, "CPack project directory");
  arg.AddArgument("--patch", argT::SPACE_ARGUMENT,
    &cpackProjectPatch, "CPack project patch");
  arg.AddArgument("--vendor", argT::SPACE_ARGUMENT,
    &cpackProjectVendor, "CPack project vendor");
  arg.AddCallback("-D", argT::SPACE_ARGUMENT,
    cpackDefinitionArgument, &definitions, "CPack Definitions");
  arg.SetUnknownArgumentCallback(cpackUnknownArgument);

  // Parse command line
  int parsed = arg.Parse();

  // Setup logging
  if ( verbose )
    {
    log.SetVerbose(verbose);
    cmCPack_Log(&log, cmCPackLog::LOG_OUTPUT, "Enable Verbose" << std::endl);
    }
  if ( debug )
    {
    log.SetDebug(debug);
    cmCPack_Log(&log, cmCPackLog::LOG_OUTPUT, "Enable Debug" << std::endl);
    }

  cmCPack_Log(&log, cmCPackLog::LOG_VERBOSE,
    "Read CPack config file: " << cpackConfigFile.c_str() << std::endl);

  cmake cminst;
  cminst.RemoveUnscriptableCommands();
  cmGlobalGenerator cmgg;
  cmgg.SetCMakeInstance(&cminst);
  std::auto_ptr<cmLocalGenerator> cmlg(cmgg.CreateLocalGenerator());
  cmMakefile* globalMF = cmlg->GetMakefile();

  bool cpackConfigFileSpecified = true;
  if ( cpackConfigFile.empty() )
    {
    cpackConfigFile = cmSystemTools::GetCurrentWorkingDirectory();
    cpackConfigFile += "/CPackConfig.cmake";
    cpackConfigFileSpecified = false;
    }

  cmCPackGeneratorFactory generators;
  generators.SetLogger(&log);
  cmCPackGenerator* cpackGenerator = 0;

  if ( !helpFull.empty() || !helpMAN.empty() ||
    !helpHTML.empty() || helpVersion )
    {
    help = true;
    }

  if ( parsed && !help )
    {
    // find out which system cpack is running on, so it can setup the search
    // paths, so FIND_XXX() commands can be used in scripts
    cminst.AddCMakePaths();
    std::string systemFile =
      globalMF->GetModulesFile("CMakeDetermineSystem.cmake");
    if (!globalMF->ReadListFile(0, systemFile.c_str()))
      {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
        "Error reading CMakeDetermineSystem.cmake" << std::endl);
      return 1;
      }

    systemFile =
      globalMF->GetModulesFile("CMakeSystemSpecificInformation.cmake");
    if (!globalMF->ReadListFile(0, systemFile.c_str()))
      {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
        "Error reading CMakeSystemSpecificInformation.cmake" << std::endl);
      return 1;
      }

    if ( cmSystemTools::FileExists(cpackConfigFile.c_str()) )
      {
      cpackConfigFile =
        cmSystemTools::CollapseFullPath(cpackConfigFile.c_str());
      cmCPack_Log(&log, cmCPackLog::LOG_VERBOSE,
        "Read CPack configuration file: " << cpackConfigFile.c_str()
        << std::endl);
      if ( !globalMF->ReadListFile(0, cpackConfigFile.c_str()) )
        {
        cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
          "Problem reading CPack config file: \""
          << cpackConfigFile.c_str() << "\"" << std::endl);
        return 1;
        }
      }
    else if ( cpackConfigFileSpecified )
      {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
        "Cannot find CPack config file: \"" << cpackConfigFile.c_str()
        << "\"" << std::endl);
      return 1;
      }

    if ( !generator.empty() )
      {
      globalMF->AddDefinition("CPACK_GENERATOR", generator.c_str());
      }
    if ( !cpackProjectName.empty() )
      {
      globalMF->AddDefinition("CPACK_PACKAGE_NAME", cpackProjectName.c_str());
      }
    if ( !cpackProjectVersion.empty() )
      {
      globalMF->AddDefinition("CPACK_PACKAGE_VERSION",
        cpackProjectVersion.c_str());
      }
    if ( !cpackProjectVendor.empty() )
      {
      globalMF->AddDefinition("CPACK_PACKAGE_VENDOR",
        cpackProjectVendor.c_str());
      }
    if ( !cpackProjectDirectory.empty() )
      {
      globalMF->AddDefinition("CPACK_PACKAGE_DIRECTORY",
        cpackProjectDirectory.c_str());
      }
    if ( !cpackBuildConfig.empty() )
      {
      globalMF->AddDefinition("CPACK_BUILD_CONFIG", cpackBuildConfig.c_str());
      }
    cpackDefinitions::MapType::iterator cdit;
    for ( cdit = definitions.Map.begin();
      cdit != definitions.Map.end();
      ++cdit )
      {
      globalMF->AddDefinition(cdit->first.c_str(), cdit->second.c_str());
      }

    const char* cpackModulesPath =
      globalMF->GetDefinition("CPACK_MODULE_PATH");
    if ( cpackModulesPath )
      {
      globalMF->AddDefinition("CMAKE_MODULE_PATH", cpackModulesPath);
      }
    const char* genList = globalMF->GetDefinition("CPACK_GENERATOR");
    if ( !genList )
      {
      cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
        "CPack generator not specified" << std::endl);
      parsed = 0;
      }
    else
      {
      std::vector<std::string> generatorsVector;
      cmSystemTools::ExpandListArgument(genList,
        generatorsVector);
      std::vector<std::string>::iterator it;
      for ( it = generatorsVector.begin();
        it != generatorsVector.end();
        ++it )
        {
        const char* gen = it->c_str();
        cmMakefile newMF(*globalMF);
        cmMakefile* mf = &newMF;
        cmCPack_Log(&log, cmCPackLog::LOG_VERBOSE,
          "Specified generator: " << gen << std::endl);
        if ( parsed && !mf->GetDefinition("CPACK_PACKAGE_NAME") )
          {
          cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
            "CPack project name not specified" << std::endl);
          parsed = 0;
          }
        if (parsed &&
            !(mf->GetDefinition("CPACK_PACKAGE_VERSION") ||
              (mf->GetDefinition("CPACK_PACKAGE_VERSION_MAJOR") &&
               mf->GetDefinition("CPACK_PACKAGE_VERSION_MINOR") &&
               mf->GetDefinition("CPACK_PACKAGE_VERSION_PATCH"))))
          {
          cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
            "CPack project version not specified" << std::endl
            << "Specify CPACK_PACKAGE_VERSION, or "
            "CPACK_PACKAGE_VERSION_MAJOR, "
            "CPACK_PACKAGE_VERSION_MINOR, and CPACK_PACKAGE_VERSION_PATCH."
            << std::endl);
          parsed = 0;
          }
        if ( parsed )
          {
          cpackGenerator = generators.NewGenerator(gen);
          if ( !cpackGenerator )
            {
            cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
              "Cannot initialize CPack generator: "
              << gen << std::endl);
            parsed = 0;
            }
          if ( parsed && !cpackGenerator->Initialize(gen, mf) )
            {
            cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
              "Cannot initialize the generator " << gen << std::endl);
            parsed = 0;
            }

          if ( !mf->GetDefinition("CPACK_INSTALL_COMMANDS") &&
            !mf->GetDefinition("CPACK_INSTALLED_DIRECTORIES") &&
            !mf->GetDefinition("CPACK_INSTALL_CMAKE_PROJECTS") )
            {
            cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
              "Please specify build tree of the project that uses CMake "
              "using CPACK_INSTALL_CMAKE_PROJECTS, specify "
              "CPACK_INSTALL_COMMANDS, or specify "
              "CPACK_INSTALLED_DIRECTORIES."
              << std::endl);
            parsed = 0;
            }
          if ( parsed )
            {
#ifdef _WIN32
            std::string comspec = "cmw9xcom.exe";
            cmSystemTools::SetWindows9xComspecSubstitute(comspec.c_str());
#endif

            const char* projName = mf->GetDefinition("CPACK_PACKAGE_NAME");
            cmCPack_Log(&log, cmCPackLog::LOG_VERBOSE, "Use generator: "
              << cpackGenerator->GetNameOfClass() << std::endl);
            cmCPack_Log(&log, cmCPackLog::LOG_VERBOSE, "For project: "
              << projName << std::endl);

            const char* projVersion =
              mf->GetDefinition("CPACK_PACKAGE_VERSION");
            if ( !projVersion )
              {
              const char* projVersionMajor
                = mf->GetDefinition("CPACK_PACKAGE_VERSION_MAJOR");
              const char* projVersionMinor
                = mf->GetDefinition("CPACK_PACKAGE_VERSION_MINOR");
              const char* projVersionPatch
                = mf->GetDefinition("CPACK_PACKAGE_VERSION_PATCH");
              cmOStringStream ostr;
              ostr << projVersionMajor << "." << projVersionMinor << "."
                << projVersionPatch;
              mf->AddDefinition("CPACK_PACKAGE_VERSION",
                                ostr.str().c_str());
              }

            int res = cpackGenerator->DoPackage();
            if ( !res )
              {
              cmCPack_Log(&log, cmCPackLog::LOG_ERROR,
                "Error when generating package: " << projName << std::endl);
              return 1;
              }
            }
          }
        }
      }
    }

  if ( help )
    {
    doc.CheckOptions(argc, argv);
    // Construct and print requested documentation.
    doc.SetName("cpack");
    doc.SetSection("Name",cmDocumentationName);
    doc.SetSection("Usage",cmDocumentationUsage);
    doc.SetSection("Description",cmDocumentationDescription);
    doc.PrependSection("Options",cmDocumentationOptions);

    std::vector<cmDocumentationEntry> v;
    cmCPackGeneratorFactory::DescriptionsMap::const_iterator generatorIt;
    for( generatorIt = generators.GetGeneratorsList().begin();
      generatorIt != generators.GetGeneratorsList().end();
      ++ generatorIt )
      {
      cmDocumentationEntry e;
      e.Name = generatorIt->first.c_str();
      e.Brief = generatorIt->second.c_str();
      e.Full = "";
      v.push_back(e);
      }
    doc.SetSection("Generators",v);

    doc.SetSeeAlsoList(cmDocumentationSeeAlso);
#undef cout
    return doc.PrintRequestedDocumentation(std::cout)? 0:1;
#define cout no_cout_use_cmCPack_Log
    }

  if (cmSystemTools::GetErrorOccuredFlag())
    {
    return 1;
    }

  return 0;
}
