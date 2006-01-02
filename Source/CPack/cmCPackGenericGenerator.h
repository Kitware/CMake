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

class cmMakefile;
class cmLocalGenerator;
class cmGlobalGenerator;
class cmake;

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
  void SetVerbose(bool val) { m_GeneratorVerbose = val; }

  /**
   * Do the actual processing. Subclass has to override it.
   * Return 0 if error.
   */
  virtual int ProcessGenerator();

  /**
   * Initialize generator
   */
  virtual int Initialize(const char* name);

  /**
   * Construct generator
   */
  cmCPackGenericGenerator();
  virtual ~cmCPackGenericGenerator();

  //! Set and get the options
  void SetOption(const char* op, const char* value);
  const char* GetOption(const char* op);

  //! Set all the variables
  int FindRunningCMake(const char* arg0);

protected:
  int PrepareNames();
  int InstallProject();
  virtual int GenerateHeader(std::ostream* os);

  virtual const char* GetOutputExtension() { return "cpack"; }
  virtual const char* GetOutputPostfix() { return 0; }
  virtual int CompressFiles(const char* outFileName, const char* toplevel,
    const std::vector<std::string>& files);
  virtual const char* GetInstallPath();
  virtual const char* GetInstallPrefix() { return "/"; }

  virtual std::string FindTemplate(const char* name);
  virtual bool ConfigureFile(const char* inName, const char* outName);

  bool m_GeneratorVerbose;
  std::string m_Name;

  std::string m_InstallPath;

  std::string m_CPackSelf;
  std::string m_CMakeSelf;
  std::string m_CMakeRoot;

private:
  cmGlobalGenerator* m_GlobalGenerator;
  cmLocalGenerator* m_LocalGenerator;
  cmMakefile* m_MakefileMap;
  cmake* m_CMakeInstance;
};

#endif


