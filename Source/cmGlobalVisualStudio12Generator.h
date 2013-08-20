/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2011 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio12Generator_h
#define cmGlobalVisualStudio12Generator_h

#include "cmGlobalVisualStudio11Generator.h"


/** \class cmGlobalVisualStudio12Generator  */
class cmGlobalVisualStudio12Generator:
  public cmGlobalVisualStudio11Generator
{
public:
  cmGlobalVisualStudio12Generator(const char* name,
    const char* platformName, const char* additionalPlatformDefinition);
  static cmGlobalGeneratorFactory* NewFactory();

  virtual void WriteSLNHeader(std::ostream& fout);

  ///! create the correct local generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /** TODO: VS 12 user macro support. */
  virtual std::string GetUserMacrosDirectory() { return ""; }

  //in Visual Studio 2013 they detached the MSBuild tools version
  //from the .Net Framework version and instead made it have it's own
  //version number
  virtual const char* GetToolsVersion() { return "12.0"; }
protected:
  virtual const char* GetIDEVersion() { return "12.0"; }
private:
  class Factory;
};
#endif
