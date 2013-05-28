/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmBuildCommand.h"

#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"

//----------------------------------------------------------------------
bool cmBuildCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  // Support the legacy signature of the command:
  //
  if(2 == args.size())
    {
    return this->TwoArgsSignature(args);
    }

  return this->MainSignature(args);
}

//----------------------------------------------------------------------
bool cmBuildCommand
::MainSignature(std::vector<std::string> const& args)
{
  if(args.size() < 1)
    {
    this->SetError("requires at least one argument naming a CMake variable");
    return false;
    }

  // The cmake variable in which to store the result.
  const char* variable = args[0].c_str();

  // Parse remaining arguments.
  const char* configuration = 0;
  const char* project_name = 0;
  const char* target = 0;
  enum Doing { DoingNone, DoingConfiguration, DoingProjectName, DoingTarget };
  Doing doing = DoingNone;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    if(args[i] == "CONFIGURATION")
      {
      doing = DoingConfiguration;
      }
    else if(args[i] == "PROJECT_NAME")
      {
      doing = DoingProjectName;
      }
    else if(args[i] == "TARGET")
      {
      doing = DoingTarget;
      }
    else if(doing == DoingConfiguration)
      {
      doing = DoingNone;
      configuration = args[i].c_str();
      }
    else if(doing == DoingProjectName)
      {
      doing = DoingNone;
      project_name = args[i].c_str();
      }
    else if(doing == DoingTarget)
      {
      doing = DoingNone;
      target = args[i].c_str();
      }
    else
      {
      cmOStringStream e;
      e << "unknown argument \"" << args[i] << "\"";
      this->SetError(e.str().c_str());
      return false;
      }
    }

  const char* makeprogram
    = this->Makefile->GetDefinition("CMAKE_MAKE_PROGRAM");
  if(!makeprogram)
    {
    this->Makefile->IssueMessage(
      cmake::FATAL_ERROR,
      "build_command() requires CMAKE_MAKE_PROGRAM to be defined.  "
      "Call project() or enable_language() first.");
    return true;
    }

  // If null/empty CONFIGURATION argument, GenerateBuildCommand uses 'Debug'
  // in the currently implemented multi-configuration global generators...
  // so we put this code here to end up with the same default configuration
  // as the original 2-arg build_command signature:
  //
  if(!configuration || !*configuration)
    {
    configuration = getenv("CMAKE_CONFIG_TYPE");
    }
  if(!configuration || !*configuration)
    {
    configuration = "Release";
    }

  // If null/empty PROJECT_NAME argument, use the Makefile's project name:
  //
  if(!project_name || !*project_name)
    {
    project_name = this->Makefile->GetProjectName();
    }

  // If null/empty TARGET argument, GenerateBuildCommand omits any mention
  // of a target name on the build command line...
  //
  std::string makecommand = this->Makefile->GetLocalGenerator()
    ->GetGlobalGenerator()->GenerateBuildCommand
    (makeprogram, project_name, 0, 0, target, configuration, true, false);

  this->Makefile->AddDefinition(variable, makecommand.c_str());

  return true;
}

//----------------------------------------------------------------------
bool cmBuildCommand
::TwoArgsSignature(std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with less than two arguments");
    return false;
    }

  const char* define = args[0].c_str();
  const char* cacheValue
    = this->Makefile->GetDefinition(define);
  std::string makeprogram = args[1];

  std::string configType = "Release";
  const char* cfg = getenv("CMAKE_CONFIG_TYPE");
  if ( cfg )
    {
    configType = cfg;
    }

  std::string makecommand = this->Makefile->GetLocalGenerator()
    ->GetGlobalGenerator()->GenerateBuildCommand
    (makeprogram.c_str(), this->Makefile->GetProjectName(), 0, 0,
     0, configType.c_str(), true, false);

  if(cacheValue)
    {
    return true;
    }
  this->Makefile->AddCacheDefinition(define,
                                 makecommand.c_str(),
                                 "Command used to build entire project "
                                 "from the command line.",
                                 cmCacheManager::STRING);
  return true;
}
