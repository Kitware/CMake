/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalBorlandMakefileGenerator_h
#define cmGlobalBorlandMakefileGenerator_h

#include "cmGlobalNMakeMakefileGenerator.h"

/** \class cmGlobalBorlandMakefileGenerator
 * \brief Write a Borland makefiles.
 *
 * cmGlobalBorlandMakefileGenerator manages nmake build process for a tree
 */
class cmGlobalBorlandMakefileGenerator : public cmGlobalNMakeMakefileGenerator
{
public:
  cmGlobalBorlandMakefileGenerator();
  static cmGlobalGeneratorFactory* NewFactory() {
    return new cmGlobalGeneratorSimpleFactory
      <cmGlobalBorlandMakefileGenerator>(); }

  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalBorlandMakefileGenerator::GetActualName();}
  static const char* GetActualName() {return "Borland Makefiles";}

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

  virtual bool AllowNotParallel() const { return false; }
};

#endif
