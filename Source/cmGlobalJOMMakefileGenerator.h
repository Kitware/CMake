/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalJOMMakefileGenerator_h
#define cmGlobalJOMMakefileGenerator_h

#include "cmGlobalUnixMakefileGenerator3.h"

/** \class cmGlobalJOMMakefileGenerator
 * \brief Write a JOM makefiles.
 *
 * cmGlobalJOMMakefileGenerator manages nmake build process for a tree
 */
class cmGlobalJOMMakefileGenerator : public cmGlobalUnixMakefileGenerator3
{
public:
  cmGlobalJOMMakefileGenerator();
  static cmGlobalGenerator* New() {
    return new cmGlobalJOMMakefileGenerator; }
  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalJOMMakefileGenerator::GetActualName();}
  // use NMake Makefiles in the name so that scripts/tests that depend on the
  // name NMake Makefiles will work
  static const char* GetActualName() {return "NMake Makefiles JOM";}

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;
  
  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages,
                              cmMakefile *, bool optional);
};

#endif
