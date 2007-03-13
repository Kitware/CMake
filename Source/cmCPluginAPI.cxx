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
#include "cmVersion.h"

#include "cmSourceFile.h"

#include <stdlib.h>

#ifdef __QNX__
# include <malloc.h> /* for malloc/free on QNX */
#endif

extern "C"
{

void CCONV *cmGetClientData(void *info)
{
  return ((cmLoadedCommandInfo *)info)->ClientData;
}

void CCONV cmSetClientData(void *info, void *cd)
{
  ((cmLoadedCommandInfo *)info)->ClientData = cd;
}

void CCONV cmSetError(void *info, const char *err)
{
  if (((cmLoadedCommandInfo *)info)->Error)
    {
    free(((cmLoadedCommandInfo *)info)->Error);
    }
  ((cmLoadedCommandInfo *)info)->Error = strdup(err);
}

unsigned int CCONV  cmGetCacheMajorVersion(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetCacheMajorVersion();
}
unsigned int CCONV cmGetCacheMinorVersion(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetCacheMinorVersion();
}

unsigned int CCONV cmGetMajorVersion(void *)
{
  return cmVersion::GetMajorVersion();
}

unsigned int CCONV cmGetMinorVersion(void *)
{
  return cmVersion::GetMinorVersion();
}

void CCONV cmAddDefinition(void *arg, const char* name, const char* value)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  mf->AddDefinition(name,value);
}

/* Add a definition to this makefile and the global cmake cache. */
void CCONV cmAddCacheDefinition(void *arg, const char* name,
  const char* value, const char* doc, int type)
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

const char* CCONV cmGetProjectName(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetProjectName();
}

const char* CCONV cmGetHomeDirectory(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetHomeDirectory();
}
const char* CCONV cmGetHomeOutputDirectory(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetHomeOutputDirectory();
}
const char* CCONV cmGetStartDirectory(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetStartDirectory();
}
const char* CCONV cmGetStartOutputDirectory(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetStartOutputDirectory();
}
const char* CCONV cmGetCurrentDirectory(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetCurrentDirectory();
}
const char* CCONV cmGetCurrentOutputDirectory(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetCurrentOutputDirectory();
}
const char* CCONV cmGetDefinition(void *arg,const char*def)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return mf->GetDefinition(def);
}

int CCONV cmIsOn(void *arg, const char* name)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return static_cast<int>(mf->IsOn(name));
}

/** Check if a command exists. */
int CCONV cmCommandExists(void *arg, const char* name)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return static_cast<int>(mf->CommandExists(name));
}

void CCONV cmAddDefineFlag(void *arg, const char* definition)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  mf->AddDefineFlag(definition);
}

void CCONV cmAddLinkDirectoryForTarget(void *arg, const char *tgt,
  const char* d)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  mf->AddLinkDirectoryForTarget(tgt,d);
}


void CCONV cmAddExecutable(void *arg, const char *exename,
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

void CCONV cmAddUtilityCommand(void *arg, const char* utilityName,
                         const char* command,
                         const char* arguments,
                         int all,
                         int numDepends,
                         const char **depends,
                         int,
                         const char **)
{
  // Get the makefile instance.  Perform an extra variable expansion
  // now because the API caller expects it.
  cmMakefile* mf = static_cast<cmMakefile*>(arg);

  // Construct the command line for the command.
  cmCustomCommandLine commandLine;
  std::string expand = command;
  commandLine.push_back(mf->ExpandVariablesInString(expand));
  if(arguments && arguments[0])
    {
    // TODO: Parse arguments!
    expand = arguments;
    commandLine.push_back(mf->ExpandVariablesInString(expand));
    }
  cmCustomCommandLines commandLines;
  commandLines.push_back(commandLine);

  // Accumulate the list of dependencies.
  std::vector<std::string> depends2;
  for(int i = 0; i < numDepends; ++i)
    {
    expand = depends[i];
    depends2.push_back(mf->ExpandVariablesInString(expand));
    }

  // Pass the call to the makefile instance.
  mf->AddUtilityCommand(utilityName, (all ? false : true),
                        0, depends2, commandLines);
}
void CCONV cmAddCustomCommand(void *arg, const char* source,
                        const char* command,
                        int numArgs, const char **args,
                        int numDepends, const char **depends,
                        int numOutputs, const char **outputs,
                        const char *target)
{
  // Get the makefile instance.  Perform an extra variable expansion
  // now because the API caller expects it.
  cmMakefile* mf = static_cast<cmMakefile*>(arg);

  // Construct the command line for the command.
  cmCustomCommandLine commandLine;
  std::string expand = command;
  commandLine.push_back(mf->ExpandVariablesInString(expand));
  for(int i=0; i < numArgs; ++i)
    {
    expand = args[i];
    commandLine.push_back(mf->ExpandVariablesInString(expand));
    }
  cmCustomCommandLines commandLines;
  commandLines.push_back(commandLine);

  // Accumulate the list of dependencies.
  std::vector<std::string> depends2;
  for(int i = 0; i < numDepends; ++i)
    {
    expand = depends[i];
    depends2.push_back(mf->ExpandVariablesInString(expand));
    }

  // Accumulate the list of outputs.
  std::vector<std::string> outputs2;
  for(int i = 0; i < numOutputs; ++i)
    {
    expand = outputs[i];
    outputs2.push_back(mf->ExpandVariablesInString(expand));
    }

  // Pass the call to the makefile instance.
  const char* no_comment = 0;
  mf->AddCustomCommandOldStyle(target, outputs2, depends2, source,
                               commandLines, no_comment);
}

void CCONV cmAddCustomCommandToOutput(void *arg, const char* output,
                                const char* command,
                                int numArgs, const char **args,
                                const char* main_dependency,
                                int numDepends, const char **depends)
{
  // Get the makefile instance.  Perform an extra variable expansion
  // now because the API caller expects it.
  cmMakefile* mf = static_cast<cmMakefile*>(arg);

  // Construct the command line for the command.
  cmCustomCommandLine commandLine;
  std::string expand = command;
  commandLine.push_back(mf->ExpandVariablesInString(expand));
  for(int i=0; i < numArgs; ++i)
    {
    expand = args[i];
    commandLine.push_back(mf->ExpandVariablesInString(expand));
    }
  cmCustomCommandLines commandLines;
  commandLines.push_back(commandLine);

  // Accumulate the list of dependencies.
  std::vector<std::string> depends2;
  for(int i = 0; i < numDepends; ++i)
    {
    expand = depends[i];
    depends2.push_back(mf->ExpandVariablesInString(expand));
    }

  // Pass the call to the makefile instance.
  const char* no_comment = 0;
  const char* no_working_dir = 0;
  mf->AddCustomCommandToOutput(output, depends2, main_dependency,
                               commandLines, no_comment, no_working_dir);
}

void CCONV cmAddCustomCommandToTarget(void *arg, const char* target,
                                const char* command,
                                int numArgs, const char **args,
                                int commandType)
{
  // Get the makefile instance.
  cmMakefile* mf = static_cast<cmMakefile*>(arg);

  // Construct the command line for the command.  Perform an extra
  // variable expansion now because the API caller expects it.
  cmCustomCommandLine commandLine;
  std::string expand = command;
  commandLine.push_back(mf->ExpandVariablesInString(expand));
  for(int i=0; i < numArgs; ++i)
    {
    expand = args[i];
    commandLine.push_back(mf->ExpandVariablesInString(expand));
    }
  cmCustomCommandLines commandLines;
  commandLines.push_back(commandLine);

  // Select the command type.
  cmTarget::CustomCommandType cctype = cmTarget::POST_BUILD;
  switch (commandType)
    {
    case CM_PRE_BUILD:
      cctype = cmTarget::PRE_BUILD;
      break;
    case CM_PRE_LINK:
      cctype = cmTarget::PRE_LINK;
      break;
    case CM_POST_BUILD:
      cctype = cmTarget::POST_BUILD;
      break;
    }

  // Pass the call to the makefile instance.
  std::vector<std::string> no_depends;
  const char* no_comment = 0;
  const char* no_working_dir = 0;
  mf->AddCustomCommandToTarget(target, no_depends, commandLines,
                               cctype, no_comment, no_working_dir);
}

void CCONV cmAddLinkLibraryForTarget(void *arg, const char *tgt,
  const char*value, int libtype)
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

void CCONV cmAddLibrary(void *arg, const char *libname, int shared,
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

char CCONV *cmExpandVariablesInString(void *arg, const char *source,
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


int CCONV cmExecuteCommand(void *arg, const char *name,
                     int numArgs, const char **args)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  cmListFileFunction lff;
  lff.Name = name;
  for(int i = 0; i < numArgs; ++i)
    {
    // Assume all arguments are quoted.
    lff.Arguments.push_back(cmListFileArgument(args[i], true,
                                               "[CMake-Plugin]", 0));
    }
  return mf->ExecuteCommand(lff);
}

void CCONV cmExpandSourceListArguments(void *arg,
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

void CCONV cmFreeArguments(int argc, char **argv)
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

int CCONV cmGetTotalArgumentSize(int argc, char **argv)
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

void CCONV *cmGetSource(void *arg, const char *name)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  return (void *)mf->GetSource(name);
}

void * CCONV cmAddSource(void *arg, void *arg2)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg2);
  return (void *)mf->AddSource(*sf);
}


void * CCONV cmCreateSourceFile()
{
  return (void *)(new cmSourceFile);
}

void * CCONV cmCreateNewSourceFile(void *arg)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  cmSourceFile *sf = new cmSourceFile;
  sf->SetMakefile(mf);
  return (void *)sf;
}

void CCONV cmDestroySourceFile(void *arg)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  delete sf;
}

const char * CCONV cmSourceFileGetSourceName(void *arg)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  return sf->GetSourceName().c_str();
}

const char * CCONV  cmSourceFileGetFullPath(void *arg)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  return sf->GetFullPath().c_str();
}

const char * CCONV  cmSourceFileGetProperty(void *arg,const char *prop)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  return sf->GetProperty(prop);
}

int CCONV cmSourceFileGetPropertyAsBool(void *arg,const char *prop)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  return (sf->GetPropertyAsBool(prop) ? 1: 0);
}

void CCONV cmSourceFileSetProperty(void *arg,const char *prop,
  const char *val)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  sf->SetProperty(prop,val);
}

void CCONV cmSourceFileAddDepend(void *arg, const char *depend)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  sf->GetDepends().push_back(depend);
}

void CCONV cmSourceFileSetName(void *arg, const char* name, const char* dir,
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

void CCONV cmSourceFileSetName2(void *arg, const char* name, const char* dir,
                          const char *ext, int headerFileOnly)
{
  cmSourceFile *sf = static_cast<cmSourceFile *>(arg);
  sf->SetName(name,dir,ext,(headerFileOnly ? true : false));
}


char * CCONV cmGetFilenameWithoutExtension(const char *name)
{
  std::string sres = cmSystemTools::GetFilenameWithoutExtension(name);
  char *result = (char *)malloc(sres.size()+1);
  strcpy(result,sres.c_str());
  return result;
}

char * CCONV cmGetFilenamePath(const char *name)
{
  std::string sres = cmSystemTools::GetFilenamePath(name);
  char *result = (char *)malloc(sres.size()+1);
  strcpy(result,sres.c_str());
  return result;
}

char * CCONV cmCapitalized(const char *name)
{
  std::string sres = cmSystemTools::Capitalized(name);
  char *result = (char *)malloc(sres.size()+1);
  strcpy(result,sres.c_str());
  return result;
}

void CCONV cmCopyFileIfDifferent(const char *name1, const char *name2)
{
  cmSystemTools::CopyFileIfDifferent(name1,name2);
}

void CCONV cmRemoveFile(const char *name)
{
  cmSystemTools::RemoveFile(name);
}

void CCONV cmDisplayStatus(void *arg, const char* message)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  mf->DisplayStatus(message, -1);
}

void CCONV cmFree(void *data)
{
  free(data);
}

void CCONV DefineSourceFileProperty (void *arg, const char *name,
  const char *briefDocs, 
  const char *longDocs,
  int chained)
{
  cmMakefile *mf = static_cast<cmMakefile *>(arg);
  mf->GetCMakeInstance()->DefineProperty(name,cmProperty::SOURCE_FILE,
                                         briefDocs, longDocs, 
                                         chained != 0);
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
  cmCreateNewSourceFile,
  DefineSourceFileProperty,
};

