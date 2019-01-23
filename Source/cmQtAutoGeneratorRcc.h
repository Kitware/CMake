/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoGeneratorRcc_h
#define cmQtAutoGeneratorRcc_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmFileLock.h"
#include "cmQtAutoGenerator.h"
#include "cm_uv.h"

#include <string>
#include <vector>

class cmMakefile;

// @brief AUTORCC generator
class cmQtAutoGeneratorRcc : public cmQtAutoGenerator
{
public:
  cmQtAutoGeneratorRcc();
  ~cmQtAutoGeneratorRcc() override;

  cmQtAutoGeneratorRcc(cmQtAutoGeneratorRcc const&) = delete;
  cmQtAutoGeneratorRcc& operator=(cmQtAutoGeneratorRcc const&) = delete;

private:
  // -- Types

  /// @brief Processing stage
  enum class StageT : unsigned char
  {
    SETTINGS_READ,
    TEST_QRC_RCC_FILES,
    TEST_RESOURCES_READ,
    TEST_RESOURCES,
    TEST_INFO_FILE,
    GENERATE,
    GENERATE_RCC,
    GENERATE_WRAPPER,
    SETTINGS_WRITE,
    FINISH,
    END
  };

  // -- Abstract processing interface
  bool Init(cmMakefile* makefile) override;
  bool Process() override;
  // -- Process stage
  static void UVPollStage(uv_async_t* handle);
  void PollStage();
  void SetStage(StageT stage);
  // -- Settings file
  bool SettingsFileRead();
  void SettingsFileWrite();
  // -- Tests
  bool TestQrcRccFiles();
  bool TestResourcesRead();
  bool TestResources();
  void TestInfoFile();
  // -- Generation
  void GenerateParentDir();
  bool GenerateRcc();
  void GenerateWrapper();

  // -- Utility
  bool IsMultiConfig() const { return MultiConfig_; }
  std::string MultiConfigOutput() const;
  bool StartProcess(std::string const& workingDirectory,
                    std::vector<std::string> const& command,
                    bool mergedOutput);

private:
  // -- Config settings
  bool MultiConfig_ = false;
  // -- Directories
  std::string AutogenBuildDir_;
  std::string IncludeDir_;
  // -- Qt environment
  std::string RccExecutable_;
  std::vector<std::string> RccListOptions_;
  // -- Job
  std::string LockFile_;
  cmFileLock LockFileLock_;
  std::string QrcFile_;
  std::string QrcFileName_;
  std::string QrcFileDir_;
  std::string RccPathChecksum_;
  std::string RccFileName_;
  std::string RccFileOutput_;
  std::string RccFilePublic_;
  std::vector<std::string> Options_;
  std::vector<std::string> Inputs_;
  // -- Subprocess
  ProcessResultT ProcessResult_;
  std::unique_ptr<ReadOnlyProcessT> Process_;
  // -- Settings file
  std::string SettingsFile_;
  std::string SettingsString_;
  bool SettingsChanged_ = false;
  // -- libuv loop
  StageT Stage_ = StageT::SETTINGS_READ;
  bool Error_ = false;
  bool Generate_ = false;
  bool BuildFileChanged_ = false;
};

#endif
