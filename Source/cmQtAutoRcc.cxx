/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoRcc.h"

#include <algorithm>
#include <string>
#include <vector>

#include <cmext/algorithm>

#include "cmCryptoHash.h"
#include "cmDuration.h"
#include "cmFileLock.h"
#include "cmFileLockResult.h"
#include "cmFileTime.h"
#include "cmProcessOutput.h"
#include "cmQtAutoGen.h"
#include "cmQtAutoGenerator.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {

/** \class cmQtAutoRccT
 * \brief AUTORCC generator
 */
class cmQtAutoRccT : public cmQtAutoGenerator
{
public:
  cmQtAutoRccT();
  ~cmQtAutoRccT() override;

  cmQtAutoRccT(cmQtAutoRccT const&) = delete;
  cmQtAutoRccT& operator=(cmQtAutoRccT const&) = delete;

private:
  // -- Utility
  bool IsMultiConfig() const { return MultiConfig_; }
  std::string MultiConfigOutput() const;

  // -- Abstract processing interface
  bool InitFromInfo(InfoT const& info) override;
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

cmQtAutoRccT::cmQtAutoRccT()
  : cmQtAutoGenerator(GenT::RCC)
{
}
cmQtAutoRccT::~cmQtAutoRccT() = default;

bool cmQtAutoRccT::InitFromInfo(InfoT const& info)
{
  // -- Required settings
  if (!info.GetBool("MULTI_CONFIG", MultiConfig_, true) ||
      !info.GetString("BUILD_DIR", AutogenBuildDir_, true) ||
      !info.GetStringConfig("INCLUDE_DIR", IncludeDir_, true) ||
      !info.GetString("RCC_EXECUTABLE", RccExecutable_, true) ||
      !info.GetArray("RCC_LIST_OPTIONS", RccListOptions_, false) ||
      !info.GetString("LOCK_FILE", LockFile_, true) ||
      !info.GetStringConfig("SETTINGS_FILE", SettingsFile_, true) ||
      !info.GetString("SOURCE", QrcFile_, true) ||
      !info.GetString("OUTPUT_CHECKSUM", RccPathChecksum_, true) ||
      !info.GetString("OUTPUT_NAME", RccFileName_, true) ||
      !info.GetArray("OPTIONS", Options_, false) ||
      !info.GetArray("INPUTS", Inputs_, false)) {
    return false;
  }

  // -- Derive information
  QrcFileName_ = cmSystemTools::GetFilenameName(QrcFile_);
  QrcFileDir_ = cmSystemTools::GetFilenamePath(QrcFile_);
  RccFilePublic_ =
    cmStrCat(AutogenBuildDir_, '/', RccPathChecksum_, '/', RccFileName_);

  // rcc output file name
  if (IsMultiConfig()) {
    RccFileOutput_ = cmStrCat(IncludeDir_, '/', MultiConfigOutput());
  } else {
    RccFileOutput_ = RccFilePublic_;
  }

  // -- Checks
  if (!RccExecutableTime_.Load(RccExecutable_)) {
    return info.LogError(cmStrCat(
      "The rcc executable ", MessagePath(RccExecutable_), " does not exist."));
  }

  return true;
}

bool cmQtAutoRccT::Process()
{
  if (!SettingsFileRead()) {
    return false;
  }

  // Test if the rcc output needs to be regenerated
  bool generate = false;
  if (!TestQrcRccFiles(generate)) {
    return false;
  }
  if (!generate && !TestResources(generate)) {
    return false;
  }
  // Generate on demand
  if (generate) {
    if (!GenerateRcc()) {
      return false;
    }
  } else {
    // Test if the info file is newer than the output file
    if (!TestInfoFile()) {
      return false;
    }
  }

  if (!GenerateWrapper()) {
    return false;
  }

  return SettingsFileWrite();
}

std::string cmQtAutoRccT::MultiConfigOutput() const
{
  return cmStrCat(RccPathChecksum_, '/',
                  AppendFilenameSuffix(RccFileName_, "_CMAKE_"));
}

bool cmQtAutoRccT::SettingsFileRead()
{
  // Compose current settings strings
  {
    cmCryptoHash cryptoHash(cmCryptoHash::AlgoSHA256);
    auto cha = [&cryptoHash](cm::string_view value) {
      cryptoHash.Append(value);
      cryptoHash.Append(";");
    };
    cha(RccExecutable_);
    std::for_each(RccListOptions_.begin(), RccListOptions_.end(), cha);
    cha(QrcFile_);
    cha(RccPathChecksum_);
    cha(RccFileName_);
    std::for_each(Options_.begin(), Options_.end(), cha);
    std::for_each(Inputs_.begin(), Inputs_.end(), cha);
    SettingsString_ = cryptoHash.FinalizeHex();
  }

  // Make sure the settings file exists
  if (!cmSystemTools::FileExists(SettingsFile_, true)) {
    // Touch the settings file to make sure it exists
    if (!cmSystemTools::Touch(SettingsFile_, true)) {
      Log().Error(GenT::RCC,
                  cmStrCat("Touching the settings file ",
                           MessagePath(SettingsFile_), " failed."));
      return false;
    }
  }

  // Lock the lock file
  {
    // Make sure the lock file exists
    if (!cmSystemTools::FileExists(LockFile_, true)) {
      if (!cmSystemTools::Touch(LockFile_, true)) {
        Log().Error(GenT::RCC,
                    cmStrCat("Touching the lock file ", MessagePath(LockFile_),
                             " failed."));
        return false;
      }
    }
    // Lock the lock file
    cmFileLockResult lockResult =
      LockFileLock_.Lock(LockFile_, static_cast<unsigned long>(-1));
    if (!lockResult.IsOk()) {
      Log().Error(GenT::RCC,
                  cmStrCat("Locking of the lock file ", MessagePath(LockFile_),
                           " failed.\n", lockResult.GetOutputMessage()));
      return false;
    }
  }

  // Read old settings
  {
    std::string content;
    if (FileRead(content, SettingsFile_)) {
      SettingsChanged_ = (SettingsString_ != SettingsFind(content, "rcc"));
      // In case any setting changed clear the old settings file.
      // This triggers a full rebuild on the next run if the current
      // build is aborted before writing the current settings in the end.
      if (SettingsChanged_) {
        std::string error;
        if (!FileWrite(SettingsFile_, "", &error)) {
          Log().Error(GenT::RCC,
                      cmStrCat("Clearing of the settings file ",
                               MessagePath(SettingsFile_), " failed.\n",
                               error));
          return false;
        }
      }
    } else {
      SettingsChanged_ = true;
    }
  }

  return true;
}

bool cmQtAutoRccT::SettingsFileWrite()
{
  // Only write if any setting changed
  if (SettingsChanged_) {
    if (Log().Verbose()) {
      Log().Info(GenT::RCC,
                 "Writing settings file " + MessagePath(SettingsFile_));
    }
    // Write settings file
    std::string content = cmStrCat("rcc:", SettingsString_, '\n');
    std::string error;
    if (!FileWrite(SettingsFile_, content, &error)) {
      Log().Error(GenT::RCC,
                  cmStrCat("Writing of the settings file ",
                           MessagePath(SettingsFile_), " failed.\n", error));
      // Remove old settings file to trigger a full rebuild on the next run
      cmSystemTools::RemoveFile(SettingsFile_);
      return false;
    }
  }

  // Unlock the lock file
  LockFileLock_.Release();
  return true;
}

/// Do basic checks if rcc generation is required
bool cmQtAutoRccT::TestQrcRccFiles(bool& generate)
{
  // Test if the rcc input file exists
  if (!QrcFileTime_.Load(QrcFile_)) {
    Log().Error(GenT::RCC,
                cmStrCat("The resources file ", MessagePath(QrcFile_),
                         " does not exist"));
    return false;
  }

  // Test if the rcc output file exists
  if (!RccFileTime_.Load(RccFileOutput_)) {
    if (Log().Verbose()) {
      Reason =
        cmStrCat("Generating ", MessagePath(RccFileOutput_),
                 ", because it doesn't exist, from ", MessagePath(QrcFile_));
    }
    generate = true;
    return true;
  }

  // Test if the settings changed
  if (SettingsChanged_) {
    if (Log().Verbose()) {
      Reason = cmStrCat("Generating ", MessagePath(RccFileOutput_),
                        ", because the rcc settings changed, from ",
                        MessagePath(QrcFile_));
    }
    generate = true;
    return true;
  }

  // Test if the rcc output file is older than the .qrc file
  if (RccFileTime_.Older(QrcFileTime_)) {
    if (Log().Verbose()) {
      Reason = cmStrCat("Generating ", MessagePath(RccFileOutput_),
                        ", because it is older than ", MessagePath(QrcFile_),
                        ", from ", MessagePath(QrcFile_));
    }
    generate = true;
    return true;
  }

  // Test if the rcc output file is older than the rcc executable
  if (RccFileTime_.Older(RccExecutableTime_)) {
    if (Log().Verbose()) {
      Reason = cmStrCat("Generating ", MessagePath(RccFileOutput_),
                        ", because it is older than the rcc executable, from ",
                        MessagePath(QrcFile_));
    }
    generate = true;
    return true;
  }

  return true;
}

bool cmQtAutoRccT::TestResources(bool& generate)
{
  // Read resource files list
  if (Inputs_.empty()) {
    std::string error;
    RccLister const lister(RccExecutable_, RccListOptions_);
    if (!lister.list(QrcFile_, Inputs_, error, Log().Verbose())) {
      Log().Error(
        GenT::RCC,
        cmStrCat("Listing of ", MessagePath(QrcFile_), " failed.\n", error));
      return false;
    }
  }

  // Check if any resource file is newer than the rcc output file
  for (std::string const& resFile : Inputs_) {
    // Check if the resource file exists
    cmFileTime fileTime;
    if (!fileTime.Load(resFile)) {
      Log().Error(GenT::RCC,
                  cmStrCat("The resource file ", MessagePath(resFile),
                           " listed in ", MessagePath(QrcFile_),
                           " does not exist."));
      return false;
    }
    // Check if the resource file is newer than the rcc output file
    if (RccFileTime_.Older(fileTime)) {
      if (Log().Verbose()) {
        Reason = cmStrCat("Generating ", MessagePath(RccFileOutput_),
                          ", because it is older than ", MessagePath(resFile),
                          ", from ", MessagePath(QrcFile_));
      }
      generate = true;
      break;
    }
  }
  return true;
}

bool cmQtAutoRccT::TestInfoFile()
{
  // Test if the rcc output file is older than the info file
  if (RccFileTime_.Older(InfoFileTime())) {
    if (Log().Verbose()) {
      Log().Info(GenT::RCC,
                 cmStrCat("Touching ", MessagePath(RccFileOutput_),
                          " because it is older than ",
                          MessagePath(InfoFile())));
    }
    // Touch build file
    if (!cmSystemTools::Touch(RccFileOutput_, false)) {
      Log().Error(
        GenT::RCC,
        cmStrCat("Touching ", MessagePath(RccFileOutput_), " failed."));
      return false;
    }
    BuildFileChanged_ = true;
  }

  return true;
}

bool cmQtAutoRccT::GenerateRcc()
{
  // Make parent directory
  if (!MakeParentDirectory(RccFileOutput_)) {
    Log().Error(GenT::RCC,
                cmStrCat("Could not create parent directory of ",
                         MessagePath(RccFileOutput_)));
    return false;
  }

  // Compose rcc command
  std::vector<std::string> cmd;
  cmd.push_back(RccExecutable_);
  cm::append(cmd, Options_);
  cmd.emplace_back("-o");
  cmd.push_back(RccFileOutput_);
  cmd.push_back(QrcFile_);

  // Log reason and command
  if (Log().Verbose()) {
    Log().Info(GenT::RCC,
               cmStrCat(Reason, cmHasSuffix(Reason, '\n') ? "" : "\n",
                        QuotedCommand(cmd), '\n'));
  }

  std::string rccStdOut;
  std::string rccStdErr;
  int retVal = 0;
  bool result = cmSystemTools::RunSingleCommand(
    cmd, &rccStdOut, &rccStdErr, &retVal, AutogenBuildDir_.c_str(),
    cmSystemTools::OUTPUT_NONE, cmDuration::zero(), cmProcessOutput::Auto);
  if (!result || (retVal != 0)) {
    // rcc process failed
    Log().ErrorCommand(GenT::RCC,
                       cmStrCat("The rcc process failed to compile\n  ",
                                MessagePath(QrcFile_), "\ninto\n  ",
                                MessagePath(RccFileOutput_)),
                       cmd, rccStdOut + rccStdErr);
    cmSystemTools::RemoveFile(RccFileOutput_);
    return false;
  }

  // rcc process success
  // Print rcc output
  if (!rccStdOut.empty()) {
    Log().Info(GenT::RCC, rccStdOut);
  }
  BuildFileChanged_ = true;

  return true;
}

bool cmQtAutoRccT::GenerateWrapper()
{
  // Generate a wrapper source file on demand
  if (IsMultiConfig()) {
    // Wrapper file content
    std::string content =
      cmStrCat("// This is an autogenerated configuration wrapper file.\n",
               "// Changes will be overwritten.\n", "#include <",
               MultiConfigOutput(), ">\n");

    // Compare with existing file content
    bool fileDiffers = true;
    {
      std::string oldContents;
      if (FileRead(oldContents, RccFilePublic_)) {
        fileDiffers = (oldContents != content);
      }
    }
    if (fileDiffers) {
      // Write new wrapper file
      if (Log().Verbose()) {
        Log().Info(GenT::RCC,
                   cmStrCat("Generating RCC wrapper file ",
                            MessagePath(RccFilePublic_)));
      }
      std::string error;
      if (!FileWrite(RccFilePublic_, content, &error)) {
        Log().Error(GenT::RCC,
                    cmStrCat("Generating RCC wrapper file ",
                             MessagePath(RccFilePublic_), " failed.\n",
                             error));
        return false;
      }
    } else if (BuildFileChanged_) {
      // Just touch the wrapper file
      if (Log().Verbose()) {
        Log().Info(
          GenT::RCC,
          cmStrCat("Touching RCC wrapper file ", MessagePath(RccFilePublic_)));
      }
      if (!cmSystemTools::Touch(RccFilePublic_, false)) {
        Log().Error(GenT::RCC,
                    cmStrCat("Touching RCC wrapper file ",
                             MessagePath(RccFilePublic_), " failed."));
        return false;
      }
    }
  }
  return true;
}

} // End of unnamed namespace

bool cmQtAutoRcc(cm::string_view infoFile, cm::string_view config)
{
  return cmQtAutoRccT().Run(infoFile, config);
}
