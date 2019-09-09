/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGenerator.h"

#include "cm_memory.hxx"
#include "cmsys/FStream.hxx"

#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmQtAutoGen.h"
#include "cmState.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

cmQtAutoGenerator::Logger::Logger()
{
  // Initialize logger
  {
    std::string verbose;
    if (cmSystemTools::GetEnv("VERBOSE", verbose) && !verbose.empty()) {
      unsigned long iVerbose = 0;
      if (cmStrToULong(verbose, &iVerbose)) {
        SetVerbosity(static_cast<unsigned int>(iVerbose));
      } else {
        // Non numeric verbosity
        SetVerbose(cmIsOn(verbose));
      }
    }
  }
  {
    std::string colorEnv;
    cmSystemTools::GetEnv("COLOR", colorEnv);
    if (!colorEnv.empty()) {
      SetColorOutput(cmIsOn(colorEnv));
    } else {
      SetColorOutput(true);
    }
  }
}

cmQtAutoGenerator::Logger::~Logger() = default;

void cmQtAutoGenerator::Logger::RaiseVerbosity(std::string const& value)
{
  unsigned long verbosity = 0;
  if (cmStrToULong(value, &verbosity)) {
    if (this->Verbosity_ < verbosity) {
      this->Verbosity_ = static_cast<unsigned int>(verbosity);
    }
  }
}

void cmQtAutoGenerator::Logger::SetColorOutput(bool value)
{
  ColorOutput_ = value;
}

std::string cmQtAutoGenerator::Logger::HeadLine(cm::string_view title)
{
  return cmStrCat(title, '\n', std::string(title.size(), '-'), '\n');
}

void cmQtAutoGenerator::Logger::Info(GenT genType,
                                     cm::string_view message) const
{
  std::string msg = cmStrCat(GeneratorName(genType), ": ", message,
                             cmHasSuffix(message, '\n') ? "" : "\n");
  {
    std::lock_guard<std::mutex> lock(Mutex_);
    cmSystemTools::Stdout(msg);
  }
}

void cmQtAutoGenerator::Logger::Warning(GenT genType,
                                        cm::string_view message) const
{
  std::string msg;
  if (message.find('\n') == std::string::npos) {
    // Single line message
    msg = cmStrCat(GeneratorName(genType), " warning: ", message,
                   cmHasSuffix(message, '\n') ? "\n" : "\n\n");
  } else {
    // Multi line message
    msg = cmStrCat(HeadLine(cmStrCat(GeneratorName(genType), " warning")),
                   message, cmHasSuffix(message, '\n') ? "\n" : "\n\n");
  }
  {
    std::lock_guard<std::mutex> lock(Mutex_);
    cmSystemTools::Stdout(msg);
  }
}

void cmQtAutoGenerator::Logger::Error(GenT genType,
                                      cm::string_view message) const
{
  std::string msg =
    cmStrCat('\n', HeadLine(cmStrCat(GeneratorName(genType), " error")),
             message, cmHasSuffix(message, '\n') ? "\n" : "\n\n");
  {
    std::lock_guard<std::mutex> lock(Mutex_);
    cmSystemTools::Stderr(msg);
  }
}

void cmQtAutoGenerator::Logger::ErrorCommand(
  GenT genType, cm::string_view message,
  std::vector<std::string> const& command, std::string const& output) const
{
  std::string msg = cmStrCat(
    '\n', HeadLine(cmStrCat(GeneratorName(genType), " subprocess error")),
    message, cmHasSuffix(message, '\n') ? "\n" : "\n\n");
  msg += cmStrCat(HeadLine("Command"), QuotedCommand(command), "\n\n");
  msg += cmStrCat(HeadLine("Output"), output,
                  cmHasSuffix(output, '\n') ? "\n" : "\n\n");
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
    using IsIt = std::istreambuf_iterator<char>;
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
    cmSystemTools::Stderr(cmStrCat("AutoGen: The info file ",
                                   Quoted(InfoFile_), " is not readable\n"));
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
  std::string prefix = cmStrCat(key, ':');
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

std::string cmQtAutoGenerator::MessagePath(cm::string_view path) const
{
  std::string res;
  if (cmHasPrefix(path, ProjectDirs().Source)) {
    res = cmStrCat("SRC:", path.substr(ProjectDirs().Source.size()));
  } else if (cmHasPrefix(path, ProjectDirs().Binary)) {
    res = cmStrCat("BIN:", path.substr(ProjectDirs().Binary.size()));
  } else {
    res = std::string(path);
  }
  return cmQtAutoGen::Quoted(res);
}
