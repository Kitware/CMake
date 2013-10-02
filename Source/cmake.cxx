/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmake.h"
#include "cmDocumentVariables.h"
#include "time.h"
#include "cmCacheManager.h"
#include "cmMakefile.h"
#include "cmLocalGenerator.h"
#include "cmExternalMakefileProjectGenerator.h"
#include "cmCommands.h"
#include "cmCommand.h"
#include "cmFileTimeComparison.h"
#include "cmGeneratedFileStream.h"
#include "cmQtAutomoc.h"
#include "cmSourceFile.h"
#include "cmVersion.h"
#include "cmTest.h"
#include "cmDocumentationFormatterText.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
# include "cmGraphVizWriter.h"
# include "cmDependsFortran.h" // For -E cmake_copy_f90_mod callback.
# include "cmVariableWatch.h"
# include <cmsys/Terminal.h>
# include <cmsys/CommandLineArguments.hxx>
#endif

#include <cmsys/Directory.hxx>
#include <cmsys/Process.h>
#include <cmsys/Glob.hxx>
#include <cmsys/RegularExpression.hxx>

// only build kdevelop generator on non-windows platforms
// when not bootstrapping cmake
#if !defined(_WIN32)
# if defined(CMAKE_BUILD_WITH_CMAKE)
#   define CMAKE_USE_KDEVELOP
# endif
#endif

#if defined(CMAKE_BUILD_WITH_CMAKE)
#  define CMAKE_USE_ECLIPSE
#endif

#if defined(__MINGW32__) && !defined(CMAKE_BUILD_WITH_CMAKE)
# define CMAKE_BOOT_MINGW
#endif

// include the generator
#if defined(_WIN32) && !defined(__CYGWIN__)
#  if !defined(CMAKE_BOOT_MINGW)
#    include "cmGlobalVisualStudio6Generator.h"
#    include "cmGlobalVisualStudio7Generator.h"
#    include "cmGlobalVisualStudio71Generator.h"
#    include "cmGlobalVisualStudio8Generator.h"
#    include "cmGlobalVisualStudio9Generator.h"
#    include "cmGlobalVisualStudio10Generator.h"
#    include "cmGlobalVisualStudio11Generator.h"
#    include "cmGlobalVisualStudio12Generator.h"
#    include "cmGlobalBorlandMakefileGenerator.h"
#    include "cmGlobalNMakeMakefileGenerator.h"
#    include "cmGlobalJOMMakefileGenerator.h"
#    include "cmGlobalWatcomWMakeGenerator.h"
#    define CMAKE_HAVE_VS_GENERATORS
#  endif
#  include "cmGlobalMSYSMakefileGenerator.h"
#  include "cmGlobalMinGWMakefileGenerator.h"
#  include "cmWin32ProcessExecution.h"
#else
#endif
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmGlobalNinjaGenerator.h"


#if defined(CMAKE_HAVE_VS_GENERATORS)
#include "cmCallVisualStudioMacro.h"
#include "cmVisualStudioWCEPlatformParser.h"
#endif

#if !defined(CMAKE_BOOT_MINGW)
# include "cmExtraCodeBlocksGenerator.h"
#endif
#include "cmExtraSublimeTextGenerator.h"

#ifdef CMAKE_USE_KDEVELOP
# include "cmGlobalKdevelopGenerator.h"
#endif

#ifdef CMAKE_USE_ECLIPSE
# include "cmExtraEclipseCDT4Generator.h"
#endif

#include <stdlib.h> // required for atoi

#if defined( __APPLE__ )
#  if defined(CMAKE_BUILD_WITH_CMAKE)
#    include "cmGlobalXCodeGenerator.h"
#    define CMAKE_USE_XCODE 1
#  endif
#  include <sys/types.h>
#  include <sys/time.h>
#  include <sys/resource.h>
#endif

#include <sys/stat.h> // struct stat

static bool cmakeCheckStampFile(const char* stampName);
static bool cmakeCheckStampList(const char* stampName);

void cmNeedBackwardsCompatibility(const std::string& variable,
  int access_type, void*, const char*, const cmMakefile*)
{
#ifdef CMAKE_BUILD_WITH_CMAKE
  if (access_type == cmVariableWatch::UNKNOWN_VARIABLE_READ_ACCESS)
    {
    std::string message = "An attempt was made to access a variable: ";
    message += variable;
    message +=
      " that has not been defined. Some variables were always defined "
      "by CMake in versions prior to 1.6. To fix this you might need to set "
      "the cache value of CMAKE_BACKWARDS_COMPATIBILITY to 1.4 or less. If "
      "you are writing a CMakeLists file, (or have already set "
      "CMAKE_BACKWARDS_COMPATIBILITY to 1.4 or less) then you probably need "
      "to include a CMake module to test for the feature this variable "
      "defines.";
    cmSystemTools::Error(message.c_str());
    }
#else
  (void)variable;
  (void)access_type;
#endif
}

void cmWarnUnusedCliWarning(const std::string& variable,
  int, void* ctx, const char*, const cmMakefile*)
{
  cmake* cm = reinterpret_cast<cmake*>(ctx);
  cm->MarkCliAsUsed(variable);
}

cmake::cmake()
{
  this->Trace = false;
  this->WarnUninitialized = false;
  this->WarnUnused = false;
  this->WarnUnusedCli = true;
  this->CheckSystemVars = false;
  this->SuppressDevWarnings = false;
  this->DoSuppressDevWarnings = false;
  this->DebugOutput = false;
  this->DebugTryCompile = false;
  this->ClearBuildSystem = false;
  this->FileComparison = new cmFileTimeComparison;

  this->Policies = new cmPolicies();
  this->InitializeProperties();

#ifdef __APPLE__
  struct rlimit rlp;
  if(!getrlimit(RLIMIT_STACK, &rlp))
    {
    if(rlp.rlim_cur != rlp.rlim_max)
      {
        rlp.rlim_cur = rlp.rlim_max;
         setrlimit(RLIMIT_STACK, &rlp);
      }
    }
#endif

  this->Verbose = false;
  this->InTryCompile = false;
  this->CacheManager = new cmCacheManager(this);
  this->GlobalGenerator = 0;
  this->ProgressCallback = 0;
  this->ProgressCallbackClientData = 0;
  this->CurrentWorkingMode = NORMAL_MODE;

#ifdef CMAKE_BUILD_WITH_CMAKE
  this->VariableWatch = new cmVariableWatch;
  this->VariableWatch->AddWatch("CMAKE_WORDS_BIGENDIAN",
                            cmNeedBackwardsCompatibility);
  this->VariableWatch->AddWatch("CMAKE_SIZEOF_INT",
                            cmNeedBackwardsCompatibility);
  this->VariableWatch->AddWatch("CMAKE_X_LIBS",
                            cmNeedBackwardsCompatibility);
#endif

  this->AddDefaultGenerators();
  this->AddDefaultExtraGenerators();
  this->AddDefaultCommands();

  // Make sure we can capture the build tool output.
  cmSystemTools::EnableVSConsoleOutput();
}

cmake::~cmake()
{
  delete this->CacheManager;
  delete this->Policies;
  if (this->GlobalGenerator)
    {
    delete this->GlobalGenerator;
    this->GlobalGenerator = 0;
    }
  for(RegisteredCommandsMap::iterator j = this->Commands.begin();
      j != this->Commands.end(); ++j)
    {
    delete (*j).second;
    }
  for(RegisteredGeneratorsVector::iterator j = this->Generators.begin();
      j != this->Generators.end(); ++j)
    {
    delete *j;
    }
#ifdef CMAKE_BUILD_WITH_CMAKE
  delete this->VariableWatch;
#endif
  delete this->FileComparison;
}

void cmake::InitializeProperties()
{
  this->Properties.clear();
  this->Properties.SetCMakeInstance(this);
  this->AccessedProperties.clear();
  this->PropertyDefinitions.clear();

  // initialize properties
  cmCacheManager::DefineProperties(this);
  cmSourceFile::DefineProperties(this);
  cmTarget::DefineProperties(this);
  cmMakefile::DefineProperties(this);
  cmTest::DefineProperties(this);
  cmake::DefineProperties(this);
}

void cmake::CleanupCommandsAndMacros()
{
  this->InitializeProperties();
  std::vector<cmCommand*> commands;
  for(RegisteredCommandsMap::iterator j = this->Commands.begin();
      j != this->Commands.end(); ++j)
    {
    if ( !j->second->IsA("cmMacroHelperCommand") &&
         !j->second->IsA("cmFunctionHelperCommand"))
      {
      commands.push_back(j->second);
      }
    else
      {
      delete j->second;
      }
    }
  this->Commands.erase(this->Commands.begin(), this->Commands.end());
  std::vector<cmCommand*>::iterator it;
  for ( it = commands.begin(); it != commands.end();
    ++ it )
    {
    this->Commands[cmSystemTools::LowerCase((*it)->GetName())] = *it;
    }
}

bool cmake::CommandExists(const char* name) const
{
  std::string sName = cmSystemTools::LowerCase(name);
  return (this->Commands.find(sName) != this->Commands.end());
}

cmCommand *cmake::GetCommand(const char *name)
{
  cmCommand* rm = 0;
  std::string sName = cmSystemTools::LowerCase(name);
  RegisteredCommandsMap::iterator pos = this->Commands.find(sName);
  if (pos != this->Commands.end())
    {
    rm = (*pos).second;
    }
  return rm;
}

void cmake::RenameCommand(const char*oldName, const char* newName)
{
  // if the command already exists, free the old one
  std::string sOldName = cmSystemTools::LowerCase(oldName);
  std::string sNewName = cmSystemTools::LowerCase(newName);
  RegisteredCommandsMap::iterator pos = this->Commands.find(sOldName);
  if ( pos == this->Commands.end() )
    {
    return;
    }
  cmCommand* cmd = pos->second;

  pos = this->Commands.find(sNewName);
  if (pos != this->Commands.end())
    {
    delete pos->second;
    this->Commands.erase(pos);
    }
  this->Commands.insert(RegisteredCommandsMap::value_type(sNewName, cmd));
  pos = this->Commands.find(sOldName);
  this->Commands.erase(pos);
}

void cmake::RemoveCommand(const char* name)
{
  std::string sName = cmSystemTools::LowerCase(name);
  RegisteredCommandsMap::iterator pos = this->Commands.find(sName);
  if ( pos != this->Commands.end() )
    {
    delete pos->second;
    this->Commands.erase(pos);
    }
}

void cmake::AddCommand(cmCommand* wg)
{
  std::string name = cmSystemTools::LowerCase(wg->GetName());
  // if the command already exists, free the old one
  RegisteredCommandsMap::iterator pos = this->Commands.find(name);
  if (pos != this->Commands.end())
    {
    delete pos->second;
    this->Commands.erase(pos);
    }
  this->Commands.insert( RegisteredCommandsMap::value_type(name, wg));
}


void cmake::RemoveUnscriptableCommands()
{
  std::vector<std::string> unscriptableCommands;
  cmake::RegisteredCommandsMap* commands = this->GetCommands();
  for (cmake::RegisteredCommandsMap::const_iterator pos = commands->begin();
       pos != commands->end();
       ++pos)
    {
    if (!pos->second->IsScriptable())
      {
      unscriptableCommands.push_back(pos->first);
      }
    }

  for(std::vector<std::string>::const_iterator it=unscriptableCommands.begin();
      it != unscriptableCommands.end();
      ++it)
    {
    this->RemoveCommand(it->c_str());
    }
}

// Parse the args
bool cmake::SetCacheArgs(const std::vector<std::string>& args)
{
  bool findPackageMode = false;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-D",0) == 0)
      {
      std::string entry = arg.substr(2);
      if(entry.size() == 0)
        {
        ++i;
        if(i < args.size())
          {
          entry = args[i];
          }
        else
          {
          cmSystemTools::Error("-D must be followed with VAR=VALUE.");
          return false;
          }
        }
      std::string var, value;
      cmCacheManager::CacheEntryType type = cmCacheManager::UNINITIALIZED;
      if(cmCacheManager::ParseEntry(entry.c_str(), var, value, type))
        {
        // The value is transformed if it is a filepath for example, so
        // we can't compare whether the value is already in the cache until
        // after we call AddCacheEntry.
        const char *cachedValue =
                              this->CacheManager->GetCacheValue(var.c_str());

        this->CacheManager->AddCacheEntry(var.c_str(), value.c_str(),
          "No help, variable specified on the command line.", type);
        if(this->WarnUnusedCli)
          {
          if (!cachedValue
              || strcmp(this->CacheManager->GetCacheValue(var.c_str()),
                        cachedValue) != 0)
            {
            this->WatchUnusedCli(var.c_str());
            }
          }
        }
      else
        {
        std::cerr << "Parse error in command line argument: " << arg << "\n"
                  << "Should be: VAR:type=value\n";
        cmSystemTools::Error("No cmake script provided.");
        return false;
        }
      }
    else if(arg.find("-Wno-dev",0) == 0)
      {
      this->SuppressDevWarnings = true;
      this->DoSuppressDevWarnings = true;
      }
    else if(arg.find("-Wdev",0) == 0)
      {
      this->SuppressDevWarnings = false;
      this->DoSuppressDevWarnings = true;
      }
    else if(arg.find("-U",0) == 0)
      {
      std::string entryPattern = arg.substr(2);
      if(entryPattern.size() == 0)
        {
        ++i;
        if(i < args.size())
          {
          entryPattern = args[i];
          }
        else
          {
          cmSystemTools::Error("-U must be followed with VAR.");
          return false;
          }
        }
      cmsys::RegularExpression regex(
        cmsys::Glob::PatternToRegex(entryPattern.c_str(), true, true).c_str());
      //go through all cache entries and collect the vars which will be removed
      std::vector<std::string> entriesToDelete;
      cmCacheManager::CacheIterator it =
                                    this->CacheManager->GetCacheIterator();
      for ( it.Begin(); !it.IsAtEnd(); it.Next() )
        {
        cmCacheManager::CacheEntryType t = it.GetType();
        if(t != cmCacheManager::STATIC)
          {
          std::string entryName = it.GetName();
          if (regex.find(entryName.c_str()))
            {
            entriesToDelete.push_back(entryName);
            }
          }
        }

      // now remove them from the cache
      for(std::vector<std::string>::const_iterator currentEntry =
          entriesToDelete.begin();
          currentEntry != entriesToDelete.end();
          ++currentEntry)
        {
        this->CacheManager->RemoveCacheEntry(currentEntry->c_str());
        }
      }
    else if(arg.find("-C",0) == 0)
      {
      std::string path = arg.substr(2);
      if ( path.size() == 0 )
        {
        ++i;
        if(i < args.size())
          {
          path = args[i];
          }
        else
          {
          cmSystemTools::Error("-C must be followed by a file name.");
          return false;
          }
        }
      std::cerr << "loading initial cache file " << path.c_str() << "\n";
      this->ReadListFile(args, path.c_str());
      }
    else if(arg.find("-P",0) == 0)
      {
      i++;
      if(i >= args.size())
        {
        cmSystemTools::Error("-P must be followed by a file name.");
        return false;
        }
      std::string path = args[i];
      if ( path.size() == 0 )
        {
        cmSystemTools::Error("No cmake script provided.");
        return false;
        }
      this->ReadListFile(args, path.c_str());
      }
    else if (arg.find("--find-package",0) == 0)
      {
      findPackageMode = true;
      }
    }

  if (findPackageMode)
    {
    return this->FindPackage(args);
    }

  return true;
}

void cmake::ReadListFile(const std::vector<std::string>& args,
                         const char *path)
{
  // if a generator was not yet created, temporarily create one
  cmGlobalGenerator *gg = this->GetGlobalGenerator();
  bool created = false;

  // if a generator was not specified use a generic one
  if (!gg)
    {
    gg = new cmGlobalGenerator;
    gg->SetCMakeInstance(this);
    created = true;
    }

  // read in the list file to fill the cache
  if(path)
    {
    cmsys::auto_ptr<cmLocalGenerator> lg(gg->CreateLocalGenerator());
    lg->GetMakefile()->SetHomeOutputDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    lg->GetMakefile()->SetStartOutputDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    lg->GetMakefile()->SetHomeDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    lg->GetMakefile()->SetStartDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    if (this->GetWorkingMode() != NORMAL_MODE)
      {
      std::string file(cmSystemTools::CollapseFullPath(path));
      cmSystemTools::ConvertToUnixSlashes(file);
      lg->GetMakefile()->SetScriptModeFile(file.c_str());

      lg->GetMakefile()->SetArgcArgv(args);
      }
    if (!lg->GetMakefile()->ReadListFile(0, path))
      {
      cmSystemTools::Error("Error processing file: ", path);
      }
    }

  // free generic one if generated
  if (created)
    {
    delete gg;
    }
}


bool cmake::FindPackage(const std::vector<std::string>& args)
{
  // if a generator was not yet created, temporarily create one
  cmGlobalGenerator *gg = new cmGlobalGenerator;
  gg->SetCMakeInstance(this);
  this->SetGlobalGenerator(gg);

  // read in the list file to fill the cache
  cmsys::auto_ptr<cmLocalGenerator> lg(gg->CreateLocalGenerator());
  cmMakefile* mf = lg->GetMakefile();
  mf->SetHomeOutputDirectory
    (cmSystemTools::GetCurrentWorkingDirectory().c_str());
  mf->SetStartOutputDirectory
    (cmSystemTools::GetCurrentWorkingDirectory().c_str());
  mf->SetHomeDirectory
    (cmSystemTools::GetCurrentWorkingDirectory().c_str());
  mf->SetStartDirectory
    (cmSystemTools::GetCurrentWorkingDirectory().c_str());

  mf->SetArgcArgv(args);

  std::string systemFile = mf->GetModulesFile("CMakeFindPackageMode.cmake");
  mf->ReadListFile(0, systemFile.c_str());

  std::string language = mf->GetSafeDefinition("LANGUAGE");
  std::string mode = mf->GetSafeDefinition("MODE");
  std::string packageName = mf->GetSafeDefinition("NAME");
  bool packageFound = mf->IsOn("PACKAGE_FOUND");
  bool quiet = mf->IsOn("PACKAGE_QUIET");

  if (!packageFound)
    {
    if (!quiet)
      {
      printf("%s not found.\n", packageName.c_str());
      }
    }
  else if (mode == "EXIST")
    {
    if (!quiet)
      {
      printf("%s found.\n", packageName.c_str());
      }
    }
  else if (mode == "COMPILE")
    {
    std::string includes = mf->GetSafeDefinition("PACKAGE_INCLUDE_DIRS");
    std::vector<std::string> includeDirs;
    cmSystemTools::ExpandListArgument(includes, includeDirs);

    std::string includeFlags = lg->GetIncludeFlags(includeDirs, 0,
                                                   language.c_str(), false);

    std::string definitions = mf->GetSafeDefinition("PACKAGE_DEFINITIONS");
    printf("%s %s\n", includeFlags.c_str(), definitions.c_str());
    }
  else if (mode == "LINK")
    {
    const char* targetName = "dummy";
    std::vector<std::string> srcs;
    cmTarget* tgt = mf->AddExecutable(targetName, srcs, true);
    tgt->SetProperty("LINKER_LANGUAGE", language.c_str());

    std::string libs = mf->GetSafeDefinition("PACKAGE_LIBRARIES");
    std::vector<std::string> libList;
    cmSystemTools::ExpandListArgument(libs, libList);
    for(std::vector<std::string>::const_iterator libIt=libList.begin();
            libIt != libList.end();
            ++libIt)
      {
      mf->AddLinkLibraryForTarget(targetName, libIt->c_str(),
                                  cmTarget::GENERAL);
      }


    std::string linkLibs;
    std::string frameworkPath;
    std::string linkPath;
    std::string flags;
    std::string linkFlags;
    cmGeneratorTarget gtgt(tgt);
    lg->GetTargetFlags(linkLibs, frameworkPath, linkPath, flags, linkFlags,
                       &gtgt);
    linkLibs = frameworkPath + linkPath + linkLibs;

    printf("%s\n", linkLibs.c_str() );

/*    if ( use_win32 )
      {
      tgt->SetProperty("WIN32_EXECUTABLE", "ON");
      }
    if ( use_macbundle)
      {
      tgt->SetProperty("MACOSX_BUNDLE", "ON");
      }*/
    }

  // free generic one if generated
//  this->SetGlobalGenerator(0); // setting 0-pointer is not possible
//  delete gg; // this crashes inside the cmake instance

  return packageFound;
}


// Parse the args
void cmake::SetArgs(const std::vector<std::string>& args,
                    bool directoriesSetBefore)
{
  bool directoriesSet = directoriesSetBefore;
  bool haveToolset = false;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-H",0) == 0)
      {
      directoriesSet = true;
      std::string path = arg.substr(2);
      path = cmSystemTools::CollapseFullPath(path.c_str());
      cmSystemTools::ConvertToUnixSlashes(path);
      this->SetHomeDirectory(path.c_str());
      }
    else if(arg.find("-S",0) == 0)
      {
      // There is no local generate anymore.  Ignore -S option.
      }
    else if(arg.find("-O",0) == 0)
      {
      // There is no local generate anymore.  Ignore -O option.
      }
    else if(arg.find("-B",0) == 0)
      {
      directoriesSet = true;
      std::string path = arg.substr(2);
      path = cmSystemTools::CollapseFullPath(path.c_str());
      cmSystemTools::ConvertToUnixSlashes(path);
      this->SetHomeOutputDirectory(path.c_str());
      }
    else if((i < args.size()-1) && (arg.find("--check-build-system",0) == 0))
      {
      this->CheckBuildSystemArgument = args[++i];
      this->ClearBuildSystem = (atoi(args[++i].c_str()) > 0);
      }
    else if((i < args.size()-1) && (arg.find("--check-stamp-file",0) == 0))
      {
      this->CheckStampFile = args[++i];
      }
    else if((i < args.size()-1) && (arg.find("--check-stamp-list",0) == 0))
      {
      this->CheckStampList = args[++i];
      }
#if defined(CMAKE_HAVE_VS_GENERATORS)
    else if((i < args.size()-1) && (arg.find("--vs-solution-file",0) == 0))
      {
      this->VSSolutionFile = args[++i];
      }
#endif
    else if(arg.find("-V",0) == 0)
      {
        this->Verbose = true;
      }
    else if(arg.find("-D",0) == 0)
      {
      // skip for now
      }
    else if(arg.find("-U",0) == 0)
      {
      // skip for now
      }
    else if(arg.find("-C",0) == 0)
      {
      // skip for now
      }
    else if(arg.find("-P",0) == 0)
      {
      // skip for now
      i++;
      }
    else if(arg.find("--find-package",0) == 0)
      {
      // skip for now
      i++;
      }
    else if(arg.find("-Wno-dev",0) == 0)
      {
      // skip for now
      }
    else if(arg.find("-Wdev",0) == 0)
      {
      // skip for now
      }
    else if(arg.find("--graphviz=",0) == 0)
      {
      std::string path = arg.substr(strlen("--graphviz="));
      path = cmSystemTools::CollapseFullPath(path.c_str());
      cmSystemTools::ConvertToUnixSlashes(path);
      this->GraphVizFile = path;
      if ( this->GraphVizFile.empty() )
        {
        cmSystemTools::Error("No file specified for --graphviz");
        }
      }
    else if(arg.find("--debug-trycompile",0) == 0)
      {
      std::cout << "debug trycompile on\n";
      this->DebugTryCompileOn();
      }
    else if(arg.find("--debug-output",0) == 0)
      {
      std::cout << "Running with debug output on.\n";
      this->SetDebugOutputOn(true);
      }
    else if(arg.find("--trace",0) == 0)
      {
      std::cout << "Running with trace output on.\n";
      this->SetTrace(true);
      }
    else if(arg.find("--warn-uninitialized",0) == 0)
      {
      std::cout << "Warn about uninitialized values.\n";
      this->SetWarnUninitialized(true);
      }
    else if(arg.find("--warn-unused-vars",0) == 0)
      {
      std::cout << "Finding unused variables.\n";
      this->SetWarnUnused(true);
      }
    else if(arg.find("--no-warn-unused-cli",0) == 0)
      {
      std::cout << "Not searching for unused variables given on the " <<
                   "command line.\n";
      this->SetWarnUnusedCli(false);
      }
    else if(arg.find("--check-system-vars",0) == 0)
      {
      std::cout << "Also check system files when warning about unused and " <<
                   "uninitialized variables.\n";
      this->SetCheckSystemVars(true);
      }
    else if(arg.find("-T",0) == 0)
      {
      std::string value = arg.substr(2);
      if(value.size() == 0)
        {
        ++i;
        if(i >= args.size())
          {
          cmSystemTools::Error("No toolset specified for -T");
          return;
          }
        value = args[i];
        }
      if(haveToolset)
        {
        cmSystemTools::Error("Multiple -T options not allowed");
        return;
        }
      this->GeneratorToolset = value;
      haveToolset = true;
      }
    else if(arg.find("-G",0) == 0)
      {
      std::string value = arg.substr(2);
      if(value.size() == 0)
        {
        ++i;
        if(i >= args.size())
          {
          cmSystemTools::Error("No generator specified for -G");
          return;
          }
        value = args[i];
        }
      cmGlobalGenerator* gen =
        this->CreateGlobalGenerator(value.c_str());
      if(!gen)
        {
        cmSystemTools::Error("Could not create named generator ",
                             value.c_str());
        }
      else
        {
        this->SetGlobalGenerator(gen);
        }
      }
    // no option assume it is the path to the source
    else
      {
      directoriesSet = true;
      this->SetDirectoriesFromFile(arg.c_str());
      }
    }
  if(!directoriesSet)
    {
    this->SetHomeOutputDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    this->SetStartOutputDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    this->SetHomeDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    this->SetStartDirectory
      (cmSystemTools::GetCurrentWorkingDirectory().c_str());
    }

  this->SetStartDirectory(this->GetHomeDirectory());
  this->SetStartOutputDirectory(this->GetHomeOutputDirectory());
}

//----------------------------------------------------------------------------
void cmake::SetDirectoriesFromFile(const char* arg)
{
  // Check if the argument refers to a CMakeCache.txt or
  // CMakeLists.txt file.
  std::string listPath;
  std::string cachePath;
  bool argIsFile = false;
  if(cmSystemTools::FileIsDirectory(arg))
    {
    std::string path = cmSystemTools::CollapseFullPath(arg);
    cmSystemTools::ConvertToUnixSlashes(path);
    std::string cacheFile = path;
    cacheFile += "/CMakeCache.txt";
    std::string listFile = path;
    listFile += "/CMakeLists.txt";
    if(cmSystemTools::FileExists(cacheFile.c_str()))
      {
      cachePath = path;
      }
    if(cmSystemTools::FileExists(listFile.c_str()))
      {
      listPath = path;
      }
    }
  else if(cmSystemTools::FileExists(arg))
    {
    argIsFile = true;
    std::string fullPath = cmSystemTools::CollapseFullPath(arg);
    std::string name = cmSystemTools::GetFilenameName(fullPath.c_str());
    name = cmSystemTools::LowerCase(name);
    if(name == "cmakecache.txt")
      {
      cachePath = cmSystemTools::GetFilenamePath(fullPath.c_str());
      }
    else if(name == "cmakelists.txt")
      {
      listPath = cmSystemTools::GetFilenamePath(fullPath.c_str());
      }
    }
  else
    {
    // Specified file or directory does not exist.  Try to set things
    // up to produce a meaningful error message.
    std::string fullPath = cmSystemTools::CollapseFullPath(arg);
    std::string name = cmSystemTools::GetFilenameName(fullPath.c_str());
    name = cmSystemTools::LowerCase(name);
    if(name == "cmakecache.txt" || name == "cmakelists.txt")
      {
      argIsFile = true;
      listPath = cmSystemTools::GetFilenamePath(fullPath.c_str());
      }
    else
      {
      listPath = fullPath;
      }
    }

  // If there is a CMakeCache.txt file, use its settings.
  if(cachePath.length() > 0)
    {
    cmCacheManager* cachem = this->GetCacheManager();
    cmCacheManager::CacheIterator it = cachem->NewIterator();
    if(cachem->LoadCache(cachePath.c_str()) &&
      it.Find("CMAKE_HOME_DIRECTORY"))
      {
      this->SetHomeOutputDirectory(cachePath.c_str());
      this->SetStartOutputDirectory(cachePath.c_str());
      this->SetHomeDirectory(it.GetValue());
      this->SetStartDirectory(it.GetValue());
      return;
      }
    }

  // If there is a CMakeLists.txt file, use it as the source tree.
  if(listPath.length() > 0)
    {
    this->SetHomeDirectory(listPath.c_str());
    this->SetStartDirectory(listPath.c_str());

    if(argIsFile)
      {
      // Source CMakeLists.txt file given.  It was probably dropped
      // onto the executable in a GUI.  Default to an in-source build.
      this->SetHomeOutputDirectory(listPath.c_str());
      this->SetStartOutputDirectory(listPath.c_str());
      }
    else
      {
      // Source directory given on command line.  Use current working
      // directory as build tree.
      std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
      this->SetHomeOutputDirectory(cwd.c_str());
      this->SetStartOutputDirectory(cwd.c_str());
      }
    return;
    }

  // We didn't find a CMakeLists.txt or CMakeCache.txt file from the
  // argument.  Assume it is the path to the source tree, and use the
  // current working directory as the build tree.
  std::string full = cmSystemTools::CollapseFullPath(arg);
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  this->SetHomeDirectory(full.c_str());
  this->SetStartDirectory(full.c_str());
  this->SetHomeOutputDirectory(cwd.c_str());
  this->SetStartOutputDirectory(cwd.c_str());
}

// at the end of this CMAKE_ROOT and CMAKE_COMMAND should be added to the
// cache
int cmake::AddCMakePaths()
{
  // Find the cmake executable
  std::string cMakeSelf = cmSystemTools::GetExecutableDirectory();
  cMakeSelf = cmSystemTools::GetRealPath(cMakeSelf.c_str());
  cMakeSelf += "/cmake";
  cMakeSelf += cmSystemTools::GetExecutableExtension();
#ifdef __APPLE__
  // on the apple this might be the gui bundle
  if(!cmSystemTools::FileExists(cMakeSelf.c_str()))
    {
    cMakeSelf = cmSystemTools::GetExecutableDirectory();
    cMakeSelf = cmSystemTools::GetRealPath(cMakeSelf.c_str());
    cMakeSelf += "../../../..";
    cMakeSelf = cmSystemTools::GetRealPath(cMakeSelf.c_str());
    cMakeSelf = cmSystemTools::CollapseFullPath(cMakeSelf.c_str());
    cMakeSelf += "/cmake";
    std::cerr << cMakeSelf.c_str() << "\n";
    }
#endif
  if(!cmSystemTools::FileExists(cMakeSelf.c_str()))
    {
    cmSystemTools::Error("CMake executable cannot be found at ",
                         cMakeSelf.c_str());
    return 0;
    }
  // Save the value in the cache
  this->CacheManager->AddCacheEntry
    ("CMAKE_COMMAND",cMakeSelf.c_str(), "Path to CMake executable.",
     cmCacheManager::INTERNAL);
  // if the edit command is not yet in the cache,
  // or if CMakeEditCommand has been set on this object,
  // then set the CMAKE_EDIT_COMMAND in the cache
  // This will mean that the last gui to edit the cache
  // will be the one that make edit_cache uses.
  if(!this->GetCacheDefinition("CMAKE_EDIT_COMMAND")
    || !this->CMakeEditCommand.empty())
    {
    // Find and save the command to edit the cache
    std::string editCacheCommand;
    if(!this->CMakeEditCommand.empty())
      {
      editCacheCommand = cmSystemTools::GetFilenamePath(cMakeSelf)
        + std::string("/")
        + this->CMakeEditCommand
        + cmSystemTools::GetFilenameExtension(cMakeSelf);
      }
    if( !cmSystemTools::FileExists(editCacheCommand.c_str()))
      {
      editCacheCommand = cmSystemTools::GetFilenamePath(cMakeSelf) +
        "/ccmake" + cmSystemTools::GetFilenameExtension(cMakeSelf);
      }
    if( !cmSystemTools::FileExists(editCacheCommand.c_str()))
      {
      editCacheCommand = cmSystemTools::GetFilenamePath(cMakeSelf) +
        "/cmake-gui" + cmSystemTools::GetFilenameExtension(cMakeSelf);
      }
    if(cmSystemTools::FileExists(editCacheCommand.c_str()))
      {
      this->CacheManager->AddCacheEntry
        ("CMAKE_EDIT_COMMAND", editCacheCommand.c_str(),
         "Path to cache edit program executable.", cmCacheManager::INTERNAL);
      }
    }
  std::string ctestCommand = cmSystemTools::GetFilenamePath(cMakeSelf) +
    "/ctest" + cmSystemTools::GetFilenameExtension(cMakeSelf);
  if(cmSystemTools::FileExists(ctestCommand.c_str()))
    {
    this->CacheManager->AddCacheEntry
      ("CMAKE_CTEST_COMMAND", ctestCommand.c_str(),
       "Path to ctest program executable.", cmCacheManager::INTERNAL);
    }
  std::string cpackCommand = cmSystemTools::GetFilenamePath(cMakeSelf) +
    "/cpack" + cmSystemTools::GetFilenameExtension(cMakeSelf);
  if(cmSystemTools::FileExists(cpackCommand.c_str()))
    {
    this->CacheManager->AddCacheEntry
      ("CMAKE_CPACK_COMMAND", cpackCommand.c_str(),
       "Path to cpack program executable.", cmCacheManager::INTERNAL);
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
    cMakeRoot = cmSystemTools::GetRealPath(cMakeSelf.c_str());
    cMakeRoot = cmSystemTools::GetProgramPath(cMakeRoot.c_str());
    std::string::size_type slashPos = cMakeRoot.rfind("/");
    if(slashPos != std::string::npos)
      {
      cMakeRoot = cMakeRoot.substr(0, slashPos);
      }
    // is there no Modules directory there?
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
  if (!cmSystemTools::FileExists(modules.c_str()))
    {
    // try
    cMakeRoot  = cmSystemTools::GetProgramPath(cMakeSelf.c_str());
    cMakeRoot += CMAKE_DATA_DIR;
    modules = cMakeRoot +  "/Modules/CMake.cmake";
    }
  if(!cmSystemTools::FileExists(modules.c_str()))
    {
    // next try exe
    cMakeRoot  = cmSystemTools::GetProgramPath(cMakeSelf.c_str());
    // is there no Modules directory there?
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
  this->CacheManager->AddCacheEntry
    ("CMAKE_ROOT", cMakeRoot.c_str(),
     "Path to CMake installation.", cmCacheManager::INTERNAL);

#ifdef _WIN32
  std::string comspec = "cmw9xcom.exe";
  cmSystemTools::SetWindows9xComspecSubstitute(comspec.c_str());
#endif
  return 1;
}



void CMakeCommandUsage(const char* program)
{
  cmOStringStream errorStream;

#ifdef CMAKE_BUILD_WITH_CMAKE
  errorStream
    << "cmake version " << cmVersion::GetCMakeVersion() << "\n";
#else
  errorStream
    << "cmake bootstrap\n";
#endif
  // If you add new commands, change here,
  // and in cmakemain.cxx in the options table
  errorStream
    << "Usage: " << program << " -E [command] [arguments ...]\n"
    << "Available commands: \n"
    << "  chdir dir cmd [args]...   - run command in a given directory\n"
    << "  compare_files file1 file2 - check if file1 is same as file2\n"
    << "  copy file destination     - copy file to destination (either file "
       "or directory)\n"
    << "  copy_directory source destination   - copy directory 'source' "
       "content to directory 'destination'\n"
    << "  copy_if_different in-file out-file  - copy file if input has "
       "changed\n"
    << "  echo [string]...          - displays arguments as text\n"
    << "  echo_append [string]...   - displays arguments as text but no new "
       "line\n"
    << "  environment               - display the current environment\n"
    << "  make_directory dir        - create a directory\n"
    << "  md5sum file1 [...]        - compute md5sum of files\n"
    << "  remove [-f] file1 file2 ... - remove the file(s), use -f to force "
       "it\n"
    << "  remove_directory dir      - remove a directory and its contents\n"
    << "  rename oldname newname    - rename a file or directory "
       "(on one volume)\n"
    << "  tar [cxt][vfz][cvfj] file.tar [file/dir1 file/dir2 ...]\n"
    << "                            - create or extract a tar or zip archive\n"
    << "  time command [args] ...   - run command and return elapsed time\n"
    << "  touch file                - touch a file.\n"
    << "  touch_nocreate file       - touch a file but do not create it.\n"
#if defined(_WIN32) && !defined(__CYGWIN__)
    << "Available on Windows only:\n"
    << "  comspec                   - on windows 9x use this for RunCommand\n"
    << "  delete_regv key           - delete registry value\n"
    << "  env_vs8_wince sdkname     - displays a batch file which sets the "
       "environment for the provided Windows CE SDK installed in VS2005\n"
    << "  env_vs9_wince sdkname     - displays a batch file which sets the "
       "environment for the provided Windows CE SDK installed in VS2008\n"
    << "  write_regv key value      - write registry value\n"
#else
    << "Available on UNIX only:\n"
    << "  create_symlink old new    - create a symbolic link new -> old\n"
#endif
    ;

  cmSystemTools::Error(errorStream.str().c_str());
}

int cmake::ExecuteCMakeCommand(std::vector<std::string>& args)
{
  // IF YOU ADD A NEW COMMAND, DOCUMENT IT ABOVE and in cmakemain.cxx
  if (args.size() > 1)
    {
    // Copy file
    if (args[1] == "copy" && args.size() == 4)
      {
      if(!cmSystemTools::cmCopyFile(args[2].c_str(), args[3].c_str()))
        {
        std::cerr << "Error copying file \"" << args[2].c_str()
                  << "\" to \"" << args[3].c_str() << "\".\n";
        return 1;
        }
      return 0;
      }

    // Copy file if different.
    if (args[1] == "copy_if_different" && args.size() == 4)
      {
      if(!cmSystemTools::CopyFileIfDifferent(args[2].c_str(),
          args[3].c_str()))
        {
        std::cerr << "Error copying file (if different) from \""
                  << args[2].c_str() << "\" to \"" << args[3].c_str()
                  << "\".\n";
        return 1;
        }
      return 0;
      }

    // Copy directory content
    if (args[1] == "copy_directory" && args.size() == 4)
      {
      if(!cmSystemTools::CopyADirectory(args[2].c_str(), args[3].c_str()))
        {
        std::cerr << "Error copying directory from \""
                  << args[2].c_str() << "\" to \"" << args[3].c_str()
                  << "\".\n";
        return 1;
        }
      return 0;
      }

    // Rename a file or directory
    if (args[1] == "rename" && args.size() == 4)
      {
      if(!cmSystemTools::RenameFile(args[2].c_str(), args[3].c_str()))
        {
        std::string e = cmSystemTools::GetLastSystemError();
        std::cerr << "Error renaming from \""
                  << args[2].c_str() << "\" to \"" << args[3].c_str()
                  << "\": " << e << "\n";
        return 1;
        }
      return 0;
      }

    // Compare files
    if (args[1] == "compare_files" && args.size() == 4)
      {
      if(cmSystemTools::FilesDiffer(args[2].c_str(), args[3].c_str()))
        {
        std::cerr << "Files \""
                  << args[2].c_str() << "\" to \"" << args[3].c_str()
                  << "\" are different.\n";
        return 1;
        }
      return 0;
      }

    // Echo string
    else if (args[1] == "echo" )
      {
      unsigned int cc;
      const char* space = "";
      for ( cc = 2; cc < args.size(); cc ++ )
        {
        std::cout << space << args[cc];
        space = " ";
        }
      std::cout << std::endl;
      return 0;
      }

    // Echo string no new line
    else if (args[1] == "echo_append" )
      {
      unsigned int cc;
      const char* space = "";
      for ( cc = 2; cc < args.size(); cc ++ )
        {
        std::cout << space << args[cc];
        space = " ";
        }
      return 0;
      }

#if defined(CMAKE_BUILD_WITH_CMAKE)
    // Command to create a symbolic link.  Fails on platforms not
    // supporting them.
    else if (args[1] == "environment" )
      {
      std::vector<std::string> env = cmSystemTools::GetEnvironmentVariables();
      std::vector<std::string>::iterator it;
      for ( it = env.begin(); it != env.end(); ++ it )
        {
        std::cout << it->c_str() << std::endl;
        }
      return 0;
      }
#endif

    else if (args[1] == "make_directory" && args.size() == 3)
      {
      if(!cmSystemTools::MakeDirectory(args[2].c_str()))
        {
        std::cerr << "Error making directory \"" << args[2].c_str()
                  << "\".\n";
        return 1;
        }
      return 0;
      }

    else if (args[1] == "remove_directory" && args.size() == 3)
      {
      if(cmSystemTools::FileIsDirectory(args[2].c_str()) &&
         !cmSystemTools::RemoveADirectory(args[2].c_str()))
        {
        std::cerr << "Error removing directory \"" << args[2].c_str()
                  << "\".\n";
        return 1;
        }
      return 0;
      }

    // Remove file
    else if (args[1] == "remove" && args.size() > 2)
      {
      bool force = false;
      for (std::string::size_type cc = 2; cc < args.size(); cc ++)
        {
        if(args[cc] == "\\-f" || args[cc] == "-f")
          {
          force = true;
          }
        else
          {
          // Complain if the file could not be removed, still exists,
          // and the -f option was not given.
          if(!cmSystemTools::RemoveFile(args[cc].c_str()) && !force &&
             cmSystemTools::FileExists(args[cc].c_str()))
            {
            return 1;
            }
          }
        }
      return 0;
      }
    // Touch file
    else if (args[1] == "touch" && args.size() > 2)
      {
      for (std::string::size_type cc = 2; cc < args.size(); cc ++)
        {
        // Complain if the file could not be removed, still exists,
        // and the -f option was not given.
        if(!cmSystemTools::Touch(args[cc].c_str(), true))
          {
          return 1;
          }
        }
      return 0;
      }
    // Touch file
    else if (args[1] == "touch_nocreate" && args.size() > 2)
      {
      for (std::string::size_type cc = 2; cc < args.size(); cc ++)
        {
        // Complain if the file could not be removed, still exists,
        // and the -f option was not given.
        if(!cmSystemTools::Touch(args[cc].c_str(), false))
          {
          return 1;
          }
        }
      return 0;
      }

    // Clock command
    else if (args[1] == "time" && args.size() > 2)
      {
      std::string command = args[2];
      for (std::string::size_type cc = 3; cc < args.size(); cc ++)
        {
        command += " ";
        command += args[cc];
        }

      clock_t clock_start, clock_finish;
      time_t time_start, time_finish;

      time(&time_start);
      clock_start = clock();
      int ret =0;
      cmSystemTools::RunSingleCommand(command.c_str(), 0, &ret);

      clock_finish = clock();
      time(&time_finish);

      double clocks_per_sec = static_cast<double>(CLOCKS_PER_SEC);
      std::cout << "Elapsed time: "
        << static_cast<long>(time_finish - time_start) << " s. (time)"
        << ", "
        << static_cast<double>(clock_finish - clock_start) / clocks_per_sec
        << " s. (clock)"
        << "\n";
      return ret;
      }
    // Command to calculate the md5sum of a file
    else if (args[1] == "md5sum" && args.size() >= 3)
      {
      char md5out[32];
      int retval = 0;
      for (std::string::size_type cc = 2; cc < args.size(); cc ++)
        {
        const char *filename = args[cc].c_str();
        // Cannot compute md5sum of a directory
        if(cmSystemTools::FileIsDirectory(filename))
          {
          std::cerr << "Error: " << filename << " is a directory" << std::endl;
          retval++;
          }
        else if(!cmSystemTools::ComputeFileMD5(filename, md5out))
          {
          // To mimic md5sum behavior in a shell:
          std::cerr << filename << ": No such file or directory" << std::endl;
          retval++;
          }
        else
          {
          std::cout << std::string(md5out,32) << "  " << filename << std::endl;
          }
        }
      return retval;
      }

    // Command to change directory and run a program.
    else if (args[1] == "chdir" && args.size() >= 4)
      {
      std::string directory = args[2];
      if(!cmSystemTools::FileExists(directory.c_str()))
        {
        cmSystemTools::Error("Directory does not exist for chdir command: ",
                             args[2].c_str());
        return 1;
        }

      std::string command = "\"";
      command += args[3];
      command += "\"";
      for (std::string::size_type cc = 4; cc < args.size(); cc ++)
        {
        command += " \"";
        command += args[cc];
        command += "\"";
        }
      int retval = 0;
      int timeout = 0;
      if ( cmSystemTools::RunSingleCommand(command.c_str(), 0, &retval,
             directory.c_str(), cmSystemTools::OUTPUT_NORMAL, timeout) )
        {
        return retval;
        }

      return 1;
      }

    // Command to start progress for a build
    else if (args[1] == "cmake_progress_start" && args.size() == 4)
      {
      // basically remove the directory
      std::string dirName = args[2];
      dirName += "/Progress";
      cmSystemTools::RemoveADirectory(dirName.c_str());

      // is the last argument a filename that exists?
      FILE *countFile = fopen(args[3].c_str(),"r");
      int count;
      if (countFile)
        {
        if (1!=fscanf(countFile,"%i",&count))
          {
          cmSystemTools::Message("Could not read from count file.");
          }
        fclose(countFile);
        }
      else
        {
        count = atoi(args[3].c_str());
        }
      if (count)
        {
        cmSystemTools::MakeDirectory(dirName.c_str());
        // write the count into the directory
        std::string fName = dirName;
        fName += "/count.txt";
        FILE *progFile = fopen(fName.c_str(),"w");
        if (progFile)
          {
          fprintf(progFile,"%i\n",count);
          fclose(progFile);
          }
        }
      return 0;
      }

    // Command to report progress for a build
    else if (args[1] == "cmake_progress_report" && args.size() >= 3)
      {
      std::string dirName = args[2];
      dirName += "/Progress";
      std::string fName;
      FILE *progFile;

      // read the count
      fName = dirName;
      fName += "/count.txt";
      progFile = fopen(fName.c_str(),"r");
      int count = 0;
      if (!progFile)
        {
        return 0;
        }
      else
        {
        if (1!=fscanf(progFile,"%i",&count))
          {
          cmSystemTools::Message("Could not read from progress file.");
          }
        fclose(progFile);
        }
      unsigned int i;
      for (i = 3; i < args.size(); ++i)
        {
        fName = dirName;
        fName += "/";
        fName += args[i];
        progFile = fopen(fName.c_str(),"w");
        if (progFile)
          {
          fprintf(progFile,"empty");
          fclose(progFile);
          }
        }
      int fileNum = static_cast<int>
        (cmsys::Directory::GetNumberOfFilesInDirectory(dirName.c_str()));
      if (count > 0)
        {
        // print the progress
        fprintf(stdout,"[%3i%%] ",((fileNum-3)*100)/count);
        }
      return 0;
      }

    // Command to create a symbolic link.  Fails on platforms not
    // supporting them.
    else if (args[1] == "create_symlink" && args.size() == 4)
      {
      const char* destinationFileName = args[3].c_str();
      if ( cmSystemTools::FileExists(destinationFileName) )
        {
        if ( cmSystemTools::FileIsSymlink(destinationFileName) )
          {
          if ( !cmSystemTools::RemoveFile(destinationFileName) ||
            cmSystemTools::FileExists(destinationFileName) )
            {
            return 0;
            }
          }
        else
          {
          return 0;
          }
        }
      return cmSystemTools::CreateSymlink(args[2].c_str(),
                                          args[3].c_str())? 0:1;
      }

    // Internal CMake shared library support.
    else if (args[1] == "cmake_symlink_library" && args.size() == 5)
      {
      return cmake::SymlinkLibrary(args);
      }
    // Internal CMake versioned executable support.
    else if (args[1] == "cmake_symlink_executable" && args.size() == 4)
      {
      return cmake::SymlinkExecutable(args);
      }

#if defined(CMAKE_HAVE_VS_GENERATORS)
    // Internal CMake support for calling Visual Studio macros.
    else if (args[1] == "cmake_call_visual_studio_macro" && args.size() >= 4)
      {
      // args[2] = full path to .sln file or "ALL"
      // args[3] = name of Visual Studio macro to call
      // args[4..args.size()-1] = [optional] args for Visual Studio macro

      std::string macroArgs;

      if (args.size() > 4)
        {
        macroArgs = args[4];

        for (size_t i = 5; i < args.size(); ++i)
          {
          macroArgs += " ";
          macroArgs += args[i];
          }
        }

      return cmCallVisualStudioMacro::CallMacro(args[2], args[3],
        macroArgs, true);
      }
#endif

    // Internal CMake dependency scanning support.
    else if (args[1] == "cmake_depends" && args.size() >= 6)
      {
      // Use the make system's VERBOSE environment variable to enable
      // verbose output. This can be skipped by also setting CMAKE_NO_VERBOSE
      // (which is set by the Eclipse and KDevelop generators).
      bool verbose = ((cmSystemTools::GetEnv("VERBOSE") != 0)
                       && (cmSystemTools::GetEnv("CMAKE_NO_VERBOSE") == 0));

      // Create a cmake object instance to process dependencies.
      cmake cm;
      std::string gen;
      std::string homeDir;
      std::string startDir;
      std::string homeOutDir;
      std::string startOutDir;
      std::string depInfo;
      bool color = false;
      if(args.size() >= 8)
        {
        // Full signature:
        //
        //   -E cmake_depends <generator>
        //                    <home-src-dir> <start-src-dir>
        //                    <home-out-dir> <start-out-dir>
        //                    <dep-info> [--color=$(COLOR)]
        //
        // All paths are provided.
        gen = args[2];
        homeDir = args[3];
        startDir = args[4];
        homeOutDir = args[5];
        startOutDir = args[6];
        depInfo = args[7];
        if(args.size() >= 9 &&
           args[8].length() >= 8 &&
           args[8].substr(0, 8) == "--color=")
          {
          // Enable or disable color based on the switch value.
          color = (args[8].size() == 8 ||
                   cmSystemTools::IsOn(args[8].substr(8).c_str()));
          }
        }
      else
        {
        // Support older signature for existing makefiles:
        //
        //   -E cmake_depends <generator>
        //                    <home-out-dir> <start-out-dir>
        //                    <dep-info>
        //
        // Just pretend the source directories are the same as the
        // binary directories so at least scanning will work.
        gen = args[2];
        homeDir = args[3];
        startDir = args[4];
        homeOutDir = args[3];
        startOutDir = args[3];
        depInfo = args[5];
        }

      // Create a local generator configured for the directory in
      // which dependencies will be scanned.
      homeDir = cmSystemTools::CollapseFullPath(homeDir.c_str());
      startDir = cmSystemTools::CollapseFullPath(startDir.c_str());
      homeOutDir = cmSystemTools::CollapseFullPath(homeOutDir.c_str());
      startOutDir = cmSystemTools::CollapseFullPath(startOutDir.c_str());
      cm.SetHomeDirectory(homeDir.c_str());
      cm.SetStartDirectory(startDir.c_str());
      cm.SetHomeOutputDirectory(homeOutDir.c_str());
      cm.SetStartOutputDirectory(startOutDir.c_str());
      if(cmGlobalGenerator* ggd = cm.CreateGlobalGenerator(gen.c_str()))
        {
        cm.SetGlobalGenerator(ggd);
        cmsys::auto_ptr<cmLocalGenerator> lgd(ggd->CreateLocalGenerator());
        lgd->GetMakefile()->SetStartDirectory(startDir.c_str());
        lgd->GetMakefile()->SetStartOutputDirectory(startOutDir.c_str());
        lgd->GetMakefile()->MakeStartDirectoriesCurrent();

        // Actually scan dependencies.
        return lgd->UpdateDependencies(depInfo.c_str(),
                                       verbose, color)? 0 : 2;
        }
      return 1;
      }

    // Internal CMake link script support.
    else if (args[1] == "cmake_link_script" && args.size() >= 3)
      {
      return cmake::ExecuteLinkScript(args);
      }

    // Internal CMake unimplemented feature notification.
    else if (args[1] == "cmake_unimplemented_variable")
      {
      std::cerr << "Feature not implemented for this platform.";
      if(args.size() == 3)
        {
        std::cerr << "  Variable " << args[2] << " is not set.";
        }
      std::cerr << std::endl;
      return 1;
      }
    else if (args[1] == "vs_link_exe")
      {
      return cmake::VisualStudioLink(args, 1);
      }
    else if (args[1] == "vs_link_dll")
      {
      return cmake::VisualStudioLink(args, 2);
      }
#ifdef CMAKE_BUILD_WITH_CMAKE
    // Internal CMake color makefile support.
    else if (args[1] == "cmake_echo_color")
      {
      return cmake::ExecuteEchoColor(args);
      }
    else if (args[1] == "cmake_automoc")
      {
        cmQtAutomoc automoc;
        const char *config = args[3].empty() ? 0 : args[3].c_str();
        bool automocSuccess = automoc.Run(args[2].c_str(), config);
        return automocSuccess ? 0 : 1;
      }
#endif

    // Tar files
    else if (args[1] == "tar" && args.size() > 3)
      {
      std::string flags = args[2];
      std::string outFile = args[3];
      std::vector<cmStdString> files;
      for (std::string::size_type cc = 4; cc < args.size(); cc ++)
        {
        files.push_back(args[cc]);
        }
      bool gzip = false;
      bool bzip2 = false;
      bool verbose = false;
      if ( flags.find_first_of('j') != flags.npos )
        {
        bzip2 = true;
        }
      if ( flags.find_first_of('z') != flags.npos )
        {
        gzip = true;
        }
      if ( flags.find_first_of('v') != flags.npos )
        {
        verbose = true;
        }

      if ( flags.find_first_of('t') != flags.npos )
        {
        if ( !cmSystemTools::ListTar(outFile.c_str(), gzip, verbose) )
          {
          cmSystemTools::Error("Problem creating tar: ", outFile.c_str());
          return 1;
          }
        }
      else if ( flags.find_first_of('c') != flags.npos )
        {
        if ( !cmSystemTools::CreateTar(
               outFile.c_str(), files, gzip, bzip2, verbose) )
          {
          cmSystemTools::Error("Problem creating tar: ", outFile.c_str());
          return 1;
          }
        }
      else if ( flags.find_first_of('x') != flags.npos )
        {
        if ( !cmSystemTools::ExtractTar(
            outFile.c_str(), gzip, verbose) )
          {
          cmSystemTools::Error("Problem extracting tar: ", outFile.c_str());
          return 1;
          }
#ifdef WIN32
        // OK, on windows 7 after we untar some files,
        // sometimes we can not rename the directory after
        // the untar is done. This breaks the external project
        // untar and rename code.  So, by default we will wait
        // 1/10th of a second after the untar.  If CMAKE_UNTAR_DELAY
        // is set in the env, its value will be used instead of 100.
        int delay = 100;
        const char* delayVar = cmSystemTools::GetEnv("CMAKE_UNTAR_DELAY");
        if(delayVar)
          {
          delay = atoi(delayVar);
          }
        if(delay)
          {
          cmSystemTools::Delay(delay);
          }
#endif
        }
      return 0;
      }

#if defined(CMAKE_BUILD_WITH_CMAKE)
    // Internal CMake Fortran module support.
    else if (args[1] == "cmake_copy_f90_mod" && args.size() >= 4)
      {
      return cmDependsFortran::CopyModule(args)? 0 : 1;
      }
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
    // Write registry value
    else if (args[1] == "write_regv" && args.size() > 3)
      {
      return cmSystemTools::WriteRegistryValue(args[2].c_str(),
                                               args[3].c_str()) ? 0 : 1;
      }

    // Delete registry value
    else if (args[1] == "delete_regv" && args.size() > 2)
      {
      return cmSystemTools::DeleteRegistryValue(args[2].c_str()) ? 0 : 1;
      }
    // Remove file
    else if (args[1] == "comspec" && args.size() > 2)
      {
      unsigned int cc;
      std::string command = args[2];
      for ( cc = 3; cc < args.size(); cc ++ )
        {
        command += " " + args[cc];
        }
      return cmWin32ProcessExecution::Windows9xHack(command.c_str());
      }
    else if (args[1] == "env_vs8_wince" && args.size() == 3)
      {
      return cmake::WindowsCEEnvironment("8.0", args[2]);
      }
    else if (args[1] == "env_vs9_wince" && args.size() == 3)
      {
      return cmake::WindowsCEEnvironment("9.0", args[2]);
      }
#endif
    }

  ::CMakeCommandUsage(args[0].c_str());
  return 1;
}

void cmake::AddExtraGenerator(const char* name,
                              CreateExtraGeneratorFunctionType newFunction)
{
  cmExternalMakefileProjectGenerator* extraGenerator = newFunction();
  const std::vector<std::string>& supportedGlobalGenerators =
                                extraGenerator->GetSupportedGlobalGenerators();

  for(std::vector<std::string>::const_iterator
      it = supportedGlobalGenerators.begin();
      it != supportedGlobalGenerators.end();
      ++it )
    {
    std::string fullName = cmExternalMakefileProjectGenerator::
                                    CreateFullGeneratorName(it->c_str(), name);
    this->ExtraGenerators[fullName.c_str()] = newFunction;
    }
  delete extraGenerator;
}

void cmake::AddDefaultExtraGenerators()
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
#if defined(_WIN32) && !defined(__CYGWIN__)
  // e.g. kdevelop4 ?
#endif

  this->AddExtraGenerator(cmExtraCodeBlocksGenerator::GetActualName(),
                          &cmExtraCodeBlocksGenerator::New);
  this->AddExtraGenerator(cmExtraSublimeTextGenerator::GetActualName(),
                          &cmExtraSublimeTextGenerator::New);

#ifdef CMAKE_USE_ECLIPSE
  this->AddExtraGenerator(cmExtraEclipseCDT4Generator::GetActualName(),
                          &cmExtraEclipseCDT4Generator::New);
#endif

#ifdef CMAKE_USE_KDEVELOP
  this->AddExtraGenerator(cmGlobalKdevelopGenerator::GetActualName(),
                          &cmGlobalKdevelopGenerator::New);
  // for kdevelop also add the generator with just the name of the
  // extra generator, since it was this way since cmake 2.2
  this->ExtraGenerators[cmGlobalKdevelopGenerator::GetActualName()]
                                             = &cmGlobalKdevelopGenerator::New;
#endif

#endif
}


//----------------------------------------------------------------------------
void cmake::GetRegisteredGenerators(std::vector<std::string>& names)
{
  for(RegisteredGeneratorsVector::const_iterator i = this->Generators.begin();
      i != this->Generators.end(); ++i)
    {
    (*i)->GetGenerators(names);
    }
  for(RegisteredExtraGeneratorsMap::const_iterator
      i = this->ExtraGenerators.begin();
      i != this->ExtraGenerators.end(); ++i)
    {
    names.push_back(i->first);
    }
}

cmGlobalGenerator* cmake::CreateGlobalGenerator(const char* name)
{
  cmExternalMakefileProjectGenerator* extraGenerator = 0;
  RegisteredExtraGeneratorsMap::const_iterator extraGenIt =
                                            this->ExtraGenerators.find(name);
  if (extraGenIt != this->ExtraGenerators.end())
    {
    extraGenerator = (extraGenIt->second)();
    name = extraGenerator->GetGlobalGeneratorName(name);
    }

  cmGlobalGenerator* generator = 0;
  for (RegisteredGeneratorsVector::const_iterator i =
    this->Generators.begin(); i != this->Generators.end(); ++i)
    {
    generator = (*i)->CreateGlobalGenerator(name);
    if (generator)
      {
      break;
      }
    }

  if (generator)
    {
    generator->SetCMakeInstance(this);
    generator->SetExternalMakefileProjectGenerator(extraGenerator);
    }
  else
    {
    delete extraGenerator;
    }

  return generator;
}

void cmake::SetHomeDirectory(const char* dir)
{
  this->cmHomeDirectory = dir;
  cmSystemTools::ConvertToUnixSlashes(this->cmHomeDirectory);
}

void cmake::SetHomeOutputDirectory(const char* lib)
{
  this->HomeOutputDirectory = lib;
  cmSystemTools::ConvertToUnixSlashes(this->HomeOutputDirectory);
}

void cmake::SetGlobalGenerator(cmGlobalGenerator *gg)
{
  if(!gg)
    {
    cmSystemTools::Error("Error SetGlobalGenerator called with null");
    return;
    }
  // delete the old generator
  if (this->GlobalGenerator)
    {
    delete this->GlobalGenerator;
    // restore the original environment variables CXX and CC
    // Restore CC
    std::string env = "CC=";
    if(this->CCEnvironment.size())
      {
      env += this->CCEnvironment;
      }
    cmSystemTools::PutEnv(env.c_str());
    env = "CXX=";
    if(this->CXXEnvironment.size())
      {
      env += this->CXXEnvironment;
      }
    cmSystemTools::PutEnv(env.c_str());
    }

  // set the new
  this->GlobalGenerator = gg;

  // set the global flag for unix style paths on cmSystemTools as soon as
  // the generator is set.  This allows gmake to be used on windows.
  cmSystemTools::SetForceUnixPaths
    (this->GlobalGenerator->GetForceUnixPaths());

  // Save the environment variables CXX and CC
  const char* cxx = getenv("CXX");
  const char* cc = getenv("CC");
  if(cxx)
    {
    this->CXXEnvironment = cxx;
    }
  else
    {
    this->CXXEnvironment = "";
    }
  if(cc)
    {
    this->CCEnvironment = cc;
    }
  else
    {
    this->CCEnvironment = "";
    }
  // set the cmake instance just to be sure
  gg->SetCMakeInstance(this);
}

int cmake::DoPreConfigureChecks()
{
  // Make sure the Start directory contains a CMakeLists.txt file.
  std::string srcList = this->GetHomeDirectory();
  srcList += "/CMakeLists.txt";
  if(!cmSystemTools::FileExists(srcList.c_str()))
    {
    cmOStringStream err;
    if(cmSystemTools::FileIsDirectory(this->GetHomeDirectory()))
      {
      err << "The source directory \"" << this->GetHomeDirectory()
          << "\" does not appear to contain CMakeLists.txt.\n";
      }
    else if(cmSystemTools::FileExists(this->GetHomeDirectory()))
      {
      err << "The source directory \"" << this->GetHomeDirectory()
          << "\" is a file, not a directory.\n";
      }
    else
      {
      err << "The source directory \"" << this->GetHomeDirectory()
          << "\" does not exist.\n";
      }
    err << "Specify --help for usage, or press the help button on the CMake "
      "GUI.";
    cmSystemTools::Error(err.str().c_str());
    return -2;
    }

  // do a sanity check on some values
  if(this->CacheManager->GetCacheValue("CMAKE_HOME_DIRECTORY"))
    {
    std::string cacheStart =
      this->CacheManager->GetCacheValue("CMAKE_HOME_DIRECTORY");
    cacheStart += "/CMakeLists.txt";
    std::string currentStart = this->GetHomeDirectory();
    currentStart += "/CMakeLists.txt";
    if(!cmSystemTools::SameFile(cacheStart.c_str(), currentStart.c_str()))
      {
      std::string message = "The source \"";
      message += currentStart;
      message += "\" does not match the source \"";
      message += cacheStart;
      message += "\" used to generate cache.  ";
      message += "Re-run cmake with a different source directory.";
      cmSystemTools::Error(message.c_str());
      return -2;
      }
    }
  else
    {
    return 0;
    }
  return 1;
}
struct SaveCacheEntry
{
  std::string key;
  std::string value;
  std::string help;
  cmCacheManager::CacheEntryType type;
};

int cmake::HandleDeleteCacheVariables(const char* var)
{
  std::vector<std::string> argsSplit;
  cmSystemTools::ExpandListArgument(std::string(var), argsSplit, true);
  // erase the property to avoid infinite recursion
  this->SetProperty("__CMAKE_DELETE_CACHE_CHANGE_VARS_", "");
  if(this->GetIsInTryCompile())
    {
    return 0;
    }
  cmCacheManager::CacheIterator ci = this->CacheManager->NewIterator();
  std::vector<SaveCacheEntry> saved;
  cmOStringStream warning;
  warning
    << "You have changed variables that require your cache to be deleted.\n"
    << "Configure will be re-run and you may have to reset some variables.\n"
    << "The following variables have changed:\n";
  for(std::vector<std::string>::iterator i = argsSplit.begin();
      i != argsSplit.end(); ++i)
    {
    SaveCacheEntry save;
    save.key = *i;
    warning << *i << "= ";
    i++;
    save.value = *i;
    warning << *i << "\n";
    if(ci.Find(save.key.c_str()))
      {
      save.type = ci.GetType();
      save.help = ci.GetProperty("HELPSTRING");
      }
    saved.push_back(save);
    }

  // remove the cache
  this->CacheManager->DeleteCache(this->GetStartOutputDirectory());
  // load the empty cache
  this->LoadCache();
  // restore the changed compilers
  for(std::vector<SaveCacheEntry>::iterator i = saved.begin();
      i != saved.end(); ++i)
    {
    this->AddCacheEntry(i->key.c_str(), i->value.c_str(),
                        i->help.c_str(), i->type);
    }
  cmSystemTools::Message(warning.str().c_str());
  // avoid reconfigure if there were errors
  if(!cmSystemTools::GetErrorOccuredFlag())
    {
    // re-run configure
    return this->Configure();
    }
  return 0;
}

int cmake::Configure()
{
  if(this->DoSuppressDevWarnings)
    {
    if(this->SuppressDevWarnings)
      {
      this->CacheManager->
        AddCacheEntry("CMAKE_SUPPRESS_DEVELOPER_WARNINGS", "TRUE",
                      "Suppress Warnings that are meant for"
                      " the author of the CMakeLists.txt files.",
                      cmCacheManager::INTERNAL);
      }
    else
      {
      this->CacheManager->
        AddCacheEntry("CMAKE_SUPPRESS_DEVELOPER_WARNINGS", "FALSE",
                      "Suppress Warnings that are meant for"
                      " the author of the CMakeLists.txt files.",
                      cmCacheManager::INTERNAL);
      }
    }
  int ret = this->ActualConfigure();
  const char* delCacheVars =
    this->GetProperty("__CMAKE_DELETE_CACHE_CHANGE_VARS_");
  if(delCacheVars && delCacheVars[0] != 0)
    {
    return this->HandleDeleteCacheVariables(delCacheVars);
    }
  return ret;

}

int cmake::ActualConfigure()
{
  // Construct right now our path conversion table before it's too late:
  this->UpdateConversionPathTable();
  this->CleanupCommandsAndMacros();

  int res = 0;
  if ( this->GetWorkingMode() == NORMAL_MODE )
    {
    res = this->DoPreConfigureChecks();
    }
  if ( res < 0 )
    {
    return -2;
    }
  if ( !res )
    {
    this->CacheManager->AddCacheEntry
      ("CMAKE_HOME_DIRECTORY",
       this->GetHomeDirectory(),
       "Start directory with the top level CMakeLists.txt file for this "
       "project",
       cmCacheManager::INTERNAL);
    }

  // no generator specified on the command line
  if(!this->GlobalGenerator)
    {
    const char* genName =
      this->CacheManager->GetCacheValue("CMAKE_GENERATOR");
    const char* extraGenName =
      this->CacheManager->GetCacheValue("CMAKE_EXTRA_GENERATOR");
    if(genName)
      {
      std::string fullName = cmExternalMakefileProjectGenerator::
                                CreateFullGeneratorName(genName, extraGenName);
      this->GlobalGenerator = this->CreateGlobalGenerator(fullName.c_str());
      }
    if(this->GlobalGenerator)
      {
      // set the global flag for unix style paths on cmSystemTools as
      // soon as the generator is set.  This allows gmake to be used
      // on windows.
      cmSystemTools::SetForceUnixPaths
        (this->GlobalGenerator->GetForceUnixPaths());
      }
    else
      {
#if defined(__BORLANDC__) && defined(_WIN32)
      this->SetGlobalGenerator(new cmGlobalBorlandMakefileGenerator);
#elif defined(_WIN32) && !defined(__CYGWIN__) && !defined(CMAKE_BOOT_MINGW)
      std::string installedCompiler;
      // Try to find the newest VS installed on the computer and
      // use that as a default if -G is not specified
      const std::string vsregBase =
        "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\";
      std::vector<std::string> vsVerions;
      vsVerions.push_back("VisualStudio\\");
      vsVerions.push_back("VCExpress\\");
      vsVerions.push_back("WDExpress\\");
      struct VSRegistryEntryName
      {
        const char* MSVersion;
        const char* GeneratorName;
      };
      VSRegistryEntryName version[] = {
        {"6.0", "Visual Studio 6"},
        {"7.0", "Visual Studio 7"},
        {"7.1", "Visual Studio 7 .NET 2003"},
        {"8.0", "Visual Studio 8 2005"},
        {"9.0", "Visual Studio 9 2008"},
        {"10.0", "Visual Studio 10"},
        {"11.0", "Visual Studio 11"},
        {"12.0", "Visual Studio 12"},
        {0, 0}};
      for(int i=0; version[i].MSVersion != 0; i++)
        {
        for(size_t b=0; b < vsVerions.size(); b++)
          {
          std::string reg = vsregBase + vsVerions[b] + version[i].MSVersion;
          reg += ";InstallDir]";
          cmSystemTools::ExpandRegistryValues(reg,
                                              cmSystemTools::KeyWOW64_32);
          if (!(reg == "/registry"))
            {
            installedCompiler = version[i].GeneratorName;
            break;
            }
          }
        }
      cmGlobalGenerator* gen
        = this->CreateGlobalGenerator(installedCompiler.c_str());
      if(!gen)
        {
        gen = new cmGlobalNMakeMakefileGenerator;
        }
      this->SetGlobalGenerator(gen);
      std::cout << "-- Building for: " << gen->GetName() << "\n";
#else
      this->SetGlobalGenerator(new cmGlobalUnixMakefileGenerator3);
#endif
      }
    if(!this->GlobalGenerator)
      {
      cmSystemTools::Error("Could not create generator");
      return -1;
      }
    }

  const char* genName = this->CacheManager->GetCacheValue("CMAKE_GENERATOR");
  if(genName)
    {
    if(strcmp(this->GlobalGenerator->GetName(), genName) != 0)
      {
      std::string message = "Error: generator : ";
      message += this->GlobalGenerator->GetName();
      message += "\nDoes not match the generator used previously: ";
      message += genName;
      message +=
        "\nEither remove the CMakeCache.txt file or choose a different"
        " binary directory.";
      cmSystemTools::Error(message.c_str());
      return -2;
      }
    }
  if(!this->CacheManager->GetCacheValue("CMAKE_GENERATOR"))
    {
    this->CacheManager->AddCacheEntry("CMAKE_GENERATOR",
                                      this->GlobalGenerator->GetName(),
                                      "Name of generator.",
                                      cmCacheManager::INTERNAL);
    this->CacheManager->AddCacheEntry("CMAKE_EXTRA_GENERATOR",
                                this->GlobalGenerator->GetExtraGeneratorName(),
                                "Name of external makefile project generator.",
                                cmCacheManager::INTERNAL);
    }

  if(const char* tsName =
     this->CacheManager->GetCacheValue("CMAKE_GENERATOR_TOOLSET"))
    {
    if(this->GeneratorToolset.empty())
      {
      this->GeneratorToolset = tsName;
      }
    else if(this->GeneratorToolset != tsName)
      {
      std::string message = "Error: generator toolset: ";
      message += this->GeneratorToolset;
      message += "\nDoes not match the toolset used previously: ";
      message += tsName;
      message +=
        "\nEither remove the CMakeCache.txt file or choose a different"
        " binary directory.";
      cmSystemTools::Error(message.c_str());
      return -2;
      }
    }
  else
    {
    this->CacheManager->AddCacheEntry("CMAKE_GENERATOR_TOOLSET",
                                      this->GeneratorToolset.c_str(),
                                      "Name of generator toolset.",
                                      cmCacheManager::INTERNAL);
    }
  if(!this->GeneratorToolset.empty() &&
     !this->GlobalGenerator->SetGeneratorToolset(this->GeneratorToolset))
    {
    return -2;
    }

  // reset any system configuration information, except for when we are
  // InTryCompile. With TryCompile the system info is taken from the parent's
  // info to save time
  if (!this->InTryCompile)
    {
    this->GlobalGenerator->ClearEnabledLanguages();
    }

  // Truncate log files
  if (!this->InTryCompile)
    {
    this->TruncateOutputLog("CMakeOutput.log");
    this->TruncateOutputLog("CMakeError.log");
    }

  // actually do the configure
  this->GlobalGenerator->Configure();
  // Before saving the cache
  // if the project did not define one of the entries below, add them now
  // so users can edit the values in the cache:

  // We used to always present LIBRARY_OUTPUT_PATH and
  // EXECUTABLE_OUTPUT_PATH.  They are now documented as old-style and
  // should no longer be used.  Therefore we present them only if the
  // project requires compatibility with CMake 2.4.  We detect this
  // here by looking for the old CMAKE_BACKWARDS_COMPATIBILITY
  // variable created when CMP0001 is not set to NEW.
  if(this->GetCacheManager()->GetCacheValue("CMAKE_BACKWARDS_COMPATIBILITY"))
    {
    if(!this->CacheManager->GetCacheValue("LIBRARY_OUTPUT_PATH"))
      {
      this->CacheManager->AddCacheEntry
        ("LIBRARY_OUTPUT_PATH", "",
         "Single output directory for building all libraries.",
         cmCacheManager::PATH);
      }
    if(!this->CacheManager->GetCacheValue("EXECUTABLE_OUTPUT_PATH"))
      {
      this->CacheManager->AddCacheEntry
        ("EXECUTABLE_OUTPUT_PATH", "",
         "Single output directory for building all executables.",
         cmCacheManager::PATH);
      }
    }
  if(!this->CacheManager->GetCacheValue("CMAKE_USE_RELATIVE_PATHS"))
    {
    this->CacheManager->AddCacheEntry
      ("CMAKE_USE_RELATIVE_PATHS", "OFF",
       "If true, cmake will use relative paths in makefiles and projects.",
       cmCacheManager::BOOL);
    cmCacheManager::CacheIterator it =
      this->CacheManager->GetCacheIterator("CMAKE_USE_RELATIVE_PATHS");
    if ( !it.PropertyExists("ADVANCED") )
      {
      it.SetProperty("ADVANCED", "1");
      }
    }

  if(cmSystemTools::GetFatalErrorOccured() &&
     (!this->CacheManager->GetCacheValue("CMAKE_MAKE_PROGRAM") ||
      cmSystemTools::IsOff(this->CacheManager->
                           GetCacheValue("CMAKE_MAKE_PROGRAM"))))
    {
    // We must have a bad generator selection.  Wipe the cache entry so the
    // user can select another.
    this->CacheManager->RemoveCacheEntry("CMAKE_GENERATOR");
    this->CacheManager->RemoveCacheEntry("CMAKE_EXTRA_GENERATOR");
    }

  cmMakefile* mf=this->GlobalGenerator->GetLocalGenerators()[0]->GetMakefile();
  if (mf->IsOn("CTEST_USE_LAUNCHERS")
              && !this->GetProperty("RULE_LAUNCH_COMPILE", cmProperty::GLOBAL))
    {
    cmSystemTools::Error("CTEST_USE_LAUNCHERS is enabled, but the "
                        "RULE_LAUNCH_COMPILE global property is not defined.\n"
                        "Did you forget to include(CTest) in the toplevel "
                         "CMakeLists.txt ?");
    }

  // only save the cache if there were no fatal errors
  if ( this->GetWorkingMode() == NORMAL_MODE )
    {
    this->CacheManager->SaveCache(this->GetHomeOutputDirectory());
    }
  if ( !this->GraphVizFile.empty() )
    {
    std::cout << "Generate graphviz: " << this->GraphVizFile << std::endl;
    this->GenerateGraphViz(this->GraphVizFile.c_str());
    }
  if(cmSystemTools::GetErrorOccuredFlag())
    {
    return -1;
    }
  return 0;
}

void cmake::PreLoadCMakeFiles()
{
  std::vector<std::string> args;
  std::string pre_load = this->GetHomeDirectory();
  if ( pre_load.size() > 0 )
    {
    pre_load += "/PreLoad.cmake";
    if ( cmSystemTools::FileExists(pre_load.c_str()) )
      {
      this->ReadListFile(args, pre_load.c_str());
      }
    }
  pre_load = this->GetHomeOutputDirectory();
  if ( pre_load.size() > 0 )
    {
    pre_load += "/PreLoad.cmake";
    if ( cmSystemTools::FileExists(pre_load.c_str()) )
      {
      this->ReadListFile(args, pre_load.c_str());
      }
    }
}

// handle a command line invocation
int cmake::Run(const std::vector<std::string>& args, bool noconfigure)
{
  // Process the arguments
  this->SetArgs(args);
  if(cmSystemTools::GetErrorOccuredFlag())
    {
    return -1;
    }

  // If we are given a stamp list file check if it is really out of date.
  if(!this->CheckStampList.empty() &&
     cmakeCheckStampList(this->CheckStampList.c_str()))
    {
    return 0;
    }

  // If we are given a stamp file check if it is really out of date.
  if(!this->CheckStampFile.empty() &&
     cmakeCheckStampFile(this->CheckStampFile.c_str()))
    {
    return 0;
    }

  // set the cmake command
  this->CMakeCommand = args[0];

  if ( this->GetWorkingMode() == NORMAL_MODE )
    {
    // load the cache
    if(this->LoadCache() < 0)
      {
      cmSystemTools::Error("Error executing cmake::LoadCache(). Aborting.\n");
      return -1;
      }
    }
  else
    {
    this->AddCMakePaths();
    }
  // Add any cache args
  if ( !this->SetCacheArgs(args) )
    {
    cmSystemTools::Error("Problem processing arguments. Aborting.\n");
    return -1;
    }

  // In script mode we terminate after running the script.
  if(this->GetWorkingMode() != NORMAL_MODE)
    {
    if(cmSystemTools::GetErrorOccuredFlag())
      {
      return -1;
      }
    else
      {
      return 0;
      }
    }

  // If MAKEFLAGS are given in the environment, remove the environment
  // variable.  This will prevent try-compile from succeeding when it
  // should fail (if "-i" is an option).  We cannot simply test
  // whether "-i" is given and remove it because some make programs
  // encode the MAKEFLAGS variable in a strange way.
  if(getenv("MAKEFLAGS"))
    {
    cmSystemTools::PutEnv("MAKEFLAGS=");
    }

  this->PreLoadCMakeFiles();

  if ( noconfigure )
    {
    return 0;
    }

  // now run the global generate
  // Check the state of the build system to see if we need to regenerate.
  if(!this->CheckBuildSystem())
    {
    return 0;
    }

  // If we are doing global generate, we better set start and start
  // output directory to the root of the project.
  std::string oldstartdir = this->GetStartDirectory();
  std::string oldstartoutputdir = this->GetStartOutputDirectory();
  this->SetStartDirectory(this->GetHomeDirectory());
  this->SetStartOutputDirectory(this->GetHomeOutputDirectory());
  int ret = this->Configure();
  if (ret || this->GetWorkingMode() != NORMAL_MODE)
    {
#if defined(CMAKE_HAVE_VS_GENERATORS)
    if(!this->VSSolutionFile.empty() && this->GlobalGenerator)
      {
      // CMake is running to regenerate a Visual Studio build tree
      // during a build from the VS IDE.  The build files cannot be
      // regenerated, so we should stop the build.
      cmSystemTools::Message(
        "CMake Configure step failed.  "
        "Build files cannot be regenerated correctly.  "
        "Attempting to stop IDE build.");
      cmGlobalVisualStudioGenerator* gg =
        static_cast<cmGlobalVisualStudioGenerator*>(this->GlobalGenerator);
      gg->CallVisualStudioMacro(cmGlobalVisualStudioGenerator::MacroStop,
                                this->VSSolutionFile.c_str());
      }
#endif
    return ret;
    }
  ret = this->Generate();
  std::string message = "Build files have been written to: ";
  message += this->GetHomeOutputDirectory();
  this->UpdateProgress(message.c_str(), -1);
  if(ret)
    {
    return ret;
    }
  this->SetStartDirectory(oldstartdir.c_str());
  this->SetStartOutputDirectory(oldstartoutputdir.c_str());

  return ret;
}

int cmake::Generate()
{
  if(!this->GlobalGenerator)
    {
    return -1;
    }
  this->GlobalGenerator->Generate();
  if(this->WarnUnusedCli)
    {
    this->RunCheckForUnusedVariables();
    }
  if(cmSystemTools::GetErrorOccuredFlag())
    {
    return -1;
    }
  if (this->GetProperty("REPORT_UNDEFINED_PROPERTIES"))
    {
    this->ReportUndefinedPropertyAccesses
      (this->GetProperty("REPORT_UNDEFINED_PROPERTIES"));
    }
  // Save the cache again after a successful Generate so that any internal
  // variables created during Generate are saved. (Specifically target GUIDs
  // for the Visual Studio and Xcode generators.)
  if ( this->GetWorkingMode() == NORMAL_MODE )
    {
    this->CacheManager->SaveCache(this->GetHomeOutputDirectory());
    }
  return 0;
}

void cmake::AddCacheEntry(const char* key, const char* value,
                          const char* helpString,
                          int type)
{
  this->CacheManager->AddCacheEntry(key, value,
                                    helpString,
                                    cmCacheManager::CacheEntryType(type));
}

const char* cmake::GetCacheDefinition(const char* name) const
{
  return this->CacheManager->GetCacheValue(name);
}

void cmake::AddDefaultCommands()
{
  std::list<cmCommand*> commands;
  GetBootstrapCommands1(commands);
  GetBootstrapCommands2(commands);
  GetPredefinedCommands(commands);
  for(std::list<cmCommand*>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    this->AddCommand(*i);
    }
}

void cmake::AddDefaultGenerators()
{
#if defined(_WIN32) && !defined(__CYGWIN__)
# if !defined(CMAKE_BOOT_MINGW)
  this->Generators.push_back(
    cmGlobalVisualStudio6Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalVisualStudio7Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalVisualStudio10Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalVisualStudio11Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalVisualStudio12Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalVisualStudio71Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalVisualStudio8Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalVisualStudio9Generator::NewFactory());
  this->Generators.push_back(
    cmGlobalBorlandMakefileGenerator::NewFactory());
  this->Generators.push_back(
    cmGlobalNMakeMakefileGenerator::NewFactory());
  this->Generators.push_back(
    cmGlobalJOMMakefileGenerator::NewFactory());
  this->Generators.push_back(
    cmGlobalWatcomWMakeGenerator::NewFactory());
# endif
  this->Generators.push_back(
    cmGlobalMSYSMakefileGenerator::NewFactory());
  this->Generators.push_back(
    cmGlobalMinGWMakefileGenerator::NewFactory());
#endif
  this->Generators.push_back(
    cmGlobalUnixMakefileGenerator3::NewFactory());
  this->Generators.push_back(
    cmGlobalNinjaGenerator::NewFactory());
#ifdef CMAKE_USE_XCODE
  this->Generators.push_back(
    cmGlobalXCodeGenerator::NewFactory());
#endif
}

int cmake::LoadCache()
{
  // could we not read the cache
  if (!this->CacheManager->LoadCache(this->GetHomeOutputDirectory()))
    {
    // if it does exist, but isn't readable then warn the user
    std::string cacheFile = this->GetHomeOutputDirectory();
    cacheFile += "/CMakeCache.txt";
    if(cmSystemTools::FileExists(cacheFile.c_str()))
      {
      cmSystemTools::Error(
        "There is a CMakeCache.txt file for the current binary tree but "
        "cmake does not have permission to read it. Please check the "
        "permissions of the directory you are trying to run CMake on.");
      return -1;
      }
    }

  if (this->CMakeCommand.size() < 2)
    {
    cmSystemTools::Error(
      "cmake command was not specified prior to loading the cache in "
      "cmake.cxx");
    return -1;
    }

  // setup CMAKE_ROOT and CMAKE_COMMAND
  if(!this->AddCMakePaths())
    {
    return -3;
    }
  return 0;
}

void cmake::SetProgressCallback(ProgressCallbackType f, void *cd)
{
  this->ProgressCallback = f;
  this->ProgressCallbackClientData = cd;
}

void cmake::UpdateProgress(const char *msg, float prog)
{
  if(this->ProgressCallback && !this->InTryCompile)
    {
    (*this->ProgressCallback)(msg, prog, this->ProgressCallbackClientData);
    return;
    }
}

void cmake::GetCommandDocumentation(std::vector<cmDocumentationEntry>& v,
                                    bool withCurrentCommands,
                                    bool withCompatCommands) const
{
  for(RegisteredCommandsMap::const_iterator j = this->Commands.begin();
      j != this->Commands.end(); ++j)
    {
    if (((  withCompatCommands == false) && ( (*j).second->IsDiscouraged()))
        || ((withCurrentCommands == false) && (!(*j).second->IsDiscouraged()))
        || (!((*j).second->ShouldAppearInDocumentation()))
        )
      {
      continue;
      }

    cmDocumentationEntry e((*j).second->GetName(),
                           (*j).second->GetTerseDocumentation(),
                           (*j).second->GetFullDocumentation());
    v.push_back(e);
    }
}

void cmake::GetPolicyDocumentation(std::vector<cmDocumentationEntry>& v)
{
  this->Policies->GetDocumentation(v);
}

void cmake::GetPropertiesDocumentation(std::map<std::string,
                                       cmDocumentationSection *>& v)
{
  // loop over the properties and put them into the doc structure
  std::map<cmProperty::ScopeType, cmPropertyDefinitionMap>::iterator i;
  i = this->PropertyDefinitions.begin();
  for (;i != this->PropertyDefinitions.end(); ++i)
    {
    i->second.GetPropertiesDocumentation(v);
    }
}

void cmake::GetGeneratorDocumentation(std::vector<cmDocumentationEntry>& v)
{
  for(RegisteredGeneratorsVector::const_iterator i =
      this->Generators.begin(); i != this->Generators.end(); ++i)
    {
    cmDocumentationEntry e;
    (*i)->GetDocumentation(e);
    v.push_back(e);
    }
  for(RegisteredExtraGeneratorsMap::const_iterator i =
      this->ExtraGenerators.begin(); i != this->ExtraGenerators.end(); ++i)
    {
    cmDocumentationEntry e;
    cmExternalMakefileProjectGenerator* generator = (i->second)();
    generator->GetDocumentation(e, i->first.c_str());
    e.Name = i->first;
    delete generator;
    v.push_back(e);
    }
}

void cmake::UpdateConversionPathTable()
{
  // Update the path conversion table with any specified file:
  const char* tablepath =
    this->CacheManager->GetCacheValue("CMAKE_PATH_TRANSLATION_FILE");

  if(tablepath)
    {
    std::ifstream table( tablepath );
    if(!table)
      {
      cmSystemTools::Error("CMAKE_PATH_TRANSLATION_FILE set to ", tablepath,
        ". CMake can not open file.");
      cmSystemTools::ReportLastSystemError("CMake can not open file.");
      }
    else
      {
      std::string a, b;
      while(!table.eof())
        {
        // two entries per line
        table >> a; table >> b;
        cmSystemTools::AddTranslationPath( a.c_str(), b.c_str());
        }
      }
    }
}

//----------------------------------------------------------------------------
int cmake::CheckBuildSystem()
{
  // We do not need to rerun CMake.  Check dependency integrity.  Use
  // the make system's VERBOSE environment variable to enable verbose
  // output. This can be skipped by setting CMAKE_NO_VERBOSE (which is set
  // by the Eclipse and KDevelop generators).
  bool verbose = ((cmSystemTools::GetEnv("VERBOSE") != 0)
                   && (cmSystemTools::GetEnv("CMAKE_NO_VERBOSE") == 0));

  // This method will check the integrity of the build system if the
  // option was given on the command line.  It reads the given file to
  // determine whether CMake should rerun.

  // If no file is provided for the check, we have to rerun.
  if(this->CheckBuildSystemArgument.size() == 0)
    {
    if(verbose)
      {
      cmOStringStream msg;
      msg << "Re-run cmake no build system arguments\n";
      cmSystemTools::Stdout(msg.str().c_str());
      }
    return 1;
    }

  // If the file provided does not exist, we have to rerun.
  if(!cmSystemTools::FileExists(this->CheckBuildSystemArgument.c_str()))
    {
    if(verbose)
      {
      cmOStringStream msg;
      msg << "Re-run cmake missing file: "
          << this->CheckBuildSystemArgument.c_str() << "\n";
      cmSystemTools::Stdout(msg.str().c_str());
      }
    return 1;
    }

  // Read the rerun check file and use it to decide whether to do the
  // global generate.
  cmake cm;
  cmGlobalGenerator gg;
  gg.SetCMakeInstance(&cm);
  cmsys::auto_ptr<cmLocalGenerator> lg(gg.CreateLocalGenerator());
  cmMakefile* mf = lg->GetMakefile();
  if(!mf->ReadListFile(0, this->CheckBuildSystemArgument.c_str()) ||
     cmSystemTools::GetErrorOccuredFlag())
    {
    if(verbose)
      {
      cmOStringStream msg;
      msg << "Re-run cmake error reading : "
          << this->CheckBuildSystemArgument.c_str() << "\n";
      cmSystemTools::Stdout(msg.str().c_str());
      }
    // There was an error reading the file.  Just rerun.
    return 1;
    }

  if(this->ClearBuildSystem)
    {
    // Get the generator used for this build system.
    const char* genName = mf->GetDefinition("CMAKE_DEPENDS_GENERATOR");
    if(!genName || genName[0] == '\0')
      {
      genName = "Unix Makefiles";
      }

    // Create the generator and use it to clear the dependencies.
    cmsys::auto_ptr<cmGlobalGenerator>
      ggd(this->CreateGlobalGenerator(genName));
    if(ggd.get())
      {
      cmsys::auto_ptr<cmLocalGenerator> lgd(ggd->CreateLocalGenerator());
      lgd->ClearDependencies(mf, verbose);
      }
    }

  // If any byproduct of makefile generation is missing we must re-run.
  std::vector<std::string> products;
  if(const char* productStr = mf->GetDefinition("CMAKE_MAKEFILE_PRODUCTS"))
    {
    cmSystemTools::ExpandListArgument(productStr, products);
    }
  for(std::vector<std::string>::const_iterator pi = products.begin();
      pi != products.end(); ++pi)
    {
    if(!(cmSystemTools::FileExists(pi->c_str()) ||
         cmSystemTools::FileIsSymlink(pi->c_str())))
      {
      if(verbose)
        {
        cmOStringStream msg;
        msg << "Re-run cmake, missing byproduct: " << *pi << "\n";
        cmSystemTools::Stdout(msg.str().c_str());
        }
      return 1;
      }
    }

  // Get the set of dependencies and outputs.
  std::vector<std::string> depends;
  std::vector<std::string> outputs;
  const char* dependsStr = mf->GetDefinition("CMAKE_MAKEFILE_DEPENDS");
  const char* outputsStr = mf->GetDefinition("CMAKE_MAKEFILE_OUTPUTS");
  if(dependsStr && outputsStr)
    {
    cmSystemTools::ExpandListArgument(dependsStr, depends);
    cmSystemTools::ExpandListArgument(outputsStr, outputs);
    }
  if(depends.empty() || outputs.empty())
    {
    // Not enough information was provided to do the test.  Just rerun.
    if(verbose)
      {
      cmOStringStream msg;
      msg << "Re-run cmake no CMAKE_MAKEFILE_DEPENDS "
        "or CMAKE_MAKEFILE_OUTPUTS :\n";
      cmSystemTools::Stdout(msg.str().c_str());
      }
    return 1;
    }

  // Find the newest dependency.
  std::vector<std::string>::iterator dep = depends.begin();
  std::string dep_newest = *dep++;
  for(;dep != depends.end(); ++dep)
    {
    int result = 0;
    if(this->FileComparison->FileTimeCompare(dep_newest.c_str(),
                                             dep->c_str(), &result))
      {
      if(result < 0)
        {
        dep_newest = *dep;
        }
      }
    else
      {
      if(verbose)
        {
        cmOStringStream msg;
        msg << "Re-run cmake: build system dependency is missing\n";
        cmSystemTools::Stdout(msg.str().c_str());
        }
      return 1;
      }
    }

  // Find the oldest output.
  std::vector<std::string>::iterator out = outputs.begin();
  std::string out_oldest = *out++;
  for(;out != outputs.end(); ++out)
    {
    int result = 0;
    if(this->FileComparison->FileTimeCompare(out_oldest.c_str(),
                                             out->c_str(), &result))
      {
      if(result > 0)
        {
        out_oldest = *out;
        }
      }
    else
      {
      if(verbose)
        {
        cmOStringStream msg;
        msg << "Re-run cmake: build system output is missing\n";
        cmSystemTools::Stdout(msg.str().c_str());
        }
      return 1;
      }
    }

  // If any output is older than any dependency then rerun.
  {
  int result = 0;
  if(!this->FileComparison->FileTimeCompare(out_oldest.c_str(),
                                            dep_newest.c_str(),
                                            &result) ||
     result < 0)
    {
    if(verbose)
      {
      cmOStringStream msg;
      msg << "Re-run cmake file: " << out_oldest.c_str()
          << " older than: " << dep_newest.c_str() << "\n";
      cmSystemTools::Stdout(msg.str().c_str());
      }
    return 1;
    }
  }

  // No need to rerun.
  return 0;
}

//----------------------------------------------------------------------------
void cmake::TruncateOutputLog(const char* fname)
{
  std::string fullPath = this->GetHomeOutputDirectory();
  fullPath += "/";
  fullPath += fname;
  struct stat st;
  if ( ::stat(fullPath.c_str(), &st) )
    {
    return;
    }
  if ( !this->CacheManager->GetCacheValue("CMAKE_CACHEFILE_DIR") )
    {
    cmSystemTools::RemoveFile(fullPath.c_str());
    return;
    }
  off_t fsize = st.st_size;
  const off_t maxFileSize = 50 * 1024;
  if ( fsize < maxFileSize )
    {
    //TODO: truncate file
    return;
    }
}

inline std::string removeQuotes(const std::string& s)
{
  if(s[0] == '\"' && s[s.size()-1] == '\"')
    {
    return s.substr(1, s.size()-2);
    }
  return s;
}

std::string cmake::FindCMakeProgram(const char* name) const
{
  std::string path;
  if ((name) && (*name))
    {
    const cmMakefile* mf
        = this->GetGlobalGenerator()->GetLocalGenerators()[0]->GetMakefile();
#ifdef CMAKE_BUILD_WITH_CMAKE
    path = mf->GetRequiredDefinition("CMAKE_COMMAND");
    path = removeQuotes(path);
    path = cmSystemTools::GetFilenamePath(path.c_str());
    path += "/";
    path += name;
    path += cmSystemTools::GetExecutableExtension();
    if(!cmSystemTools::FileExists(path.c_str()))
    {
      path = mf->GetRequiredDefinition("CMAKE_COMMAND");
      path = cmSystemTools::GetFilenamePath(path.c_str());
      path += "/Debug/";
      path += name;
      path += cmSystemTools::GetExecutableExtension();
    }
    if(!cmSystemTools::FileExists(path.c_str()))
    {
      path = mf->GetRequiredDefinition("CMAKE_COMMAND");
      path = cmSystemTools::GetFilenamePath(path.c_str());
      path += "/Release/";
      path += name;
      path += cmSystemTools::GetExecutableExtension();
    }
#else
    // Only for bootstrap
    path += mf->GetSafeDefinition("EXECUTABLE_OUTPUT_PATH");
    path += "/";
    path += name;
    path += cmSystemTools::GetExecutableExtension();
#endif
    }
  return path;
}

const char* cmake::GetCTestCommand()
{
  if ( this->CTestCommand.empty() )
    {
    this->CTestCommand = this->FindCMakeProgram("ctest");
    }
  if ( this->CTestCommand.empty() )
    {
    cmSystemTools::Error("Cannot find the CTest executable");
    this->CTestCommand = "CTEST-COMMAND-NOT-FOUND";
    }
  return this->CTestCommand.c_str();
}

const char* cmake::GetCPackCommand()
{
  if ( this->CPackCommand.empty() )
    {
    this->CPackCommand = this->FindCMakeProgram("cpack");
    }
  if ( this->CPackCommand.empty() )
    {
    cmSystemTools::Error("Cannot find the CPack executable");
    this->CPackCommand = "CPACK-COMMAND-NOT-FOUND";
    }
    return this->CPackCommand.c_str();
}


const char* cmake::GetCMakeCommand()
{
  return this->CMakeCommand.c_str();
}


void cmake::MarkCliAsUsed(const std::string& variable)
{
  this->UsedCliVariables[variable] = true;
}

void cmake::GenerateGraphViz(const char* fileName) const
{
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmsys::auto_ptr<cmGraphVizWriter> gvWriter(
       new cmGraphVizWriter(this->GetGlobalGenerator()->GetLocalGenerators()));

  std::string settingsFile = this->GetHomeOutputDirectory();
  settingsFile += "/CMakeGraphVizOptions.cmake";
  std::string fallbackSettingsFile = this->GetHomeDirectory();
  fallbackSettingsFile += "/CMakeGraphVizOptions.cmake";

  gvWriter->ReadSettings(settingsFile.c_str(), fallbackSettingsFile.c_str());

  gvWriter->WritePerTargetFiles(fileName);
  gvWriter->WriteTargetDependersFiles(fileName);
  gvWriter->WriteGlobalFile(fileName);

#endif
}


//----------------------------------------------------------------------------
int cmake::SymlinkLibrary(std::vector<std::string>& args)
{
  int result = 0;
  std::string realName = args[2];
  std::string soName = args[3];
  std::string name = args[4];
  if(soName != realName)
    {
    if(!cmake::SymlinkInternal(realName, soName))
      {
      cmSystemTools::ReportLastSystemError("cmake_symlink_library");
      result = 1;
      }
    }
  if(name != soName)
    {
    if(!cmake::SymlinkInternal(soName, name))
      {
      cmSystemTools::ReportLastSystemError("cmake_symlink_library");
      result = 1;
      }
    }
  return result;
}

//----------------------------------------------------------------------------
int cmake::SymlinkExecutable(std::vector<std::string>& args)
{
  int result = 0;
  std::string realName = args[2];
  std::string name = args[3];
  if(name != realName)
    {
    if(!cmake::SymlinkInternal(realName, name))
      {
      cmSystemTools::ReportLastSystemError("cmake_symlink_executable");
      result = 1;
      }
    }
  return result;
}

//----------------------------------------------------------------------------
bool cmake::SymlinkInternal(std::string const& file, std::string const& link)
{
  if(cmSystemTools::FileExists(link.c_str()) ||
     cmSystemTools::FileIsSymlink(link.c_str()))
    {
    cmSystemTools::RemoveFile(link.c_str());
    }
#if defined(_WIN32) && !defined(__CYGWIN__)
  return cmSystemTools::CopyFileAlways(file.c_str(), link.c_str());
#else
  std::string linktext = cmSystemTools::GetFilenameName(file);
  return cmSystemTools::CreateSymlink(linktext.c_str(), link.c_str());
#endif
}

//----------------------------------------------------------------------------
#ifdef CMAKE_BUILD_WITH_CMAKE
int cmake::ExecuteEchoColor(std::vector<std::string>& args)
{
  // The arguments are
  //   argv[0] == <cmake-executable>
  //   argv[1] == cmake_echo_color

  bool enabled = true;
  int color = cmsysTerminal_Color_Normal;
  bool newline = true;
  for(unsigned int i=2; i < args.size(); ++i)
    {
    if(args[i].find("--switch=") == 0)
      {
      // Enable or disable color based on the switch value.
      std::string value = args[i].substr(9);
      if(!value.empty())
        {
        if(cmSystemTools::IsOn(value.c_str()))
          {
          enabled = true;
          }
        else
          {
          enabled = false;
          }
        }
      }
    else if(args[i] == "--normal")
      {
      color = cmsysTerminal_Color_Normal;
      }
    else if(args[i] == "--black")
      {
      color = cmsysTerminal_Color_ForegroundBlack;
      }
    else if(args[i] == "--red")
      {
      color = cmsysTerminal_Color_ForegroundRed;
      }
    else if(args[i] == "--green")
      {
      color = cmsysTerminal_Color_ForegroundGreen;
      }
    else if(args[i] == "--yellow")
      {
      color = cmsysTerminal_Color_ForegroundYellow;
      }
    else if(args[i] == "--blue")
      {
      color = cmsysTerminal_Color_ForegroundBlue;
      }
    else if(args[i] == "--magenta")
      {
      color = cmsysTerminal_Color_ForegroundMagenta;
      }
    else if(args[i] == "--cyan")
      {
      color = cmsysTerminal_Color_ForegroundCyan;
      }
    else if(args[i] == "--white")
      {
      color = cmsysTerminal_Color_ForegroundWhite;
      }
    else if(args[i] == "--bold")
      {
      color |= cmsysTerminal_Color_ForegroundBold;
      }
    else if(args[i] == "--no-newline")
      {
      newline = false;
      }
    else if(args[i] == "--newline")
      {
      newline = true;
      }
    else
      {
      // Color is enabled.  Print with the current color.
      cmSystemTools::MakefileColorEcho(color, args[i].c_str(),
                                       newline, enabled);
      }
    }

  return 0;
}
#else
int cmake::ExecuteEchoColor(std::vector<std::string>&)
{
  return 1;
}
#endif

//----------------------------------------------------------------------------
int cmake::ExecuteLinkScript(std::vector<std::string>& args)
{
  // The arguments are
  //   argv[0] == <cmake-executable>
  //   argv[1] == cmake_link_script
  //   argv[2] == <link-script-name>
  //   argv[3] == --verbose=?
  bool verbose = false;
  if(args.size() >= 4)
    {
    if(args[3].find("--verbose=") == 0)
      {
      if(!cmSystemTools::IsOff(args[3].substr(10).c_str()))
        {
        verbose = true;
        }
      }
    }

  // Allocate a process instance.
  cmsysProcess* cp = cmsysProcess_New();
  if(!cp)
    {
    std::cerr << "Error allocating process instance in link script."
              << std::endl;
    return 1;
    }

  // Children should share stdout and stderr with this process.
  cmsysProcess_SetPipeShared(cp, cmsysProcess_Pipe_STDOUT, 1);
  cmsysProcess_SetPipeShared(cp, cmsysProcess_Pipe_STDERR, 1);

  // Run the command lines verbatim.
  cmsysProcess_SetOption(cp, cmsysProcess_Option_Verbatim, 1);

  // Read command lines from the script.
  std::ifstream fin(args[2].c_str());
  if(!fin)
    {
    std::cerr << "Error opening link script \""
              << args[2] << "\"" << std::endl;
    return 1;
    }

  // Run one command at a time.
  std::string command;
  int result = 0;
  while(result == 0 && cmSystemTools::GetLineFromStream(fin, command))
    {
    // Skip empty command lines.
    if(command.find_first_not_of(" \t") == command.npos)
      {
      continue;
      }

    // Setup this command line.
    const char* cmd[2] = {command.c_str(), 0};
    cmsysProcess_SetCommand(cp, cmd);

    // Report the command if verbose output is enabled.
    if(verbose)
      {
      std::cout << command << std::endl;
      }

    // Run the command and wait for it to exit.
    cmsysProcess_Execute(cp);
    cmsysProcess_WaitForExit(cp, 0);

    // Report failure if any.
    switch(cmsysProcess_GetState(cp))
      {
      case cmsysProcess_State_Exited:
        {
        int value = cmsysProcess_GetExitValue(cp);
        if(value != 0)
          {
          result = value;
          }
        }
        break;
      case cmsysProcess_State_Exception:
        std::cerr << "Error running link command: "
                  << cmsysProcess_GetExceptionString(cp) << std::endl;
        result = 1;
        break;
      case cmsysProcess_State_Error:
        std::cerr << "Error running link command: "
                  << cmsysProcess_GetErrorString(cp) << std::endl;
        result = 2;
        break;
      default:
        break;
      };
    }

  // Free the process instance.
  cmsysProcess_Delete(cp);

  // Return the final resulting return value.
  return result;
}

void cmake::DefineProperties(cmake *cm)
{
  cm->DefineProperty
    ("REPORT_UNDEFINED_PROPERTIES", cmProperty::GLOBAL,
     "If set, report any undefined properties to this file.",
     "If this property is set to a filename then when CMake runs "
     "it will report any properties or variables that were accessed "
     "but not defined into the filename specified in this property."
     );

  cm->DefineProperty
    ("TARGET_SUPPORTS_SHARED_LIBS", cmProperty::GLOBAL,
     "Does the target platform support shared libraries.",
     "TARGET_SUPPORTS_SHARED_LIBS is a boolean specifying whether the target "
     "platform supports shared libraries. Basically all current general "
     "general purpose OS do so, the exception are usually embedded systems "
     "with no or special OSs.");

  cm->DefineProperty
    ("TARGET_ARCHIVES_MAY_BE_SHARED_LIBS", cmProperty::GLOBAL,
     "Set if shared libraries may be named like archives.",
     "On AIX shared libraries may be named \"lib<name>.a\".  "
     "This property is set to true on such platforms.");

  cm->DefineProperty
    ("FIND_LIBRARY_USE_LIB64_PATHS", cmProperty::GLOBAL,
     "Whether FIND_LIBRARY should automatically search lib64 directories.",
     "FIND_LIBRARY_USE_LIB64_PATHS is a boolean specifying whether the "
     "FIND_LIBRARY command should automatically search the lib64 variant of "
     "directories called lib in the search path when building 64-bit "
     "binaries.");
  cm->DefineProperty
    ("FIND_LIBRARY_USE_OPENBSD_VERSIONING", cmProperty::GLOBAL,
     "Whether FIND_LIBRARY should find OpenBSD-style shared libraries.",
     "This property is a boolean specifying whether the FIND_LIBRARY "
     "command should find shared libraries with OpenBSD-style versioned "
     "extension: \".so.<major>.<minor>\".  "
     "The property is set to true on OpenBSD and false on other platforms.");
  cm->DefineProperty
    ("ENABLED_FEATURES", cmProperty::GLOBAL,
     "List of features which are enabled during the CMake run.",
     "List of features which are enabled during the CMake run. By default "
     "it contains the names of all packages which were found. This is "
     "determined using the <NAME>_FOUND variables. Packages which are "
     "searched QUIET are not listed. A project can add its own features to "
     "this list. "
     "This property is used by the macros in FeatureSummary.cmake.");
  cm->DefineProperty
    ("DISABLED_FEATURES", cmProperty::GLOBAL,
     "List of features which are disabled during the CMake run.",
     "List of features which are disabled during the CMake run. By default "
     "it contains the names of all packages which were not found. This is "
     "determined using the <NAME>_FOUND variables. Packages which are "
     "searched QUIET are not listed. A project can add its own features to "
     "this list. "
     "This property is used by the macros in FeatureSummary.cmake.");
  cm->DefineProperty
    ("PACKAGES_FOUND", cmProperty::GLOBAL,
     "List of packages which were found during the CMake run.",
     "List of packages which were found during the CMake run. Whether a "
     "package has been found is determined using the <NAME>_FOUND variables.");
  cm->DefineProperty
    ("PACKAGES_NOT_FOUND", cmProperty::GLOBAL,
     "List of packages which were not found during the CMake run.",
     "List of packages which were not found during the CMake run. Whether a "
     "package has been found is determined using the <NAME>_FOUND variables.");

  cm->DefineProperty(
    "__CMAKE_DELETE_CACHE_CHANGE_VARS_", cmProperty::GLOBAL,
    "Internal property",
    "Used to detect compiler changes, Do not set.");

  cm->DefineProperty(
    "DEBUG_CONFIGURATIONS", cmProperty::GLOBAL,
    "Specify which configurations are for debugging.",
    "The value must be a semi-colon separated list of configuration names.  "
    "Currently this property is used only by the target_link_libraries "
    "command (see its documentation for details).  "
    "Additional uses may be defined in the future.  "
    "\n"
    "This property must be set at the top level of the project and before "
    "the first target_link_libraries command invocation.  "
    "If any entry in the list does not match a valid configuration for "
    "the project the behavior is undefined.");

  cm->DefineProperty(
    "GLOBAL_DEPENDS_DEBUG_MODE", cmProperty::GLOBAL,
    "Enable global target dependency graph debug mode.",
    "CMake automatically analyzes the global inter-target dependency graph "
    "at the beginning of native build system generation.  "
    "This property causes it to display details of its analysis to stderr.");

  cm->DefineProperty(
    "GLOBAL_DEPENDS_NO_CYCLES", cmProperty::GLOBAL,
    "Disallow global target dependency graph cycles.",
    "CMake automatically analyzes the global inter-target dependency graph "
    "at the beginning of native build system generation.  "
    "It reports an error if the dependency graph contains a cycle that "
    "does not consist of all STATIC library targets.  "
    "This property tells CMake to disallow all cycles completely, even "
    "among static libraries.");

  cm->DefineProperty(
    "ALLOW_DUPLICATE_CUSTOM_TARGETS", cmProperty::GLOBAL,
    "Allow duplicate custom targets to be created.",
    "Normally CMake requires that all targets built in a project have "
    "globally unique logical names (see policy CMP0002).  "
    "This is necessary to generate meaningful project file names in "
    "Xcode and VS IDE generators.  "
    "It also allows the target names to be referenced unambiguously.\n"
    "Makefile generators are capable of supporting duplicate custom target "
    "names.  "
    "For projects that care only about Makefile generators and do "
    "not wish to support Xcode or VS IDE generators, one may set this "
    "property to true to allow duplicate custom targets.  "
    "The property allows multiple add_custom_target command calls in "
    "different directories to specify the same target name.  "
    "However, setting this property will cause non-Makefile generators "
    "to produce an error and refuse to generate the project."
    );

  cm->DefineProperty
    ("IN_TRY_COMPILE", cmProperty::GLOBAL,
     "Read-only property that is true during a try-compile configuration.",
     "True when building a project inside a TRY_COMPILE or TRY_RUN command.");
  cm->DefineProperty
    ("ENABLED_LANGUAGES", cmProperty::GLOBAL,
     "Read-only property that contains the list of currently "
     "enabled languages",
     "Set to list of currently enabled languages.");

  cm->DefineProperty
    ("RULE_LAUNCH_COMPILE", cmProperty::GLOBAL,
     "Specify a launcher for compile rules.",
     "Makefile generators prefix compiler commands with the given "
     "launcher command line.  "
     "This is intended to allow launchers to intercept build problems "
     "with high granularity.  "
     "Non-Makefile generators currently ignore this property.");
  cm->DefineProperty
    ("RULE_LAUNCH_LINK", cmProperty::GLOBAL,
     "Specify a launcher for link rules.",
     "Makefile generators prefix link and archive commands with the given "
     "launcher command line.  "
     "This is intended to allow launchers to intercept build problems "
     "with high granularity.  "
     "Non-Makefile generators currently ignore this property.");
  cm->DefineProperty
    ("RULE_LAUNCH_CUSTOM", cmProperty::GLOBAL,
     "Specify a launcher for custom rules.",
     "Makefile generators prefix custom commands with the given "
     "launcher command line.  "
     "This is intended to allow launchers to intercept build problems "
     "with high granularity.  "
     "Non-Makefile generators currently ignore this property.");

  cm->DefineProperty
    ("RULE_MESSAGES", cmProperty::GLOBAL,
     "Specify whether to report a message for each make rule.",
     "This property specifies whether Makefile generators should add a "
     "progress message describing what each build rule does.  "
     "If the property is not set the default is ON.  "
     "Set the property to OFF to disable granular messages and report only "
     "as each target completes.  "
     "This is intended to allow scripted builds to avoid the build time "
     "cost of detailed reports.  "
     "If a CMAKE_RULE_MESSAGES cache entry exists its value initializes "
     "the value of this property.  "
     "Non-Makefile generators currently ignore this property.");

  cm->DefineProperty
    ("USE_FOLDERS", cmProperty::GLOBAL,
     "Use the FOLDER target property to organize targets into folders.",
     "If not set, CMake treats this property as OFF by default. "
     "CMake generators that are capable of organizing into a "
     "hierarchy of folders use the values of the FOLDER target "
     "property to name those folders. See also the documentation "
     "for the FOLDER target property.");

  cm->DefineProperty
    ("AUTOMOC_TARGETS_FOLDER", cmProperty::GLOBAL,
     "Name of FOLDER for *_automoc targets that are added automatically by "
     "CMake for targets for which AUTOMOC is enabled.",
     "If not set, CMake uses the FOLDER property of the parent target as a "
     "default value for this property. See also the documentation for the "
     "FOLDER target property and the AUTOMOC target property.");

  cm->DefineProperty
    ("PREDEFINED_TARGETS_FOLDER", cmProperty::GLOBAL,
     "Name of FOLDER for targets that are added automatically by CMake.",
     "If not set, CMake uses \"CMakePredefinedTargets\" as a default "
     "value for this property. Targets such as INSTALL, PACKAGE and "
     "RUN_TESTS will be organized into this FOLDER. See also the "
     "documentation for the FOLDER target property.");

  // ================================================================
  // define variables as well
  // ================================================================
  cmDocumentVariables::DefineVariables(cm);
}


void cmake::DefineProperty(const char *name, cmProperty::ScopeType scope,
                           const char *ShortDescription,
                           const char *FullDescription,
                           bool chained, const char *docSection)
{
  this->PropertyDefinitions[scope].DefineProperty(name,scope,ShortDescription,
                                                  FullDescription,
                                                  docSection,
                                                  chained);
}

bool cmake::GetIsPropertyDefined(const char *name,
                                 cmProperty::ScopeType scope)
{
  return this->PropertyDefinitions[scope].find(name) !=
                                      this->PropertyDefinitions[scope].end();
}

cmPropertyDefinition *cmake
::GetPropertyDefinition(const char *name,
                        cmProperty::ScopeType scope)
{
  if (this->IsPropertyDefined(name,scope))
    {
    return &(this->PropertyDefinitions[scope][name]);
    }
  return 0;
}

void cmake::RecordPropertyAccess(const char *name,
                                 cmProperty::ScopeType scope)
{
  this->AccessedProperties.insert
    (std::pair<cmStdString,cmProperty::ScopeType>(name,scope));
}

void cmake::ReportUndefinedPropertyAccesses(const char *filename)
{
  if(!this->GlobalGenerator)
    { return; }
  FILE *progFile = fopen(filename,"w");
  if(!progFile)
    { return; }

  // what are the enabled languages?
  std::vector<std::string> enLangs;
  this->GlobalGenerator->GetEnabledLanguages(enLangs);

  // Common configuration names.
  // TODO: Compute current configuration(s).
  std::vector<std::string> enConfigs;
  enConfigs.push_back("");
  enConfigs.push_back("DEBUG");
  enConfigs.push_back("RELEASE");
  enConfigs.push_back("MINSIZEREL");
  enConfigs.push_back("RELWITHDEBINFO");

  // take all the defined properties and add definitions for all the enabled
  // languages
  std::set<std::pair<cmStdString,cmProperty::ScopeType> > aliasedProperties;
  std::map<cmProperty::ScopeType, cmPropertyDefinitionMap>::iterator i;
  i = this->PropertyDefinitions.begin();
  for (;i != this->PropertyDefinitions.end(); ++i)
    {
    cmPropertyDefinitionMap::iterator j;
    for (j = i->second.begin(); j != i->second.end(); ++j)
      {
      // TODO: What if both <LANG> and <CONFIG> appear?
      if (j->first.find("<CONFIG>") != std::string::npos)
        {
        std::vector<std::string>::const_iterator k;
        for (k = enConfigs.begin(); k != enConfigs.end(); ++k)
          {
          std::string tmp = j->first;
          cmSystemTools::ReplaceString(tmp, "<CONFIG>", k->c_str());
          // add alias
          aliasedProperties.insert
            (std::pair<cmStdString,cmProperty::ScopeType>(tmp,i->first));
          }
        }
      if (j->first.find("<LANG>") != std::string::npos)
        {
        std::vector<std::string>::const_iterator k;
        for (k = enLangs.begin(); k != enLangs.end(); ++k)
          {
          std::string tmp = j->first;
          cmSystemTools::ReplaceString(tmp, "<LANG>", k->c_str());
          // add alias
          aliasedProperties.insert
            (std::pair<cmStdString,cmProperty::ScopeType>(tmp,i->first));
          }
        }
      }
    }

  std::set<std::pair<cmStdString,cmProperty::ScopeType> >::const_iterator ap;
  ap = this->AccessedProperties.begin();
  for (;ap != this->AccessedProperties.end(); ++ap)
    {
    if (!this->IsPropertyDefined(ap->first.c_str(),ap->second) &&
        aliasedProperties.find(std::pair<cmStdString,cmProperty::ScopeType>
                               (ap->first,ap->second)) ==
        aliasedProperties.end())
      {
      const char *scopeStr = "";
      switch (ap->second)
        {
        case cmProperty::TARGET:
          scopeStr = "TARGET";
          break;
        case cmProperty::SOURCE_FILE:
          scopeStr = "SOURCE_FILE";
        break;
        case cmProperty::DIRECTORY:
          scopeStr = "DIRECTORY";
          break;
        case cmProperty::TEST:
          scopeStr = "TEST";
          break;
        case cmProperty::VARIABLE:
          scopeStr = "VARIABLE";
          break;
        case cmProperty::CACHED_VARIABLE:
          scopeStr = "CACHED_VARIABLE";
          break;
        default:
          scopeStr = "unknown";
        break;
        }
      fprintf(progFile, "%s with scope %s\n", ap->first.c_str(), scopeStr);
      }
    }
  fclose(progFile);
}

bool cmake::IsPropertyDefined(const char *name, cmProperty::ScopeType scope)
{
  return this->PropertyDefinitions[scope].IsPropertyDefined(name);
}

bool cmake::IsPropertyChained(const char *name, cmProperty::ScopeType scope)
{
  return this->PropertyDefinitions[scope].IsPropertyChained(name);
}

void cmake::SetProperty(const char* prop, const char* value)
{
  if (!prop)
    {
    return;
    }

  // Special hook to invalidate cached value.
  if(strcmp(prop, "DEBUG_CONFIGURATIONS") == 0)
    {
    this->DebugConfigs.clear();
    }

  this->Properties.SetProperty(prop, value, cmProperty::GLOBAL);
}

void cmake::AppendProperty(const char* prop, const char* value, bool asString)
{
  if (!prop)
    {
    return;
    }

  // Special hook to invalidate cached value.
  if(strcmp(prop, "DEBUG_CONFIGURATIONS") == 0)
    {
    this->DebugConfigs.clear();
    }

  this->Properties.AppendProperty(prop, value, cmProperty::GLOBAL, asString);
}

const char *cmake::GetProperty(const char* prop)
{
  return this->GetProperty(prop, cmProperty::GLOBAL);
}

const char *cmake::GetProperty(const char* prop, cmProperty::ScopeType scope)
{
  if(!prop)
    {
    return 0;
    }
  bool chain = false;

  // watch for special properties
  std::string propname = prop;
  std::string output = "";
  if ( propname == "CACHE_VARIABLES" )
    {
    cmCacheManager::CacheIterator cit =
      this->GetCacheManager()->GetCacheIterator();
    for ( cit.Begin(); !cit.IsAtEnd(); cit.Next() )
      {
      if ( output.size() )
        {
        output += ";";
        }
      output += cit.GetName();
      }
    this->SetProperty("CACHE_VARIABLES", output.c_str());
    }
  else if ( propname == "COMMANDS" )
    {
    cmake::RegisteredCommandsMap::iterator cmds
        = this->GetCommands()->begin();
    for (unsigned int cc=0 ; cmds != this->GetCommands()->end(); ++ cmds )
      {
      if ( cc > 0 )
        {
        output += ";";
        }
      output += cmds->first.c_str();
      cc++;
      }
    this->SetProperty("COMMANDS",output.c_str());
    }
  else if ( propname == "IN_TRY_COMPILE" )
    {
    this->SetProperty("IN_TRY_COMPILE",
                      this->GetIsInTryCompile()? "1":"0");
    }
  else if ( propname == "ENABLED_LANGUAGES" )
    {
    std::string lang;
    if(this->GlobalGenerator)
      {
      std::vector<std::string> enLangs;
      this->GlobalGenerator->GetEnabledLanguages(enLangs);
      const char* sep = "";
      for(std::vector<std::string>::iterator i = enLangs.begin();
          i != enLangs.end(); ++i)
        {
        lang += sep;
        sep = ";";
        lang += *i;
        }
      }
    this->SetProperty("ENABLED_LANGUAGES", lang.c_str());
    }
  return this->Properties.GetPropertyValue(prop, scope, chain);
}

bool cmake::GetPropertyAsBool(const char* prop)
{
  return cmSystemTools::IsOn(this->GetProperty(prop));
}

int cmake::GetSystemInformation(std::vector<std::string>& args)
{
  // so create the directory
  std::string resultFile;
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  std::string destPath = cwd + "/__cmake_systeminformation";
  cmSystemTools::RemoveADirectory(destPath.c_str());
  if (!cmSystemTools::MakeDirectory(destPath.c_str()))
    {
    std::cerr << "Error: --system-information must be run from a "
      "writable directory!\n";
    return 1;
    }

  // process the arguments
  bool writeToStdout = true;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-V",0) == 0)
      {
      this->Verbose = true;
      }
    else if(arg.find("-G",0) == 0)
      {
      std::string value = arg.substr(2);
      if(value.size() == 0)
        {
        ++i;
        if(i >= args.size())
          {
          cmSystemTools::Error("No generator specified for -G");
          return -1;
          }
        value = args[i];
        }
      cmGlobalGenerator* gen =
        this->CreateGlobalGenerator(value.c_str());
      if(!gen)
        {
        cmSystemTools::Error("Could not create named generator ",
                             value.c_str());
        }
      else
        {
        this->SetGlobalGenerator(gen);
        }
      }
    // no option assume it is the output file
    else
      {
      if (!cmSystemTools::FileIsFullPath(arg.c_str()))
        {
        resultFile = cwd;
        resultFile += "/";
        }
      resultFile += arg;
      writeToStdout = false;
      }
    }


  // we have to find the module directory, so we can copy the files
  this->AddCMakePaths();
  std::string modulesPath =
    this->CacheManager->GetCacheValue("CMAKE_ROOT");
  modulesPath += "/Modules";
  std::string inFile = modulesPath;
  inFile += "/SystemInformation.cmake";
  std::string outFile = destPath;
  outFile += "/CMakeLists.txt";

  // Copy file
  if(!cmSystemTools::cmCopyFile(inFile.c_str(), outFile.c_str()))
    {
    std::cerr << "Error copying file \"" << inFile.c_str()
              << "\" to \"" << outFile.c_str() << "\".\n";
    return 1;
    }

  // do we write to a file or to stdout?
  if (resultFile.size() == 0)
    {
    resultFile = cwd;
    resultFile += "/__cmake_systeminformation/results.txt";
    }

  // now run cmake on the CMakeLists file
  cmSystemTools::ChangeDirectory(destPath.c_str());
  std::vector<std::string> args2;
  args2.push_back(args[0]);
  args2.push_back(destPath);
  std::string resultArg = "-DRESULT_FILE=";
  resultArg += resultFile;
  args2.push_back(resultArg);
  int res = this->Run(args2, false);

  if (res != 0)
    {
    std::cerr << "Error: --system-information failed on internal CMake!\n";
    return res;
    }

  // change back to the original directory
  cmSystemTools::ChangeDirectory(cwd.c_str());

  // echo results to stdout if needed
  if (writeToStdout)
    {
    FILE* fin = fopen(resultFile.c_str(), "r");
    if(fin)
      {
      const int bufferSize = 4096;
      char buffer[bufferSize];
      size_t n;
      while((n = fread(buffer, 1, bufferSize, fin)) > 0)
        {
        for(char* c = buffer; c < buffer+n; ++c)
          {
          putc(*c, stdout);
          }
        fflush(stdout);
        }
      fclose(fin);
      }
    }

  // clean up the directory
  cmSystemTools::RemoveADirectory(destPath.c_str());
  return 0;
}

//----------------------------------------------------------------------------
static bool cmakeCheckStampFile(const char* stampName)
{
  // The stamp file does not exist.  Use the stamp dependencies to
  // determine whether it is really out of date.  This works in
  // conjunction with cmLocalVisualStudio7Generator to avoid
  // repeatedly re-running CMake when the user rebuilds the entire
  // solution.
  std::string stampDepends = stampName;
  stampDepends += ".depend";
#if defined(_WIN32) || defined(__CYGWIN__)
  std::ifstream fin(stampDepends.c_str(), std::ios::in | std::ios::binary);
#else
  std::ifstream fin(stampDepends.c_str(), std::ios::in);
#endif
  if(!fin)
    {
    // The stamp dependencies file cannot be read.  Just assume the
    // build system is really out of date.
    std::cout << "CMake is re-running because " << stampName
              << " dependency file is missing.\n";
    return false;
    }

  // Compare the stamp dependencies against the dependency file itself.
  cmFileTimeComparison ftc;
  std::string dep;
  while(cmSystemTools::GetLineFromStream(fin, dep))
    {
    int result;
    if(dep.length() >= 1 && dep[0] != '#' &&
       (!ftc.FileTimeCompare(stampDepends.c_str(), dep.c_str(), &result)
        || result < 0))
      {
      // The stamp depends file is older than this dependency.  The
      // build system is really out of date.
      std::cout << "CMake is re-running because " << stampName
                << " is out-of-date.\n";
      std::cout << "  the file '" << dep << "'\n";
      std::cout << "  is newer than '" << stampDepends << "'\n";
      std::cout << "  result='" << result << "'\n";
      return false;
      }
    }

  // The build system is up to date.  The stamp file has been removed
  // by the VS IDE due to a "rebuild" request.  Restore it atomically.
  cmOStringStream stampTempStream;
  stampTempStream << stampName << ".tmp" << cmSystemTools::RandomSeed();
  std::string stampTempString = stampTempStream.str();
  const char* stampTemp = stampTempString.c_str();
  {
  // TODO: Teach cmGeneratedFileStream to use a random temp file (with
  // multiple tries in unlikely case of conflict) and use that here.
  std::ofstream stamp(stampTemp);
  stamp << "# CMake generation timestamp file for this directory.\n";
  }
  if(cmSystemTools::RenameFile(stampTemp, stampName))
    {
    // Notify the user why CMake is not re-running.  It is safe to
    // just print to stdout here because this code is only reachable
    // through an undocumented flag used by the VS generator.
    std::cout << "CMake does not need to re-run because "
              << stampName << " is up-to-date.\n";
    return true;
    }
  else
    {
    cmSystemTools::RemoveFile(stampTemp);
    cmSystemTools::Error("Cannot restore timestamp ", stampName);
    return false;
    }
}

//----------------------------------------------------------------------------
static bool cmakeCheckStampList(const char* stampList)
{
  // If the stamp list does not exist CMake must rerun to generate it.
  if(!cmSystemTools::FileExists(stampList))
    {
    std::cout << "CMake is re-running because generate.stamp.list "
              << "is missing.\n";
    return false;
    }
  std::ifstream fin(stampList);
  if(!fin)
    {
    std::cout << "CMake is re-running because generate.stamp.list "
              << "could not be read.\n";
    return false;
    }

  // Check each stamp.
  std::string stampName;
  while(cmSystemTools::GetLineFromStream(fin, stampName))
    {
    if(!cmakeCheckStampFile(stampName.c_str()))
      {
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
int cmake::WindowsCEEnvironment(const char* version, const std::string& name)
{
#if defined(CMAKE_HAVE_VS_GENERATORS)
  cmVisualStudioWCEPlatformParser parser(name.c_str());
  parser.ParseVersion(version);
  if (parser.Found())
    {
    std::cout << "@echo off" << std::endl;
    std::cout << "echo Environment Selection: " << name << std::endl;
    std::cout << "set PATH=" << parser.GetPathDirectories() << std::endl;
    std::cout << "set INCLUDE=" << parser.GetIncludeDirectories() <<std::endl;
    std::cout << "set LIB=" << parser.GetLibraryDirectories() <<std::endl;
    return 0;
    }
#else
  (void)version;
#endif

  std::cerr << "Could not find " << name;
  return -1;
}

// For visual studio 2005 and newer manifest files need to be embedded into
// exe and dll's.  This code does that in such a way that incremental linking
// still works.
int cmake::VisualStudioLink(std::vector<std::string>& args, int type)
{
  if(args.size() < 2)
    {
    return -1;
    }
  bool verbose = false;
  if(cmSystemTools::GetEnv("VERBOSE"))
    {
    verbose = true;
    }
  std::vector<std::string> expandedArgs;
  for(std::vector<std::string>::iterator i = args.begin();
      i != args.end(); ++i)
    {
    // check for nmake temporary files
    if((*i)[0] == '@' && i->find("@CMakeFiles") != 0 )
      {
      std::ifstream fin(i->substr(1).c_str());
      std::string line;
      while(cmSystemTools::GetLineFromStream(fin,
                                             line))
        {
        cmSystemTools::ParseWindowsCommandLine(line.c_str(), expandedArgs);
        }
      }
    else
      {
      expandedArgs.push_back(*i);
      }
    }
  bool hasIncremental = false;
  bool hasManifest = true;
  for(std::vector<std::string>::iterator i = expandedArgs.begin();
      i != expandedArgs.end(); ++i)
    {
    if(cmSystemTools::Strucmp(i->c_str(), "/INCREMENTAL:YES") == 0)
      {
      hasIncremental = true;
      }
    if(cmSystemTools::Strucmp(i->c_str(), "/INCREMENTAL") == 0)
      {
      hasIncremental = true;
      }
    if(cmSystemTools::Strucmp(i->c_str(), "/MANIFEST:NO") == 0)
      {
      hasManifest = false;
      }
    }
  if(hasIncremental && hasManifest)
    {
    if(verbose)
      {
      std::cout << "Visual Studio Incremental Link with embedded manifests\n";
      }
    return cmake::VisualStudioLinkIncremental(expandedArgs, type, verbose);
    }
  if(verbose)
    {
    if(!hasIncremental)
      {
      std::cout << "Visual Studio Non-Incremental Link\n";
      }
    else
      {
      std::cout << "Visual Studio Incremental Link without manifests\n";
      }
    }
  return cmake::VisualStudioLinkNonIncremental(expandedArgs,
                                               type, hasManifest, verbose);
}

int cmake::ParseVisualStudioLinkCommand(std::vector<std::string>& args,
                                        std::vector<cmStdString>& command,
                                        std::string& targetName)
{
  std::vector<std::string>::iterator i = args.begin();
  i++; // skip -E
  i++; // skip vs_link_dll or vs_link_exe
  command.push_back(*i);
  i++; // move past link command
  for(; i != args.end(); ++i)
    {
    command.push_back(*i);
    if(i->find("/Fe") == 0)
      {
      targetName = i->substr(3);
      }
    if(i->find("/out:") == 0)
      {
      targetName = i->substr(5);
      }
    }
  if(targetName.size() == 0 || command.size() == 0)
    {
    return -1;
    }
  return 0;
}

bool cmake::RunCommand(const char* comment,
                       std::vector<cmStdString>& command,
                       bool verbose,
                       int* retCodeOut)
{
  if(verbose)
    {
    std::cout << comment << ":\n";
    for(std::vector<cmStdString>::iterator i = command.begin();
        i != command.end(); ++i)
      {
      std::cout << i->c_str() << " ";
      }
    std::cout << "\n";
    }
  std::string output;
  int retCode =0;
  // use rc command to create .res file
  cmSystemTools::RunSingleCommand(command,
                                  &output,
                                  &retCode, 0, cmSystemTools::OUTPUT_NONE);
  // always print the output of the command, unless
  // it is the dumb rc command banner, but if the command
  // returned an error code then print the output anyway as
  // the banner may be mixed with some other important information.
  if(output.find("Resource Compiler Version") == output.npos
     || retCode !=0)
    {
    std::cout << output;
    }
  // if retCodeOut is requested then always return true
  // and set the retCodeOut to retCode
  if(retCodeOut)
    {
    *retCodeOut = retCode;
    return true;
    }
  if(retCode != 0)
    {
    std::cout << comment << " failed. with " << retCode << "\n";
    }
  return retCode == 0;
}

int cmake::VisualStudioLinkIncremental(std::vector<std::string>& args,
                                       int type, bool verbose)
{
  // This follows the steps listed here:
  // http://blogs.msdn.com/zakramer/archive/2006/05/22/603558.aspx

  //    1.  Compiler compiles the application and generates the *.obj files.
  //    2.  An empty manifest file is generated if this is a clean build and if
  //    not the previous one is reused.
  //    3.  The resource compiler (rc.exe) compiles the *.manifest file to a
  //    *.res file.
  //    4.  Linker generates the binary (EXE or DLL) with the /incremental
  //    switch and embeds the dummy manifest file. The linker also generates
  //    the real manifest file based on the binaries that your binary depends
  //    on.
  //    5.  The manifest tool (mt.exe) is then used to generate the final
  //    manifest.

  // If the final manifest is changed, then 6 and 7 are run, if not
  // they are skipped, and it is done.

  //    6.  The resource compiler is invoked one more time.
  //    7.  Finally, the Linker does another incremental link, but since the
  //    only thing that has changed is the *.res file that contains the
  //    manifest it is a short link.
  std::vector<cmStdString> linkCommand;
  std::string targetName;
  if(cmake::ParseVisualStudioLinkCommand(args, linkCommand, targetName) == -1)
    {
    return -1;
    }
  std::string manifestArg = "/MANIFESTFILE:";
  std::vector<cmStdString> rcCommand;
  rcCommand.push_back(cmSystemTools::FindProgram("rc.exe"));
  std::vector<cmStdString> mtCommand;
  mtCommand.push_back(cmSystemTools::FindProgram("mt.exe"));
  std::string tempManifest;
  tempManifest = targetName;
  tempManifest += ".intermediate.manifest";
  std::string resourceInputFile = targetName;
  resourceInputFile += ".resource.txt";
  if(verbose)
    {
    std::cout << "Create " << resourceInputFile.c_str() << "\n";
    }
  // Create input file for rc command
  std::ofstream fout(resourceInputFile.c_str());
  if(!fout)
    {
    return -1;
    }
  std::string manifestFile = targetName;
  manifestFile += ".embed.manifest";
  std::string fullPath= cmSystemTools::CollapseFullPath(manifestFile.c_str());
  fout << type << " /* CREATEPROCESS_MANIFEST_RESOURCE_ID "
    "*/ 24 /* RT_MANIFEST */ " << "\"" << fullPath.c_str() << "\"";
  fout.close();
  manifestArg += tempManifest;
  // add the manifest arg to the linkCommand
  linkCommand.push_back("/MANIFEST");
  linkCommand.push_back(manifestArg);
  // if manifestFile is not yet created, create an
  // empty one
  if(!cmSystemTools::FileExists(manifestFile.c_str()))
    {
    if(verbose)
      {
      std::cout << "Create empty: " << manifestFile.c_str() << "\n";
      }
    std::ofstream foutTmp(manifestFile.c_str());
    }
  std::string resourceFile = manifestFile;
  resourceFile += ".res";
  // add the resource file to the end of the link command
  linkCommand.push_back(resourceFile);
  std::string outputOpt = "/fo";
  outputOpt += resourceFile;
  rcCommand.push_back(outputOpt);
  rcCommand.push_back(resourceInputFile);
  // Run rc command to create resource
  if(!cmake::RunCommand("RC Pass 1", rcCommand, verbose))
    {
    return -1;
    }
  // Now run the link command to link and create manifest
  if(!cmake::RunCommand("LINK Pass 1", linkCommand, verbose))
    {
    return -1;
    }
  // create mt command
  std::string outArg("/out:");
  outArg+= manifestFile;
  mtCommand.push_back("/nologo");
  mtCommand.push_back(outArg);
  mtCommand.push_back("/notify_update");
  mtCommand.push_back("/manifest");
  mtCommand.push_back(tempManifest);
  //  now run mt.exe to create the final manifest file
  int mtRet =0;
  cmake::RunCommand("MT", mtCommand, verbose, &mtRet);
  // if mt returns 0, then the manifest was not changed and
  // we do not need to do another link step
  if(mtRet == 0)
    {
    return 0;
    }
  // check for magic mt return value if mt returns the magic number
  // 1090650113 then it means that it updated the manifest file and we need
  // to do the final link.  If mt has any value other than 0 or 1090650113
  // then there was some problem with the command itself and there was an
  // error so return the error code back out of cmake so make can report it.
  if(mtRet != 1090650113)
    {
    return mtRet;
    }
  // update the resource file with the new manifest from the mt command.
  if(!cmake::RunCommand("RC Pass 2", rcCommand, verbose))
    {
    return -1;
    }
  // Run the final incremental link that will put the new manifest resource
  // into the file incrementally.
  if(!cmake::RunCommand("FINAL LINK", linkCommand, verbose))
    {
    return -1;
    }
  return 0;
}

int cmake::VisualStudioLinkNonIncremental(std::vector<std::string>& args,
                                          int type,
                                          bool hasManifest,
                                          bool verbose)
{
  std::vector<cmStdString> linkCommand;
  std::string targetName;
  if(cmake::ParseVisualStudioLinkCommand(args, linkCommand, targetName) == -1)
    {
    return -1;
    }
  // Run the link command as given
  if (hasManifest)
    {
    linkCommand.push_back("/MANIFEST");
    }
  if(!cmake::RunCommand("LINK", linkCommand, verbose))
    {
    return -1;
    }
  if(!hasManifest)
    {
    return 0;
    }
  std::vector<cmStdString> mtCommand;
  mtCommand.push_back(cmSystemTools::FindProgram("mt.exe"));
  mtCommand.push_back("/nologo");
  mtCommand.push_back("/manifest");
  std::string manifestFile = targetName;
  manifestFile += ".manifest";
  mtCommand.push_back(manifestFile);
  std::string outresource = "/outputresource:";
  outresource += targetName;
  outresource += ";#";
  if(type == 1)
    {
    outresource += "1";
    }
  else if(type == 2)
    {
    outresource += "2";
    }
  mtCommand.push_back(outresource);
  // Now use the mt tool to embed the manifest into the exe or dll
  if(!cmake::RunCommand("MT", mtCommand, verbose))
    {
    return -1;
    }
  return 0;
}

//----------------------------------------------------------------------------
void cmake::IssueMessage(cmake::MessageType t, std::string const& text,
                         cmListFileBacktrace const& backtrace)
{
  cmOStringStream msg;
  bool isError = false;
  // Construct the message header.
  if(t == cmake::FATAL_ERROR)
    {
    isError = true;
    msg << "CMake Error";
    }
  else if(t == cmake::INTERNAL_ERROR)
    {
    isError = true;
    msg << "CMake Internal Error (please report a bug)";
    }
  else if(t == cmake::LOG)
    {
    msg << "CMake Debug Log";
    }
  else if(t == cmake::DEPRECATION_ERROR)
    {
    msg << "CMake Deprecation Error";
    isError = true;
    }
  else if (t == cmake::DEPRECATION_WARNING)
    {
    msg << "CMake Deprecation Warning";
    }
  else
    {
    msg << "CMake Warning";
    if(t == cmake::AUTHOR_WARNING)
      {
      // Allow suppression of these warnings.
      cmCacheManager::CacheIterator it = this->CacheManager
        ->GetCacheIterator("CMAKE_SUPPRESS_DEVELOPER_WARNINGS");
      if(!it.IsAtEnd() && it.GetValueAsBool())
        {
        return;
        }
      msg << " (dev)";
      }
    }

  // Add the immediate context.
  cmListFileBacktrace::const_iterator i = backtrace.begin();
  if(i != backtrace.end())
    {
    cmListFileContext const& lfc = *i;
    msg << (lfc.Line? " at ": " in ") << lfc;
    ++i;
    }

  // Add the message text.
  {
  msg << ":\n";
  cmDocumentationFormatterText formatter;
  formatter.SetIndent("  ");
  formatter.PrintFormatted(msg, text.c_str());
  }

  // Add the rest of the context.
  if(i != backtrace.end())
    {
    msg << "Call Stack (most recent call first):\n";
    while(i != backtrace.end())
      {
      cmListFileContext const& lfc = *i;
      msg << "  " << lfc << "\n";
      ++i;
      }
    }

  // Add a note about warning suppression.
  if(t == cmake::AUTHOR_WARNING)
    {
    msg <<
      "This warning is for project developers.  Use -Wno-dev to suppress it.";
    }

  // Add a terminating blank line.
  msg << "\n";

  // Output the message.
  if(isError)
    {
    cmSystemTools::SetErrorOccured();
    cmSystemTools::Message(msg.str().c_str(), "Error");
    }
  else
    {
    cmSystemTools::Message(msg.str().c_str(), "Warning");
    }
}

//----------------------------------------------------------------------------
std::vector<std::string> const& cmake::GetDebugConfigs()
{
  // Compute on-demand.
  if(this->DebugConfigs.empty())
    {
    if(const char* config_list = this->GetProperty("DEBUG_CONFIGURATIONS"))
      {
      // Expand the specified list and convert to upper-case.
      cmSystemTools::ExpandListArgument(config_list, this->DebugConfigs);
      for(std::vector<std::string>::iterator i = this->DebugConfigs.begin();
          i != this->DebugConfigs.end(); ++i)
        {
        *i = cmSystemTools::UpperCase(*i);
        }
      }
    // If no configurations were specified, use a default list.
    if(this->DebugConfigs.empty())
      {
      this->DebugConfigs.push_back("DEBUG");
      }
    }
  return this->DebugConfigs;
}


int cmake::Build(const std::string& dir,
                 const std::string& target,
                 const std::string& config,
                 const std::vector<std::string>& nativeOptions,
                 bool clean,
                 cmSystemTools::OutputOption outputflag)
{
  if(!cmSystemTools::FileIsDirectory(dir.c_str()))
    {
    std::cerr << "Error: " << dir << " is not a directory\n";
    return 1;
    }
  std::string cachePath = dir;
  cmSystemTools::ConvertToUnixSlashes(cachePath);
  cmCacheManager* cachem = this->GetCacheManager();
  cmCacheManager::CacheIterator it = cachem->NewIterator();
  if(!cachem->LoadCache(cachePath.c_str()))
    {
    std::cerr << "Error: could not load cache\n";
    return 1;
    }
  if(!it.Find("CMAKE_GENERATOR"))
    {
    std::cerr << "Error: could find generator in Cache\n";
    return 1;
    }
  cmsys::auto_ptr<cmGlobalGenerator> gen(
    this->CreateGlobalGenerator(it.GetValue()));
  std::string output;
  std::string projName;
  std::string makeProgram;
  if(!it.Find("CMAKE_PROJECT_NAME"))
    {
    std::cerr << "Error: could not find CMAKE_PROJECT_NAME in Cache\n";
    return 1;
    }
  projName = it.GetValue();
  if(!it.Find("CMAKE_MAKE_PROGRAM"))
    {
    std::cerr << "Error: could not find CMAKE_MAKE_PROGRAM in Cache\n";
    return 1;
    }
  makeProgram = it.GetValue();
  return gen->Build(0, dir.c_str(),
                    projName.c_str(), target.c_str(),
                    &output,
                    makeProgram.c_str(),
                    config.c_str(), clean, false, 0, outputflag,
                    0, nativeOptions);
}

void cmake::WatchUnusedCli(const char* var)
{
#ifdef CMAKE_BUILD_WITH_CMAKE
  this->VariableWatch->AddWatch(var, cmWarnUnusedCliWarning, this);
  if(this->UsedCliVariables.find(var) == this->UsedCliVariables.end())
    {
    this->UsedCliVariables[var] = false;
    }
#endif
}

void cmake::UnwatchUnusedCli(const char* var)
{
#ifdef CMAKE_BUILD_WITH_CMAKE
  this->VariableWatch->RemoveWatch(var, cmWarnUnusedCliWarning);
  this->UsedCliVariables.erase(var);
#endif
}

void cmake::RunCheckForUnusedVariables()
{
#ifdef CMAKE_BUILD_WITH_CMAKE
  bool haveUnused = false;
  cmOStringStream msg;
  msg << "Manually-specified variables were not used by the project:";
  for(std::map<cmStdString, bool>::const_iterator
        it = this->UsedCliVariables.begin();
      it != this->UsedCliVariables.end(); ++it)
    {
    if(!it->second)
      {
      haveUnused = true;
      msg << "\n  " << it->first;
      }
    }
  if(haveUnused)
    {
    this->IssueMessage(cmake::WARNING, msg.str(), cmListFileBacktrace());
    }
#endif
}
