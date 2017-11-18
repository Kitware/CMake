/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGen.h"
#include "cmQtAutoGeneratorRcc.h"

#include "cmAlgorithms.h"
#include "cmCryptoHash.h"
#include "cmMakefile.h"
#include "cmOutputConverter.h"
#include "cmSystemTools.h"

// -- Static variables

static const char* SettingsKeyRcc = "ARCC_SETTINGS_HASH";

// -- Class methods

cmQtAutoGeneratorRcc::cmQtAutoGeneratorRcc()
  : MultiConfig(cmQtAutoGen::WRAP)
  , SettingsChanged(false)
{
}

bool cmQtAutoGeneratorRcc::InfoFileRead(cmMakefile* makefile)
{
  // Utility lambdas
  auto InfoGet = [makefile](const char* key) {
    return makefile->GetSafeDefinition(key);
  };
  auto InfoGetList = [makefile](const char* key) -> std::vector<std::string> {
    std::vector<std::string> list;
    cmSystemTools::ExpandListArgument(makefile->GetSafeDefinition(key), list);
    return list;
  };
  auto InfoGetConfig = [makefile, this](const char* key) -> std::string {
    const char* valueConf = nullptr;
    {
      std::string keyConf = key;
      keyConf += '_';
      keyConf += this->GetInfoConfig();
      valueConf = makefile->GetDefinition(keyConf);
    }
    if (valueConf == nullptr) {
      valueConf = makefile->GetSafeDefinition(key);
    }
    return std::string(valueConf);
  };
  auto InfoGetConfigList =
    [&InfoGetConfig](const char* key) -> std::vector<std::string> {
    std::vector<std::string> list;
    cmSystemTools::ExpandListArgument(InfoGetConfig(key), list);
    return list;
  };

  // -- Read info file
  if (!makefile->ReadListFile(this->GetInfoFile().c_str())) {
    this->LogFileError(cmQtAutoGen::RCC, this->GetInfoFile(),
                       "File processing failed");
    return false;
  }

  // -- Meta
  this->MultiConfig =
    cmQtAutoGen::MultiConfigType(InfoGet("ARCC_MULTI_CONFIG"));
  this->ConfigSuffix = InfoGetConfig("ARCC_CONFIG_SUFFIX");
  if (this->ConfigSuffix.empty()) {
    this->ConfigSuffix = "_";
    this->ConfigSuffix += this->GetInfoConfig();
  }

  this->SettingsFile = InfoGetConfig("ARCC_SETTINGS_FILE");
  if (!this->SettingsFile.empty()) {
    if (this->MultiConfig != cmQtAutoGen::SINGLE) {
      this->SettingsFile = cmQtAutoGen::AppendFilenameSuffix(
        this->SettingsFile, this->ConfigSuffix);
    }
  }

  // - Files and directories
  this->ProjectSourceDir = InfoGet("ARCC_CMAKE_SOURCE_DIR");
  this->ProjectBinaryDir = InfoGet("ARCC_CMAKE_BINARY_DIR");
  this->CurrentSourceDir = InfoGet("ARCC_CMAKE_CURRENT_SOURCE_DIR");
  this->CurrentBinaryDir = InfoGet("ARCC_CMAKE_CURRENT_BINARY_DIR");
  this->AutogenBuildDir = InfoGet("ARCC_BUILD_DIR");

  // - Qt environment
  this->QtMajorVersion = InfoGet("ARCC_QT_VERSION_MAJOR");
  this->RccExecutable = InfoGet("ARCC_QT_RCC_EXECUTABLE");

  // - Job
  this->QrcFile = InfoGet("ARCC_SOURCE");
  this->RccFile = InfoGet("ARCC_OUTPUT");
  this->Options = InfoGetConfigList("ARCC_OPTIONS");
  this->Inputs = InfoGetList("ARCC_INPUTS");

  // - Validity checks
  if (this->SettingsFile.empty()) {
    this->LogFileError(cmQtAutoGen::RCC, this->GetInfoFile(),
                       "Settings file name missing");
    return false;
  }
  if (this->AutogenBuildDir.empty()) {
    this->LogFileError(cmQtAutoGen::RCC, this->GetInfoFile(),
                       "Autogen build directory missing");
    return false;
  }
  if (this->RccExecutable.empty()) {
    this->LogFileError(cmQtAutoGen::RCC, this->GetInfoFile(),
                       "rcc executable missing");
    return false;
  }
  if (this->QrcFile.empty()) {
    this->LogFileError(cmQtAutoGen::RCC, this->GetInfoFile(),
                       "rcc input file missing");
    return false;
  }
  if (this->RccFile.empty()) {
    this->LogFileError(cmQtAutoGen::RCC, this->GetInfoFile(),
                       "rcc output file missing");
    return false;
  }

  // Init derived information
  // ------------------------

  // Init file path checksum generator
  this->FilePathChecksum.setupParentDirs(
    this->CurrentSourceDir, this->CurrentBinaryDir, this->ProjectSourceDir,
    this->ProjectBinaryDir);

  return true;
}

void cmQtAutoGeneratorRcc::SettingsFileRead(cmMakefile* makefile)
{
  // Compose current settings strings
  {
    cmCryptoHash crypt(cmCryptoHash::AlgoSHA256);
    std::string const sep(" ~~~ ");
    {
      std::string str;
      str += this->RccExecutable;
      str += sep;
      str += this->QrcFile;
      str += sep;
      str += this->RccFile;
      str += sep;
      str += cmJoin(this->Options, ";");
      str += sep;
      str += cmJoin(this->Inputs, ";");
      str += sep;
      this->SettingsString = crypt.HashString(str);
    }
  }

  // Read old settings
  if (makefile->ReadListFile(this->SettingsFile.c_str())) {
    {
      auto SMatch = [makefile](const char* key, std::string const& value) {
        return (value == makefile->GetSafeDefinition(key));
      };
      if (!SMatch(SettingsKeyRcc, this->SettingsString)) {
        this->SettingsChanged = true;
      }
    }
    // In case any setting changed remove the old settings file.
    // This triggers a full rebuild on the next run if the current
    // build is aborted before writing the current settings in the end.
    if (this->SettingsChanged) {
      cmSystemTools::RemoveFile(this->SettingsFile);
    }
  } else {
    // If the file could not be read re-generate everythiung.
    this->SettingsChanged = true;
  }
}

bool cmQtAutoGeneratorRcc::SettingsFileWrite()
{
  bool success = true;
  // Only write if any setting changed
  if (this->SettingsChanged) {
    if (this->GetVerbose()) {
      this->LogInfo(cmQtAutoGen::RCC, "Writing settings file " +
                      cmQtAutoGen::Quoted(this->SettingsFile));
    }
    // Compose settings file content
    std::string settings;
    {
      auto SettingAppend = [&settings](const char* key,
                                       std::string const& value) {
        settings += "set(";
        settings += key;
        settings += " ";
        settings += cmOutputConverter::EscapeForCMake(value);
        settings += ")\n";
      };
      SettingAppend(SettingsKeyRcc, this->SettingsString);
    }
    // Write settings file
    if (!this->FileWrite(cmQtAutoGen::RCC, this->SettingsFile, settings)) {
      this->LogFileError(cmQtAutoGen::RCC, this->SettingsFile,
                         "Settings file writing failed");
      // Remove old settings file to trigger a full rebuild on the next run
      cmSystemTools::RemoveFile(this->SettingsFile);
      success = false;
    }
  }
  return success;
}

bool cmQtAutoGeneratorRcc::Process(cmMakefile* makefile)
{
  // Read info file
  if (!this->InfoFileRead(makefile)) {
    return false;
  }
  // Read latest settings
  this->SettingsFileRead(makefile);
  // Generate rcc file
  if (!this->RccGenerate()) {
    return false;
  }
  // Write latest settings
  if (!this->SettingsFileWrite()) {
    return false;
  }
  return true;
}

/**
 * @return True on success
 */
bool cmQtAutoGeneratorRcc::RccGenerate()
{
  bool success = true;
  bool rccGenerated = false;

  std::string rccFileAbs;
  {
    std::string suffix;
    switch (this->MultiConfig) {
      case cmQtAutoGen::SINGLE:
        break;
      case cmQtAutoGen::WRAP:
        suffix = "_CMAKE";
        suffix += this->ConfigSuffix;
        suffix += "_";
        break;
      case cmQtAutoGen::FULL:
        suffix = this->ConfigSuffix;
        break;
    }
    rccFileAbs = cmQtAutoGen::AppendFilenameSuffix(this->RccFile, suffix);
  }
  std::string const rccFileRel = cmSystemTools::RelativePath(
    this->AutogenBuildDir.c_str(), rccFileAbs.c_str());

  // Check if regeneration is required
  bool generate = false;
  std::string generateReason;
  if (!cmSystemTools::FileExists(this->QrcFile)) {
    {
      std::string error = "Could not find the file\n  ";
      error += cmQtAutoGen::Quoted(this->QrcFile);
      this->LogError(cmQtAutoGen::RCC, error);
    }
    success = false;
  }
  if (success && !generate && !cmSystemTools::FileExists(rccFileAbs.c_str())) {
    if (this->GetVerbose()) {
      generateReason = "Generating ";
      generateReason += cmQtAutoGen::Quoted(rccFileAbs);
      generateReason += " from its source file ";
      generateReason += cmQtAutoGen::Quoted(this->QrcFile);
      generateReason += " because it doesn't exist";
    }
    generate = true;
  }
  if (success && !generate && this->SettingsChanged) {
    if (this->GetVerbose()) {
      generateReason = "Generating ";
      generateReason += cmQtAutoGen::Quoted(rccFileAbs);
      generateReason += " from ";
      generateReason += cmQtAutoGen::Quoted(this->QrcFile);
      generateReason += " because the RCC settings changed";
    }
    generate = true;
  }
  if (success && !generate) {
    std::string error;
    if (FileIsOlderThan(rccFileAbs, this->QrcFile, &error)) {
      if (this->GetVerbose()) {
        generateReason = "Generating ";
        generateReason += cmQtAutoGen::Quoted(rccFileAbs);
        generateReason += " because it is older than ";
        generateReason += cmQtAutoGen::Quoted(this->QrcFile);
      }
      generate = true;
    } else {
      if (!error.empty()) {
        this->LogError(cmQtAutoGen::RCC, error);
        success = false;
      }
    }
  }
  if (success && !generate) {
    // Acquire input file list
    std::vector<std::string> readFiles;
    std::vector<std::string> const* files = nullptr;
    if (!this->Inputs.empty()) {
      files = &this->Inputs;
    } else {
      // Read input file list from qrc file
      std::string error;
      if (cmQtAutoGen::RccListInputs(this->QtMajorVersion, this->RccExecutable,
                                     this->QrcFile, readFiles, &error)) {
        files = &readFiles;
      } else {
        this->LogFileError(cmQtAutoGen::RCC, this->QrcFile, error);
        success = false;
      }
    }
    // Test if any input file is newer than the build file
    if (files != nullptr) {
      std::string error;
      for (std::string const& resFile : *files) {
        if (!cmSystemTools::FileExists(resFile.c_str())) {
          error = "Could not find the file\n  ";
          error += cmQtAutoGen::Quoted(resFile);
          error += "\nwhich is listed in\n  ";
          error += cmQtAutoGen::Quoted(this->QrcFile);
          break;
        }
        if (FileIsOlderThan(rccFileAbs, resFile, &error)) {
          if (this->GetVerbose()) {
            generateReason = "Generating ";
            generateReason += cmQtAutoGen::Quoted(rccFileAbs);
            generateReason += " from ";
            generateReason += cmQtAutoGen::Quoted(this->QrcFile);
            generateReason += " because it is older than ";
            generateReason += cmQtAutoGen::Quoted(resFile);
          }
          generate = true;
          break;
        }
        if (!error.empty()) {
          break;
        }
      }
      // Print error
      if (!error.empty()) {
        this->LogError(cmQtAutoGen::RCC, error);
        success = false;
      }
    }
  }
  // Regenerate on demand
  if (generate) {
    // Log
    if (this->GetVerbose()) {
      this->LogBold("Generating RCC source " + rccFileRel);
      this->LogInfo(cmQtAutoGen::RCC, generateReason);
    }

    // Make sure the parent directory exists
    if (this->MakeParentDirectory(cmQtAutoGen::RCC, rccFileAbs)) {
      // Compose rcc command
      std::vector<std::string> cmd;
      cmd.push_back(this->RccExecutable);
      cmd.insert(cmd.end(), this->Options.begin(), this->Options.end());
      cmd.push_back("-o");
      cmd.push_back(rccFileAbs);
      cmd.push_back(this->QrcFile);

      std::string output;
      if (this->RunCommand(cmd, output)) {
        // Success
        rccGenerated = true;
      } else {
        {
          std::string emsg = "rcc failed for\n  ";
          emsg += cmQtAutoGen::Quoted(this->QrcFile);
          this->LogCommandError(cmQtAutoGen::RCC, emsg, cmd, output);
        }
        cmSystemTools::RemoveFile(rccFileAbs);
        success = false;
      }
    } else {
      // Parent directory creation failed
      success = false;
    }
  }

  // Generate a wrapper source file on demand
  if (success && (this->MultiConfig == cmQtAutoGen::WRAP)) {
    // Wrapper file name
    std::string const& wrapperFileAbs = this->RccFile;
    std::string const wrapperFileRel = cmSystemTools::RelativePath(
      this->AutogenBuildDir.c_str(), wrapperFileAbs.c_str());
    // Wrapper file content
    std::string content = "// This is an autogenerated configuration "
                          "wrapper file. Changes will be overwritten.\n"
                          "#include \"";
    content += cmSystemTools::GetFilenameName(rccFileRel);
    content += "\"\n";
    // Write content to file
    if (this->FileDiffers(wrapperFileAbs, content)) {
      // Write new wrapper file
      if (this->GetVerbose()) {
        this->LogBold("Generating RCC wrapper " + wrapperFileRel);
      }
      if (!this->FileWrite(cmQtAutoGen::RCC, wrapperFileAbs, content)) {
        this->LogFileError(cmQtAutoGen::RCC, wrapperFileAbs,
                           "rcc wrapper file writing failed");
        success = false;
      }
    } else if (rccGenerated) {
      // Just touch the wrapper file
      if (this->GetVerbose()) {
        this->LogInfo(cmQtAutoGen::RCC,
                      "Touching RCC wrapper " + wrapperFileRel);
      }
      cmSystemTools::Touch(wrapperFileAbs, false);
    }
  }

  return success;
}
