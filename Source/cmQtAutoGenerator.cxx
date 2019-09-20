/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGenerator.h"

#include "cmQtAutoGen.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cm_jsoncpp_reader.h"
#include "cmsys/FStream.hxx"

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

void cmQtAutoGenerator::Logger::RaiseVerbosity(unsigned int value)
{
  if (this->Verbosity_ < value) {
    this->Verbosity_ = value;
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

cmQtAutoGenerator::cmQtAutoGenerator(GenT genType)
  : GenType_(genType)
{
}

cmQtAutoGenerator::~cmQtAutoGenerator() = default;

bool cmQtAutoGenerator::Run(std::string const& infoFile,
                            std::string const& config)
{
  // Info settings
  InfoFile_ = infoFile;
  cmSystemTools::CollapseFullPath(InfoFile_);
  if (!InfoFileTime_.Load(InfoFile_)) {
    cmSystemTools::Stderr(cmStrCat("AutoGen: The info file ",
                                   Quoted(InfoFile_), " is not readable\n"));
    return false;
  }
  InfoDir_ = cmSystemTools::GetFilenamePath(infoFile);
  InfoConfig_ = config;

  // Read info file
  {
    cmsys::ifstream ifs(InfoFile_.c_str(), (std::ios::in | std::ios::binary));
    if (!ifs) {
      Log().Error(GenType_,
                  cmStrCat("Could not to open info file ", Quoted(InfoFile_)));
      return false;
    }
    try {
      ifs >> Info_;
    } catch (...) {
      Log().Error(GenType_,
                  cmStrCat("Could not read info file ", Quoted(InfoFile_)));
      return false;
    }
  }
  // Info: setup logger
  {
    unsigned int value = 0;
    if (!InfoUInt("VERBOSITY", value, false)) {
      return false;
    }
    Logger_.RaiseVerbosity(value);
  }
  // Info: setup project directories
  if (!InfoString("CMAKE_SOURCE_DIR", ProjectDirs_.Source, true) ||
      !InfoString("CMAKE_BINARY_DIR", ProjectDirs_.Binary, true) ||
      !InfoString("CMAKE_CURRENT_SOURCE_DIR", ProjectDirs_.CurrentSource,
                  true) ||
      !InfoString("CMAKE_CURRENT_BINARY_DIR", ProjectDirs_.CurrentBinary,
                  true)) {
    return false;
  }

  if (!this->InitFromInfo()) {
    return false;
  }
  // Clear info
  Info_ = Json::nullValue;

  return this->Process();
}

bool cmQtAutoGenerator::LogInfoError(GenT genType,
                                     cm::string_view message) const
{
  this->Log().Error(
    genType,
    cmStrCat("Info error in info file\n", Quoted(InfoFile()), ":\n", message));
  return false;
}

bool cmQtAutoGenerator::LogInfoError(cm::string_view message) const
{
  return LogInfoError(GenType_, message);
}

bool cmQtAutoGenerator::JsonGetArray(std::vector<std::string>& list,
                                     Json::Value const& jval)
{
  Json::ArrayIndex const arraySize = jval.size();
  if (arraySize == 0) {
    return false;
  }

  bool picked = false;
  list.reserve(list.size() + arraySize);
  for (Json::ArrayIndex ii = 0; ii != arraySize; ++ii) {
    Json::Value const& ival = jval[ii];
    if (ival.isString()) {
      list.emplace_back(ival.asString());
      picked = true;
    }
  }
  return picked;
}

bool cmQtAutoGenerator::JsonGetArray(std::unordered_set<std::string>& list,
                                     Json::Value const& jval)
{
  Json::ArrayIndex const arraySize = jval.size();
  if (arraySize == 0) {
    return false;
  }

  bool picked = false;
  list.reserve(list.size() + arraySize);
  for (Json::ArrayIndex ii = 0; ii != arraySize; ++ii) {
    Json::Value const& ival = jval[ii];
    if (ival.isString()) {
      list.emplace(ival.asString());
      picked = true;
    }
  }
  return picked;
}

std::string cmQtAutoGenerator::InfoConfigKey(std::string const& key) const
{
  return cmStrCat(key, '_', InfoConfig());
}

bool cmQtAutoGenerator::InfoString(std::string const& key, std::string& value,
                                   bool required) const
{
  Json::Value const& jval = Info()[key];
  if (!jval.isString()) {
    if (!jval.isNull() || required) {
      return LogInfoError(cmStrCat(key, " is not a string."));
    }
  } else {
    value = jval.asString();
    if (value.empty() && required) {
      return LogInfoError(cmStrCat(key, " is empty."));
    }
  }
  return true;
}

bool cmQtAutoGenerator::InfoStringConfig(std::string const& key,
                                         std::string& value,

                                         bool required) const
{
  { // Try config
    std::string const configKey = InfoConfigKey(key);
    Json::Value const& jval = Info_[configKey];
    if (!jval.isNull()) {
      if (!jval.isString()) {
        return LogInfoError(cmStrCat(configKey, " is not a string."));
      }
      value = jval.asString();
      if (required && value.empty()) {
        return LogInfoError(cmStrCat(configKey, " is empty."));
      }
      return true;
    }
  }
  // Try plain
  return InfoString(key, value, required);
}

bool cmQtAutoGenerator::InfoBool(std::string const& key, bool& value,
                                 bool required) const
{
  Json::Value const& jval = Info()[key];
  if (jval.isBool()) {
    value = jval.asBool();
  } else {
    if (!jval.isNull() || required) {
      return LogInfoError(cmStrCat(key, " is not a boolean."));
    }
  }
  return true;
}

bool cmQtAutoGenerator::InfoUInt(std::string const& key, unsigned int& value,
                                 bool required) const
{
  Json::Value const& jval = Info()[key];
  if (jval.isUInt()) {
    value = jval.asUInt();
  } else {
    if (!jval.isNull() || required) {
      return LogInfoError(cmStrCat(key, " is not an unsigned integer."));
    }
  }
  return true;
}

bool cmQtAutoGenerator::InfoArray(std::string const& key,
                                  std::vector<std::string>& list,
                                  bool required) const
{
  Json::Value const& jval = Info()[key];
  if (!jval.isArray()) {
    if (!jval.isNull() || required) {
      return LogInfoError(cmStrCat(key, " is not an array."));
    }
  }
  return JsonGetArray(list, jval) || !required;
}

bool cmQtAutoGenerator::InfoArray(std::string const& key,
                                  std::unordered_set<std::string>& list,
                                  bool required) const
{
  Json::Value const& jval = Info()[key];
  if (!jval.isArray()) {
    if (!jval.isNull() || required) {
      return LogInfoError(cmStrCat(key, " is not an array."));
    }
  }
  return JsonGetArray(list, jval) || !required;
}

bool cmQtAutoGenerator::InfoArrayConfig(std::string const& key,
                                        std::vector<std::string>& list,
                                        bool required) const
{
  { // Try config
    std::string const configKey = InfoConfigKey(key);
    Json::Value const& jval = Info()[configKey];
    if (!jval.isNull()) {
      if (!jval.isArray()) {
        return LogInfoError(cmStrCat(configKey, " is not an array string."));
      }
      if (!JsonGetArray(list, jval) && required) {
        return LogInfoError(cmStrCat(configKey, " is empty."));
      }
      return true;
    }
  }
  // Try plain
  return InfoArray(key, list, required);
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
