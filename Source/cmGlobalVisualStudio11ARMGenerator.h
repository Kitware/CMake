/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2011 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio11ARMGenerator_h
#define cmGlobalVisualStudio11ARMGenerator_h

#include "cmGlobalVisualStudio11Generator.h"

class cmGlobalVisualStudio11ARMGenerator :
  public cmGlobalVisualStudio11Generator
{
public:
  cmGlobalVisualStudio11ARMGenerator() {}
  static cmGlobalGenerator* New() {
    return new cmGlobalVisualStudio11ARMGenerator; }

  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalVisualStudio11ARMGenerator::GetActualName();}
  static const char* GetActualName() {return "Visual Studio 11 ARM";}

  virtual const char* GetPlatformName() const {return "ARM";}

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;

  virtual void AddPlatformDefinitions(cmMakefile* mf);
};
#endif
