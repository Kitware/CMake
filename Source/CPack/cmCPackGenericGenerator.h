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

#ifndef cmCPackGenericGenerator_h
#define cmCPackGenericGenerator_h


#include "cmObject.h"

#define cmCPackTypeMacro(class, superclass) \
  cmTypeMacro(class, superclass); \
  static cmCPackGenericGenerator* CreateGenerator() { return new class; }

#define cmCPackLogger(logType, msg) \
  do { \
    cmOStringStream cmCPackLog_msg; \
    cmCPackLog_msg << msg; \
    this->Logger->Log(logType, __FILE__, __LINE__,\
                      cmCPackLog_msg.str().c_str());\
  } while ( 0 )

#ifdef cerr
#  undef cerr
#endif
#define cerr no_cerr_use_cmCPack_Log

#ifdef cout
#  undef cout
#endif
#define cout no_cout_use_cmCPack_Log

class cmMakefile;
class cmCPackLog;

/** \class cmCPackGenericGenerator
 * \brief A superclass of all CPack Generators
 *
 */
class cmCPackGenericGenerator : public cmObject
{
public:
  cmTypeMacro(cmCPackGenericGenerator, cmObject);
  /**
   * If verbose then more informaiton is printed out
   */
  void SetVerbose(bool val) { this->GeneratorVerbose = val; }

  /**
   * Do the actual processing. Subclass has to override it.
   * Return 0 if error.
   */
  virtual int ProcessGenerator();

  /**
   * Initialize generator
   */
  int Initialize(const char* name, cmMakefile* mf, const char* argv0);

  /**
   * Construct generator
   */
  cmCPackGenericGenerator();
  virtual ~cmCPackGenericGenerator();

  //! Set and get the options
  void SetOption(const char* op, const char* value);
  void SetOptionIfNotSet(const char* op, const char* value);
  const char* GetOption(const char* op);

  //! Set all the variables
  int FindRunningCMake(const char* arg0);

  //! Set the logger
  void SetLogger(cmCPackLog* log) { this->Logger = log; }

  //! Display verbose information via logger
  void DisplayVerboseOutput(const char* msg, float progress);

protected:
  int PrepareNames();
  int InstallProject();

  virtual const char* GetOutputExtension() { return "cpack"; }
  virtual const char* GetOutputPostfix() { return 0; }
  virtual int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);
  virtual const char* GetInstallPath();
  virtual const char* GetInstallPrefix() { return "/"; }

  virtual std::string FindTemplate(const char* name);
  virtual bool ConfigureFile(const char* inName, const char* outName);
  virtual bool ConfigureString(const std::string& input, std::string& output);
  virtual int InitializeInternal();

  bool GeneratorVerbose;
  std::string Name;

  std::string InstallPath;

  std::string CPackSelf;
  std::string CMakeSelf;
  std::string CMakeRoot;

  cmCPackLog* Logger;

private:
  cmMakefile* MakefileMap;
};

#endif
