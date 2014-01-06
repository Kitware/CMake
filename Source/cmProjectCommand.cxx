/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmProjectCommand.h"

// cmProjectCommand
bool cmProjectCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->SetError("PROJECT called with incorrect number of arguments");
    return false;
    }
  this->Makefile->SetProjectName(args[0].c_str());

  std::string bindir = args[0];
  bindir += "_BINARY_DIR";
  std::string srcdir = args[0];
  srcdir += "_SOURCE_DIR";

  this->Makefile->AddCacheDefinition
    (bindir.c_str(),
     this->Makefile->GetCurrentOutputDirectory(),
     "Value Computed by CMake", cmCacheManager::STATIC);
  this->Makefile->AddCacheDefinition
    (srcdir.c_str(),
     this->Makefile->GetCurrentDirectory(),
     "Value Computed by CMake", cmCacheManager::STATIC);

  bindir = "PROJECT_BINARY_DIR";
  srcdir = "PROJECT_SOURCE_DIR";

  this->Makefile->AddDefinition(bindir.c_str(),
          this->Makefile->GetCurrentOutputDirectory());
  this->Makefile->AddDefinition(srcdir.c_str(),
          this->Makefile->GetCurrentDirectory());

  this->Makefile->AddDefinition("PROJECT_NAME", args[0].c_str());

  // Set the CMAKE_PROJECT_NAME variable to be the highest-level
  // project name in the tree. If there are two project commands
  // in the same CMakeLists.txt file, and it is the top level
  // CMakeLists.txt file, then go with the last one, so that
  // CMAKE_PROJECT_NAME will match PROJECT_NAME, and cmake --build
  // will work.
  if(!this->Makefile->GetDefinition("CMAKE_PROJECT_NAME")
     || (this->Makefile->GetLocalGenerator()->GetParent() == 0) )
    {
    this->Makefile->AddDefinition("CMAKE_PROJECT_NAME", args[0].c_str());
    this->Makefile->AddCacheDefinition
      ("CMAKE_PROJECT_NAME",
       args[0].c_str(),
       "Value Computed by CMake", cmCacheManager::STATIC);
    }

  std::string version;
  std::vector<std::string> languages;
  bool doingVersion = false;
  for(size_t i =1; i < args.size(); ++i)
    {
    if (doingVersion)
      {
      doingVersion = false;
      version = args[i];
      }
    else
      {
      if (args[i] == "VERSION")
        {
        doingVersion = true;
        }
      else
        {
        languages.push_back(args[i]);
        }
      }
    }

  if (version.size() > 0)
    {
    // A version was set, set the variables.
    unsigned int versionMajor = 0;
    unsigned int versionMinor = 0;
    unsigned int versionPatch = 0;
    unsigned int versionTweak= 0;
    int versionCount = sscanf(version.c_str(), "%u.%u.%u.%u",
                              &versionMajor, &versionMinor,
                              &versionPatch, &versionTweak);

    char buffer[1024];

    std::string versionVar = args[0];
    versionVar += "_VERSION_TWEAK";
    sprintf(buffer, "%d", versionCount >=4 ? versionTweak : 0);
    this->Makefile->AddDefinition("PROJECT_VERSION_TWEAK", buffer);
    this->Makefile->AddDefinition(versionVar.c_str(), buffer);

    versionVar = args[0];
    versionVar += "_VERSION_PATCH";
    sprintf(buffer, "%d", versionCount >=3 ? versionPatch : 0);
    this->Makefile->AddDefinition("PROJECT_VERSION_PATCH", buffer);
    this->Makefile->AddDefinition(versionVar.c_str(), buffer);

    versionVar = args[0];
    versionVar += "_VERSION_MINOR";
    sprintf(buffer, "%d", versionCount >=2 ? versionMinor : 0);
    this->Makefile->AddDefinition("PROJECT_VERSION_MINOR", buffer);
    this->Makefile->AddDefinition(versionVar.c_str(), buffer);

    versionVar = args[0];
    versionVar += "_VERSION_MAJOR";
    sprintf(buffer, "%d", versionCount >=1 ? versionMajor : 0);
    this->Makefile->AddDefinition("PROJECT_VERSION_MAJOR", buffer);
    this->Makefile->AddDefinition(versionVar.c_str(), buffer);

    switch(versionCount)
    {
      case 4:
        sprintf(buffer, "%d.%d.%d.%d",
                versionMajor, versionMinor, versionPatch, versionTweak);
        break;
      case 3:
        sprintf(buffer, "%d.%d.%d", versionMajor, versionMinor, versionPatch);
        break;
      case 2:
        sprintf(buffer, "%d.%d", versionMajor, versionMinor);
        break;
      case 1:
        sprintf(buffer, "%d", versionMajor);
        break;
      case 0:
        sprintf(buffer, "0");
        break;
    }

    versionVar = args[0];
    versionVar += "_VERSION";
    this->Makefile->AddDefinition("PROJECT_VERSION", buffer);
    this->Makefile->AddDefinition(versionVar.c_str(), buffer);
  }

  if (languages.size() == 0)
    {
    // if no language is specified do c and c++
    languages.push_back("C");
    languages.push_back("CXX");
    }
  this->Makefile->EnableLanguage(languages, false);
  std::string extraInclude = "CMAKE_PROJECT_" + args[0] + "_INCLUDE";
  const char* include = this->Makefile->GetDefinition(extraInclude.c_str());
  if(include)
    {
    std::string fullFilePath;
    bool readit =
      this->Makefile->ReadListFile( this->Makefile->GetCurrentListFile(),
                                    include);
    if(!readit && !cmSystemTools::GetFatalErrorOccured())
      {
      std::string m =
        "could not find file:\n"
        "  ";
      m += include;
      this->SetError(m.c_str());
      return false;
      }
    }
  return true;
}

