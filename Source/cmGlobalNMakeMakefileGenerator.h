/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalNMakeMakefileGenerator_h
#define cmGlobalNMakeMakefileGenerator_h

#include "cmGlobalUnixMakefileGenerator3.h"

/** \class cmGlobalNMakeMakefileGenerator
 * \brief Write a NMake makefiles.
 *
 * cmGlobalNMakeMakefileGenerator manages nmake build process for a tree
 */
class cmGlobalNMakeMakefileGenerator : public cmGlobalUnixMakefileGenerator3
{
public:
  cmGlobalNMakeMakefileGenerator();
  static cmGlobalGeneratorFactory* NewFactory() {
    return new cmGlobalGeneratorSimpleFactory
      <cmGlobalNMakeMakefileGenerator>(); }
  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalNMakeMakefileGenerator::GetActualName();}
  static const char* GetActualName() {return "NMake Makefiles";}

  /** Get the documentation entry for this generator.  */
  static void GetDocumentation(cmDocumentationEntry& entry);

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
