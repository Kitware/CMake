/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoRcc.h"

#include "cmAlgorithms.h"
#include "cmCryptoHash.h"
#include "cmDuration.h"
#include "cmFileLockResult.h"
#include "cmMakefile.h"
#include "cmProcessOutput.h"
#include "cmQtAutoGen.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cm_string_view.hxx"

#include <algorithm>

cmQtAutoRcc::cmQtAutoRcc() = default;

cmQtAutoRcc::~cmQtAutoRcc() = default;

bool cmQtAutoRcc::Init(cmMakefile* makefile)
{
  // -- Utility lambdas
  auto InfoGet = [makefile](cm::string_view key) {
    return makefile->GetSafeDefinition(std::string(key));
  };
  auto InfoGetList =
    [makefile](cm::string_view key) -> std::vector<std::string> {
    return cmExpandedList(makefile->GetSafeDefinition(std::string(key)));
  };
  auto InfoGetConfig = [makefile, this](cm::string_view key) -> std::string {
    if (const char* valueConf =
          makefile->GetDefinition(cmStrCat(key, '_', InfoConfig()))) {
      return std::string(valueConf);
    }
    return makefile->GetSafeDefinition(std::string(key));
  };
  auto InfoGetConfigList =
    [&InfoGetConfig](cm::string_view key) -> std::vector<std::string> {
    return cmExpandedList(InfoGetConfig(key));
  };
  auto LogInfoError = [this](cm::string_view msg) -> bool {
    this->Log().Error(GenT::RCC,
                      cmStrCat("In ", Quoted(this->InfoFile()), ":\n", msg));
    return false;
  };

  // -- Read info file
  if (!makefile->ReadListFile(InfoFile())) {
    return LogInfoError("File processing failed.");
  }

  // - Configurations
  Logger_.RaiseVerbosity(InfoGet("ARCC_VERBOSITY"));
  MultiConfig_ = makefile->IsOn("ARCC_MULTI_CONFIG");

  // - Directories
  AutogenBuildDir_ = InfoGet("ARCC_BUILD_DIR");
  if (AutogenBuildDir_.empty()) {
    return LogInfoError("Build directory empty.");
  }

  IncludeDir_ = InfoGetConfig("ARCC_INCLUDE_DIR");
  if (IncludeDir_.empty()) {
    return LogInfoError("Include directory empty.");
  }

  // - Rcc executable
  RccExecutable_ = InfoGet("ARCC_RCC_EXECUTABLE");
  if (!RccExecutableTime_.Load(RccExecutable_)) {
    return LogInfoError(cmStrCat("The rcc executable ", Quoted(RccExecutable_),
                                 " does not exist."));
  }
  RccListOptions_ = InfoGetList("ARCC_RCC_LIST_OPTIONS");

  // - Job
  LockFile_ = InfoGet("ARCC_LOCK_FILE");
  QrcFile_ = InfoGet("ARCC_SOURCE");
  QrcFileName_ = cmSystemTools::GetFilenameName(QrcFile_);
  QrcFileDir_ = cmSystemTools::GetFilenamePath(QrcFile_);
  RccPathChecksum_ = InfoGet("ARCC_OUTPUT_CHECKSUM");
  RccFileName_ = InfoGet("ARCC_OUTPUT_NAME");
  Options_ = InfoGetConfigList("ARCC_OPTIONS");
  Inputs_ = InfoGetList("ARCC_INPUTS");

  // - Settings file
  SettingsFile_ = InfoGetConfig("ARCC_SETTINGS_FILE");

  // - Validity checks
  if (LockFile_.empty()) {
    return LogInfoError("Lock file name missing.");
  }
  if (SettingsFile_.empty()) {
    return LogInfoError("Settings file name missing.");
  }
  if (AutogenBuildDir_.empty()) {
    return LogInfoError("Autogen build directory missing.");
  }
  if (RccExecutable_.empty()) {
    return LogInfoError("rcc executable missing.");
  }
  if (QrcFile_.empty()) {
    return LogInfoError("rcc input file missing.");
  }
  if (RccFileName_.empty()) {
    return LogInfoError("rcc output file missing.");
  }

  // Init derived information
  // ------------------------

  RccFilePublic_ =
    cmStrCat(AutogenBuildDir_, '/', RccPathChecksum_, '/', RccFileName_);

  // Compute rcc output file name
  if (IsMultiConfig()) {
    RccFileOutput_ = cmStrCat(IncludeDir_, '/', MultiConfigOutput());
  } else {
    RccFileOutput_ = RccFilePublic_;
  }

  return true;
}

bool cmQtAutoRcc::Process()
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

std::string cmQtAutoRcc::MultiConfigOutput() const
{
  return cmStrCat(RccPathChecksum_, '/',
                  AppendFilenameSuffix(RccFileName_, "_CMAKE_"));
}

bool cmQtAutoRcc::SettingsFileRead()
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
      Log().ErrorFile(GenT::RCC, SettingsFile_,
                      "Settings file creation failed.");
      return false;
    }
  }

  // Lock the lock file
  {
    // Make sure the lock file exists
    if (!cmSystemTools::FileExists(LockFile_, true)) {
      if (!cmSystemTools::Touch(LockFile_, true)) {
        Log().ErrorFile(GenT::RCC, LockFile_, "Lock file creation failed.");
        return false;
      }
    }
    // Lock the lock file
    cmFileLockResult lockResult =
      LockFileLock_.Lock(LockFile_, static_cast<unsigned long>(-1));
    if (!lockResult.IsOk()) {
      Log().ErrorFile(GenT::RCC, LockFile_,
                      "File lock failed: " + lockResult.GetOutputMessage());
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
          Log().ErrorFile(GenT::RCC, SettingsFile_,
                          "Settings file clearing failed. " + error);
          return false;
        }
      }
    } else {
      SettingsChanged_ = true;
    }
  }

  return true;
}

bool cmQtAutoRcc::SettingsFileWrite()
{
  // Only write if any setting changed
  if (SettingsChanged_) {
    if (Log().Verbose()) {
      Log().Info(GenT::RCC, "Writing settings file " + Quoted(SettingsFile_));
    }
    // Write settings file
    std::string content = cmStrCat("rcc:", SettingsString_, '\n');
    std::string error;
    if (!FileWrite(SettingsFile_, content, &error)) {
      Log().ErrorFile(GenT::RCC, SettingsFile_,
                      "Settings file writing failed. " + error);
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
bool cmQtAutoRcc::TestQrcRccFiles(bool& generate)
{
  // Test if the rcc input file exists
  if (!QrcFileTime_.Load(QrcFile_)) {
    Log().ErrorFile(
      GenT::RCC, QrcFile_,
      cmStrCat("The resources file ", Quoted(QrcFile_), " does not exist"));
    return false;
  }

  // Test if the rcc output file exists
  if (!RccFileTime_.Load(RccFileOutput_)) {
    if (Log().Verbose()) {
      Reason = cmStrCat("Generating ", Quoted(RccFileOutput_),
                        ", because it doesn't exist, from ", Quoted(QrcFile_));
    }
    generate = true;
    return true;
  }

  // Test if the settings changed
  if (SettingsChanged_) {
    if (Log().Verbose()) {
      Reason = cmStrCat("Generating ", Quoted(RccFileOutput_),
                        ", because the rcc settings changed, from ",
                        Quoted(QrcFile_));
    }
    generate = true;
    return true;
  }

  // Test if the rcc output file is older than the .qrc file
  if (RccFileTime_.Older(QrcFileTime_)) {
    if (Log().Verbose()) {
      Reason = cmStrCat("Generating ", Quoted(RccFileOutput_),
                        ", because it is older than ", Quoted(QrcFile_),
                        ", from ", Quoted(QrcFile_));
    }
    generate = true;
    return true;
  }

  // Test if the rcc output file is older than the rcc executable
  if (RccFileTime_.Older(RccExecutableTime_)) {
    if (Log().Verbose()) {
      Reason = cmStrCat("Generating ", Quoted(RccFileOutput_),
                        ", because it is older than the rcc executable, from ",
                        Quoted(QrcFile_));
    }
    generate = true;
    return true;
  }

  return true;
}

bool cmQtAutoRcc::TestResources(bool& generate)
{
  // Read resource files list
  if (Inputs_.empty()) {
    std::string error;
    RccLister const lister(RccExecutable_, RccListOptions_);
    if (!lister.list(QrcFile_, Inputs_, error, Log().Verbose())) {
      Log().ErrorFile(GenT::RCC, QrcFile_, error);
      return false;
    }
  }

  // Check if any resource file is newer than the rcc output file
  for (std::string const& resFile : Inputs_) {
    // Check if the resource file exists
    cmFileTime fileTime;
    if (!fileTime.Load(resFile)) {
      Log().ErrorFile(GenT::RCC, QrcFile_,
                      cmStrCat("Could not find the resource file\n  ",
                               Quoted(resFile), '\n'));
      return false;
    }
    // Check if the resource file is newer than the rcc output file
    if (RccFileTime_.Older(fileTime)) {
      if (Log().Verbose()) {
        Reason = cmStrCat("Generating ", Quoted(RccFileOutput_),
                          ", because it is older than ", Quoted(resFile),
                          ", from ", Quoted(QrcFile_));
      }
      generate = true;
      break;
    }
  }
  return true;
}

bool cmQtAutoRcc::TestInfoFile()
{
  // Test if the rcc output file is older than the info file
  if (RccFileTime_.Older(InfoFileTime())) {
    if (Log().Verbose()) {
      Log().Info(GenT::RCC,
                 cmStrCat("Touching ", Quoted(RccFileOutput_),
                          " because it is older than ", Quoted(InfoFile())));
    }
    // Touch build file
    if (!cmSystemTools::Touch(RccFileOutput_, false)) {
      Log().ErrorFile(GenT::RCC, RccFileOutput_, "Build file touch failed");
      return false;
    }
    BuildFileChanged_ = true;
  }

  return true;
}

bool cmQtAutoRcc::GenerateRcc()
{
  // Make parent directory
  if (!MakeParentDirectory(RccFileOutput_)) {
    Log().ErrorFile(GenT::RCC, RccFileOutput_,
                    "Could not create parent directory");
    return false;
  }

  // Compose rcc command
  std::vector<std::string> cmd;
  cmd.push_back(RccExecutable_);
  cmAppend(cmd, Options_);
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
                                Quoted(QrcFile_), "\ninto\n  ",
                                Quoted(RccFileOutput_)),
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

bool cmQtAutoRcc::GenerateWrapper()
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
        Log().Info(GenT::RCC, "Generating RCC wrapper file " + RccFilePublic_);
      }
      std::string error;
      if (!FileWrite(RccFilePublic_, content, &error)) {
        Log().ErrorFile(GenT::RCC, RccFilePublic_,
                        "RCC wrapper file writing failed. " + error);
        return false;
      }
    } else if (BuildFileChanged_) {
      // Just touch the wrapper file
      if (Log().Verbose()) {
        Log().Info(GenT::RCC, "Touching RCC wrapper file " + RccFilePublic_);
      }
      if (!cmSystemTools::Touch(RccFilePublic_, false)) {
        Log().ErrorFile(GenT::RCC, RccFilePublic_,
                        "RCC wrapper file touch failed.");
        return false;
      }
    }
  }
  return true;
}
