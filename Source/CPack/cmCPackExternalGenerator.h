/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <memory>
#include <string>

#include "cm_sys_stat.h"

#include "cmCPackGenerator.h"

class cmGlobalGenerator;
namespace Json {
class Value;
}

/** \class cmCPackExternalGenerator
 * \brief A generator for CPack External packaging tools
 */
class cmCPackExternalGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackExternalGenerator, cmCPackGenerator);

  char const* GetOutputExtension() override { return ".json"; }

protected:
  int InitializeInternal() override;

  int PackageFiles() override;

  bool SupportsComponentInstallation() const override;

  int InstallProjectViaInstallCommands(
    bool setDestDir, std::string const& tempInstallDirectory) override;
  int InstallProjectViaInstallScript(
    bool setDestDir, std::string const& tempInstallDirectory) override;
  int InstallProjectViaInstalledDirectories(
    bool setDestDir, std::string const& tempInstallDirectory,
    mode_t const* default_dir_mode) override;

  int RunPreinstallTarget(std::string const& installProjectName,
                          std::string const& installDirectory,
                          cmGlobalGenerator* globalGenerator,
                          std::string const& buildConfig) override;
  int InstallCMakeProject(bool setDestDir, std::string const& installDirectory,
                          std::string const& baseTempInstallDirectory,
                          mode_t const* default_dir_mode,
                          std::string const& component, bool componentInstall,
                          std::string const& installSubDirectory,
                          std::string const& buildConfig,
                          std::string& absoluteDestFiles) override;

private:
  bool StagingEnabled() const;

  class cmCPackExternalVersionGenerator
  {
  public:
    cmCPackExternalVersionGenerator(cmCPackExternalGenerator* parent);

    virtual ~cmCPackExternalVersionGenerator() = default;

    virtual int WriteToJSON(Json::Value& root);

  protected:
    virtual int GetVersionMajor() = 0;
    virtual int GetVersionMinor() = 0;

    int WriteVersion(Json::Value& root);

    cmCPackExternalGenerator* Parent;
  };

  class cmCPackExternalVersion1Generator
    : public cmCPackExternalVersionGenerator
  {
  public:
    using cmCPackExternalVersionGenerator::cmCPackExternalVersionGenerator;

  protected:
    int GetVersionMajor() override { return 1; }
    int GetVersionMinor() override { return 0; }
  };

  std::unique_ptr<cmCPackExternalVersionGenerator> Generator;
};
