/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>

#include <cm/optional>

#include "cmGlobalVisualStudio12Generator.h"

class cmGlobalGeneratorFactory;
class cmMakefile;
class cmake;

/** \class cmGlobalVisualStudio14Generator  */
class cmGlobalVisualStudio14Generator : public cmGlobalVisualStudio12Generator
{
public:
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory();

  bool MatchesGeneratorName(std::string const& name) const override;

  char const* GetAndroidApplicationTypeRevision() const override
  {
    return "2.0";
  }

protected:
  cmGlobalVisualStudio14Generator(cmake* cm, std::string const& name);

  bool InitializeWindowsStore(cmMakefile* mf) override;
  bool InitializeAndroid(cmMakefile* mf) override;
  bool SelectWindowsStoreToolset(std::string& toolset) const override;

  // These aren't virtual because we need to check if the selected version
  // of the toolset is installed
  bool IsWindowsStoreToolsetInstalled() const;

  virtual bool IsWin81SDKInstalled() const;

  bool InitializePlatformWindows(cmMakefile* mf) override;
  bool VerifyNoGeneratorPlatformVersion(cmMakefile* mf) const override;

  bool ProcessGeneratorPlatformField(std::string const& key,
                                     std::string const& value) override;

  // Used to adjust the max-SDK-version calculation to accommodate user
  // configuration.
  std::string GetWindows10SDKMaxVersion(cmMakefile* mf) const;

  // Used to make sure that the Windows 10 SDK selected can work with the
  // version of the toolset.
  virtual std::string GetWindows10SDKMaxVersionDefault(cmMakefile* mf) const;

  virtual bool SelectWindows10SDK(cmMakefile* mf);

  void SetWindowsTargetPlatformVersion(std::string const& version,
                                       cmMakefile* mf);

  // Used to verify that the Desktop toolset for the current generator is
  // installed on the machine.
  bool IsWindowsDesktopToolsetInstalled() const override;

  std::string GetWindows10SDKVersion(cmMakefile* mf);

  void AddSolutionItems(cmLocalGenerator* root) override;

  void WriteFolderSolutionItems(std::ostream& fout,
                                cmVisualStudioFolder const& folder) override;

private:
  class Factory;
  friend class Factory;

  cm::optional<std::string> GeneratorPlatformVersion;
};
