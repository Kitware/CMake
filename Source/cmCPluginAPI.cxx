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
/* 
   this file contains the implementation of the C API to CMake. Generally
   these routines just manipulate arguments and then call the associated
   methods on the CMake classes. */

#include "cmMakefile.h"
#include "cmCPluginAPI.h"

#include "cmSourceFile.h"

extern "C" 
{
  
void *cmGetClientData(void *info)
{
  return ((cmLoadedCommandInfo *)info)->ClientData;
}

void cmSetClientData(void *info, void *cd)
{
  ((cmLoadedCommandInfo *)info)->ClientData = cd;
}

void cmSetError(void *info, const char *err)
{
  if (((cmLoadedCommandInfo *)info)->Error)
    {
    free(((cmLoadedCommandInfo *)info)->Error);
    }
  ((cmLoadedCommandInfo *)info)->Error = strdup(err);
}

unsigned int cmGetCacheMajorVersion(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetCacheMajorVersion();
}
unsigned int cmGetCacheMinorVersion(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetCacheMinorVersion();
}

unsigned int cmGetMajorVersion(void *)
{
  return cmMakefile::GetMajorVersion();
}

unsigned int cmGetMinorVersion(void *)
{
  return cmMakefile::GetMinorVersion();
}

void cmAddDefinition(void *arg, const char* name, const char* value)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  mf->AddDefinition(name,value);
}

/* Add a definition to this makefile and the global cmake cache. */
void cmAddCacheDefinition(void *arg, const char* name, const char* value, 
                          const char* doc,
                          int type)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  
  switch (type)
    {
    case CM_CACHE_BOOL:
      mf->AddCacheDefinition(name,value,doc,
                             cmCacheManager::BOOL);
      break;
    case CM_CACHE_PATH:
      mf->AddCacheDefinition(name,value,doc,
                             cmCacheManager::PATH);
      break;
    case CM_CACHE_FILEPATH:
      mf->AddCacheDefinition(name,value,doc,
                             cmCacheManager::FILEPATH);
      break;
    case CM_CACHE_STRING:
      mf->AddCacheDefinition(name,value,doc,
                             cmCacheManager::STRING);
      break;
    case CM_CACHE_INTERNAL:
      mf->AddCacheDefinition(name,value,doc,
                             cmCacheManager::INTERNAL);
      break;
    case CM_CACHE_STATIC:
      mf->AddCacheDefinition(name,value,doc,
                             cmCacheManager::STATIC);
      break;
    }
}

const char* cmGetProjectName(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetProjectName();
}

const char* cmGetHomeDirectory(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetHomeDirectory();
}
const char* cmGetHomeOutputDirectory(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetHomeOutputDirectory();
}
const char* cmGetStartDirectory(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetStartDirectory();
}
const char* cmGetStartOutputDirectory(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetStartOutputDirectory();
}
const char* cmGetCurrentDirectory(void *arg) 
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetCurrentDirectory();
}
const char* cmGetCurrentOutputDirectory(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetCurrentOutputDirectory();
}
const char* cmGetDefinition(void *arg,const char*def)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetDefinition(def);
}

int cmIsOn(void *arg, const char* name)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return static_cast<int>(mf->IsOn(name));
}

/** Check if a command exists. */
int cmCommandExists(void *arg, const char* name)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return static_cast<int>(mf->CommandExists(name));
}

void cmAddDefineFlag(void *arg, const char* definition)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  mf->AddDefineFlag(definition);
}

void cmAddLinkDirectoryForTarget(void *arg, const char *tgt, const char* d)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  mf->AddLinkDirectoryForTarget(tgt,d);
}


void cmAddExecutable(void *arg, const char *exename, 
                     int numSrcs, const char **srcs, int win32)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  std::vector<std::string> srcs2;
  int i;
  for (i = 0; i < numSrcs; ++i)
    {
    srcs2.push_back(srcs[i]);
    }
 cmTarget* tg =  mf->AddExecutable(exename, srcs2);
 if ( win32 )
   {
   tg->SetProperty("WIN32_EXECUTABLE", "ON");
   }
}

void cmAddUtilityCommand(void *arg, const char* utilityName,
                         const char* command,
                         const char* arguments,
                         int all,
                         int numDepends,
                         const char **depends,
                         int numOutputs,
                         const char **outputs)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  std::vector<std::string> depends2;
  int i;
  for (i = 0; i < numDepends; ++i)
    {
    depends2.push_back(depends[i]);
    }
  std::vector<std::string> outputs2;
  for (i = 0; i < numOutputs; ++i)
    {
    outputs2.push_back(outputs[i]);
    }
  mf->AddUtilityCommand(utilityName,command,arguments, (all ? true : false),
                        depends2, outputs2);
}
void cmAddCustomCommand(void *arg, const char* source,
                        const char* command,
                        int numArgs, const char **args,
                        int numDepends, const char **depends,
                        int numOutputs, const char **outputs,
                        const char *target)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  int i;
  std::vector<std::string> args2;
  for (i = 0; i < numArgs; ++i)
    {
    args2.push_back(args[i]);
    }
  std::vector<std::string> depends2;
  for (i = 0; i < numDepends; ++i)
    {
    depends2.push_back(depends[i]);
    }
  std::vector<std::string> outputs2;
  for (i = 0; i < numOutputs; ++i)
    {
    outputs2.push_back(outputs[i]);
    }
  mf->AddCustomCommand(source, command, args2, depends2, outputs2, target);
}

void cmAddCustomCommandToOutput(void *arg, const char* output,
                                const char* command,
                                int numArgs, const char **args,
                                const char* main_dependency,
                                int numDepends, const char **depends)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  int i;
  std::vector<std::string> args2;
  for (i = 0; i < numArgs; ++i)
    {
    args2.push_back(args[i]);
    }
  std::vector<std::string> depends2;
  for (i = 0; i < numDepends; ++i)
    {
    depends2.push_back(depends[i]);
    }
  mf->AddCustomCommandToOutput(output, command, args2, main_dependency,
                               depends2);
}

void cmAddCustomCommandToTarget(void *arg, const char* target,
                                const char* command,
                                int numArgs, const char **args,
                                int commandType)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  int i;
  std::vector<std::string> args2;
  for (i = 0; i < numArgs; ++i)
    {
    args2.push_back(args[i]);
    }
  switch (commandType)
    {
    case CM_PRE_BUILD:
      mf->AddCustomCommandToTarget(target, command, args2, 
                                   cmTarget::PRE_BUILD);
      break;
    case CM_PRE_LINK:
      mf->AddCustomCommandToTarget(target, command, args2, 
                                   cmTarget::PRE_LINK);
      break;
    case CM_POST_BUILD:
      mf->AddCustomCommandToTarget(target, command, args2, 
                                   cmTarget::POST_BUILD);
      break;
    }
}

void cmAddLinkLibraryForTarget(void *arg, const char *tgt, const char*value, 
                               int libtype)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  
  switch (libtype)
    {
    case CM_LIBRARY_GENERAL:
      mf->AddLinkLibraryForTarget(tgt,value, cmTarget::GENERAL);
      break;
    case CM_LIBRARY_DEBUG:
      mf->AddLinkLibraryForTarget(tgt,value, cmTarget::DEBUG);
      break;
    case CM_LIBRARY_OPTIMIZED:
      mf->AddLinkLibraryForTarget(tgt,value, cmTarget::OPTIMIZED);
      break;
    }
}

void cmAddLibrary(void *arg, const char *libname, int shared,
                  int numSrcs, const char **srcs)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  std::vector<std::string> srcs2;
  int i;
  for (i = 0; i < numSrcs; ++i)
    {
    srcs2.push_back(srcs[i]);
    }
  mf->AddLibrary(libname, (shared ? true : false), srcs2);
}

char *cmExpandVariablesInString(void *arg, const char *source, 
                                int escapeQuotes, int atOnly)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  std::string barf = source;
  std::string result = 
    mf->ExpandVariablesInString(barf, 
                                (escapeQuotes ? true : false),
                                (atOnly ? true : false));
  char *res = static_cast<char *>(malloc(result.size() + 1));
  if (result.size())
    {
    strcpy(res,result.c_str());
    }
  res[result.size()] = '\0';
  return res;
}


int cmExecuteCommand(void *arg, const char *name, 
                     int numArgs, const char **args)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  cmListFileFunction lff;
  lff.m_Name = name;
  for(int i = 0; i < numArgs; ++i)
    {
    // Assume all arguments are quoted.
    lff.m_Arguments.push_back(cmListFileArgument(args[i], true,
                                                 "[CMake-Plugin]", 0));
    }
  return mf->ExecuteCommand(lff);
}

void cmExpandSourceListArguments(void *arg, 
                                 int numArgs,
                                 const char **args,
                                 int *resArgc,
                                 char ***resArgv,
                                 unsigned int startArgumentIndex)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  std::vector<std::string> result;
  std::vector<std::string> args2;
  int i;
  for (i = 0; i < numArgs; ++i)
    {
    args2.push_back(args[i]);
    }
  mf->ExpandSourceListArguments(args2, result, startArgumentIndex);
  int resargc = static_cast<int>(result.size());
  char **resargv = 0;
  if (resargc)
    {
    resargv = (char **)malloc(resargc*sizeof(char *));
    }
  for (i = 0; i < resargc; ++i)
    {
    resargv[i] = strdup(result[i].c_str());
    }
  *resArgc = resargc;
  *resArgv = resargv;
}

void cmFreeArguments(int argc, char **argv)
{
  int i;
  for (i = 0; i < argc; ++i)
    {
    free(argv[i]);
    }
  if (argv)
    {
    free(argv);
    }
}

int cmGetTotalArgumentSize(int argc, char **argv)
{
  int i;
  int result = 0;
  for (i = 0; i < argc; ++i)
    {
    if (argv[i])
      {
      result = result + static_cast<int>(strlen(argv[i]));
      }
    }
  return result;
}

void *cmGetSource(void *arg, const char *name)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return (void *)mf->GetSource(name);
}

void * cmAddSource(void *arg, void *arg2)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg2);
  return (void *)mf->AddSource(*sf);
}


void * cmCreateSourceFile()
{
  return (void *)(new cmSourceFile);
}

void cmDestroySourceFile(void *arg)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  delete sf;
}

const char *cmSourceFileGetSourceName(void *arg)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  return sf->GetSourceName().c_str();
}

const char *cmSourceFileGetFullPath(void *arg)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  return sf->GetFullPath().c_str();
}

const char *cmSourceFileGetProperty(void *arg,const char *prop)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  return sf->GetProperty(prop);
}

int cmSourceFileGetPropertyAsBool(void *arg,const char *prop)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  return (sf->GetPropertyAsBool(prop) ? 1: 0);
}

void cmSourceFileSetProperty(void *arg,const char *prop, const char *val)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  sf->SetProperty(prop,val);
}

void cmSourceFileAddDepend(void *arg, const char *depend)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  sf->GetDepends().push_back(depend);
}

void cmSourceFileSetName(void *arg, const char* name, const char* dir,
                         int numSourceExtensions,
                         const char **sourceExtensions,
                         int numHeaderExtensions,
                         const char **headerExtensions)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  std::vector<std::string> srcs;
  std::vector<std::string> hdrs;
  int i;
  for (i = 0; i < numSourceExtensions; ++i)
    {
    srcs.push_back(sourceExtensions[i]);
    }
  for (i = 0; i < numHeaderExtensions; ++i)
    {
    hdrs.push_back(headerExtensions[i]);
    }
  sf->SetName(name,dir, srcs, hdrs);
}

void cmSourceFileSetName2(void *arg, const char* name, const char* dir, 
                          const char *ext, int headerFileOnly)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  sf->SetName(name,dir,ext,(headerFileOnly ? true : false));
}


char *cmGetFilenameWithoutExtension(const char *name)
{
  std::string sres = cmSystemTools::GetFilenameWithoutExtension(name);
  char *result = (char *)malloc(sres.size()+1);  
  strcpy(result,sres.c_str());
  return result;
}

char *cmGetFilenamePath(const char *name)
{
  std::string sres = cmSystemTools::GetFilenamePath(name);
  char *result = (char *)malloc(sres.size()+1);  
  strcpy(result,sres.c_str());
  return result;
}

char *cmCapitalized(const char *name)
{
  std::string sres = cmSystemTools::Capitalized(name);
  char *result = (char *)malloc(sres.size()+1);  
  strcpy(result,sres.c_str());
  return result;
}

void cmCopyFileIfDifferent(const char *name1, const char *name2)
{
  cmSystemTools::CopyFileIfDifferent(name1,name2);
}

void cmRemoveFile(const char *name)
{
  cmSystemTools::RemoveFile(name);
}

void cmDisplayStatus(void *arg, const char* message)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  mf->DisplayStatus(message, -1);
}

void cmFree(void *data)
{
  free(data);
}

} // close the extern "C" scope

cmCAPI cmStaticCAPI =
{
  cmGetClientData,
  cmGetTotalArgumentSize,
  cmFreeArguments,
  cmSetClientData,
  cmSetError,
  cmAddCacheDefinition,
  cmAddCustomCommand,
  cmAddDefineFlag,
  cmAddDefinition,
  cmAddExecutable,
  cmAddLibrary,
  cmAddLinkDirectoryForTarget,
  cmAddLinkLibraryForTarget,
  cmAddUtilityCommand,
  cmCommandExists,
  cmExecuteCommand,
  cmExpandSourceListArguments,
  cmExpandVariablesInString,
  cmGetCacheMajorVersion,
  cmGetCacheMinorVersion,
  cmGetCurrentDirectory,
  cmGetCurrentOutputDirectory,
  cmGetDefinition,
  cmGetHomeDirectory,
  cmGetHomeOutputDirectory,
  cmGetMajorVersion,
  cmGetMinorVersion,
  cmGetProjectName,
  cmGetStartDirectory,
  cmGetStartOutputDirectory,
  cmIsOn,
  
  cmAddSource,
  cmCreateSourceFile,
  cmDestroySourceFile,
  cmGetSource,
  cmSourceFileAddDepend,
  cmSourceFileGetProperty,
  cmSourceFileGetPropertyAsBool,
  cmSourceFileGetSourceName,
  cmSourceFileGetFullPath,
  cmSourceFileSetName,
  cmSourceFileSetName2,
  cmSourceFileSetProperty,
  
  cmCapitalized,
  cmCopyFileIfDifferent,
  cmGetFilenameWithoutExtension,
  cmGetFilenamePath,
  cmRemoveFile,
  cmFree,

  cmAddCustomCommandToOutput,
  cmAddCustomCommandToTarget,
  cmDisplayStatus,
};

