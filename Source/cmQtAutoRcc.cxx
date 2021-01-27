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
  bool IsMultiConfig() const { return this->MultiConfig_; }
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
  if (!info.GetBool("MULTI_CONFIG", this->MultiConfig_, true) ||
      !info.GetString("BUILD_DIR", this->AutogenBuildDir_, true) ||
      !info.GetStringConfig("INCLUDE_DIR", this->IncludeDir_, true) ||
      !info.GetString("RCC_EXECUTABLE", this->RccExecutable_, true) ||
      !info.GetArray("RCC_LIST_OPTIONS", this->RccListOptions_, false) ||
      !info.GetString("LOCK_FILE", this->LockFile_, true) ||
      !info.GetStringConfig("SETTINGS_FILE", this->SettingsFile_, true) ||
      !info.GetString("SOURCE", this->QrcFile_, true) ||
      !info.GetString("OUTPUT_CHECKSUM", this->RccPathChecksum_, true) ||
      !info.GetString("OUTPUT_NAME", this->RccFileName_, true) ||
      !info.GetArray("OPTIONS", this->Options_, false) ||
      !info.GetArray("INPUTS", this->Inputs_, false)) {
    return false;
  }

  // -- Derive information
  this->QrcFileName_ = cmSystemTools::GetFilenameName(this->QrcFile_);
  this->QrcFileDir_ = cmSystemTools::GetFilenamePath(this->QrcFile_);
  this->RccFilePublic_ =
    cmStrCat(this->AutogenBuildDir_, '/', this->RccPathChecksum_, '/',
             this->RccFileName_);

  // rcc output file name
  if (this->IsMultiConfig()) {
    this->RccFileOutput_ =
      cmStrCat(this->IncludeDir_, '/', this->MultiConfigOutput());
  } else {
    this->RccFileOutput_ = this->RccFilePublic_;
  }

  // -- Checks
  if (!this->RccExecutableTime_.Load(this->RccExecutable_)) {
    return info.LogError(cmStrCat("The rcc executable ",
                                  this->MessagePath(this->RccExecutable_),
                                  " does not exist."));
  }

  return true;
}

bool cmQtAutoRccT::Process()
{
  if (!this->SettingsFileRead()) {
    return false;
  }

  // Test if the rcc output needs to be regenerated
  bool generate = false;
  if (!this->TestQrcRccFiles(generate)) {
    return false;
  }
  if (!generate && !this->TestResources(generate)) {
    return false;
  }
  // Generate on demand
  if (generate) {
    if (!this->GenerateRcc()) {
      return false;
    }
  } else {
    // Test if the info file is newer than the output file
    if (!this->TestInfoFile()) {
      return false;
    }
  }

  if (!this->GenerateWrapper()) {
    return false;
  }

  return this->SettingsFileWrite();
}

std::string cmQtAutoRccT::MultiConfigOutput() const
{
  return cmStrCat(this->RccPathChecksum_, '/',
                  AppendFilenameSuffix(this->RccFileName_, "_CMAKE_"));
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
    cha(this->RccExecutable_);
    std::for_each(this->RccListOptions_.begin(), this->RccListOptions_.end(),
                  cha);
    cha(this->QrcFile_);
    cha(this->RccPathChecksum_);
    cha(this->RccFileName_);
    std::for_each(this->Options_.begin(), this->Options_.end(), cha);
    std::for_each(this->Inputs_.begin(), this->Inputs_.end(), cha);
    this->SettingsString_ = cryptoHash.FinalizeHex();
  }

  // Make sure the settings file exists
  if (!cmSystemTools::FileExists(this->SettingsFile_, true)) {
    // Touch the settings file to make sure it exists
    if (!cmSystemTools::Touch(this->SettingsFile_, true)) {
      this->Log().Error(GenT::RCC,
                        cmStrCat("Touching the settings file ",
                                 this->MessagePath(this->SettingsFile_),
                                 " failed."));
      return false;
    }
  }

  // Lock the lock file
  {
    // Make sure the lock file exists
    if (!cmSystemTools::FileExists(this->LockFile_, true)) {
      if (!cmSystemTools::Touch(this->LockFile_, true)) {
        this->Log().Error(GenT::RCC,
                          cmStrCat("Touching the lock file ",
                                   this->MessagePath(this->LockFile_),
                                   " failed."));
        return false;
      }
    }
    // Lock the lock file
    cmFileLockResult lockResult = this->LockFileLock_.Lock(
      this->LockFile_, static_cast<unsigned long>(-1));
    if (!lockResult.IsOk()) {
      this->Log().Error(GenT::RCC,
                        cmStrCat("Locking of the lock file ",
                                 this->MessagePath(this->LockFile_),
                                 " failed.\n", lockResult.GetOutputMessage()));
      return false;
    }
  }

  // Read old settings
  {
    std::string content;
    if (FileRead(content, this->SettingsFile_)) {
      this->SettingsChanged_ =
        (this->SettingsString_ != SettingsFind(content, "rcc"));
      // In case any setting changed clear the old settings file.
      // This triggers a full rebuild on the next run if the current
      // build is aborted before writing the current settings in the end.
      if (this->SettingsChanged_) {
        std::string error;
        if (!FileWrite(this->SettingsFile_, "", &error)) {
          this->Log().Error(GenT::RCC,
                            cmStrCat("Clearing of the settings file ",
                                     this->MessagePath(this->SettingsFile_),
                                     " failed.\n", error));
          return false;
        }
      }
    } else {
      this->SettingsChanged_ = true;
    }
  }

  return true;
}

bool cmQtAutoRccT::SettingsFileWrite()
{
  // Only write if any setting changed
  if (this->SettingsChanged_) {
    if (this->Log().Verbose()) {
      this->Log().Info(GenT::RCC,
                       "Writing settings file " +
                         this->MessagePath(this->SettingsFile_));
    }
    // Write settings file
    std::string content = cmStrCat("rcc:", this->SettingsString_, '\n');
    std::string error;
    if (!FileWrite(this->SettingsFile_, content, &error)) {
      this->Log().Error(GenT::RCC,
                        cmStrCat("Writing of the settings file ",
                                 this->MessagePath(this->SettingsFile_),
                                 " failed.\n", error));
      // Remove old settings file to trigger a full rebuild on the next run
      cmSystemTools::RemoveFile(this->SettingsFile_);
      return false;
    }
  }

  // Unlock the lock file
  this->LockFileLock_.Release();
  return true;
}

/// Do basic checks if rcc generation is required
bool cmQtAutoRccT::TestQrcRccFiles(bool& generate)
{
  // Test if the rcc input file exists
  if (!this->QrcFileTime_.Load(this->QrcFile_)) {
    this->Log().Error(GenT::RCC,
                      cmStrCat("The resources file ",
                               this->MessagePath(this->QrcFile_),
                               " does not exist"));
    return false;
  }

  // Test if the rcc output file exists
  if (!this->RccFileTime_.Load(this->RccFileOutput_)) {
    if (this->Log().Verbose()) {
      this->Reason =
        cmStrCat("Generating ", this->MessagePath(this->RccFileOutput_),
                 ", because it doesn't exist, from ",
                 this->MessagePath(this->QrcFile_));
    }
    generate = true;
    return true;
  }

  // Test if the settings changed
  if (this->SettingsChanged_) {
    if (this->Log().Verbose()) {
      this->Reason =
        cmStrCat("Generating ", this->MessagePath(this->RccFileOutput_),
                 ", because the rcc settings changed, from ",
                 this->MessagePath(this->QrcFile_));
    }
    generate = true;
    return true;
  }

  // Test if the rcc output file is older than the .qrc file
  if (this->RccFileTime_.Older(this->QrcFileTime_)) {
    if (this->Log().Verbose()) {
      this->Reason = cmStrCat(
        "Generating ", this->MessagePath(this->RccFileOutput_),
        ", because it is older than ", this->MessagePath(this->QrcFile_),
        ", from ", this->MessagePath(this->QrcFile_));
    }
    generate = true;
    return true;
  }

  // Test if the rcc output file is older than the rcc executable
  if (this->RccFileTime_.Older(this->RccExecutableTime_)) {
    if (this->Log().Verbose()) {
      this->Reason =
        cmStrCat("Generating ", this->MessagePath(this->RccFileOutput_),
                 ", because it is older than the rcc executable, from ",
                 this->MessagePath(this->QrcFile_));
    }
    generate = true;
    return true;
  }

  return true;
}

bool cmQtAutoRccT::TestResources(bool& generate)
{
  // Read resource files list
  if (this->Inputs_.empty()) {
    std::string error;
    RccLister const lister(this->RccExecutable_, this->RccListOptions_);
    if (!lister.list(this->QrcFile_, this->Inputs_, error,
                     this->Log().Verbose())) {
      this->Log().Error(GenT::RCC,
                        cmStrCat("Listing of ",
                                 this->MessagePath(this->QrcFile_),
                                 " failed.\n", error));
      return false;
    }
  }

  // Check if any resource file is newer than the rcc output file
  for (std::string const& resFile : this->Inputs_) {
    // Check if the resource file exists
    cmFileTime fileTime;
    if (!fileTime.Load(resFile)) {
      this->Log().Error(GenT::RCC,
                        cmStrCat("The resource file ",
                                 this->MessagePath(resFile), " listed in ",
                                 this->MessagePath(this->QrcFile_),
                                 " does not exist."));
      return false;
    }
    // Check if the resource file is newer than the rcc output file
    if (this->RccFileTime_.Older(fileTime)) {
      if (this->Log().Verbose()) {
        this->Reason =
          cmStrCat("Generating ", this->MessagePath(this->RccFileOutput_),
                   ", because it is older than ", this->MessagePath(resFile),
                   ", from ", this->MessagePath(this->QrcFile_));
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
  if (this->RccFileTime_.Older(this->InfoFileTime())) {
    if (this->Log().Verbose()) {
      this->Log().Info(GenT::RCC,
                       cmStrCat("Touching ",
                                this->MessagePath(this->RccFileOutput_),
                                " because it is older than ",
                                this->MessagePath(this->InfoFile())));
    }
    // Touch build file
    if (!cmSystemTools::Touch(this->RccFileOutput_, false)) {
      this->Log().Error(GenT::RCC,
                        cmStrCat("Touching ",
                                 this->MessagePath(this->RccFileOutput_),
                                 " failed."));
      return false;
    }
    this->BuildFileChanged_ = true;
  }

  return true;
}

bool cmQtAutoRccT::GenerateRcc()
{
  // Make parent directory
  if (!MakeParentDirectory(this->RccFileOutput_)) {
    this->Log().Error(GenT::RCC,
                      cmStrCat("Could not create parent directory of ",
                               this->MessagePath(this->RccFileOutput_)));
    return false;
  }

  // Compose rcc command
  std::vector<std::string> cmd;
  cmd.push_back(this->RccExecutable_);
  cm::append(cmd, this->Options_);
  cmd.emplace_back("-o");
  cmd.push_back(this->RccFileOutput_);
  cmd.push_back(this->QrcFile_);

  // Log reason and command
  if (this->Log().Verbose()) {
    this->Log().Info(GenT::RCC,
                     cmStrCat(this->Reason,
                              cmHasSuffix(this->Reason, '\n') ? "" : "\n",
                              QuotedCommand(cmd), '\n'));
  }

  std::string rccStdOut;
  std::string rccStdErr;
  int retVal = 0;
  bool result = cmSystemTools::RunSingleCommand(
    cmd, &rccStdOut, &rccStdErr, &retVal, this->AutogenBuildDir_.c_str(),
    cmSystemTools::OUTPUT_NONE, cmDuration::zero(), cmProcessOutput::Auto);
  if (!result || (retVal != 0)) {
    // rcc process failed
    this->Log().ErrorCommand(GenT::RCC,
                             cmStrCat("The rcc process failed to compile\n  ",
                                      this->MessagePath(this->QrcFile_),
                                      "\ninto\n  ",
                                      this->MessagePath(this->RccFileOutput_)),
                             cmd, rccStdOut + rccStdErr);
    cmSystemTools::RemoveFile(this->RccFileOutput_);
    return false;
  }

  // rcc process success
  // Print rcc output
  if (!rccStdOut.empty()) {
    this->Log().Info(GenT::RCC, rccStdOut);
  }
  this->BuildFileChanged_ = true;

  return true;
}

bool cmQtAutoRccT::GenerateWrapper()
{
  // Generate a wrapper source file on demand
  if (this->IsMultiConfig()) {
    // Wrapper file content
    std::string content =
      cmStrCat("// This is an autogenerated configuration wrapper file.\n",
               "// Changes will be overwritten.\n", "#include <",
               this->MultiConfigOutput(), ">\n");

    // Compare with existing file content
    bool fileDiffers = true;
    {
      std::string oldContents;
      if (FileRead(oldContents, this->RccFilePublic_)) {
        fileDiffers = (oldContents != content);
      }
    }
    if (fileDiffers) {
      // Write new wrapper file
      if (this->Log().Verbose()) {
        this->Log().Info(GenT::RCC,
                         cmStrCat("Generating RCC wrapper file ",
                                  this->MessagePath(this->RccFilePublic_)));
      }
      std::string error;
      if (!FileWrite(this->RccFilePublic_, content, &error)) {
        this->Log().Error(GenT::RCC,
                          cmStrCat("Generating RCC wrapper file ",
                                   this->MessagePath(this->RccFilePublic_),
                                   " failed.\n", error));
        return false;
      }
    } else if (this->BuildFileChanged_) {
      // Just touch the wrapper file
      if (this->Log().Verbose()) {
        this->Log().Info(GenT::RCC,
                         cmStrCat("Touching RCC wrapper file ",
                                  this->MessagePath(this->RccFilePublic_)));
      }
      if (!cmSystemTools::Touch(this->RccFilePublic_, false)) {
        this->Log().Error(GenT::RCC,
                          cmStrCat("Touching RCC wrapper file ",
                                   this->MessagePath(this->RccFilePublic_),
                                   " failed."));
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
