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
#include "cmakewizard.h"
#include "cmake.h"
#include "cmCacheManager.h"
#include "cmDynamicLoader.h"
#include "cmListFileCache.h"
#include "cmDocumentation.h"

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationName[] =
{
  {0,
   "  cmake - Cross-Platform Makefile Generator.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationUsage[] =
{
  {0,
   "  cmake [options] <path-to-source>", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationDescription[] =
{
  {0,
   "The \"cmake\" executable is the CMake command-line interface.  It may "
   "be used to configure projects in scripts.  Project configuration settings "
   "may be specified on the command line with the -D option.  The -i option "
   "will cause cmake to interactively prompt for such settings.", 0},
  CMAKE_STANDARD_INTRODUCTION,
  {0,0,0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationOptions[] =
{
  CMAKE_STANDARD_OPTIONS_TABLE,
  {"-E", "CMake command mode.",
   "For true platform independence, CMake provides a list of commands "
   "that can be used on all systems. Run with -E help for the usage "
   "information."},
  {"-i", "Run in wizard mode.",
   "Wizard mode runs cmake interactively without a GUI.  The user is "
   "prompted to answer questions about the project configuration.  "
   "The answers are used to set cmake cache values."},
  {"-L[A][H]", "List non-advanced cached variables.",
   "List cache variables will run CMake and list all the variables from the "
   "CMake cache that are not marked as INTERNAL or ADVANCED. This will "
   "effectively display current CMake settings, which can be then changed "
   "with -D option. Changing some of the variable may result in more "
   "variables being created. If A is specified, then it will display also "
   "advanced variables. If H is specified, it will also display help for "
   "each variable."},
  {"-N", "View mode only.",
   "Only load the cache. Do not actually run configure and generate steps."},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationSeeAlso[] =
{
  {0, "ccmake", 0},
  {0, "ctest", 0},
  {0, 0, 0}
};

//----------------------------------------------------------------------------
static const cmDocumentationEntry cmDocumentationNOTE[] =
{
  {0,
   "CMake no longer configures a project when run with no arguments.  "
   "In order to configure the project in the current directory, run\n"
   "  cmake .", 0},
  {0,0,0}
};

int do_cmake(int ac, char** av);
void updateProgress(const char *msg, float prog, void *cd);

int main(int ac, char** av)
{
  cmSystemTools::EnableMSVCDebugHook();
  int ret = do_cmake(ac, av);
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmDynamicLoader::FlushCache();
#endif
  cmListFileCache::GetInstance()->ClearCache(); 
  return ret;
}

int do_cmake(int ac, char** av)
{
  cmDocumentation doc;
  if(doc.CheckOptions(ac, av))
    {
    // Construct and print requested documentation.
    cmake hcm;
    std::vector<cmDocumentationEntry> commands;
    std::vector<cmDocumentationEntry> generators;
    hcm.GetCommandDocumentation(commands);
    hcm.GetGeneratorDocumentation(generators);
    doc.SetName("cmake");
    doc.SetNameSection(cmDocumentationName);
    doc.SetUsageSection(cmDocumentationUsage);
    doc.SetDescriptionSection(cmDocumentationDescription);
    doc.SetGeneratorsSection(&generators[0]);
    doc.SetOptionsSection(cmDocumentationOptions);
    doc.SetCommandsSection(&commands[0]);
    doc.SetSeeAlsoList(cmDocumentationSeeAlso);
    int result = doc.PrintRequestedDocumentation(std::cout)? 0:1;
    
    // If we were run with no arguments, but a CMakeLists.txt file
    // exists, the user may have been trying to use the old behavior
    // of cmake to build a project in-source.  Print a message
    // explaining the change to standard error and return an error
    // condition in case the program is running from a script.
    if((ac == 1) && cmSystemTools::FileExists("CMakeLists.txt"))
      {
      doc.ClearSections();
      doc.AddSection("NOTE", cmDocumentationNOTE);
      doc.Print(cmDocumentation::UsageForm, std::cerr);
      return 1;
      }
    return result;
    }
  
  bool wiz = false;
  bool command = false;
  bool list_cached = false;
  bool list_all_cached = false;
  bool list_help = false;
  bool view_only = false;
  std::vector<std::string> args;
  for(int i =0; i < ac; ++i)
    {
    if(strcmp(av[i], "-i") == 0)
      {
      wiz = true;
      }
    else if (strcmp(av[i], "-E") == 0)
      {
      command = true;
      }
    else if (strcmp(av[i], "-N") == 0)
      {
      view_only = true;
      }
    else if (strcmp(av[i], "-L") == 0)
      {
      list_cached = true;
      }
    else if (strcmp(av[i], "-LA") == 0)
      {
      list_all_cached = true;
      }
    else if (strcmp(av[i], "-LH") == 0)
      {
      list_cached = true;
      list_help = true;
      }
    else if (strcmp(av[i], "-LAH") == 0)
      {
      list_all_cached = true;
      list_help = true;
      }
    else 
      {
      args.push_back(av[i]);
      }
    }

  if(command)
    {
    int ret = cmake::CMakeCommand(args);
    return ret;
    }
  if (wiz)
    {
    cmakewizard wizard;
    wizard.RunWizard(args); 
    return 0;
    }
  cmake cm;  
  cm.SetProgressCallback(updateProgress, 0);
  int res = cm.Run(args, view_only);
  if ( list_cached || list_all_cached )
    {
    cmCacheManager::CacheIterator it = cm.GetCacheManager()->GetCacheIterator();
    std::cout << "-- Cache values" << std::endl;
    for ( it.Begin(); !it.IsAtEnd(); it.Next() )
      {
      cmCacheManager::CacheEntryType t = it.GetType();
      if ( t != cmCacheManager::INTERNAL && t != cmCacheManager::STATIC &&
           t != cmCacheManager::UNINITIALIZED )
        {
        bool advanced = it.PropertyExists("ADVANCED");
        if ( list_all_cached || !advanced)
          {
          if ( list_help )
            {
            std::cout << "// " << it.GetProperty("HELPSTRING") << std::endl;
            }
          std::cout << it.GetName() << ":" << cmCacheManager::TypeToString(it.GetType()) 
            << "=" << it.GetValue() << std::endl;
          if ( list_help )
            {
            std::cout << std::endl;
            }
          }
        }
      }
    }
  return res;
}

void updateProgress(const char *msg, float prog, void*)
{
  if ( prog < 0 )
    {
    std::cout << "-- " << msg << std::endl;
    }
  //else
  //{
  //std::cout << "-- " << msg << " " << prog << std::endl;
  //}
}
