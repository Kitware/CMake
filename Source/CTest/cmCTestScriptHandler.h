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


#include "cmStandardIncludes.h"
#include "cmListFileCache.h"

class cmMakefile;
class cmLocalGenerator;

/** \class cmCTestScriptHandler
 * \brief A class that handles ctest -S invocations
 *
 */
class cmCTestScriptHandler
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
   * If verbose then more informaiton is printed out
   */
  void SetVerbose(bool val) { m_Verbose = val; }
  
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
  
  void LocalSleep(unsigned int secondsToWait);

  // backup and restore dirs
  int BackupDirectories();
  void RestoreBackupDirectories();

  int RunConfigurationScript(const std::string& script);
  int RunConfigurationDashboard();

  std::vector<cmStdString> m_ConfigurationScripts;

  bool m_Verbose;
  bool m_Backup;

  cmStdString m_SourceDir;
  cmStdString m_BinaryDir;
  cmStdString m_BackupSourceDir;
  cmStdString m_BackupBinaryDir;
  cmStdString m_CTestRoot;
  cmStdString m_CVSCheckOut;
  cmStdString m_CTestCmd;
  cmStdString m_CVSCmd;

  double m_MinimumInterval;
  
  cmMakefile *m_Makefile;
  cmLocalGenerator *m_LocalGenerator;
};

#endif
