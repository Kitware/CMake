/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGen.h"
#include "cmQtAutoGenerator.h"

#include "cmsys/FStream.hxx"
#include "cmsys/Terminal.h"

#include "cmAlgorithms.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmSystemTools.h"
#include "cmake.h"

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

// -- Class methods

cmQtAutoGenerator::cmQtAutoGenerator()
  : Verbose(cmSystemTools::HasEnv("VERBOSE"))
  , ColorOutput(true)
{
  {
    std::string colorEnv;
    cmSystemTools::GetEnv("COLOR", colorEnv);
    if (!colorEnv.empty()) {
      this->ColorOutput = cmSystemTools::IsOn(colorEnv.c_str());
    }
  }
}

bool cmQtAutoGenerator::Run(std::string const& infoFile,
                            std::string const& config)
{
  // Info settings
  this->InfoFile = infoFile;
  cmSystemTools::ConvertToUnixSlashes(this->InfoFile);
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
  // The OLD/WARN behavior for policy CMP0053 caused a speed regression.
  // https://gitlab.kitware.com/cmake/cmake/issues/17570
  makefile->SetPolicyVersion("3.9");
  gg.SetCurrentMakefile(makefile.get());

  return this->Process(makefile.get());
}

void cmQtAutoGenerator::LogBold(std::string const& message) const
{
  cmSystemTools::MakefileColorEcho(cmsysTerminal_Color_ForegroundBlue |
                                     cmsysTerminal_Color_ForegroundBold,
                                   message.c_str(), true, this->ColorOutput);
}

void cmQtAutoGenerator::LogInfo(cmQtAutoGen::Generator genType,
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

void cmQtAutoGenerator::LogWarning(cmQtAutoGen::Generator genType,
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

void cmQtAutoGenerator::LogFileWarning(cmQtAutoGen::Generator genType,
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

void cmQtAutoGenerator::LogError(cmQtAutoGen::Generator genType,
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

void cmQtAutoGenerator::LogFileError(cmQtAutoGen::Generator genType,
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

void cmQtAutoGenerator::LogCommandError(
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
bool cmQtAutoGenerator::MakeParentDirectory(cmQtAutoGen::Generator genType,
                                            std::string const& filename) const
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

/**
 * @brief Tests if buildFile is older than sourceFile
 * @return True if buildFile  is older than sourceFile.
 *         False may indicate an error.
 */
bool cmQtAutoGenerator::FileIsOlderThan(std::string const& buildFile,
                                        std::string const& sourceFile,
                                        std::string* error)
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

bool cmQtAutoGenerator::FileRead(std::string& content,
                                 std::string const& filename,
                                 std::string* error)
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

bool cmQtAutoGenerator::FileWrite(cmQtAutoGen::Generator genType,
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

bool cmQtAutoGenerator::FileDiffers(std::string const& filename,
                                    std::string const& content)
{
  bool differs = true;
  {
    std::string oldContents;
    if (this->FileRead(oldContents, filename)) {
      differs = (oldContents != content);
    }
  }
  return differs;
}

/**
 * @brief Runs a command and returns true on success
 * @return True on success
 */
bool cmQtAutoGenerator::RunCommand(std::vector<std::string> const& command,
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
