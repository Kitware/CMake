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
#ifndef cmMakefileGenerator_h
#define cmMakefileGenerator_h

#include "cmStandardIncludes.h"

class cmMakefile;
class cmClassFile;

/** \class cmMakefileGenerator
 * \brief Provide an abstract interface for classes generating makefiles.
 *
 * Subclasses of this abstract class generate makefiles for various
 * platforms.
 */
class cmMakefileGenerator
{
public:
  ///! Create a named generator
  static cmMakefileGenerator* CreateGenerator(const char* name);
  ///! Register a generator
  static void RegisterGenerator(cmMakefileGenerator*);
  ///! delete all registered generators, useful for clean up
  static void UnRegisterGenerators();
  ///! Get the names of the current registered generators
  static void GetRegisteredGenerators(std::vector<std::string>& names);
  
  ///! Get the name for the generator.
  virtual const char* GetName() = 0;

  ///! virtual copy constructor
  virtual cmMakefileGenerator* CreateObject() = 0;
  
  ///! Set the cmMakefile instance from which to generate the makefile.
  void SetMakefile(cmMakefile*);

  /**
   * Generate the makefile using the m_Makefile, m_CustomCommands, 
   * and m_ExtraSourceFiles. All subclasses of cmMakefileGenerator
   * must implement this method.
   */
  virtual void GenerateMakefile() = 0;

  /**
   * The local setting indicates that the generator is producing a
   * fully configured makefile in the current directory. In Microsoft
   * terms it is producing a DSP file if local is true and a DSW file
   * if local is false. On UNIX when local is false it skips the
   * dependecy check and recurses the full tree building the structure
   */
  virtual void SetLocal(bool ) {};

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(const char*) = 0;

  virtual ~cmMakefileGenerator(){};
  
  /**
   * Set/Get and Clear the enabled languages.  
   */
  static void SetLanguageEnabled(const char*);
  static bool GetLanguageEnabled(const char*);
  static void ClearEnabledLanguages();
protected:
  cmMakefile* m_Makefile;
private:
  static std::map<cmStdString, cmMakefileGenerator*> s_RegisteredGenerators;
  static std::map<cmStdString, bool> s_LanguageEnabled;
};

#endif
