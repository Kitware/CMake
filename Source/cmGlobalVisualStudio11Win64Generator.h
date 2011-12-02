/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2011 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio11Win64Generator_h
#define cmGlobalVisualStudio11Win64Generator_h

#include "cmGlobalVisualStudio11Generator.h"

class cmGlobalVisualStudio11Win64Generator :
  public cmGlobalVisualStudio11Generator
{
public:
  cmGlobalVisualStudio11Win64Generator() {}
  static cmGlobalGenerator* New() {
    return new cmGlobalVisualStudio11Win64Generator; }

  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalVisualStudio11Win64Generator::GetActualName();}
  static const char* GetActualName() {return "Visual Studio 11 Win64";}

  virtual const char* GetPlatformName() const {return "x64";}

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;

  virtual void AddPlatformDefinitions(cmMakefile* mf);
};
#endif
