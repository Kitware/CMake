/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCPackExtGenerator_h
#define cmCPackExtGenerator_h

#include "cmCPackGenerator.h"
#include "cm_sys_stat.h"

#include <memory>
#include <string>

class cmGlobalGenerator;
namespace Json {
class Value;
}

/** \class cmCPackExtGenerator
 * \brief A generator for CPack External packaging tools
 */
class cmCPackExtGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackExtGenerator, cmCPackGenerator);

  const char* GetOutputExtension() override { return ".json"; }

protected:
  int InitializeInternal() override;

  int PackageFiles() override;

  bool SupportsComponentInstallation() const override;

  int InstallProjectViaInstallCommands(
    bool setDestDir, const std::string& tempInstallDirectory) override;
  int InstallProjectViaInstallScript(
    bool setDestDir, const std::string& tempInstallDirectory) override;
  int InstallProjectViaInstalledDirectories(
    bool setDestDir, const std::string& tempInstallDirectory,
    const mode_t* default_dir_mode) override;

  int RunPreinstallTarget(const std::string& installProjectName,
                          const std::string& installDirectory,
                          cmGlobalGenerator* globalGenerator,
                          const std::string& buildConfig) override;
  int InstallCMakeProject(bool setDestDir, const std::string& installDirectory,
                          const std::string& baseTempInstallDirectory,
                          const mode_t* default_dir_mode,
                          const std::string& component, bool componentInstall,
                          const std::string& installSubDirectory,
                          const std::string& buildConfig,
                          std::string& absoluteDestFiles) override;

private:
  bool StagingEnabled() const;

  class cmCPackExtVersionGenerator
  {
  public:
    cmCPackExtVersionGenerator(cmCPackExtGenerator* parent);

    virtual ~cmCPackExtVersionGenerator() = default;

    virtual int WriteToJSON(Json::Value& root);

  protected:
    virtual int GetVersionMajor() = 0;
    virtual int GetVersionMinor() = 0;

    int WriteVersion(Json::Value& root);

    cmCPackExtGenerator* Parent;
  };

  class cmCPackExtVersion1Generator : public cmCPackExtVersionGenerator
  {
  public:
    using cmCPackExtVersionGenerator::cmCPackExtVersionGenerator;

  protected:
    int GetVersionMajor() override { return 1; }
    int GetVersionMinor() override { return 0; }
  };

  std::unique_ptr<cmCPackExtVersionGenerator> Generator;
};

#endif
