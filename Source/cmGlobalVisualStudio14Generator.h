/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>

#include "cmGlobalVisualStudio12Generator.h"

class cmGlobalGeneratorFactory;
class cmMakefile;
class cmake;

/** \class cmGlobalVisualStudio14Generator  */
class cmGlobalVisualStudio14Generator : public cmGlobalVisualStudio12Generator
{
public:
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory();

  bool MatchesGeneratorName(const std::string& name) const override;

  const char* GetAndroidApplicationTypeRevision() const override
  {
    return "2.0";
  }

protected:
  cmGlobalVisualStudio14Generator(cmake* cm, const std::string& name,
                                  std::string const& platformInGeneratorName);

  bool InitializeWindows(cmMakefile* mf) override;
  bool InitializeWindowsStore(cmMakefile* mf) override;
  bool InitializeAndroid(cmMakefile* mf) override;
  bool SelectWindowsStoreToolset(std::string& toolset) const override;

  // These aren't virtual because we need to check if the selected version
  // of the toolset is installed
  bool IsWindowsStoreToolsetInstalled() const;

  // Used to adjust the max-SDK-version calculation to accommodate user
  // configuration.
  std::string GetWindows10SDKMaxVersion(cmMakefile* mf) const;

  // Used to make sure that the Windows 10 SDK selected can work with the
  // version of the toolset.
  virtual std::string GetWindows10SDKMaxVersionDefault(cmMakefile* mf) const;

  virtual bool SelectWindows10SDK(cmMakefile* mf, bool required);

  void SetWindowsTargetPlatformVersion(std::string const& version,
                                       cmMakefile* mf);

  // Used to verify that the Desktop toolset for the current generator is
  // installed on the machine.
  bool IsWindowsDesktopToolsetInstalled() const override;

  std::string GetWindows10SDKVersion(cmMakefile* mf);

private:
  class Factory;
  friend class Factory;
};
