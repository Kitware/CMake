/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2014 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio14Generator_h
#define cmGlobalVisualStudio14Generator_h

#include "cmGlobalVisualStudio12Generator.h"


/** \class cmGlobalVisualStudio14Generator  */
class cmGlobalVisualStudio14Generator:
  public cmGlobalVisualStudio12Generator
{
public:
  cmGlobalVisualStudio14Generator(cmake* cm, const std::string& name,
    const std::string& platformName);
  static cmGlobalGeneratorFactory* NewFactory();

  virtual bool MatchesGeneratorName(const std::string& name) const;

  virtual void WriteSLNHeader(std::ostream& fout);

  virtual const char* GetToolsVersion() { return "14.0"; }
protected:
  virtual bool InitializeWindows(cmMakefile* mf);
  virtual bool InitializeWindowsStore(cmMakefile* mf);
  virtual bool SelectWindowsStoreToolset(std::string& toolset) const;

  // These aren't virtual because we need to check if the selected version
  // of the toolset is installed
  bool IsWindowsStoreToolsetInstalled() const;

  virtual const char* GetIDEVersion() { return "14.0"; }
  virtual bool SelectWindows10SDK(cmMakefile* mf);

  // Used to verify that the Desktop toolset for the current generator is
  // installed on the machine.
  virtual bool IsWindowsDesktopToolsetInstalled() const;

  std::string GetWindows10SDKVersion();

private:
  class Factory;
};
#endif
