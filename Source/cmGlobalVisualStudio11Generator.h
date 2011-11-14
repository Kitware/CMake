/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2011 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio11Generator_h
#define cmGlobalVisualStudio11Generator_h

#include "cmGlobalVisualStudio10Generator.h"


/** \class cmGlobalVisualStudio11Generator  */
class cmGlobalVisualStudio11Generator:
  public cmGlobalVisualStudio10Generator
{
public:
  cmGlobalVisualStudio11Generator();
  static cmGlobalGenerator* New() {
    return new cmGlobalVisualStudio11Generator; }

  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalVisualStudio11Generator::GetActualName();}
  static const char* GetActualName() {return "Visual Studio 11";}
  virtual void AddPlatformDefinitions(cmMakefile* mf);

  virtual void WriteSLNHeader(std::ostream& fout);

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;

  ///! create the correct local generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /** TODO: VS 11 user macro support. */
  virtual std::string GetUserMacrosDirectory() { return ""; }
protected:
  virtual const char* GetIDEVersion() { return "11.0"; }
};
#endif
