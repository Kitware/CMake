/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGen.h"
#include "cmQtAutoGeneratorRcc.h"

#include "cmsys/FStream.hxx"
#include "cmsys/Terminal.h"

#include "cmAlgorithms.h"
#include "cmCryptoHash.h"
#include "cmFilePathChecksum.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmOutputConverter.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmSystemTools.h"
#include "cmake.h"

#if defined(__APPLE__)
#include <unistd.h>
#endif

// -- Static variables

static const char* SettingsKeyRcc = "ARCC_SETTINGS_HASH";

// -- Static functions

static std::string HeadLine(std::string const& title)
{
  std::string head = title;
  head += '\n';
  head.append(head.size() - 1, '-');
  head += '\n';
  return head;
}

static std::string QuotedCommand(std::vector<std::string> const& command)
{
  std::string res;
  for (std::string const& item : command) {
    if (!res.empty()) {
      res.push_back(' ');
    }
    std::string const cesc = cmQtAutoGen::Quoted(item);
    if (item.empty() || (cesc.size() > (item.size() + 2)) ||
        (cesc.find(' ') != std::string::npos)) {
      res += cesc;
    } else {
      res += item;
    }
  }
  return res;
}

static bool ReadFile(std::string& content, std::string const& filename,
                     std::string* error = nullptr)
{
  bool success = false;
  if (cmSystemTools::FileExists(filename)) {
    std::size_t const length = cmSystemTools::FileLength(filename);
    cmsys::ifstream ifs(filename.c_str(), (std::ios::in | std::ios::binary));
    if (ifs) {
      content.resize(length);
      ifs.read(&content.front(), content.size());
      if (ifs) {
        success = true;
      } else {
        content.clear();
        if (error != nullptr) {
          error->append("Reading from the file failed.");
        }
      }
    } else if (error != nullptr) {
      error->append("Opening the file for reading failed.");
    }
  } else if (error != nullptr) {
    error->append("The file does not exist.");
  }
  return success;
}

/**
 * @brief Tests if buildFile is older than sourceFile
 * @return True if buildFile  is older than sourceFile.
 *         False may indicate an error.
 */
static bool FileIsOlderThan(std::string const& buildFile,
                            std::string const& sourceFile,
                            std::string* error = nullptr)
{
  int result = 0;
  if (cmSystemTools::FileTimeCompare(buildFile, sourceFile, &result)) {
    return (result < 0);
  }
  if (error != nullptr) {
    error->append(
      "File modification time comparison failed for the files\n  ");
    error->append(cmQtAutoGen::Quoted(buildFile));
    error->append("\nand\n  ");
    error->append(cmQtAutoGen::Quoted(sourceFile));
  }
  return false;
}

// -- Class methods

cmQtAutoGeneratorRcc::cmQtAutoGeneratorRcc()
  : MultiConfig(cmQtAutoGen::WRAP)
  , Verbose(cmSystemTools::HasEnv("VERBOSE"))
  , ColorOutput(true)
  , SettingsChanged(false)
{
  {
    std::string colorEnv;
    cmSystemTools::GetEnv("COLOR", colorEnv);
    if (!colorEnv.empty()) {
      this->ColorOutput = cmSystemTools::IsOn(colorEnv.c_str());
    }
  }
}

bool cmQtAutoGeneratorRcc::Run(std::string const& infoFile,
                               std::string const& config)
{
  // Info settings
  this->InfoFile = infoFile;
  this->InfoDir = cmSystemTools::GetFilenamePath(infoFile);
  this->InfoConfig = config;

  cmake cm(cmake::RoleScript);
  cm.SetHomeOutputDirectory(this->InfoDir);
  cm.SetHomeDirectory(this->InfoDir);
  cm.GetCurrentSnapshot().SetDefaultDefinitions();
  cmGlobalGenerator gg(&cm);

  cmStateSnapshot snapshot = cm.GetCurrentSnapshot();
  snapshot.GetDirectory().SetCurrentBinary(this->InfoDir);
  snapshot.GetDirectory().SetCurrentSource(this->InfoDir);

  auto makefile = cm::make_unique<cmMakefile>(&gg, snapshot);
  gg.SetCurrentMakefile(makefile.get());

  return this->Process(makefile.get());
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
      keyConf += this->InfoConfig;
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
  if (!makefile->ReadListFile(this->InfoFile.c_str())) {
    this->LogFileError(cmQtAutoGen::RCC, this->InfoFile,
                       "File processing failed");
    return false;
  }

  // -- Meta
  this->MultiConfig =
    cmQtAutoGen::MultiConfigType(InfoGet("ARCC_MULTI_CONFIG"));
  this->ConfigSuffix = InfoGetConfig("ARCC_CONFIG_SUFFIX");
  if (this->ConfigSuffix.empty()) {
    this->ConfigSuffix = "_";
    this->ConfigSuffix += this->InfoConfig;
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
    this->LogFileError(cmQtAutoGen::RCC, this->InfoFile,
                       "Settings file name missing");
    return false;
  }
  if (this->AutogenBuildDir.empty()) {
    this->LogFileError(cmQtAutoGen::RCC, this->InfoFile,
                       "Autogen build directory missing");
    return false;
  }
  if (this->RccExecutable.empty()) {
    this->LogFileError(cmQtAutoGen::RCC, this->InfoFile,
                       "rcc executable missing");
    return false;
  }
  if (this->QrcFile.empty()) {
    this->LogFileError(cmQtAutoGen::RCC, this->InfoFile,
                       "rcc input file missing");
    return false;
  }
  if (this->RccFile.empty()) {
    this->LogFileError(cmQtAutoGen::RCC, this->InfoFile,
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
    if (this->Verbose) {
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
    if (this->Verbose) {
      generateReason = "Generating ";
      generateReason += cmQtAutoGen::Quoted(rccFileAbs);
      generateReason += " from its source file ";
      generateReason += cmQtAutoGen::Quoted(this->QrcFile);
      generateReason += " because it doesn't exist";
    }
    generate = true;
  }
  if (success && !generate && this->SettingsChanged) {
    if (this->Verbose) {
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
      if (this->Verbose) {
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
          if (this->Verbose) {
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
    if (this->Verbose) {
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
      if (this->Verbose) {
        this->LogBold("Generating RCC wrapper " + wrapperFileRel);
      }
      if (!this->FileWrite(cmQtAutoGen::RCC, wrapperFileAbs, content)) {
        this->LogFileError(cmQtAutoGen::RCC, wrapperFileAbs,
                           "rcc wrapper file writing failed");
        success = false;
      }
    } else if (rccGenerated) {
      // Just touch the wrapper file
      if (this->Verbose) {
        this->LogInfo(cmQtAutoGen::RCC,
                      "Touching RCC wrapper " + wrapperFileRel);
      }
      cmSystemTools::Touch(wrapperFileAbs, false);
    }
  }

  return success;
}

void cmQtAutoGeneratorRcc::LogBold(std::string const& message) const
{
  cmSystemTools::MakefileColorEcho(cmsysTerminal_Color_ForegroundBlue |
                                     cmsysTerminal_Color_ForegroundBold,
                                   message.c_str(), true, this->ColorOutput);
}

void cmQtAutoGeneratorRcc::LogInfo(cmQtAutoGen::Generator genType,
                                   std::string const& message) const
{
  std::string msg = cmQtAutoGen::GeneratorName(genType);
  msg += ": ";
  msg += message;
  if (msg.back() != '\n') {
    msg.push_back('\n');
  }
  cmSystemTools::Stdout(msg.c_str(), msg.size());
}

void cmQtAutoGeneratorRcc::LogWarning(cmQtAutoGen::Generator genType,
                                      std::string const& message) const
{
  std::string msg = cmQtAutoGen::GeneratorName(genType);
  msg += " warning:";
  if (message.find('\n') == std::string::npos) {
    // Single line message
    msg.push_back(' ');
  } else {
    // Multi line message
    msg.push_back('\n');
  }
  // Message
  msg += message;
  if (msg.back() != '\n') {
    msg.push_back('\n');
  }
  msg.push_back('\n');
  cmSystemTools::Stdout(msg.c_str(), msg.size());
}

void cmQtAutoGeneratorRcc::LogFileWarning(cmQtAutoGen::Generator genType,
                                          std::string const& filename,
                                          std::string const& message) const
{
  std::string msg = "  ";
  msg += cmQtAutoGen::Quoted(filename);
  msg.push_back('\n');
  // Message
  msg += message;
  this->LogWarning(genType, msg);
}

void cmQtAutoGeneratorRcc::LogError(cmQtAutoGen::Generator genType,
                                    std::string const& message) const
{
  std::string msg;
  msg.push_back('\n');
  msg += HeadLine(cmQtAutoGen::GeneratorName(genType) + " error");
  // Message
  msg += message;
  if (msg.back() != '\n') {
    msg.push_back('\n');
  }
  msg.push_back('\n');
  cmSystemTools::Stderr(msg.c_str(), msg.size());
}

void cmQtAutoGeneratorRcc::LogFileError(cmQtAutoGen::Generator genType,
                                        std::string const& filename,
                                        std::string const& message) const
{
  std::string emsg = "  ";
  emsg += cmQtAutoGen::Quoted(filename);
  emsg += '\n';
  // Message
  emsg += message;
  this->LogError(genType, emsg);
}

void cmQtAutoGeneratorRcc::LogCommandError(
  cmQtAutoGen::Generator genType, std::string const& message,
  std::vector<std::string> const& command, std::string const& output) const
{
  std::string msg;
  msg.push_back('\n');
  msg += HeadLine(cmQtAutoGen::GeneratorName(genType) + " subprocess error");
  msg += message;
  if (msg.back() != '\n') {
    msg.push_back('\n');
  }
  msg.push_back('\n');
  msg += HeadLine("Command");
  msg += QuotedCommand(command);
  if (msg.back() != '\n') {
    msg.push_back('\n');
  }
  msg.push_back('\n');
  msg += HeadLine("Output");
  msg += output;
  if (msg.back() != '\n') {
    msg.push_back('\n');
  }
  msg.push_back('\n');
  cmSystemTools::Stderr(msg.c_str(), msg.size());
}

/**
 * @brief Generates the parent directory of the given file on demand
 * @return True on success
 */
bool cmQtAutoGeneratorRcc::MakeParentDirectory(
  cmQtAutoGen::Generator genType, std::string const& filename) const
{
  bool success = true;
  std::string const dirName = cmSystemTools::GetFilenamePath(filename);
  if (!dirName.empty()) {
    if (!cmSystemTools::MakeDirectory(dirName)) {
      this->LogFileError(genType, filename,
                         "Could not create parent directory");
      success = false;
    }
  }
  return success;
}

bool cmQtAutoGeneratorRcc::FileDiffers(std::string const& filename,
                                       std::string const& content)
{
  bool differs = true;
  {
    std::string oldContents;
    if (ReadFile(oldContents, filename)) {
      differs = (oldContents != content);
    }
  }
  return differs;
}

bool cmQtAutoGeneratorRcc::FileWrite(cmQtAutoGen::Generator genType,
                                     std::string const& filename,
                                     std::string const& content)
{
  std::string error;
  // Make sure the parent directory exists
  if (this->MakeParentDirectory(genType, filename)) {
    cmsys::ofstream outfile;
    outfile.open(filename.c_str(),
                 (std::ios::out | std::ios::binary | std::ios::trunc));
    if (outfile) {
      outfile << content;
      // Check for write errors
      if (!outfile.good()) {
        error = "File writing failed";
      }
    } else {
      error = "Opening file for writing failed";
    }
  }
  if (!error.empty()) {
    this->LogFileError(genType, filename, error);
    return false;
  }
  return true;
}

/**
 * @brief Runs a command and returns true on success
 * @return True on success
 */
bool cmQtAutoGeneratorRcc::RunCommand(std::vector<std::string> const& command,
                                      std::string& output) const
{
  // Log command
  if (this->Verbose) {
    std::string qcmd = QuotedCommand(command);
    qcmd.push_back('\n');
    cmSystemTools::Stdout(qcmd.c_str(), qcmd.size());
  }
  // Execute command
  int retVal = 0;
  bool res = cmSystemTools::RunSingleCommand(
    command, &output, &output, &retVal, nullptr, cmSystemTools::OUTPUT_NONE);
  return (res && (retVal == 0));
}
