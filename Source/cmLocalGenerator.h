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
#ifndef cmLocalGenerator_h
#define cmLocalGenerator_h

#include "cmStandardIncludes.h"

class cmMakefile;
class cmGlobalGenerator;
class cmTarget;

/** \class cmLocalGenerator
 * \brief Create required build files for a directory.
 *
 * Subclasses of this abstract class generate makefiles, DSP, etc for various
 * platforms. This class should never be constructued directly. A
 * GlobalGenerator will create it and invoke the appropriate commands on it.
 */
class cmLocalGenerator
{
public:
  cmLocalGenerator();
  virtual ~cmLocalGenerator();
  
  /**
   * Generate the makefile for this directory. fromTheTop indicates if this
   * is being invoked as part of a global Generate or specific to this
   * directory. The difference is that when done from the Top we might skip
   * some steps to save time, such as dependency generation for the
   * makefiles. This is done by a direct invocation from make. 
   */
  virtual void Generate(bool /* fromTheTop */) {};

  /**
   * Process the CMakeLists files for this directory to fill in the
   * m_Makefile ivar 
   */
  virtual void Configure();

  /**
   * Perform any final calculations prior to generation
   */
  virtual void ConfigureFinalPass();

  /**
   * Generate the install rules files in this directory.
   */
  virtual void GenerateInstallRules();

  ///! Get the makefile for this generator
  cmMakefile *GetMakefile() {
    return this->m_Makefile; };
  
  ///! Get the GlobalGenerator this is associated with
  cmGlobalGenerator *GetGlobalGenerator() {
    return m_GlobalGenerator; };

  ///! Set the Global Generator, done on creation by the GlobalGenerator
  void SetGlobalGenerator(cmGlobalGenerator *gg);
  
  /** Get the full name of the target's file, without path.  */
  std::string GetFullTargetName(const char* n, const cmTarget& t);

  virtual const char* GetSafeDefinition(const char*);

  std::string ConvertToRelativeOutputPath(const char* p);
protected:
  virtual void AddInstallRule(ostream& fout, const char* dest, int type, const char* files);
  
  bool m_FromTheTop;
  cmMakefile *m_Makefile;
  cmGlobalGenerator *m_GlobalGenerator;
  // members used for relative path function ConvertToMakefilePath
  std::string m_RelativePathToSourceDir;
  std::string m_RelativePathToBinaryDir;
  std::string m_CurrentOutputDirectory;
  std::string m_HomeOutputDirectory;
  std::string m_HomeDirectory;
  std::string m_HomeOutputDirectoryNoSlash;

};

#endif
