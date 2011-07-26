/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio10IA64Generator_h
#define cmGlobalVisualStudio10IA64Generator_h

#include "cmGlobalVisualStudio10Generator.h"

class cmGlobalVisualStudio10IA64Generator :
  public cmGlobalVisualStudio10Generator
{
public:
  cmGlobalVisualStudio10IA64Generator();
  static cmGlobalGenerator* New() {
    return new cmGlobalVisualStudio10IA64Generator; }

  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalVisualStudio10IA64Generator::GetActualName();}
  static const char* GetActualName() {return "Visual Studio 10 IA64";}

  virtual const char* GetPlatformName() const {return "Itanium";}

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;

  virtual void AddPlatformDefinitions(cmMakefile* mf);

  virtual void EnableLanguage(std::vector<std::string>const& languages,
                              cmMakefile *, bool optional);
};
#endif
