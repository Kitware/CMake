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
/* This header file defines the API that loadable commands can use. In many
     of these commands C++ instances of cmMakefile of cmSourceFile are passed
     in as arguments or returned. In these cases they are passed as a void *
     argument. In the function prototypes mf is used to represent a makefile
     and sf is used to represent a source file. The functions are grouped
     loosely into four groups 1) Utility 2) cmMakefile 3) cmSourceFile 4)
     cmSystemTools. Within each grouping functions are listed alphabetically */
/*=========================================================================*/
#ifndef cmCPluginAPI_h
#define cmCPluginAPI_h

#define CMAKE_VERSION_MAJOR 1
#define CMAKE_VERSION_MINOR 7

#ifdef  __cplusplus
extern "C" {
#endif
  
/*=========================================================================
this is the structure of function entry points that a plugin may call. This
structure must be kept in sync with the static decaled at the bottom of
cmCPLuginAPI.cxx
=========================================================================*/
typedef struct 
{
  /*=========================================================================
  Here we define the set of functions that a plugin may call. The first goup
  of functions are utility functions that are specific to the plugin API
  =========================================================================*/
  /* set/Get the ClientData in the cmLoadedCommandInfo structure, this is how
     information is passed from the InitialPass to FInalPass for commands
     that need a FinalPass and need information from the InitialPass */
  void *(*GetClientData) (void *info);
  /* return the summed size in characters of all the arguments */
  int   (*GetTotalArgumentSize) (int argc, char **argv);
  /* free all the memory associated with an argc, argv pair */
  void  (*FreeArguments) (int argc, char **argv);
  /* set/Get the ClientData in the cmLoadedCommandInfo structure, this is how
     information is passed from the InitialPass to FInalPass for commands
     that need a FinalPass and need information from the InitialPass */
  void  (*SetClientData) (void *info, void *cd);
  /* when an error occurs, call this function to set the error string */
  void  (*SetError) (void *info, const char *err);
  
  /*=========================================================================
  The following functions all directly map to methods in the cmMakefile
  class. See cmMakefile.h for descriptions of what each method does. All of
  these methods take the void * makefile pointer as their first argument.
  =========================================================================*/
  void  (*AddCacheDefinition) (void *mf, const char* name, 
                               const char* value, 
                               const char* doc, int cachetype);
  void  (*AddCustomCommand) (void *mf, const char* source,
                             const char* command,
                             int numArgs, const char **args,
                             int numDepends, const char **depends,
                             int numOutputs, const char **outputs,
                             const char *target);
  void  (*AddDefineFlag) (void *mf, const char* definition);
  void  (*AddDefinition) (void *mf, const char* name, const char* value);
  void  (*AddExecutable) (void *mf, const char *exename, 
                         int numSrcs, const char **srcs, int win32);
  void  (*AddLibrary) (void *mf, const char *libname, 
                       int shared, int numSrcs, const char **srcs);
  void  (*AddLinkDirectoryForTarget) (void *mf, const char *tgt, 
                                      const char* d);
  void  (*AddLinkLibraryForTarget) (void *mf, const char *tgt, 
                                    const char *libname, int libtype);
  void  (*AddUtilityCommand) (void *mf, const char* utilityName,
                              const char *command, const char *arguments,
                              int all, int numDepends, const char **depends,
                              int numOutputs, const char **outputs);
  int   (*CommandExists) (void *mf, const char* name);
  int  (*ExecuteCommand) (void *mf, const char *name, 
                          int numArgs, const char **args);
  void  (*ExpandSourceListArguments) (void *mf,int argc, const char **argv,
                                      int *resArgc, char ***resArgv,
                                      unsigned int startArgumentIndex);
  char *(*ExpandVariablesInString) (void *mf, const char *source, 
                                    int escapeQuotes, int atOnly);
  unsigned int (*GetCacheMajorVersion) (void *mf);
  unsigned int (*GetCacheMinorVersion) (void *mf);
  const char*  (*GetCurrentDirectory) (void *mf);
  const char*  (*GetCurrentOutputDirectory) (void *mf);
  const char*  (*GetDefinition) (void *mf, const char *def);
  const char*  (*GetHomeDirectory) (void *mf);
  const char*  (*GetHomeOutputDirectory) (void *mf);
  unsigned int (*GetMajorVersion) (void *mf);
  unsigned int (*GetMinorVersion) (void *mf);
  const char*  (*GetProjectName) (void *mf);
  const char*  (*GetStartDirectory) (void *mf);
  const char*  (*GetStartOutputDirectory) (void *mf);
  int          (*IsOn) (void *mf, const char* name);
  
  
  /*=========================================================================
  The following functions are designed to operate or manipulate
  cmSourceFiles. Please see cmSourceFile.h for additional information on many
  of these methods. Some of these methods are in cmMakefile.h.
  =========================================================================*/
  void *(*AddSource) (void *mf, void *sf); 
  void *(*CreateSourceFile) ();
  void  (*DestroySourceFile) (void *sf);
  void *(*GetSource) (void *mf, const char* sourceName);
  void  (*SourceFileAddDepend) (void *sf, const char *depend);
  const char *(*SourceFileGetProperty) (void *sf, const char *prop);
  int   (*SourceFileGetPropertyAsBool) (void *sf, const char *prop);
  const char *(*SourceFileGetSourceName) (void *sf);
  const char *(*SourceFileGetFullPath) (void *sf);
  void  (*SourceFileSetName) (void *sf, const char* name, const char* dir,
                             int numSourceExtensions,
                             const char **sourceExtensions,
                             int numHeaderExtensions,
                             const char **headerExtensions);
  void  (*SourceFileSetName2) (void *sf, const char* name, const char* dir, 
                               const char *ext, int headerFileOnly);
  void  (*SourceFileSetProperty) (void *sf, const char *prop,
                                  const char *value);
  
  
  /*=========================================================================
  The following methods are from cmSystemTools.h see that file for specific
  documentation on each method.
  =========================================================================*/
  char  *(*Capitalized)(const char *);
  void   (*CopyFileIfDifferent)(const char *f1, const char *f2);
  char  *(*GetFilenameWithoutExtension)(const char *);
  char  *(*GetFilenamePath)(const char *);
  void   (*RemoveFile)(const char *f1);
  void   (*Free)(void *);
  
  /*=========================================================================
    The following are new functions added after 1.6
  =========================================================================*/
  void  (*AddCustomCommandToOutput) (void *mf, const char* output,
                                     const char* command,
                                     int numArgs, const char **args,
                                     const char* main_dependency,
                                     int numDepends, const char **depends);
  void  (*AddCustomCommandToTarget) (void *mf, const char* target,
                                     const char* command,
                                     int numArgs, const char **args,
                                     int commandType);
  
  /* this is the end of the C function stub API structure */ 
} cmCAPI;

   
/*=========================================================================
CM_PLUGIN_EXPORT should be used by plugins
=========================================================================*/
#ifdef _WIN32
#define CM_PLUGIN_EXPORT  __declspec( dllexport )
#else
#define CM_PLUGIN_EXPORT 
#endif

/*=========================================================================
define the different types of cache entries, see cmCacheManager.h for more
information
=========================================================================*/
#define CM_CACHE_BOOL 0
#define CM_CACHE_PATH 1
#define CM_CACHE_FILEPATH 2
#define CM_CACHE_STRING 3
#define CM_CACHE_INTERNAL 4
#define CM_CACHE_STATIC 5

/*=========================================================================
define the different types of compiles a library may be
=========================================================================*/
#define CM_LIBRARY_GENERAL 0
#define CM_LIBRARY_DEBUG 1
#define CM_LIBRARY_OPTIMIZED 2

/*=========================================================================
define the different types of custom commands for a target
=========================================================================*/
#define CM_PRE_BUILD  0
#define CM_PRE_LINK   1
#define CM_POST_BUILD 2
  
/*=========================================================================
Finally we define the key data structures and function prototypes
=========================================================================*/
  typedef const char* (*CM_DOC_FUNCTION)();
  typedef int (*CM_INITIAL_PASS_FUNCTION)(void *info, void *mf, 
                                          int argc, char *[]);
  typedef void (*CM_FINAL_PASS_FUNCTION)(void *info, void *mf);
  typedef void (*CM_DESTRUCTOR_FUNCTION)(void *info);
  
  typedef struct {
    unsigned long reserved1; /* Reserved for future use.  DO NOT USE.  */
    unsigned long reserved2; /* Reserved for future use.  DO NOT USE.  */
    cmCAPI *CAPI;
    int m_Inherited;
    CM_INITIAL_PASS_FUNCTION InitialPass;
    CM_FINAL_PASS_FUNCTION FinalPass;
    CM_DESTRUCTOR_FUNCTION Destructor;
    CM_DOC_FUNCTION GetTerseDocumentation;
    CM_DOC_FUNCTION GetFullDocumentation;  
    const char *Name;
    char *Error;
    void *ClientData;
  } cmLoadedCommandInfo;

  typedef void (*CM_INIT_FUNCTION)(cmLoadedCommandInfo *);
  
#ifdef  __cplusplus
}
#endif

#endif
