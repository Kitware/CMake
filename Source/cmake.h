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
// This class represents a cmake invocation. It is the top level class when
// running cmake. Most cmake based GUIS should primarily create an instance
// of this class and communicate with it.
//
// The basic process for a GUI is as follows:
//
// 1) Create a cmake instance
// 2) Set the Home & Start directories, generator, and cmake command. this
//    can be done using the Set methods or by using SetArgs and passing in
//    command line arguments.
// 3) Load the cache by calling LoadCache (duh)
// 4) if you are using command line arguments with -D or -C flags then
//    call SetCacheArgs (or if for some other reason you want to modify the 
//    cache, do it now.
// 5) Finally call Configure
// 6) Let the user change values and go back to step 5
// 7) call Generate
//
// If your GUI allows the user to change the start & home directories then
// you must at a minimum redo steps 2 through 7. 
//


#include "cmStandardIncludes.h"
#include "cmSystemTools.h"

class cmGlobalGenerator;
class cmLocalGenerator;
class cmCacheManager;
class cmMakefile;
class cmCommand;

class cmake
{
 public:
  ///! construct an instance of cmake
  cmake();
  ///! destruct an instance of cmake
  ~cmake();

  /**
   * Return major and minor version numbers for cmake.
   */
  static unsigned int GetMajorVersion(); 
  static unsigned int GetMinorVersion(); 
  static const char *GetReleaseVersion();

  //@{
  /**
   * Set/Get the home directory (or output directory) in the project. The
   * home directory is the top directory of the project. It is where
   * cmake was run. Remember that CMake processes
   * CMakeLists files by recursing up the tree starting at the StartDirectory
   * and going up until it reaches the HomeDirectory.  
   */
  void SetHomeDirectory(const char* dir);
  const char* GetHomeDirectory() const
    {
    return m_cmHomeDirectory.c_str();
    }
  void SetHomeOutputDirectory(const char* lib);
  const char* GetHomeOutputDirectory() const
    {
    return m_HomeOutputDirectory.c_str();
    }
  //@}

  //@{
  /**
   * Set/Get the start directory (or output directory). The start directory
   * is the directory of the CMakeLists.txt file that started the current
   * round of processing. Remember that CMake processes CMakeLists files by
   * recursing up the tree starting at the StartDirectory and going up until
   * it reaches the HomeDirectory.  
   */
  void SetStartDirectory(const char* dir) 
    {
      m_cmStartDirectory = dir;
      cmSystemTools::ConvertToUnixSlashes(m_cmStartDirectory);
    }
  const char* GetStartDirectory() const
    {
      return m_cmStartDirectory.c_str();
    }
  void SetStartOutputDirectory(const char* lib)
    {
      m_StartOutputDirectory = lib;
      cmSystemTools::ConvertToUnixSlashes(m_StartOutputDirectory);
    }
  const char* GetStartOutputDirectory() const
    {
      return m_StartOutputDirectory.c_str();
    }
  //@}

  /**
   * Dump documentation to a file. If 0 is returned, the
   * operation failed.
   */
  int DumpDocumentationToFile(std::ostream&);

  /**
   * Handle a command line invocation of cmake.
   */
  int Run(const std::vector<std::string>&args);

  /**
   * Generate the SourceFilesList from the SourceLists. This should only be
   * done once to be safe.  The argument is a list of command line
   * arguments.  The first argument should be the name or full path
   * to the command line version of cmake.  For building a GUI,
   * you would pass in the following arguments:
   * /path/to/cmake -H/path/to/source -B/path/to/build 
   * If you only want to parse the CMakeLists.txt files,
   * but not actually generate the makefiles, use buildMakefiles = false.
   */
  int Generate();

  /**
   * Configure the cmMakefiles. This routine will create a GlobalGenerator if
   * one has not already been set. It will then Call Configure on the
   * GlobalGenerator. This in turn will read in an process all the CMakeList
   * files for the tree. It will not produce any actual Makefiles, or
   * workspaces. Generate does that.  */
  int Configure();

  /**
   * Configure the cmMakefiles. This routine will create a GlobalGenerator if
   * one has not already been set. It will then Call Configure on the
   * GlobalGenerator. This in turn will read in an process all the CMakeList
   * files for the tree. It will not produce any actual Makefiles, or
   * workspaces. Generate does that.  */
  int LoadCache(bool fix_paths = false);

  ///! Create a GlobalGenerator
  cmGlobalGenerator* CreateGlobalGenerator(const char* name);

  ///! Return the global generator assigned to this instance of cmake
  cmGlobalGenerator* GetGlobalGenerator() { return m_GlobalGenerator; };

  ///! Return the global generator assigned to this instance of cmake
  void SetGlobalGenerator(cmGlobalGenerator *);

  ///! Get the names of the current registered generators
  void GetRegisteredGenerators(std::vector<std::string>& names);

  ///! get the cmCachemManager used by this invocation of cmake
  cmCacheManager *GetCacheManager() { return m_CacheManager; }
  
  ///! set the cmake command this instance of cmake should use
  void SetCMakeCommand(const char* cmd) { m_CMakeCommand = cmd; }
  
  /**
   * Given a variable name, return its value (as a string).
   */
  const char* GetCacheDefinition(const char*) const;

  /** 
   * Execute commands during the build process. Supports options such
   * as echo, remove file etc.
   */
  static int CMakeCommand(std::vector<std::string>&);

  /**
   * Add a command to this cmake instance
   */
  void AddCommand(cmCommand* );

  /**
   * Get a command by its name
   */
  cmCommand *GetCommand(const char *name);

  /** Check if a command exists. */
  bool CommandExists(const char* name) const;
    
  /**
   * Is cmake in the process of a local cmake invocation. If so, we know the
   * cache is already configured and ready to go. 
   */
  bool GetLocal() { return m_Local; }
  
  ///! Display command line useage
  void Usage(const char *program);

  ///! Parse command line arguments
  void SetArgs(const std::vector<std::string>&);

  ///! Is this cmake running as a result of a TRY_COMPILE command
  bool GetIsInTryCompile() { return m_InTryCompile; }
  
  ///! Is this cmake running as a result of a TRY_COMPILE command
  void SetIsInTryCompile(bool i) { m_InTryCompile = i; }
  
  ///! Parse command line arguments that might set cache values
  void SetCacheArgs(const std::vector<std::string>&);

  typedef  void (*ProgressCallback)(const char*msg, float progress, void *);
  /**
   *  Set the function used by GUI's to receive progress updates
   *  Function gets passed: message as a const char*, a progress 
   *  amount ranging from 0 to 1.0 and client data. The progress
   *  number provided may be negative in cases where a message is 
   *  to be displayed without any progress percentage.
   */
  void SetProgressCallback(ProgressCallback f, void* clientData=0);

  ///! this is called by generators to update the progress
  void UpdateProgress(const char *msg, float prog);

protected:
  typedef std::map<cmStdString, cmCommand*> RegisteredCommandsMap;
  RegisteredCommandsMap m_Commands;
  void AddDefaultCommands();

  cmGlobalGenerator *m_GlobalGenerator;
  cmCacheManager *m_CacheManager;
  std::string m_cmHomeDirectory; 
  std::string m_HomeOutputDirectory;
  std::string m_cmStartDirectory; 
  std::string m_StartOutputDirectory;

  ///! read in a cmake list file to initialize the cache
  void ReadListFile(const char *path);
  
  ///! used by Run
  int LocalGenerate();

  /**
   * Generate CMAKE_ROOT and CMAKE_COMMAND cache entries
   */
  int AddCMakePaths(const char *arg0);

private:
  ProgressCallback m_ProgressCallback;
  void* m_ProgressCallbackClientData;
  bool m_Verbose;
  bool m_Local;
  bool m_InTryCompile;
  std::string m_CMakeCommand;
};

