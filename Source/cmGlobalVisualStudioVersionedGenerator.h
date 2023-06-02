/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>

#include <cm/optional>

#include "cmGlobalVisualStudio10Generator.h"
#include "cmGlobalVisualStudio14Generator.h"
#include "cmGlobalVisualStudioGenerator.h"
#include "cmVSSetupHelper.h"

class cmGlobalGeneratorFactory;
class cmMakefile;
class cmake;

/** \class cmGlobalVisualStudioVersionedGenerator  */
class cmGlobalVisualStudioVersionedGenerator
  : public cmGlobalVisualStudio14Generator
{
public:
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory15();
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory16();
  static std::unique_ptr<cmGlobalGeneratorFactory> NewFactory17();

  bool MatchesGeneratorName(const std::string& name) const override;

  bool SetGeneratorInstance(std::string const& i, cmMakefile* mf) override;

  bool GetVSInstance(std::string& dir) const;

  cm::optional<std::string> FindMSBuildCommandEarly(cmMakefile* mf) override;

  cm::optional<std::string> GetVSInstanceVersion() const override;

  AuxToolset FindAuxToolset(std::string& version,
                            std::string& props) const override;

  bool IsStdOutEncodingSupported() const override;

  bool IsUtf8EncodingSupported() const override;

  bool IsScanDependenciesSupported() const override;

  const char* GetAndroidApplicationTypeRevision() const override;

  bool CheckCxxModuleSupport() override
  {
    return this->SupportsCxxModuleDyndep();
  }
  bool SupportsCxxModuleDyndep() const override
  {
    return this->Version >= cmGlobalVisualStudioGenerator::VSVersion::VS17;
  }

protected:
  cmGlobalVisualStudioVersionedGenerator(
    VSVersion version, cmake* cm, const std::string& name,
    std::string const& platformInGeneratorName);

  bool SelectWindowsStoreToolset(std::string& toolset) const override;

  // Used to verify that the Desktop toolset for the current generator is
  // installed on the machine.
  bool IsWindowsDesktopToolsetInstalled() const override;

  // These aren't virtual because we need to check if the selected version
  // of the toolset is installed
  bool IsWindowsStoreToolsetInstalled() const;

  bool InitializePlatformWindows(cmMakefile* mf) override;

  // Check for a Win 8 SDK known to the registry or VS installer tool.
  bool IsWin81SDKInstalled() const;

  std::string GetWindows10SDKMaxVersionDefault(cmMakefile*) const override;

  virtual bool ProcessGeneratorInstanceField(std::string const& key,
                                             std::string const& value);

  std::string FindMSBuildCommand() override;
  std::string FindDevEnvCommand() override;

private:
  class Factory15;
  friend class Factory15;
  class Factory16;
  friend class Factory16;
  class Factory17;
  friend class Factory17;
  mutable cmVSSetupAPIHelper vsSetupAPIHelper;

  bool ParseGeneratorInstance(std::string const& is, cmMakefile* mf);
  void SetVSVersionVar(cmMakefile* mf);

  std::string GeneratorInstance;
  std::string GeneratorInstanceVersion;
  cm::optional<std::string> LastGeneratorInstanceString;
};
