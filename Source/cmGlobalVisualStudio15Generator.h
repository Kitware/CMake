/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2014 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalVisualStudio15Generator_h
#define cmGlobalVisualStudio15Generator_h

#include "cmGlobalVisualStudio14Generator.h"

/** \class cmGlobalVisualStudio15Generator  */
class cmGlobalVisualStudio15Generator : public cmGlobalVisualStudio14Generator
{
public:
  cmGlobalVisualStudio15Generator(cmake* cm, const std::string& name,
                                  const std::string& platformName);
  static cmGlobalGeneratorFactory* NewFactory();

  virtual bool MatchesGeneratorName(const std::string& name) const;

  virtual void WriteSLNHeader(std::ostream& fout);

  virtual void FindMakeProgram(cmMakefile*);

  virtual const char* GetToolsVersion() { return "15.0"; }
protected:

  virtual bool SelectWindowsStoreToolset(std::string& toolset) const;

  // These aren't virtual because we need to check if the selected version
  // of the toolset is installed
  bool IsWindowsStoreToolsetInstalled() const;

  std::string const& GetMSBuildCommand();
  virtual const char* GetIDEVersion() { return "15.0"; }
  virtual bool SelectWindows10SDK(cmMakefile* mf, bool required);

  // Used to verify that the Desktop toolset for the current generator is
  // installed on the machine.
  virtual bool IsWindowsDesktopToolsetInstalled() const;

  std::string GetWindows10SDKVersion();

  std::string MSBuildCommand;
  bool MSBuildCommandInitialized;
  virtual std::string FindMSBuildCommand();
  virtual std::string FindDevEnvCommand();
  virtual std::string GetVSMakeProgram() { return this->GetMSBuildCommand(); }

private:
  class Factory;
};
#endif
