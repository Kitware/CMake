/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef cmCTestScriptHandler_h
#define cmCTestScriptHandler_h


#include "cmCTestGenericHandler.h"
#include "cmListFileCache.h"

class cmMakefile;
class cmLocalGenerator;
class cmGlobalGenerator;
class cmake;

/** \class cmCTestScriptHandler
 * \brief A class that handles ctest -S invocations
 *
 * CTest script is controlled using several variables that script has to
 * specify and some optional ones. Required ones are:
 *   CTEST_SOURCE_DIRECTORY - Source directory of the project
 *   CTEST_BINARY_DIRECTORY - Binary directory of the project
 *   CTEST_COMMAND          - Testing commands
 *
 * Optional variables are:
 *   CTEST_BACKUP_AND_RESTORE
 *   CTEST_CMAKE_COMMAND
 *   CTEST_CMAKE_OUTPUT_FILE_NAME
 *   CTEST_CONTINUOUS_DURATION
 *   CTEST_CONTINUOUS_MINIMUM_INTERVAL
 *   CTEST_CVS_CHECKOUT
 *   CTEST_CVS_COMMAND
 *   CTEST_DASHBOARD_ROOT
 *   CTEST_ENVIRONMENT
 *   CTEST_INITIAL_CACHE
 *   CTEST_START_WITH_EMPTY_BINARY_DIRECTORY
 *   CTEST_START_WITH_EMPTY_BINARY_DIRECTORY_ONCE
 *   
 * In addition the follwing variables can be used. The number can be 1-10.
 *   CTEST_EXTRA_UPDATES_1
 *   CTEST_EXTRA_UPDATES_2
 *   ...
 *   CTEST_EXTRA_UPDATES_10
 *
 * CTest script can use the following arguments CTest provides:
 *   CTEST_SCRIPT_ARG
 *   CTEST_SCRIPT_DIRECTORY
 *   CTEST_SCRIPT_NAME
 *
 */
class cmCTestScriptHandler : public cmCTestGenericHandler
{
public:

  /**
   * Add a script to run
   */
  void AddConfigurationScript(const char *);
  
  /**
   * Run a dashboard using a specified confiuration script
   */
  int RunConfigurationScript();

  /*
   * Run a script
   */
  static bool RunScript(cmCTest* ctest, const char *script);
  int RunCurrentScript();
    
  /*
   * Empty Binary Directory
   */
  static bool EmptyBinaryDirectory(const char *dir);

  /*
   * Some elapsed time handling functions
   */
  static void SleepInSeconds(unsigned int secondsToWait);
  void UpdateElapsedTime();

  cmCTestScriptHandler();
  ~cmCTestScriptHandler();
  
private:
  // reads in a script
  int ReadInScript(const std::string& total_script_arg);

  // extract vars from the script to set ivars
  int ExtractVariables();

  // perform a CVS checkout of the source dir
  int CheckOutSourceDir();

  // perform any extra cvs updates that were requested
  int PerformExtraUpdates();
  
  // backup and restore dirs
  int BackupDirectories();
  void RestoreBackupDirectories();

  int RunConfigurationScript(const std::string& script);
  int RunConfigurationDashboard();

  std::vector<cmStdString> m_ConfigurationScripts;

  bool m_Backup;
  bool m_EmptyBinDir;
  bool m_EmptyBinDirOnce;
  bool m_ScriptHasRun;
  
  cmStdString m_SourceDir;
  cmStdString m_BinaryDir;
  cmStdString m_BackupSourceDir;
  cmStdString m_BackupBinaryDir;
  cmStdString m_CTestRoot;
  cmStdString m_CVSCheckOut;
  cmStdString m_CTestCmd;
  cmStdString m_CVSCmd;
  cmStdString m_CTestEnv;
  cmStdString m_InitCache;
  cmStdString m_CMakeCmd;
  cmStdString m_CMOutFile;
  std::vector<cmStdString> m_ExtraUpdates;

  double m_MinimumInterval;
  double m_ContinuousDuration;

  // what time in seconds did this script start running
  double m_ScriptStartTime;
  
  cmMakefile *m_Makefile;
  cmLocalGenerator *m_LocalGenerator;
  cmGlobalGenerator *m_GlobalGenerator;
  cmake *m_CMake;
};

#endif
