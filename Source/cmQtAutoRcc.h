/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmQtAutoRcc_h
#define cmQtAutoRcc_h

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmFileLock.h"
#include "cmFileTime.h"
#include "cmQtAutoGenerator.h"

#include <string>
#include <vector>

class cmMakefile;

// @brief AUTORCC generator
class cmQtAutoRcc : public cmQtAutoGenerator
{
public:
  cmQtAutoRcc();
  ~cmQtAutoRcc() override;

  cmQtAutoRcc(cmQtAutoRcc const&) = delete;
  cmQtAutoRcc& operator=(cmQtAutoRcc const&) = delete;

private:
  // -- Utility
  Logger const& Log() const { return Logger_; }
  bool IsMultiConfig() const { return MultiConfig_; }
  std::string MultiConfigOutput() const;

  // -- Abstract processing interface
  bool Init(cmMakefile* makefile) override;
  bool Process() override;
  // -- Settings file
  bool SettingsFileRead();
  bool SettingsFileWrite();
  // -- Tests
  bool TestQrcRccFiles(bool& generate);
  bool TestResources(bool& generate);
  bool TestInfoFile();
  // -- Generation
  bool GenerateRcc();
  bool GenerateWrapper();

private:
  // -- Logging
  Logger Logger_;
  // -- Config settings
  bool MultiConfig_ = false;
  // -- Directories
  std::string AutogenBuildDir_;
  std::string IncludeDir_;
  // -- Qt environment
  std::string RccExecutable_;
  cmFileTime RccExecutableTime_;
  std::vector<std::string> RccListOptions_;
  // -- Job
  std::string LockFile_;
  cmFileLock LockFileLock_;
  std::string QrcFile_;
  std::string QrcFileName_;
  std::string QrcFileDir_;
  cmFileTime QrcFileTime_;
  std::string RccPathChecksum_;
  std::string RccFileName_;
  std::string RccFileOutput_;
  std::string RccFilePublic_;
  cmFileTime RccFileTime_;
  std::string Reason;
  std::vector<std::string> Options_;
  std::vector<std::string> Inputs_;
  // -- Settings file
  std::string SettingsFile_;
  std::string SettingsString_;
  bool SettingsChanged_ = false;
  bool BuildFileChanged_ = false;
};

#endif
