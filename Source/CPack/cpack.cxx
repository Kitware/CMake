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
#include "cmSystemTools.h"

// Need these for documentation support.
#include "cmake.h"
#include "cmDocumentation.h"
#include "cmCPackGenerators.h"
#include "cmCPackGenericGenerator.h"

#include <cmsys/CommandLineArguments.hxx>

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationName[] =
{
  {0,
   "  cpack - Packaging driver provided by CMake.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationUsage[] =
{
  {0,
   "  cpack -G <generator> -P <ProjectName> -R <ReleaseVersion> [options]", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationDescription[] =
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
static const cmDocumentationEntry cmDocumentationOptions[] =
{
    {"-G <generator>", "Use the specified generator to generate package.",
    "CPack may support multiple native packaging systems on certain platforms. A "
      "generator is responsible for generating input files for particular system "
      "and invoking that systems. Possible generator names are specified in the "
      "Generators section." },
    {"-P <ProjectName>", "Specify the project name.",
    "This option specifies the project name that will be used to generate the "
      "installer." },
    {"-R <ReleaseVersion>", "Specify the release version of the project.",
    "This option specifies the release version of the project that will be "
      "used by installer." },
    {"-D <var>=<value>", "Set a CPack variable.", \
    "Set a variable that can be used by the generator."}, \
    {"--patch <ReleasePatch>", "Specify the patch of the project.",
    "This option specifies the patch of the project that will be "
      "used by installer." },
    {"--vendor <ProjectVendor>", "Specify the vendor of the project.",
    "This option specifies the vendor of the project that will be "
      "used by installer." },
    {0,0,0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationSeeAlso[] =
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
typedef std::map<cmStdString, cmStdString> cpackDefinitionsMapType;

//----------------------------------------------------------------------------
int cpackDefinitionArgument(const char* argument, const char* cValue, 
    void* call_data)
{
  (void)argument;
  std::string value = cValue;
  size_t pos = value.find_first_of("=");
  if ( pos == std::string::npos )
    {
    std::cerr << "Please specify CPack definitions as: KEY=VALUE" << std::endl;
    return 0;
    }
  std::string key = value.substr(0, pos);
  value = value.c_str() + pos + 1;
  cpackDefinitionsMapType* map = static_cast<cpackDefinitionsMapType*>(call_data);
  (*map)[key] = value;
  return 1;
}

//----------------------------------------------------------------------------
// this is CPack.
int main (int argc, char *argv[])
{
  int res = 0;
  cmSystemTools::EnableMSVCDebugHook();

  if ( cmSystemTools::GetCurrentWorkingDirectory().size() == 0 )
    {
    std::cerr << "Current working directory cannot be established." << std::endl;
    }

  std::string generator;
  bool help = false;
  bool helpVersion = false;
  std::string helpFull;
  std::string helpMAN;
  std::string helpHTML;

  std::string cpackProjectName;
  std::string cpackProjectDirectory = cmsys::SystemTools::GetCurrentWorkingDirectory();
  std::string cpackBuildConfig;
  std::string cpackProjectVersion;
  std::string cpackProjectPatch;
  std::string cpackProjectVendor;
  cpackDefinitionsMapType definitionsMap;

  cmDocumentation doc;
  cmsys::CommandLineArguments arg;
  arg.Initialize(argc, argv);
  typedef cmsys::CommandLineArguments argT;
  // Help arguments
  arg.AddArgument("--help", argT::NO_ARGUMENT, &help, "CPack help");
  arg.AddArgument("--help-full", argT::SPACE_ARGUMENT, &helpFull, "CPack help");
  arg.AddArgument("--help-html", argT::SPACE_ARGUMENT, &helpHTML, "CPack help");
  arg.AddArgument("--help-man", argT::SPACE_ARGUMENT, &helpMAN, "CPack help");
  arg.AddArgument("--version", argT::NO_ARGUMENT, &helpVersion, "CPack help");

  arg.AddArgument("-C", argT::SPACE_ARGUMENT, &cpackBuildConfig, "CPack build configuration");
  arg.AddArgument("-G", argT::SPACE_ARGUMENT, &generator, "CPack generator");
  arg.AddArgument("-P", argT::SPACE_ARGUMENT, &cpackProjectName, "CPack project name");
  arg.AddArgument("-R", argT::SPACE_ARGUMENT, &cpackProjectVersion, "CPack project version");
  arg.AddArgument("-B", argT::SPACE_ARGUMENT, &cpackProjectDirectory, "CPack project directory");
  arg.AddArgument("--patch", argT::SPACE_ARGUMENT, &cpackProjectPatch, "CPack project patch");
  arg.AddArgument("--vendor", argT::SPACE_ARGUMENT, &cpackProjectVendor, "CPack project vendor");
  arg.AddCallback("-D", argT::SPACE_ARGUMENT, cpackDefinitionArgument, &definitionsMap, "CPack Definitions");
  arg.SetUnknownArgumentCallback(cpackUnknownArgument);

  int parsed = arg.Parse();

  cmCPackGenerators generators;
  cmCPackGenericGenerator* cpackGenerator = 0;

  if ( !helpFull.empty() || !helpMAN.empty() || !helpHTML.empty() || helpVersion )
    {
    help = true;
    }

  if ( parsed && !help )
    {
    if ( generator.empty() )
      {
      std::cerr << "CPack generator not specified" << std::endl;
      parsed = 0;
      }
    if ( parsed && cpackProjectName.empty() )
      {
      std::cerr << "CPack project name not specified" << std::endl;
      parsed = 0;
      }
    if ( parsed && cpackProjectVersion.empty() )
      {
      std::cerr << "CPack project version not specified" << std::endl;
      parsed = 0;
      }
    if ( parsed )
      {
      cpackGenerator = generators.NewGenerator(generator.c_str());
      if ( !cpackGenerator )
        {
        std::cerr << "Cannot initialize CPack generator: " << generator.c_str() << std::endl;
        parsed = 0;
        }
      if ( parsed && !cpackGenerator->FindRunningCMake(argv[0]) )
        {
        std::cerr << "Cannot initialize the generator" << std::endl;
        parsed = 0;
        }

      cmsys::SystemTools::ConvertToUnixSlashes(cpackProjectDirectory);
      std::string makeInstallFile = cpackProjectDirectory + "/cmake_install.cmake";
      if ( !cmsys::SystemTools::FileExists(makeInstallFile.c_str()) )
        {
        std::cerr << "Cannot find installation file: " << makeInstallFile.c_str() << std::endl;
        parsed = 0;
        }
      }
    }

  if ( !parsed || help )
    {
    doc.CheckOptions(argc, argv);
    // Construct and print requested documentation.
    doc.SetName("cpack");
    doc.SetNameSection(cmDocumentationName);
    doc.SetUsageSection(cmDocumentationUsage);
    doc.SetDescriptionSection(cmDocumentationDescription);
    doc.SetOptionsSection(cmDocumentationOptions);
    doc.SetSeeAlsoList(cmDocumentationSeeAlso);
    return doc.PrintRequestedDocumentation(std::cout)? 0:1;
    }

#ifdef _WIN32
  std::string comspec = "cmw9xcom.exe";
  cmSystemTools::SetWindows9xComspecSubstitute(comspec.c_str());
#endif

  std::cout << "Use generator: " << cpackGenerator->GetNameOfClass() << std::endl;
  std::cout << "For project: " << cpackProjectName.c_str() << std::endl;
  cpackGenerator->SetOption("CPACK_PROJECT_NAME", cpackProjectName.c_str());
  cpackGenerator->SetOption("CPACK_PROJECT_VERSION", cpackProjectVersion.c_str());
  cpackGenerator->SetOption("CPACK_PROJECT_VERSION_PATCH", cpackProjectPatch.c_str());
  cpackGenerator->SetOption("CPACK_PROJECT_VENDOR", cpackProjectVendor.c_str());
  cpackGenerator->SetOption("CPACK_PROJECT_DIRECTORY", cpackProjectDirectory.c_str());
  if ( !cpackBuildConfig.empty() )
    {
    cpackGenerator->SetOption("CPACK_BUILD_CONFIG", cpackBuildConfig.c_str());
    }
  cpackDefinitionsMapType::iterator cdit;
  for ( cdit = definitionsMap.begin(); cdit != definitionsMap.end(); ++cdit )
    {
    cpackGenerator->SetOption(cdit->first.c_str(), cdit->second.c_str());
    }

  res = cpackGenerator->ProcessGenerator();
  if ( !res )
    {
    std::cerr << "Error when generating package: " << cpackProjectName.c_str() << std::endl;
    return 1;
    }

  return 0;
}


