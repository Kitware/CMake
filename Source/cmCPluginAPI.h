/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

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

     
   
/*=========================================================================*/
/* CM_PLUGIN_EXPORT should be used by plugins */
/*=========================================================================*/
#ifdef _WIN32
#define CM_PLUGIN_EXPORT  __declspec( dllexport )
#else
#define CM_PLUGIN_EXPORT 
#endif

/*=========================================================================*/
/* CM_EXPORT is used by CMake and not by plugins */
/*=========================================================================*/
#ifndef CM_EXPORT
#ifdef _WIN32
#ifdef CMakeLib_EXPORTS
  #define CM_EXPORT  __declspec( dllexport )
#else
  #define CM_EXPORT  __declspec( dllimport )
#endif
#else
#define CM_EXPORT 
#endif
#endif


/*=========================================================================*/
/* define the different types of cache entries, see cmCacheManager.h for more
   information */
/*=========================================================================*/#define CM_CACHE_BOOL 0
#define CM_CACHE_PATH 1
#define CM_CACHE_FILEPATH 2
#define CM_CACHE_STRING 3
#define CM_CACHE_INTERNAL 4
#define CM_CACHE_STATIC 5

/*=========================================================================*/
/* define the different types of compiles a library may be */
/*=========================================================================*/
#define CM_LIBRARY_GENERAL 0
#define CM_LIBRARY_DEBUG 1
#define CM_LIBRARY_OPTIMIZED 2

#ifdef  __cplusplus
extern "C" {
#endif
  
/*=========================================================================*/
/* first we define the key data structures and function prototypes */
/*=========================================================================*/
  typedef const char* (*CM_DOC_FUNCTION)();
  typedef const char* (*CM_NAME_FUNCTION)();
  typedef int (*CM_INITIAL_PASS_FUNCTION)(void *info, void *mf, 
                                          int argc, char *[]);
  typedef void (*CM_FINAL_PASS_FUNCTION)(void *info, void *mf);
  
  typedef struct {
    int m_Inherited;
    CM_INITIAL_PASS_FUNCTION InitialPass;
    CM_FINAL_PASS_FUNCTION FinalPass;
    CM_DOC_FUNCTION GetTerseDocumentation;
    CM_DOC_FUNCTION GetFullDocumentation;  
    void *ClientData;
  } cmLoadedCommandInfo;

  typedef void (*CM_INIT_FUNCTION)(cmLoadedCommandInfo *);
  
  /*=========================================================================*/
  /* Finally we define the set of functions that a plugin may call. The first
     goup of functions are utility functions that are specific to the plugin
     API */
  /*=========================================================================*/
  /* set/Get the ClientData in the cmLoadedCommandInfo structure, this is how
     information is passed from the InitialPass to FInalPass for commands
     that need a FinalPass and need information from the InitialPass */
  extern CM_EXPORT void *cmGetClientData(void *info);
  /* return the summed size in characters of all the arguments */
  extern CM_EXPORT int cmGetTotalArgumentSize(int argc, char **argv);
  /* free all the memory associated with an argc, argv pair */
  extern CM_EXPORT void cmFreeArguments(int argc, char **argv);
  /* set/Get the ClientData in the cmLoadedCommandInfo structure, this is how
     information is passed from the InitialPass to FInalPass for commands
     that need a FinalPass and need information from the InitialPass */
  extern CM_EXPORT void cmSetClientData(void *info, void *cd);

  /*=========================================================================*/
  /* The following functions all directly map to methods in the cmMakefile
     class. See cmMakefile.h for descriptions of what each method does. All
     of these methods take the void * makefile pointer as their first
     argument. */
  /*=========================================================================*/
  extern CM_EXPORT void cmAddCacheDefinition(void *mf, const char* name, 
                                             const char* value, 
                                             const char* doc,
                                             int cachetype);
  extern CM_EXPORT void cmAddCustomCommand(void *mf, const char* source,
                                           const char* command,
                                           int numArgs,
                                           const char **args,
                                           int numDepends,
                                           const char **depends,
                                           int numOutputs,
                                           const char **outputs,
                                           const char *target);
  extern CM_EXPORT void cmAddDefineFlag(void *mf, const char* definition);
  extern CM_EXPORT void cmAddDefinition(void *mf, const char* name, 
                                        const char* value);
  extern CM_EXPORT void cmAddExecutable(void *mf, const char *exename, 
                                        int numSrcs, const char **srcs, 
                                        int win32);
  extern CM_EXPORT void cmAddLibrary(void *mf, const char *libname, 
                                     int shared, int numSrcs, 
                                     const char **srcs);
  extern CM_EXPORT void cmAddLinkDirectoryForTarget(void *mf,
                                                    const char *tgt, 
                                                    const char* d);
  extern CM_EXPORT void cmAddLinkLibraryForTarget(void *mf,
                                                  const char *tgt, 
                                                  const char *libname, 
                                                  int libtype);
  extern CM_EXPORT void cmAddUtilityCommand(void *mf, const char* utilityName,
                                            const char *command,
                                            const char *arguments,
                                            int all, int numDepends,
                                            const char **depends,
                                            int numOutputs,
                                            const char **outputs);
  extern CM_EXPORT int cmCommandExists(void *mf, const char* name);
  extern CM_EXPORT void cmExecuteCommand(void *mf, const char *name, 
                                         int numArgs, const char **args);
  extern CM_EXPORT 
  void cmExpandSourceListArguments(void *mf,int argc, const char **argv,
                                   int *resArgc, char ***resArgv,
                                   unsigned int startArgumentIndex);
  extern CM_EXPORT char *cmExpandVariablesInString(void *mf,
                                                   const char *source, 
                                                   int escapeQuotes,
                                                   int atOnly);
  extern CM_EXPORT unsigned int cmGetCacheMajorVersion(void *mf);
  extern CM_EXPORT unsigned int cmGetCacheMinorVersion(void *mf);
  extern CM_EXPORT const char*  cmGetCurrentDirectory(void *mf);
  extern CM_EXPORT const char*  cmGetCurrentOutputDirectory(void *mf);
  extern CM_EXPORT const char*  cmGetDefinition(void *mf, const char *def);
  extern CM_EXPORT const char*  cmGetHomeDirectory(void *mf);
  extern CM_EXPORT const char*  cmGetHomeOutputDirectory(void *mf);
  extern CM_EXPORT unsigned int cmGetMajorVersion(void *mf);
  extern CM_EXPORT unsigned int cmGetMinorVersion(void *mf);
  extern CM_EXPORT const char*  cmGetProjectName(void *mf);
  extern CM_EXPORT const char*  cmGetStartDirectory(void *mf);
  extern CM_EXPORT const char*  cmGetStartOutputDirectory(void *mf);
  extern CM_EXPORT int cmIsOn(void *mf, const char* name);
  
  
  /*=========================================================================*/
  /* The following functions are designed to operate or manipulate
  cmSourceFiles. Please see cmSourceFile.h for additional information on many
  of these methods. Some of these methods are in cmMakefile.h.
  /*=========================================================================*/
  extern CM_EXPORT void *cmAddSource(void *mf, void *sf); 
  extern CM_EXPORT void *cmCreateSourceFile();
  extern CM_EXPORT void *cmGetSource(void *mf, const char* sourceName);
  extern CM_EXPORT void  cmSourceFileAddDepend(void *sf, const char *depend);
  extern CM_EXPORT const char *cmSourceFileGetProperty(void *sf, 
                                                       const char *prop);
  extern CM_EXPORT int cmSourceFileGetPropertyAsBool(void *sf, 
                                                     const char *prop);
  extern CM_EXPORT const char *cmSourceFileGetSourceName(void *sf);
  extern CM_EXPORT 
  void cmSourceFileSetName(void *sf, const char* name, const char* dir,
                           int numSourceExtensions,
                           const char **sourceExtensions,
                           int numHeaderExtensions,
                           const char **headerExtensions);
  extern CM_EXPORT 
  void cmSourceFileSetName2(void *sf, const char* name, const char* dir, 
                            const char *ext, int headerFileOnly);
  extern CM_EXPORT void cmSourceFileSetProperty(void *sf, const char *prop, 
                                                const char *value);
  
  
  /*=========================================================================*/
  /* the following methods are from cmSystemTools.h see that file for
     specific documentaiton on each method. */
  /*=========================================================================*/
  extern CM_EXPORT char *cmCapitalized(const char *);
  extern CM_EXPORT void cmCopyFileIfDifferent(const char *f1, const char *f2);
  extern CM_EXPORT char *cmGetFilenameWithoutExtension(const char *);
  extern CM_EXPORT void cmRemoveFile(const char *f1);
  
#ifdef  __cplusplus
}
#endif
