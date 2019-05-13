/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGenerator.h"
#include "cmQtAutoGen.h"

#include "cmsys/FStream.hxx"

#include "cmAlgorithms.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmSystemTools.h"
#include "cmake.h"

cmQtAutoGenerator::Logger::Logger()
{
  // Initialize logger
  {
    std::string verbose;
    if (cmSystemTools::GetEnv("VERBOSE", verbose) && !verbose.empty()) {
      unsigned long iVerbose = 0;
      if (cmSystemTools::StringToULong(verbose.c_str(), &iVerbose)) {
        SetVerbosity(static_cast<unsigned int>(iVerbose));
      } else {
        // Non numeric verbosity
        SetVerbose(cmSystemTools::IsOn(verbose));
      }
    }
  }
  {
    std::string colorEnv;
    cmSystemTools::GetEnv("COLOR", colorEnv);
    if (!colorEnv.empty()) {
      SetColorOutput(cmSystemTools::IsOn(colorEnv));
    } else {
      SetColorOutput(true);
    }
  }
}

cmQtAutoGenerator::Logger::~Logger() = default;

void cmQtAutoGenerator::Logger::RaiseVerbosity(std::string const& value)
{
  unsigned long verbosity = 0;
  if (cmSystemTools::StringToULong(value.c_str(), &verbosity)) {
    if (this->Verbosity_ < verbosity) {
      this->Verbosity_ = static_cast<unsigned int>(verbosity);
    }
  }
}

void cmQtAutoGenerator::Logger::SetColorOutput(bool value)
{
  ColorOutput_ = value;
}

std::string cmQtAutoGenerator::Logger::HeadLine(std::string const& title)
{
  std::string head = title;
  head += '\n';
  head.append(head.size() - 1, '-');
  head += '\n';
  return head;
}

void cmQtAutoGenerator::Logger::Info(GenT genType,
                                     std::string const& message) const
{
  std::string msg = GeneratorName(genType);
  msg += ": ";
  msg += message;
  if (msg.back() != '\n') {
    msg.push_back('\n');
  }
  {
    std::lock_guard<std::mutex> lock(Mutex_);
    cmSystemTools::Stdout(msg);
  }
}

void cmQtAutoGenerator::Logger::Warning(GenT genType,
                                        std::string const& message) const
{
  std::string msg;
  if (message.find('\n') == std::string::npos) {
    // Single line message
    msg += GeneratorName(genType);
    msg += " warning: ";
  } else {
    // Multi line message
    msg += HeadLine(GeneratorName(genType) + " warning");
  }
  // Message
  msg += message;
  if (msg.back() != '\n') {
    msg.push_back('\n');
  }
  msg.push_back('\n');
  {
    std::lock_guard<std::mutex> lock(Mutex_);
    cmSystemTools::Stdout(msg);
  }
}

void cmQtAutoGenerator::Logger::WarningFile(GenT genType,
                                            std::string const& filename,
                                            std::string const& message) const
{
  std::string msg = "  ";
  msg += Quoted(filename);
  msg.push_back('\n');
  // Message
  msg += message;
  Warning(genType, msg);
}

void cmQtAutoGenerator::Logger::Error(GenT genType,
                                      std::string const& message) const
{
  std::string msg;
  msg += HeadLine(GeneratorName(genType) + " error");
  // Message
  msg += message;
  if (msg.back() != '\n') {
    msg.push_back('\n');
  }
  msg.push_back('\n');
  {
    std::lock_guard<std::mutex> lock(Mutex_);
    cmSystemTools::Stderr(msg);
  }
}

void cmQtAutoGenerator::Logger::ErrorFile(GenT genType,
                                          std::string const& filename,
                                          std::string const& message) const
{
  std::string emsg = "  ";
  emsg += Quoted(filename);
  emsg += '\n';
  // Message
  emsg += message;
  Error(genType, emsg);
}

void cmQtAutoGenerator::Logger::ErrorCommand(
  GenT genType, std::string const& message,
  std::vector<std::string> const& command, std::string const& output) const
{
  std::string msg;
  msg.push_back('\n');
  msg += HeadLine(GeneratorName(genType) + " subprocess error");
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
  {
    std::lock_guard<std::mutex> lock(Mutex_);
    cmSystemTools::Stderr(msg);
  }
}

bool cmQtAutoGenerator::MakeParentDirectory(std::string const& filename)
{
  bool success = true;
  std::string const dirName = cmSystemTools::GetFilenamePath(filename);
  if (!dirName.empty()) {
    success = cmSystemTools::MakeDirectory(dirName);
  }
  return success;
}

bool cmQtAutoGenerator::FileRead(std::string& content,
                                 std::string const& filename,
                                 std::string* error)
{
  content.clear();
  if (!cmSystemTools::FileExists(filename, true)) {
    if (error != nullptr) {
      *error = "Not a file.";
    }
    return false;
  }

  unsigned long const length = cmSystemTools::FileLength(filename);
  cmsys::ifstream ifs(filename.c_str(), (std::ios::in | std::ios::binary));

  // Use lambda to save destructor calls of ifs
  return [&ifs, length, &content, error]() -> bool {
    if (!ifs) {
      if (error != nullptr) {
        *error = "Opening the file for reading failed.";
      }
      return false;
    }
    content.reserve(length);
    typedef std::istreambuf_iterator<char> IsIt;
    content.assign(IsIt{ ifs }, IsIt{});
    if (!ifs) {
      content.clear();
      if (error != nullptr) {
        *error = "Reading from the file failed.";
      }
      return false;
    }
    return true;
  }();
}

bool cmQtAutoGenerator::FileWrite(std::string const& filename,
                                  std::string const& content,
                                  std::string* error)
{
  // Make sure the parent directory exists
  if (!cmQtAutoGenerator::MakeParentDirectory(filename)) {
    if (error != nullptr) {
      *error = "Could not create parent directory.";
    }
    return false;
  }
  cmsys::ofstream ofs;
  ofs.open(filename.c_str(),
           (std::ios::out | std::ios::binary | std::ios::trunc));

  // Use lambda to save destructor calls of ofs
  return [&ofs, &content, error]() -> bool {
    if (!ofs) {
      if (error != nullptr) {
        *error = "Opening file for writing failed.";
      }
      return false;
    }
    ofs << content;
    if (!ofs.good()) {
      if (error != nullptr) {
        *error = "File writing failed.";
      }
      return false;
    }
    return true;
  }();
}

bool cmQtAutoGenerator::FileDiffers(std::string const& filename,
                                    std::string const& content)
{
  bool differs = true;
  std::string oldContents;
  if (FileRead(oldContents, filename) && (oldContents == content)) {
    differs = false;
  }
  return differs;
}

cmQtAutoGenerator::cmQtAutoGenerator() = default;

cmQtAutoGenerator::~cmQtAutoGenerator() = default;

bool cmQtAutoGenerator::Run(std::string const& infoFile,
                            std::string const& config)
{
  // Info settings
  InfoFile_ = infoFile;
  cmSystemTools::ConvertToUnixSlashes(InfoFile_);
  if (!InfoFileTime_.Load(InfoFile_)) {
    std::string msg = "AutoGen: The info file ";
    msg += Quoted(InfoFile_);
    msg += " is not readable\n";
    cmSystemTools::Stderr(msg);
    return false;
  }
  InfoDir_ = cmSystemTools::GetFilenamePath(infoFile);
  InfoConfig_ = config;

  bool success = false;
  {
    cmake cm(cmake::RoleScript, cmState::Unknown);
    cm.SetHomeOutputDirectory(InfoDir());
    cm.SetHomeDirectory(InfoDir());
    cm.GetCurrentSnapshot().SetDefaultDefinitions();
    cmGlobalGenerator gg(&cm);

    cmStateSnapshot snapshot = cm.GetCurrentSnapshot();
    snapshot.GetDirectory().SetCurrentBinary(InfoDir());
    snapshot.GetDirectory().SetCurrentSource(InfoDir());

    auto makefile = cm::make_unique<cmMakefile>(&gg, snapshot);
    // The OLD/WARN behavior for policy CMP0053 caused a speed regression.
    // https://gitlab.kitware.com/cmake/cmake/issues/17570
    makefile->SetPolicyVersion("3.9", std::string());
    gg.SetCurrentMakefile(makefile.get());
    success = this->Init(makefile.get());
  }
  if (success) {
    success = this->Process();
  }
  return success;
}

std::string cmQtAutoGenerator::SettingsFind(std::string const& content,
                                            const char* key)
{
  std::string prefix(key);
  prefix += ':';
  std::string::size_type pos = content.find(prefix);
  if (pos != std::string::npos) {
    pos += prefix.size();
    if (pos < content.size()) {
      std::string::size_type posE = content.find('\n', pos);
      if ((posE != std::string::npos) && (posE != pos)) {
        return content.substr(pos, posE - pos);
      }
    }
  }
  return std::string();
}
