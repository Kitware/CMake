/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGlobalVisualStudioVersionedGenerator_h
#define cmGlobalVisualStudioVersionedGenerator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <memory>
#include <string>

#include "cmGlobalVisualStudio14Generator.h"
#include "cmVSSetupHelper.h"

class cmGlobalGeneratorFactory;
class cmake;

/** \class cmGlobalVisualStudioVersionedGenerator  */
class cmGlobalVisualStudioVersionedGenerator
  : public cmGlobalVisualStudio14Generator
{
public:
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory15();
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory16();

  bool MatchesGeneratorName(const std::string& name) const override;

  bool SetGeneratorInstance(std::string const& i, cmMakefile* mf) override;

  bool GetVSInstance(std::string& dir) const;

  bool IsDefaultToolset(const std::string& version) const override;
  std::string GetAuxiliaryToolset() const override;

protected:
  cmGlobalVisualStudioVersionedGenerator(
    VSVersion version, cmake* cm, const std::string& name,
    std::string const& platformInGeneratorName);

  bool InitializeWindows(cmMakefile* mf) override;
  bool SelectWindowsStoreToolset(std::string& toolset) const override;

  // Used to verify that the Desktop toolset for the current generator is
  // installed on the machine.
  bool IsWindowsDesktopToolsetInstalled() const override;

  // These aren't virtual because we need to check if the selected version
  // of the toolset is installed
  bool IsWindowsStoreToolsetInstalled() const;

  // Check for a Win 8 SDK known to the registry or VS installer tool.
  bool IsWin81SDKInstalled() const;

  std::string GetWindows10SDKMaxVersion() const override;

  std::string FindMSBuildCommand() override;
  std::string FindDevEnvCommand() override;

private:
  class Factory15;
  friend class Factory15;
  class Factory16;
  friend class Factory16;
  mutable cmVSSetupAPIHelper vsSetupAPIHelper;
};
#endif
