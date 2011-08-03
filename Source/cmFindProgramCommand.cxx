/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmFindProgramCommand.h"
#include "cmCacheManager.h"
#include <stdlib.h>

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif

void cmFindProgramCommand::GenerateDocumentation()
{
  this->cmFindBase::GenerateDocumentation();
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "FIND_XXX", "find_program");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_XXX_PATH", "CMAKE_PROGRAM_PATH");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_XXX_MAC_PATH",
                               "CMAKE_APPBUNDLE_PATH");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_SYSTEM_XXX_MAC_PATH",
                               "CMAKE_SYSTEM_APPBUNDLE_PATH");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "XXX_SYSTEM", "");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_SYSTEM_XXX_PATH",
                               "CMAKE_SYSTEM_PROGRAM_PATH");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "SEARCH_XXX_DESC", "program");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "SEARCH_XXX", "program");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "XXX_SUBDIR", "[s]bin");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "XXX_EXTRA_PREFIX_ENTRY", "");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_FIND_ROOT_PATH_MODE_XXX",
                               "CMAKE_FIND_ROOT_PATH_MODE_PROGRAM");
}

// cmFindProgramCommand
bool cmFindProgramCommand
::InitialPass(std::vector<std::string> const& argsIn, cmExecutionStatus &)
{
  this->VariableDocumentation = "Path to a program.";
  this->CMakePathName = "PROGRAM";
  // call cmFindBase::ParseArguments
  if(!this->ParseArguments(argsIn))
    {
    return false;
    }
  if(this->AlreadyInCache)
    {
    // If the user specifies the entry on the command line without a
    // type we should add the type and docstring but keep the original
    // value.
    if(this->AlreadyInCacheWithoutMetaInfo)
      {
      this->Makefile->AddCacheDefinition(this->VariableName.c_str(), "",
                                         this->VariableDocumentation.c_str(),
                                         cmCacheManager::FILEPATH);
      }
    return true;
    }

  std::string result = FindProgram(this->Names);
  if(result != "")
    {
    // Save the value in the cache
    this->Makefile->AddCacheDefinition(this->VariableName.c_str(),
                                       result.c_str(),
                                       this->VariableDocumentation.c_str(),
                                       cmCacheManager::FILEPATH);
    
    return true;
    }
  this->Makefile->AddCacheDefinition(this->VariableName.c_str(),
                                 (this->VariableName + "-NOTFOUND").c_str(),
                                 this->VariableDocumentation.c_str(),
                                 cmCacheManager::FILEPATH);
  return true;
}

std::string cmFindProgramCommand::FindProgram(std::vector<std::string> names)
{
  std::string program = "";

  if(this->SearchAppBundleFirst || this->SearchAppBundleOnly)
    {
    program = FindAppBundle(names);
    }
  if(program.empty() && !this->SearchAppBundleOnly)
    {
    program = cmSystemTools::FindProgram(names, this->SearchPaths, true);
    }

  if(program.empty() && this->SearchAppBundleLast)
    {
    program = this->FindAppBundle(names);
    }
  return program;
}

std::string cmFindProgramCommand
::FindAppBundle(std::vector<std::string> names)
{
  for(std::vector<std::string>::const_iterator name = names.begin();
      name != names.end() ; ++name)
    {
    
    std::string appName = *name + std::string(".app");
    std::string appPath = cmSystemTools::FindDirectory(appName.c_str(), 
                                                       this->SearchPaths, 
                                                       true);

    if ( !appPath.empty() )
      {
      std::string executable = GetBundleExecutable(appPath);
      if (!executable.empty()) 
        {
        return cmSystemTools::CollapseFullPath(executable.c_str());
        }
      } 
    } 

  // Couldn't find app bundle
  return "";
}

std::string cmFindProgramCommand::GetBundleExecutable(std::string bundlePath)
{
  std::string executable = "";
  (void)bundlePath;
#if defined(__APPLE__)
  // Started with an example on developer.apple.com about finding bundles 
  // and modified from that.
  
  // Get a CFString of the app bundle path
  // XXX - Is it safe to assume everything is in UTF8?
  CFStringRef bundlePathCFS = 
    CFStringCreateWithCString(kCFAllocatorDefault , 
                              bundlePath.c_str(), kCFStringEncodingUTF8 );
  
  // Make a CFURLRef from the CFString representation of the
  // bundle’s path.
  CFURLRef bundleURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, 
                                                     bundlePathCFS,
                                                     kCFURLPOSIXPathStyle,
                                                     true );
  
  // Make a bundle instance using the URLRef.
  CFBundleRef appBundle = CFBundleCreate( kCFAllocatorDefault, bundleURL );
  
  // returned executableURL is relative to <appbundle>/Contents/MacOS/
  CFURLRef executableURL = CFBundleCopyExecutableURL(appBundle);
  
  if (executableURL != NULL)
    {
    const int MAX_OSX_PATH_SIZE = 1024;
    char buffer[MAX_OSX_PATH_SIZE];
    
    // Convert the CFString to a C string
    CFStringGetCString( CFURLGetString(executableURL), buffer, 
                        MAX_OSX_PATH_SIZE, kCFStringEncodingUTF8 );
    
    // And finally to a c++ string
    executable = bundlePath + "/Contents/MacOS/" + std::string(buffer);
    }

  // Any CF objects returned from functions with "create" or 
  // "copy" in their names must be released by us!
  CFRelease( bundlePathCFS );
  CFRelease( bundleURL );
  CFRelease( appBundle );
  CFRelease( executableURL );
#endif

  return executable;
}

