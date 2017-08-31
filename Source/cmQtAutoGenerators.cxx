/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGen.h"
#include "cmQtAutoGenerators.h"

#include "cmsys/FStream.hxx"
#include "cmsys/Terminal.h"
#include <algorithm>
#include <array>
#include <list>
#include <sstream>
#include <string.h>
#include <utility>

#include "cmAlgorithms.h"
#include "cmCryptoHash.h"
#include "cmFilePathChecksum.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmOutputConverter.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmSystemTools.h"
#include "cm_auto_ptr.hxx"
#include "cmake.h"

#if defined(__APPLE__)
#include <unistd.h>
#endif

// -- Static variables

static const char* SettingsKeyMoc = "AM_MOC_SETTINGS_HASH";
static const char* SettingsKeyUic = "AM_UIC_SETTINGS_HASH";
static const char* SettingsKeyRcc = "AM_RCC_SETTINGS_HASH";

// -- Static functions

inline static std::string Quoted(const std::string& text)
{
  return cmQtAutoGen::Quoted(text);
}

static std::string QuotedCommand(const std::vector<std::string>& command)
{
  std::string res;
  for (const std::string& item : command) {
    if (!res.empty()) {
      res.push_back(' ');
    }
    const std::string cesc = Quoted(item);
    if (item.empty() || (cesc.size() > (item.size() + 2)) ||
        (cesc.find(' ') != std::string::npos)) {
      res += cesc;
    } else {
      res += item;
    }
  }
  return res;
}

static void InfoGet(cmMakefile* makefile, const char* key, std::string& value)
{
  value = makefile->GetSafeDefinition(key);
}

static void InfoGet(cmMakefile* makefile, const char* key, bool& value)
{
  value = makefile->IsOn(key);
}

static void InfoGet(cmMakefile* makefile, const char* key,
                    std::vector<std::string>& list)
{
  cmSystemTools::ExpandListArgument(makefile->GetSafeDefinition(key), list);
}

static std::vector<std::string> InfoGetList(cmMakefile* makefile,
                                            const char* key)
{
  std::vector<std::string> list;
  cmSystemTools::ExpandListArgument(makefile->GetSafeDefinition(key), list);
  return list;
}

static std::vector<std::vector<std::string>> InfoGetLists(cmMakefile* makefile,
                                                          const char* key)
{
  std::vector<std::vector<std::string>> lists;
  {
    const std::string value = makefile->GetSafeDefinition(key);
    std::string::size_type pos = 0;
    while (pos < value.size()) {
      std::string::size_type next = value.find(cmQtAutoGen::listSep, pos);
      std::string::size_type length =
        (next != std::string::npos) ? next - pos : value.size() - pos;
      // Remove enclosing braces
      if (length >= 2) {
        std::string::const_iterator itBeg = value.begin() + (pos + 1);
        std::string::const_iterator itEnd = itBeg + (length - 2);
        {
          std::string subValue(itBeg, itEnd);
          std::vector<std::string> list;
          cmSystemTools::ExpandListArgument(subValue, list);
          lists.push_back(std::move(list));
        }
      }
      pos += length;
      pos += cmQtAutoGen::listSep.size();
    }
  }
  return lists;
}

static void InfoGetConfig(cmMakefile* makefile, const char* key,
                          const std::string& config, std::string& value)
{
  const char* valueConf = nullptr;
  {
    std::string keyConf = key;
    if (!config.empty()) {
      keyConf += "_";
      keyConf += config;
    }
    valueConf = makefile->GetDefinition(keyConf);
  }
  if (valueConf == nullptr) {
    valueConf = makefile->GetSafeDefinition(key);
  }
  value = valueConf;
}

static void InfoGetConfig(cmMakefile* makefile, const char* key,
                          const std::string& config,
                          std::vector<std::string>& list)
{
  std::string value;
  InfoGetConfig(makefile, key, config, value);
  cmSystemTools::ExpandListArgument(value, list);
}

inline static bool SettingsMatch(cmMakefile* makefile, const char* key,
                                 const std::string& value)
{
  return (value == makefile->GetSafeDefinition(key));
}

static void SettingAppend(std::string& str, const char* key,
                          const std::string& value)
{
  if (!value.empty()) {
    str += "set(";
    str += key;
    str += " ";
    str += cmOutputConverter::EscapeForCMake(value);
    str += ")\n";
  }
}

static std::string SubDirPrefix(const std::string& fileName)
{
  std::string res(cmSystemTools::GetFilenamePath(fileName));
  if (!res.empty()) {
    res += '/';
  }
  return res;
}

static bool FileNameIsUnique(const std::string& filePath,
                             const std::map<std::string, std::string>& fileMap)
{
  size_t count(0);
  const std::string fileName = cmSystemTools::GetFilenameName(filePath);
  for (const auto& item : fileMap) {
    if (cmSystemTools::GetFilenameName(item.first) == fileName) {
      ++count;
      if (count > 1) {
        return false;
      }
    }
  }
  return true;
}

static bool ReadAll(std::string& content, const std::string& filename)
{
  bool success = false;
  {
    cmsys::ifstream ifs(filename.c_str());
    if (ifs) {
      std::ostringstream osst;
      osst << ifs.rdbuf();
      content = osst.str();
      success = true;
    }
  }
  return success;
}

/**
 * @brief Tests if buildFile doesn't exist or is older than sourceFile
 * @return True if buildFile doesn't exist or is older than sourceFile
 */
static bool FileAbsentOrOlder(const std::string& buildFile,
                              const std::string& sourceFile)
{
  int result = 0;
  bool success =
    cmSystemTools::FileTimeCompare(buildFile, sourceFile, &result);
  return (!success || (result <= 0));
}

static bool ListContains(const std::vector<std::string>& list,
                         const std::string& entry)
{
  return (std::find(list.begin(), list.end(), entry) != list.end());
}

static std::string JoinOptionsList(const std::vector<std::string>& opts)
{
  return cmOutputConverter::EscapeForCMake(cmJoin(opts, ";"));
}

static std::string JoinOptionsMap(
  const std::map<std::string, std::string>& opts)
{
  std::string result;
  for (const auto& item : opts) {
    if (!result.empty()) {
      result += cmQtAutoGen::listSep;
    }
    result += item.first;
    result += "===";
    result += item.second;
  }
  return result;
}

// -- Class methods

cmQtAutoGenerators::cmQtAutoGenerators()
  : Verbose(cmSystemTools::HasEnv("VERBOSE"))
  , ColorOutput(true)
  , MocSettingsChanged(false)
  , MocPredefsChanged(false)
  , MocRunFailed(false)
  , UicSettingsChanged(false)
  , UicRunFailed(false)
  , RccSettingsChanged(false)
  , RccRunFailed(false)
{

  std::string colorEnv;
  cmSystemTools::GetEnv("COLOR", colorEnv);
  if (!colorEnv.empty()) {
    if (cmSystemTools::IsOn(colorEnv.c_str())) {
      this->ColorOutput = true;
    } else {
      this->ColorOutput = false;
    }
  }

  // Moc macro filters
  this->MocMacroFilters.emplace_back(
    "Q_OBJECT", "[\n][ \t]*{?[ \t]*Q_OBJECT[^a-zA-Z0-9_]");
  this->MocMacroFilters.emplace_back(
    "Q_GADGET", "[\n][ \t]*{?[ \t]*Q_GADGET[^a-zA-Z0-9_]");

  // Precompile regular expressions
  this->MocRegExpInclude.compile(
    "[\n][ \t]*#[ \t]*include[ \t]+"
    "[\"<](([^ \">]+/)?moc_[^ \">/]+\\.cpp|[^ \">]+\\.moc)[\">]");
  this->UicRegExpInclude.compile("[\n][ \t]*#[ \t]*include[ \t]+"
                                 "[\"<](([^ \">]+/)?ui_[^ \">/]+\\.h)[\">]");
}

bool cmQtAutoGenerators::Run(const std::string& targetDirectory,
                             const std::string& config)
{
  cmake cm(cmake::RoleScript);
  cm.SetHomeOutputDirectory(targetDirectory);
  cm.SetHomeDirectory(targetDirectory);
  cm.GetCurrentSnapshot().SetDefaultDefinitions();
  cmGlobalGenerator gg(&cm);

  cmStateSnapshot snapshot = cm.GetCurrentSnapshot();
  snapshot.GetDirectory().SetCurrentBinary(targetDirectory);
  snapshot.GetDirectory().SetCurrentSource(targetDirectory);

  CM_AUTO_PTR<cmMakefile> mf(new cmMakefile(&gg, snapshot));
  gg.SetCurrentMakefile(mf.get());

  bool success = false;
  if (this->ReadAutogenInfoFile(mf.get(), targetDirectory, config)) {
    // Read old settings
    this->SettingsFileRead(mf.get());
    // Init and run
    this->Init(mf.get());
    if (this->RunAutogen()) {
      // Write current settings
      if (this->SettingsFileWrite()) {
        success = true;
      }
    }
  }
  return success;
}

bool cmQtAutoGenerators::MocDependFilterPush(const std::string& key,
                                             const std::string& regExp)
{
  bool success = false;
  if (!key.empty()) {
    if (!regExp.empty()) {
      KeyRegExp filter;
      filter.Key = key;
      if (filter.RegExp.compile(regExp)) {
        this->MocDependFilters.push_back(std::move(filter));
        success = true;
      } else {
        this->LogError("AutoMoc: Error in AUTOMOC_DEPEND_FILTERS: Compiling "
                       "regular expression failed.\n  Key: " +
                       Quoted(key) + "\n  RegExp.: " + Quoted(regExp));
      }
    } else {
      this->LogError("AutoMoc: Error in AUTOMOC_DEPEND_FILTERS: Regular "
                     "expression is empty");
    }
  } else {
    this->LogError("AutoMoc: Error in AUTOMOC_DEPEND_FILTERS: Key is empty");
  }
  return success;
}

bool cmQtAutoGenerators::ReadAutogenInfoFile(
  cmMakefile* makefile, const std::string& targetDirectory,
  const std::string& config)
{
  std::string filename(cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutogenInfo.cmake";

  if (!makefile->ReadListFile(filename.c_str())) {
    this->LogError("AutoGen: Error processing file: " + filename);
    return false;
  }

  // -- Meta
  InfoGetConfig(makefile, "AM_CONFIG_SUFFIX", config, this->ConfigSuffix);

  // - Old settings file
  {
    this->SettingsFile = cmSystemTools::CollapseFullPath(targetDirectory);
    cmSystemTools::ConvertToUnixSlashes(this->SettingsFile);
    this->SettingsFile += "/AutogenOldSettings";
    this->SettingsFile += this->ConfigSuffix;
    this->SettingsFile += ".cmake";
  }

  // - Files and directories
  InfoGet(makefile, "AM_CMAKE_SOURCE_DIR", this->ProjectSourceDir);
  InfoGet(makefile, "AM_CMAKE_BINARY_DIR", this->ProjectBinaryDir);
  InfoGet(makefile, "AM_CMAKE_CURRENT_SOURCE_DIR", this->CurrentSourceDir);
  InfoGet(makefile, "AM_CMAKE_CURRENT_BINARY_DIR", this->CurrentBinaryDir);
  InfoGet(makefile, "AM_CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE",
          this->IncludeProjectDirsBefore);
  InfoGet(makefile, "AM_BUILD_DIR", this->AutogenBuildDir);
  if (this->AutogenBuildDir.empty()) {
    this->LogError("AutoGen: Error: Missing autogen build directory ");
    return false;
  }
  InfoGet(makefile, "AM_SOURCES", this->Sources);
  InfoGet(makefile, "AM_HEADERS", this->Headers);

  // - Qt environment
  InfoGet(makefile, "AM_QT_VERSION_MAJOR", this->QtMajorVersion);
  if (this->QtMajorVersion.empty()) {
    InfoGet(makefile, "AM_Qt5Core_VERSION_MAJOR", this->QtMajorVersion);
  }
  InfoGet(makefile, "AM_QT_MOC_EXECUTABLE", this->MocExecutable);
  InfoGet(makefile, "AM_QT_UIC_EXECUTABLE", this->UicExecutable);
  InfoGet(makefile, "AM_QT_RCC_EXECUTABLE", this->RccExecutable);

  // Check Qt version
  if ((this->QtMajorVersion != "4") && (this->QtMajorVersion != "5")) {
    this->LogError("AutoGen: Error: Unsupported Qt version: " +
                   Quoted(this->QtMajorVersion));
    return false;
  }

  // - Moc
  if (this->MocEnabled()) {
    InfoGet(makefile, "AM_MOC_SKIP", this->MocSkipList);
    InfoGetConfig(makefile, "AM_MOC_DEFINITIONS", config,
                  this->MocDefinitions);
#ifdef _WIN32
    {
      const std::string win32("WIN32");
      if (!ListContains(this->MocDefinitions, win32)) {
        this->MocDefinitions.push_back(win32);
      }
    }
#endif
    InfoGetConfig(makefile, "AM_MOC_INCLUDES", config, this->MocIncludePaths);
    InfoGet(makefile, "AM_MOC_OPTIONS", this->MocOptions);
    InfoGet(makefile, "AM_MOC_RELAXED_MODE", this->MocRelaxedMode);
    {
      std::vector<std::string> MocMacroNames;
      InfoGet(makefile, "AM_MOC_MACRO_NAMES", MocMacroNames);
      for (const std::string& item : MocMacroNames) {
        this->MocMacroFilters.emplace_back(
          item, ("[^a-zA-Z0-9_]" + item).append("[^a-zA-Z0-9_]"));
      }
    }
    {
      std::vector<std::string> mocDependFilters;
      InfoGet(makefile, "AM_MOC_DEPEND_FILTERS", mocDependFilters);
      // Insert Q_PLUGIN_METADATA dependency filter
      if (this->QtMajorVersion != "4") {
        this->MocDependFilterPush("Q_PLUGIN_METADATA",
                                  "[\n][ \t]*Q_PLUGIN_METADATA[ \t]*\\("
                                  "[^\\)]*FILE[ \t]*\"([^\"]+)\"");
      }
      // Insert user defined dependency filters
      if ((mocDependFilters.size() % 2) == 0) {
        for (std::vector<std::string>::const_iterator
               dit = mocDependFilters.begin(),
               ditEnd = mocDependFilters.end();
             dit != ditEnd; dit += 2) {
          if (!this->MocDependFilterPush(*dit, *(dit + 1))) {
            return false;
          }
        }
      } else {
        this->LogError(
          "AutoMoc: Error: AUTOMOC_DEPEND_FILTERS list size is not "
          "a multiple of 2 in:\n" +
          Quoted(filename));
        return false;
      }
    }
    InfoGet(makefile, "AM_MOC_PREDEFS_CMD", this->MocPredefsCmd);
  }

  // - Uic
  if (this->UicEnabled()) {
    InfoGet(makefile, "AM_UIC_SKIP", this->UicSkipList);
    InfoGet(makefile, "AM_UIC_SEARCH_PATHS", this->UicSearchPaths);
    InfoGetConfig(makefile, "AM_UIC_TARGET_OPTIONS", config,
                  this->UicTargetOptions);
    {
      std::vector<std::string> uicFilesVec;
      std::vector<std::string> uicOptionsVec;
      InfoGet(makefile, "AM_UIC_OPTIONS_FILES", uicFilesVec);
      InfoGet(makefile, "AM_UIC_OPTIONS_OPTIONS", uicOptionsVec);
      // Compare list sizes
      if (uicFilesVec.size() == uicOptionsVec.size()) {
        for (std::vector<std::string>::iterator
               fileIt = uicFilesVec.begin(),
               optionIt = uicOptionsVec.begin();
             fileIt != uicFilesVec.end(); ++fileIt, ++optionIt) {
          cmSystemTools::ReplaceString(*optionIt, cmQtAutoGen::listSep, ";");
          this->UicOptions[*fileIt] = *optionIt;
        }
      } else {
        this->LogError(
          "AutoGen: Error: Uic files/options lists size missmatch in:\n" +
          Quoted(filename));
        return false;
      }
    }
  }

  // - Rcc
  if (this->RccEnabled()) {
    InfoGet(makefile, "AM_RCC_SOURCES", this->RccSources);
    // File options
    {
      std::vector<std::string> rccFilesVec;
      std::vector<std::string> rccOptionsVec;
      InfoGet(makefile, "AM_RCC_OPTIONS_FILES", rccFilesVec);
      InfoGet(makefile, "AM_RCC_OPTIONS_OPTIONS", rccOptionsVec);
      if (rccFilesVec.size() == rccOptionsVec.size()) {
        for (std::vector<std::string>::iterator
               fileIt = rccFilesVec.begin(),
               optionIt = rccOptionsVec.begin();
             fileIt != rccFilesVec.end(); ++fileIt, ++optionIt) {
          // Replace item separator
          cmSystemTools::ReplaceString(*optionIt, cmQtAutoGen::listSep, ";");
          this->RccOptions[*fileIt] = *optionIt;
        }
      } else {
        this->LogError(
          "AutoGen: Error: RCC files/options lists size missmatch in:\n" +
          Quoted(filename));
        return false;
      }
    }
    // File lists
    {
      std::vector<std::string> rccInputLists;
      InfoGet(makefile, "AM_RCC_INPUTS", rccInputLists);
      if (this->RccSources.size() == rccInputLists.size()) {
        for (std::vector<std::string>::iterator
               fileIt = this->RccSources.begin(),
               inputIt = rccInputLists.begin();
             fileIt != this->RccSources.end(); ++fileIt, ++inputIt) {
          // Remove braces
          *inputIt = inputIt->substr(1, inputIt->size() - 2);
          // Replace item separator
          cmSystemTools::ReplaceString(*inputIt, cmQtAutoGen::listSep, ";");
          std::vector<std::string> rccInputFiles;
          cmSystemTools::ExpandListArgument(*inputIt, rccInputFiles);
          this->RccInputs[*fileIt] = rccInputFiles;
        }
      } else {
        this->LogError(
          "AutoGen: Error: RCC sources/inputs lists size missmatch in:\n" +
          Quoted(filename));
        return false;
      }
    }
  }

  return true;
}

void cmQtAutoGenerators::SettingsFileRead(cmMakefile* makefile)
{
  // Compose current settings strings
  {
    cmCryptoHash crypt(cmCryptoHash::AlgoSHA256);
    const std::string sep(" ~~~ ");
    if (this->MocEnabled()) {
      std::string str;
      str += this->MocExecutable;
      str += sep;
      str += JoinOptionsList(this->MocDefinitions);
      str += sep;
      str += JoinOptionsList(this->MocIncludePaths);
      str += sep;
      str += JoinOptionsList(this->MocOptions);
      str += sep;
      str += this->IncludeProjectDirsBefore ? "TRUE" : "FALSE";
      str += sep;
      str += JoinOptionsList(this->MocPredefsCmd);
      str += sep;
      this->SettingsStringMoc = crypt.HashString(str);
    }
    if (this->UicEnabled()) {
      std::string str;
      str += this->UicExecutable;
      str += sep;
      str += JoinOptionsList(this->UicTargetOptions);
      str += sep;
      str += JoinOptionsMap(this->UicOptions);
      str += sep;
      this->SettingsStringUic = crypt.HashString(str);
    }
    if (this->RccEnabled()) {
      std::string str;
      str += this->RccExecutable;
      str += sep;
      str += JoinOptionsMap(this->RccOptions);
      str += sep;
      this->SettingsStringRcc = crypt.HashString(str);
    }
  }

  // Read old settings
  if (makefile->ReadListFile(this->SettingsFile.c_str())) {
    if (!SettingsMatch(makefile, SettingsKeyMoc, this->SettingsStringMoc)) {
      this->MocSettingsChanged = true;
    }
    if (!SettingsMatch(makefile, SettingsKeyUic, this->SettingsStringUic)) {
      this->UicSettingsChanged = true;
    }
    if (!SettingsMatch(makefile, SettingsKeyRcc, this->SettingsStringRcc)) {
      this->RccSettingsChanged = true;
    }
    // In case any setting changed remove the old settings file.
    // This triggers a full rebuild on the next run if the current
    // build is aborted before writing the current settings in the end.
    if (this->AnySettingsChanged()) {
      cmSystemTools::RemoveFile(this->SettingsFile);
    }
  } else {
    // If the file could not be read re-generate everythiung.
    this->MocSettingsChanged = true;
    this->UicSettingsChanged = true;
    this->RccSettingsChanged = true;
  }
}

bool cmQtAutoGenerators::SettingsFileWrite()
{
  bool success = true;
  // Only write if any setting changed
  if (this->AnySettingsChanged()) {
    if (this->Verbose) {
      this->LogInfo("AutoGen: Writing settings file " +
                    Quoted(this->SettingsFile));
    }
    // Compose settings file content
    std::string settings;
    SettingAppend(settings, SettingsKeyMoc, this->SettingsStringMoc);
    SettingAppend(settings, SettingsKeyUic, this->SettingsStringUic);
    SettingAppend(settings, SettingsKeyRcc, this->SettingsStringRcc);
    // Write settings file
    if (!this->FileWrite(cmQtAutoGen::GEN, this->SettingsFile, settings)) {
      this->LogError("AutoGen: Error: Could not write old settings file " +
                     Quoted(this->SettingsFile));
      // Remove old settings file to trigger a full rebuild on the next run
      cmSystemTools::RemoveFile(this->SettingsFile);
      success = false;
    }
  }
  return success;
}

void cmQtAutoGenerators::Init(cmMakefile* makefile)
{
  // Mocs compilation file
  this->MocCompFileRel = "mocs_compilation.cpp";
  this->MocCompFileAbs = cmSystemTools::CollapseCombinedPath(
    this->AutogenBuildDir, this->MocCompFileRel);

  // Mocs include directory
  this->AutogenIncludeDir = "include";
  this->AutogenIncludeDir += this->ConfigSuffix;
  this->AutogenIncludeDir += "/";

  // Moc predefs file
  if (!this->MocPredefsCmd.empty()) {
    this->MocPredefsFileRel = "moc_predefs";
    this->MocPredefsFileRel += this->ConfigSuffix;
    this->MocPredefsFileRel += ".h";
    this->MocPredefsFileAbs = cmSystemTools::CollapseCombinedPath(
      this->AutogenBuildDir, this->MocPredefsFileRel);
  }

  // Init file path checksum generator
  FPathChecksum.setupParentDirs(this->CurrentSourceDir, this->CurrentBinaryDir,
                                this->ProjectSourceDir,
                                this->ProjectBinaryDir);

  // Acquire header extensions
  this->HeaderExtensions = makefile->GetCMakeInstance()->GetHeaderExtensions();

  // Sort include directories on demand
  if (this->IncludeProjectDirsBefore) {
    // Move strings to temporary list
    std::list<std::string> includes;
    includes.insert(includes.end(), this->MocIncludePaths.begin(),
                    this->MocIncludePaths.end());
    this->MocIncludePaths.clear();
    this->MocIncludePaths.reserve(includes.size());
    // Append project directories only
    {
      const std::array<const std::string*, 2> movePaths = {
        { &this->ProjectBinaryDir, &this->ProjectSourceDir }
      };
      for (const std::string* ppath : movePaths) {
        std::list<std::string>::iterator it = includes.begin();
        while (it != includes.end()) {
          const std::string& path = *it;
          if (cmSystemTools::StringStartsWith(path, ppath->c_str())) {
            this->MocIncludePaths.push_back(path);
            it = includes.erase(it);
          } else {
            ++it;
          }
        }
      }
    }
    // Append remaining directories
    this->MocIncludePaths.insert(this->MocIncludePaths.end(), includes.begin(),
                                 includes.end());
  }
  // Compose moc includes list
  {
    std::set<std::string> frameworkPaths;
    for (const std::string& path : this->MocIncludePaths) {
      this->MocIncludes.push_back("-I" + path);
      // Extract framework path
      if (cmHasLiteralSuffix(path, ".framework/Headers")) {
        // Go up twice to get to the framework root
        std::vector<std::string> pathComponents;
        cmSystemTools::SplitPath(path, pathComponents);
        std::string frameworkPath = cmSystemTools::JoinPath(
          pathComponents.begin(), pathComponents.end() - 2);
        frameworkPaths.insert(frameworkPath);
      }
    }
    // Append framework includes
    for (const std::string& path : frameworkPaths) {
      this->MocIncludes.push_back("-F");
      this->MocIncludes.push_back(path);
    }
  }
}

bool cmQtAutoGenerators::RunAutogen()
{
  // the program goes through all .cpp files to see which moc files are
  // included. It is not really interesting how the moc file is named, but
  // what file the moc is created from. Once a moc is included the same moc
  // may not be included in the mocs_compilation.cpp file anymore.
  // OTOH if there's a header containing Q_OBJECT where no corresponding
  // moc file is included anywhere a moc_<filename>.cpp file is created and
  // included in the mocs_compilation.cpp file.

  // Create AUTOGEN include directory
  {
    const std::string incDirAbs = cmSystemTools::CollapseCombinedPath(
      this->AutogenBuildDir, this->AutogenIncludeDir);
    if (!cmSystemTools::MakeDirectory(incDirAbs)) {
      this->LogError("AutoGen: Error: Could not create include directory " +
                     Quoted(incDirAbs));
      return false;
    }
  }

  // key = moc source filepath, value = moc output filepath
  std::map<std::string, std::string> mocsIncluded;
  std::map<std::string, std::string> mocsNotIncluded;
  std::map<std::string, std::set<std::string>> mocDepends;
  std::map<std::string, std::vector<std::string>> uisIncluded;
  // collects all headers which may need to be mocced
  std::set<std::string> mocHeaderFiles;
  std::set<std::string> uicHeaderFiles;

  // Parse sources
  for (const std::string& src : this->Sources) {
    // Parse source file for MOC/UIC
    if (!this->ParseSourceFile(src, mocsIncluded, mocDepends, uisIncluded,
                               this->MocRelaxedMode)) {
      return false;
    }
    // Find additional headers
    this->SearchHeadersForSourceFile(src, mocHeaderFiles, uicHeaderFiles);
  }

  // Parse headers
  for (const std::string& hdr : this->Headers) {
    if (!this->MocSkip(hdr)) {
      mocHeaderFiles.insert(hdr);
    }
    if (!this->UicSkip(hdr)) {
      uicHeaderFiles.insert(hdr);
    }
  }
  if (!this->ParseHeaders(mocHeaderFiles, uicHeaderFiles, mocsIncluded,
                          mocsNotIncluded, mocDepends, uisIncluded)) {
    return false;
  };

  // Generate files
  if (!this->MocGenerateAll(mocsIncluded, mocsNotIncluded, mocDepends)) {
    return false;
  }
  if (!this->UicGenerateAll(uisIncluded)) {
    return false;
  }
  if (!this->RccGenerateAll()) {
    return false;
  }

  return true;
}

/**
 * @brief Tests if the C++ content requires moc processing
 * @return True if moc is required
 */
bool cmQtAutoGenerators::MocRequired(const std::string& contentText,
                                     std::string* macroName)
{
  for (KeyRegExp& filter : this->MocMacroFilters) {
    // Run a simple find string operation before the expensive
    // regular expression check
    if (contentText.find(filter.Key) != std::string::npos) {
      if (filter.RegExp.find(contentText)) {
        // Return macro name on demand
        if (macroName != nullptr) {
          *macroName = filter.Key;
        }
        return true;
      }
    }
  }
  return false;
}

void cmQtAutoGenerators::MocFindDepends(
  const std::string& absFilename, const std::string& contentText,
  std::map<std::string, std::set<std::string>>& mocDepends)
{
  for (KeyRegExp& filter : this->MocDependFilters) {
    // Run a simple find string operation before the expensive
    // regular expression check
    if (contentText.find(filter.Key) != std::string::npos) {
      // Run regular expression check loop
      const std::string sourcePath = SubDirPrefix(absFilename);
      const char* contentChars = contentText.c_str();
      while (filter.RegExp.find(contentChars)) {
        // Evaluate match
        const std::string match = filter.RegExp.match(1);
        if (!match.empty()) {
          // Find the dependency file
          std::string incFile;
          if (this->MocFindIncludedFile(incFile, sourcePath, match)) {
            mocDepends[absFilename].insert(incFile);
            if (this->Verbose) {
              this->LogInfo("AutoMoc: Found dependency:\n  " +
                            Quoted(absFilename) + "\n  " + Quoted(incFile));
            }
          } else {
            this->LogWarning("AutoMoc: Warning: " + Quoted(absFilename) +
                             "\n" + "Could not find dependency file " +
                             Quoted(match));
          }
        }
        contentChars += filter.RegExp.end();
      }
    }
  }
}

/**
 * @brief Tests if the file should be ignored for moc scanning
 * @return True if the file should be ignored
 */
bool cmQtAutoGenerators::MocSkip(const std::string& absFilename) const
{
  if (this->MocEnabled()) {
    // Test if the file name is on the skip list
    if (!ListContains(this->MocSkipList, absFilename)) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Tests if the file name is in the skip list
 */
bool cmQtAutoGenerators::UicSkip(const std::string& absFilename) const
{
  if (this->UicEnabled()) {
    // Test if the file name is on the skip list
    if (!ListContains(this->UicSkipList, absFilename)) {
      return false;
    }
  }
  return true;
}

/**
 * @return True on success
 */
bool cmQtAutoGenerators::ParseSourceFile(
  const std::string& absFilename,
  std::map<std::string, std::string>& mocsIncluded,
  std::map<std::string, std::set<std::string>>& mocDepends,
  std::map<std::string, std::vector<std::string>>& uisIncluded, bool relaxed)
{
  std::string contentText;
  bool success = ReadAll(contentText, absFilename);
  if (success) {
    if (!contentText.empty()) {
      // Parse source contents for MOC
      if (success && !this->MocSkip(absFilename)) {
        success = this->MocParseSourceContent(
          absFilename, contentText, mocsIncluded, mocDepends, relaxed);
      }
      // Parse source contents for UIC
      if (success && !this->UicSkip(absFilename)) {
        this->UicParseContent(absFilename, contentText, uisIncluded);
      }
    } else {
      std::ostringstream ost;
      ost << "AutoGen: Warning: The file is empty:\n"
          << Quoted(absFilename) << "\n";
      this->LogWarning(ost.str());
    }
  } else {
    std::ostringstream ost;
    ost << "AutoGen: Error: Could not read file:\n" << Quoted(absFilename);
    this->LogError(ost.str());
  }
  return success;
}

void cmQtAutoGenerators::UicParseContent(
  const std::string& absFilename, const std::string& contentText,
  std::map<std::string, std::vector<std::string>>& uisIncluded)
{
  if (this->Verbose) {
    this->LogInfo("AutoUic: Checking " + absFilename);
  }

  const char* contentChars = contentText.c_str();
  if (strstr(contentChars, "ui_") != nullptr) {
    while (this->UicRegExpInclude.find(contentChars)) {
      uisIncluded[absFilename].push_back(this->UicRegExpInclude.match(1));
      contentChars += this->UicRegExpInclude.end();
    }
  }
}

/**
 * @return True on success
 */
bool cmQtAutoGenerators::MocParseSourceContent(
  const std::string& absFilename, const std::string& contentText,
  std::map<std::string, std::string>& mocsIncluded,
  std::map<std::string, std::set<std::string>>& mocDepends, bool relaxed)
{
  if (this->Verbose) {
    this->LogInfo("AutoMoc: Checking " + absFilename);
  }

  const std::string scannedFileAbsPath = SubDirPrefix(absFilename);
  const std::string scannedFileBasename =
    cmSystemTools::GetFilenameWithoutLastExtension(absFilename);

  std::string macroName;
  const bool requiresMoc = this->MocRequired(contentText, &macroName);
  bool ownDotMocIncluded = false;
  std::string ownMocUnderscoreInclude;
  std::string ownMocUnderscoreHeader;

  // first a simple string check for "moc" is *much* faster than the regexp,
  // and if the string search already fails, we don't have to try the
  // expensive regexp
  const char* contentChars = contentText.c_str();
  if (strstr(contentChars, "moc") != nullptr) {
    // Iterate over all included moc files
    while (this->MocRegExpInclude.find(contentChars)) {
      const std::string incString = this->MocRegExpInclude.match(1);
      // Basename of the moc include
      const std::string incSubDir(SubDirPrefix(incString));
      const std::string incBasename =
        cmSystemTools::GetFilenameWithoutLastExtension(incString);

      // If the moc include is of the moc_foo.cpp style we expect
      // the Q_OBJECT class declaration in a header file.
      // If the moc include is of the foo.moc style we need to look for
      // a Q_OBJECT macro in the current source file, if it contains the
      // macro we generate the moc file from the source file.
      if (cmHasLiteralPrefix(incBasename, "moc_")) {
        // Include: moc_FOO.cxx
        // Remove the moc_ part
        const std::string incRealBasename = incBasename.substr(4);
        const std::string headerToMoc =
          this->MocFindHeader(scannedFileAbsPath, incSubDir + incRealBasename);
        if (!headerToMoc.empty()) {
          if (!this->MocSkip(headerToMoc)) {
            // Register moc job
            mocsIncluded[headerToMoc] = incString;
            this->MocFindDepends(headerToMoc, contentText, mocDepends);
            // Store meta information for relaxed mode
            if (relaxed && (incRealBasename == scannedFileBasename)) {
              ownMocUnderscoreInclude = incString;
              ownMocUnderscoreHeader = headerToMoc;
            }
          }
        } else {
          std::ostringstream ost;
          ost << "AutoMoc: Error: " << Quoted(absFilename) << "\n"
              << "The file includes the moc file " << Quoted(incString)
              << ", but could not find header "
              << Quoted(incRealBasename + ".{" +
                        cmJoin(this->HeaderExtensions, ",") + "}");
          this->LogError(ost.str());
          return false;
        }
      } else {
        // Include: FOO.moc
        std::string fileToMoc;
        if (relaxed) {
          // Mode: Relaxed
          if (requiresMoc && (incBasename == scannedFileBasename)) {
            // Include self
            fileToMoc = absFilename;
            ownDotMocIncluded = true;
          } else {
            // In relaxed mode try to find a header instead but issue a warning
            const std::string headerToMoc =
              this->MocFindHeader(scannedFileAbsPath, incSubDir + incBasename);
            if (!headerToMoc.empty()) {
              if (!this->MocSkip(headerToMoc)) {
                // This is for KDE4 compatibility:
                fileToMoc = headerToMoc;
                if (!requiresMoc && (incBasename == scannedFileBasename)) {
                  std::ostringstream ost;
                  ost
                    << "AutoMoc: Warning: " << Quoted(absFilename) << "\n"
                    << "The file includes the moc file " << Quoted(incString)
                    << ", but does not contain a Q_OBJECT or Q_GADGET macro.\n"
                    << "Running moc on " << Quoted(headerToMoc) << "!\n"
                    << "Include " << Quoted("moc_" + incBasename + ".cpp")
                    << " for a compatibility with strict mode (see "
                       "CMAKE_AUTOMOC_RELAXED_MODE).\n";
                  this->LogWarning(ost.str());
                } else {
                  std::ostringstream ost;
                  ost << "AutoMoc: Warning: " << Quoted(absFilename) << "\n"
                      << "The file includes the moc file " << Quoted(incString)
                      << " instead of "
                      << Quoted("moc_" + incBasename + ".cpp") << ".\n"
                      << "Running moc on " << Quoted(headerToMoc) << "!\n"
                      << "Include " << Quoted("moc_" + incBasename + ".cpp")
                      << " for compatibility with strict mode (see "
                         "CMAKE_AUTOMOC_RELAXED_MODE).\n";
                  this->LogWarning(ost.str());
                }
              }
            } else {
              std::ostringstream ost;
              ost << "AutoMoc: Error: " << Quoted(absFilename) << "\n"
                  << "The file includes the moc file " << Quoted(incString)
                  << ", which seems to be the moc file from a different "
                     "source file. CMake also could not find a matching "
                     "header.";
              this->LogError(ost.str());
              return false;
            }
          }
        } else {
          // Mode: Strict
          if (incBasename == scannedFileBasename) {
            // Include self
            fileToMoc = absFilename;
            ownDotMocIncluded = true;
            // Accept but issue a warning if moc isn't required
            if (!requiresMoc) {
              std::ostringstream ost;
              ost << "AutoMoc: Warning: " << Quoted(absFilename) << "\n"
                  << "The file includes the moc file " << Quoted(incString)
                  << ", but does not contain a Q_OBJECT or Q_GADGET "
                     "macro.";
              this->LogWarning(ost.str());
            }
          } else {
            // Don't allow FOO.moc include other than self in strict mode
            std::ostringstream ost;
            ost << "AutoMoc: Error: " << Quoted(absFilename) << "\n"
                << "The file includes the moc file " << Quoted(incString)
                << ", which seems to be the moc file from a different "
                   "source file. This is not supported. Include "
                << Quoted(scannedFileBasename + ".moc")
                << " to run moc on this source file.";
            this->LogError(ost.str());
            return false;
          }
        }
        if (!fileToMoc.empty()) {
          mocsIncluded[fileToMoc] = incString;
          this->MocFindDepends(fileToMoc, contentText, mocDepends);
        }
      }
      // Forward content pointer
      contentChars += this->MocRegExpInclude.end();
    }
  }

  if (requiresMoc && !ownDotMocIncluded) {
    // In this case, check whether the scanned file itself contains a Q_OBJECT.
    // If this is the case, the moc_foo.cpp should probably be generated from
    // foo.cpp instead of foo.h, because otherwise it won't build.
    // But warn, since this is not how it is supposed to be used.
    if (relaxed && !ownMocUnderscoreInclude.empty()) {
      // This is for KDE4 compatibility:
      std::ostringstream ost;
      ost << "AutoMoc: Warning: " << Quoted(absFilename) << "\n"
          << "The file contains a " << macroName
          << " macro, but does not include "
          << Quoted(scannedFileBasename + ".moc") << ", but instead includes "
          << Quoted(ownMocUnderscoreInclude) << ".\n"
          << "Running moc on " << Quoted(absFilename) << "!\n"
          << "Better include " << Quoted(scannedFileBasename + ".moc")
          << " for compatibility with strict mode (see "
             "CMAKE_AUTOMOC_RELAXED_MODE).";
      this->LogWarning(ost.str());

      // Use scanned source file instead of scanned header file as moc source
      mocsIncluded[absFilename] = ownMocUnderscoreInclude;
      this->MocFindDepends(absFilename, contentText, mocDepends);
      // Remove
      mocsIncluded.erase(ownMocUnderscoreHeader);
    } else {
      // Otherwise always error out since it will not compile:
      std::ostringstream ost;
      ost << "AutoMoc: Error: " << Quoted(absFilename) << "\n"
          << "The file contains a " << macroName
          << " macro, but does not include "
          << Quoted(scannedFileBasename + ".moc") << "!\n"
          << "Consider adding the include or enabling SKIP_AUTOMOC for this "
             "file.";
      this->LogError(ost.str());
      return false;
    }
  }

  return true;
}

void cmQtAutoGenerators::MocParseHeaderContent(
  const std::string& absFilename, const std::string& contentText,
  std::map<std::string, std::string>& mocsNotIncluded,
  std::map<std::string, std::set<std::string>>& mocDepends)
{
  // Log
  if (this->Verbose) {
    this->LogInfo("AutoMoc: Checking " + absFilename);
  }
  if (this->MocRequired(contentText)) {
    // Register moc job
    mocsNotIncluded[absFilename] =
      this->ChecksumedPath(absFilename, "moc_", this->ConfigSuffix + ".cpp");
    this->MocFindDepends(absFilename, contentText, mocDepends);
  }
}

void cmQtAutoGenerators::SearchHeadersForSourceFile(
  const std::string& absFilename, std::set<std::string>& mocHeaderFiles,
  std::set<std::string>& uicHeaderFiles) const
{
  std::array<std::string, 2> basepaths;
  {
    std::string bpath = SubDirPrefix(absFilename);
    bpath += cmSystemTools::GetFilenameWithoutLastExtension(absFilename);
    // search for default header files and private header files
    basepaths[0] = bpath;
    basepaths[1] = bpath;
    basepaths[1] += "_p";
  }

  for (const std::string& bPath : basepaths) {
    std::string headerName;
    if (this->FindHeader(headerName, bPath)) {
      // Moc headers
      if (!this->MocSkip(absFilename) && !this->MocSkip(headerName)) {
        mocHeaderFiles.insert(headerName);
      }
      // Uic headers
      if (!this->UicSkip(absFilename) && !this->UicSkip(headerName)) {
        uicHeaderFiles.insert(headerName);
      }
    }
  }
}

bool cmQtAutoGenerators::ParseHeaders(
  const std::set<std::string>& mocHeaderFiles,
  const std::set<std::string>& uicHeaderFiles,
  const std::map<std::string, std::string>& mocsIncluded,
  std::map<std::string, std::string>& mocsNotIncluded,
  std::map<std::string, std::set<std::string>>& mocDepends,
  std::map<std::string, std::vector<std::string>>& uisIncluded)
{
  bool success = true;
  // Merged header files list to read files only once
  std::set<std::string> headerFiles;
  headerFiles.insert(mocHeaderFiles.begin(), mocHeaderFiles.end());
  headerFiles.insert(uicHeaderFiles.begin(), uicHeaderFiles.end());

  for (const std::string& headerName : headerFiles) {
    std::string contentText;
    if (ReadAll(contentText, headerName)) {
      // Parse header content for MOC
      if ((mocHeaderFiles.find(headerName) != mocHeaderFiles.end()) &&
          (mocsIncluded.find(headerName) == mocsIncluded.end())) {
        this->MocParseHeaderContent(headerName, contentText, mocsNotIncluded,
                                    mocDepends);
      }
      // Parse header content for UIC
      if (uicHeaderFiles.find(headerName) != uicHeaderFiles.end()) {
        this->UicParseContent(headerName, contentText, uisIncluded);
      }
    } else {
      std::ostringstream ost;
      ost << "AutoGen: Error: Could not read header file:\n"
          << Quoted(headerName);
      this->LogError(ost.str());
      success = false;
      break;
    }
  }
  return success;
}

bool cmQtAutoGenerators::MocGenerateAll(
  const std::map<std::string, std::string>& mocsIncluded,
  const std::map<std::string, std::string>& mocsNotIncluded,
  const std::map<std::string, std::set<std::string>>& mocDepends)
{
  if (!this->MocEnabled()) {
    return true;
  }

  // Look for name collisions
  {
    std::multimap<std::string, std::string> collisions;
    // Test merged map of included and notIncluded
    std::map<std::string, std::string> mergedMocs(mocsIncluded);
    mergedMocs.insert(mocsNotIncluded.begin(), mocsNotIncluded.end());
    if (this->NameCollisionTest(mergedMocs, collisions)) {
      std::ostringstream ost;
      ost << "AutoMoc: Error: "
             "The same moc file will be generated "
             "from different sources.\n"
             "To avoid this error either\n"
             "- rename the source files or\n"
             "- do not include the (moc_NAME.cpp|NAME.moc) file";
      this->LogErrorNameCollision(ost.str(), collisions);
      return false;
    }
  }

  // Generate moc_predefs
  if (!this->MocPredefsCmd.empty()) {
    if (this->MocSettingsChanged ||
        FileAbsentOrOlder(this->MocPredefsFileAbs, this->SettingsFile)) {
      if (this->Verbose) {
        this->LogBold("Generating MOC predefs " + this->MocPredefsFileRel);
      }

      std::string output;
      {
        // Compose command
        std::vector<std::string> cmd = this->MocPredefsCmd;
        // Add includes
        cmd.insert(cmd.end(), this->MocIncludes.begin(),
                   this->MocIncludes.end());
        // Add definitions
        for (const std::string& def : this->MocDefinitions) {
          cmd.push_back("-D" + def);
        }
        // Add options
        cmd.insert(cmd.end(), this->MocOptions.begin(),
                   this->MocOptions.end());
        // Execute command
        if (!this->RunCommand(cmd, output, false)) {
          {
            std::ostringstream ost;
            ost << "AutoMoc: Error: moc predefs generation command failed\n";
            ost << "AutoMoc: Command:\n" << QuotedCommand(cmd) << "\n";
            ost << "AutoMoc: Command output:\n" << output << "\n";
            this->LogError(ost.str());
          }
          return false;
        }
      }
      // (Re)write predefs file only on demand
      if (this->FileDiffers(this->MocPredefsFileAbs, output)) {
        if (this->FileWrite(cmQtAutoGen::MOC, this->MocPredefsFileAbs,
                            output)) {
          this->MocPredefsChanged = true;
        } else {
          return false;
        }
      }
    }
  }

  // Generate moc files that are included by source files.
  for (std::map<std::string, std::string>::const_iterator it =
         mocsIncluded.begin();
       it != mocsIncluded.end(); ++it) {
    if (!this->MocGenerateFile(it->first, it->second, mocDepends, true)) {
      if (this->MocRunFailed) {
        return false;
      }
    }
  }

  // Generate moc files that are _not_ included by source files.
  bool mocCompFileGenerated = false;
  for (std::map<std::string, std::string>::const_iterator it =
         mocsNotIncluded.begin();
       it != mocsNotIncluded.end(); ++it) {
    if (this->MocGenerateFile(it->first, it->second, mocDepends, false)) {
      mocCompFileGenerated = true;
    } else {
      if (this->MocRunFailed) {
        return false;
      }
    }
  }

  // Compose mocs compilation file content
  std::string automocSource;
  {
    std::ostringstream ost;
    ost << "/* This file is autogenerated, do not edit*/\n";
    if (mocsNotIncluded.empty()) {
      // Dummy content
      ost << "enum some_compilers { need_more_than_nothing };\n";
    } else {
      // Valid content
      for (std::map<std::string, std::string>::const_iterator it =
             mocsNotIncluded.begin();
           it != mocsNotIncluded.end(); ++it) {
        ost << "#include \"" << it->second << "\"\n";
      }
    }
    automocSource = ost.str();
  }

  if (this->FileDiffers(this->MocCompFileAbs, automocSource)) {
    // Actually write mocs compilation file
    if (this->Verbose) {
      this->LogBold("Generating MOC compilation " + this->MocCompFileRel);
    }
    if (!this->FileWrite(cmQtAutoGen::MOC, this->MocCompFileAbs,
                         automocSource)) {
      return false;
    }
  } else if (mocCompFileGenerated) {
    // Only touch mocs compilation file
    if (this->Verbose) {
      this->LogInfo("Touching MOC compilation " + this->MocCompFileRel);
    }
    cmSystemTools::Touch(this->MocCompFileAbs, false);
  }

  return true;
}

/**
 * @return True if a moc file was created. False may indicate an error.
 */
bool cmQtAutoGenerators::MocGenerateFile(
  const std::string& sourceFile, const std::string& mocFileName,
  const std::map<std::string, std::set<std::string>>& mocDepends,
  bool included)
{
  bool mocGenerated = false;
  bool generateMoc = this->MocSettingsChanged || this->MocPredefsChanged;

  const std::string mocFileRel =
    included ? (this->AutogenIncludeDir + mocFileName) : mocFileName;
  const std::string mocFileAbs =
    cmSystemTools::CollapseCombinedPath(this->AutogenBuildDir, mocFileRel);

  if (!generateMoc) {
    // Test if the source file is newer that the build file
    generateMoc = FileAbsentOrOlder(mocFileAbs, sourceFile);
    if (!generateMoc) {
      // Test if a dependency file changed
      std::map<std::string, std::set<std::string>>::const_iterator dit =
        mocDepends.find(sourceFile);
      if (dit != mocDepends.end()) {
        for (std::set<std::string>::const_iterator fit = dit->second.begin();
             fit != dit->second.end(); ++fit) {
          if (FileAbsentOrOlder(mocFileAbs, *fit)) {
            generateMoc = true;
            break;
          }
        }
      }
    }
  }
  if (generateMoc) {
    // Log
    if (this->Verbose) {
      this->LogBold("Generating MOC source " + mocFileRel);
    }

    // Make sure the parent directory exists
    if (this->MakeParentDirectory(cmQtAutoGen::MOC, mocFileAbs)) {
      // Compose moc command
      std::vector<std::string> cmd;
      cmd.push_back(this->MocExecutable);
      // Add includes
      cmd.insert(cmd.end(), this->MocIncludes.begin(),
                 this->MocIncludes.end());
      // Add definitions
      for (const std::string& def : this->MocDefinitions) {
        cmd.push_back("-D" + def);
      }
      // Add options
      cmd.insert(cmd.end(), this->MocOptions.begin(), this->MocOptions.end());
      // Add predefs include
      if (!this->MocPredefsFileAbs.empty()) {
        cmd.push_back("--include");
        cmd.push_back(this->MocPredefsFileAbs);
      }
      cmd.push_back("-o");
      cmd.push_back(mocFileAbs);
      cmd.push_back(sourceFile);

      // Execute moc command
      std::string output;
      if (this->RunCommand(cmd, output)) {
        // Success
        mocGenerated = true;
      } else {
        // Command failed
        {
          std::ostringstream ost;
          ost << "AutoMoc: Error: moc process failed for\n";
          ost << Quoted(mocFileRel) << "\n";
          ost << "AutoMoc: Command:\n" << QuotedCommand(cmd) << "\n";
          ost << "AutoMoc: Command output:\n" << output << "\n";
          this->LogError(ost.str());
        }
        cmSystemTools::RemoveFile(mocFileAbs);
        this->MocRunFailed = true;
      }
    } else {
      // Parent directory creation failed
      this->MocRunFailed = true;
    }
  }
  return mocGenerated;
}

bool cmQtAutoGenerators::UicFindIncludedFile(std::string& absFile,
                                             const std::string& sourceFile,
                                             const std::string& searchPath,
                                             const std::string& searchFile)
{
  bool success = false;
  std::vector<std::string> testFiles;
  // Collect search paths list
  {
    std::string searchFileFull;
    if (!searchPath.empty()) {
      searchFileFull = searchPath;
      searchFileFull += searchFile;
    }
    // Vicinity of the source
    {
      const std::string sourcePath = SubDirPrefix(sourceFile);
      testFiles.push_back(sourcePath + searchFile);
      if (!searchPath.empty()) {
        testFiles.push_back(sourcePath + searchFileFull);
      }
    }
    // AUTOUIC search paths
    if (!this->UicSearchPaths.empty()) {
      for (const std::string& sPath : this->UicSearchPaths) {
        testFiles.push_back((sPath + "/").append(searchFile));
      }
      if (!searchPath.empty()) {
        for (const std::string& sPath : this->UicSearchPaths) {
          testFiles.push_back((sPath + "/").append(searchFileFull));
        }
      }
    }
  }

  // Search for the .ui file!
  for (const std::string& testFile : testFiles) {
    if (cmSystemTools::FileExists(testFile.c_str())) {
      absFile = cmSystemTools::GetRealPath(testFile);
      success = true;
      break;
    }
  }

  // Log error
  if (!success) {
    std::ostringstream ost;
    ost << "AutoUic: Error: " << Quoted(sourceFile) << "\n";
    ost << "Could not find " << Quoted(searchFile) << " in\n";
    for (const std::string& testFile : testFiles) {
      ost << "  " << Quoted(testFile) << "\n";
    }
    this->LogError(ost.str());
  }

  return success;
}

bool cmQtAutoGenerators::UicGenerateAll(
  const std::map<std::string, std::vector<std::string>>& uisIncluded)
{
  if (!this->UicEnabled()) {
    return true;
  }

  // single map with input / output names
  std::map<std::string, std::map<std::string, std::string>> sourceGenMap;
  {
    // Collision lookup map
    std::map<std::string, std::string> testMap;
    // Compile maps
    for (std::map<std::string, std::vector<std::string>>::const_iterator sit =
           uisIncluded.begin();
         sit != uisIncluded.end(); ++sit) {
      const std::string& source(sit->first);
      const std::vector<std::string>& sourceIncs(sit->second);
      // insert new source/destination map
      std::map<std::string, std::string>& uiGenMap = sourceGenMap[source];
      for (const std::string& inc : sourceIncs) {
        // Remove ui_ from the begin filename by substr()
        const std::string uiBasePath = SubDirPrefix(inc);
        const std::string uiBaseName =
          cmSystemTools::GetFilenameWithoutLastExtension(inc).substr(3);
        const std::string uiFileName = uiBaseName + ".ui";
        std::string uiInputFile;
        if (UicFindIncludedFile(uiInputFile, source, uiBasePath, uiFileName)) {
          std::string uiOutputFile = uiBasePath;
          uiOutputFile += "ui_";
          uiOutputFile += uiBaseName;
          uiOutputFile += ".h";
          cmSystemTools::ReplaceString(uiOutputFile, "..", "__");
          uiGenMap[uiInputFile] = uiOutputFile;
          testMap[uiInputFile] = uiOutputFile;
        } else {
          return false;
        }
      }
    }
    // look for name collisions
    {
      std::multimap<std::string, std::string> collisions;
      if (this->NameCollisionTest(testMap, collisions)) {
        std::ostringstream ost;
        ost << "AutoUic: Error: The same ui_NAME.h file will be generated "
               "from different sources.\n"
               "To avoid this error rename the source files.\n";
        this->LogErrorNameCollision(ost.str(), collisions);
        return false;
      }
    }
  }

  // generate ui files
  for (const auto& srcItem : sourceGenMap) {
    for (const auto& item : srcItem.second) {
      if (!this->UicGenerateFile(srcItem.first, item.first, item.second)) {
        if (this->UicRunFailed) {
          return false;
        }
      }
    }
  }

  return true;
}

/**
 * @return True if a uic file was created. False may indicate an error.
 */
bool cmQtAutoGenerators::UicGenerateFile(const std::string& realName,
                                         const std::string& uiInputFile,
                                         const std::string& uiOutputFile)
{
  bool uicGenerated = false;
  bool generateUic = this->UicSettingsChanged;

  const std::string uicFileRel = this->AutogenIncludeDir + uiOutputFile;
  const std::string uicFileAbs =
    cmSystemTools::CollapseCombinedPath(this->AutogenBuildDir, uicFileRel);

  if (!generateUic) {
    // Test if the source file is newer that the build file
    generateUic = FileAbsentOrOlder(uicFileAbs, uiInputFile);
  }
  if (generateUic) {
    // Log
    if (this->Verbose) {
      this->LogBold("Generating UIC header " + uicFileRel);
    }

    // Make sure the parent directory exists
    if (this->MakeParentDirectory(cmQtAutoGen::UIC, uicFileAbs)) {
      // Compose uic command
      std::vector<std::string> cmd;
      cmd.push_back(this->UicExecutable);
      {
        std::vector<std::string> allOpts = this->UicTargetOptions;
        std::map<std::string, std::string>::const_iterator optionIt =
          this->UicOptions.find(uiInputFile);
        if (optionIt != this->UicOptions.end()) {
          std::vector<std::string> fileOpts;
          cmSystemTools::ExpandListArgument(optionIt->second, fileOpts);
          cmQtAutoGen::UicMergeOptions(allOpts, fileOpts,
                                       (this->QtMajorVersion == "5"));
        }
        cmd.insert(cmd.end(), allOpts.begin(), allOpts.end());
      }
      cmd.push_back("-o");
      cmd.push_back(uicFileAbs);
      cmd.push_back(uiInputFile);

      std::string output;
      if (this->RunCommand(cmd, output)) {
        // Success
        uicGenerated = true;
      } else {
        // Command failed
        {
          std::ostringstream ost;
          ost << "AutoUic: Error: uic process failed for\n";
          ost << Quoted(uicFileRel) << " needed by\n";
          ost << Quoted(realName) << "\n";
          ost << "AutoUic: Command:\n" << QuotedCommand(cmd) << "\n";
          ost << "AutoUic: Command output:\n" << output << "\n";
          this->LogError(ost.str());
        }
        cmSystemTools::RemoveFile(uicFileAbs);
        this->UicRunFailed = true;
      }
    } else {
      // Parent directory creation failed
      this->UicRunFailed = true;
    }
  }
  return uicGenerated;
}

bool cmQtAutoGenerators::RccGenerateAll()
{
  if (!this->RccEnabled()) {
    return true;
  }

  // generate single map with input / output names
  std::map<std::string, std::string> qrcGenMap;
  {
    const std::string qrcPrefix = "qrc_";
    const std::string qrcSuffix = this->ConfigSuffix + ".cpp";
    for (const std::string& src : this->RccSources) {
      qrcGenMap[src] = this->ChecksumedPath(src, qrcPrefix, qrcSuffix);
    }
  }

  // look for name collisions
  {
    std::multimap<std::string, std::string> collisions;
    if (this->NameCollisionTest(qrcGenMap, collisions)) {
      std::ostringstream ost;
      ost << "AutoRcc: Error: The same qrc_NAME.cpp file"
             " will be generated from different sources.\n"
             "To avoid this error rename the source .qrc files.\n";
      this->LogErrorNameCollision(ost.str(), collisions);
      return false;
    }
  }

  // generate qrc files
  for (const auto& item : qrcGenMap) {
    bool unique = FileNameIsUnique(item.first, qrcGenMap);
    if (!this->RccGenerateFile(item.first, item.second, unique)) {
      if (this->RccRunFailed) {
        return false;
      }
    }
  }
  return true;
}

/**
 * @return True if a rcc file was created. False may indicate an error.
 */
bool cmQtAutoGenerators::RccGenerateFile(const std::string& rccInputFile,
                                         const std::string& rccOutputFile,
                                         bool unique_n)
{
  bool rccGenerated = false;
  bool generateRcc = this->RccSettingsChanged;
  const std::string rccBuildFile =
    cmSystemTools::CollapseCombinedPath(this->AutogenBuildDir, rccOutputFile);

  // Check if regeneration is required
  if (!generateRcc) {
    // Test if the resources list file is newer than build file
    generateRcc = FileAbsentOrOlder(rccBuildFile, rccInputFile);
    if (!generateRcc) {
      // Acquire input file list
      std::vector<std::string> readFiles;
      const std::vector<std::string>* files = &this->RccInputs[rccInputFile];
      if (files->empty()) {
        // Read input file list from qrc file
        std::string error;
        if (cmQtAutoGen::RccListInputs(this->QtMajorVersion,
                                       this->RccExecutable, rccInputFile,
                                       readFiles, &error)) {
          files = &readFiles;
        } else {
          files = nullptr;
          this->LogError(error);
          this->RccRunFailed = true;
        }
      }
      // Test if any input file is newer than the build file
      if (files != nullptr) {
        for (const std::string& file : *files) {
          if (FileAbsentOrOlder(rccBuildFile, file)) {
            generateRcc = true;
            break;
          }
        }
      }
    }
  }
  // Regenerate on demand
  if (generateRcc) {
    // Log
    if (this->Verbose) {
      this->LogBold("Generating RCC source " + rccOutputFile);
    }

    // Make sure the parent directory exists
    if (this->MakeParentDirectory(cmQtAutoGen::RCC, rccBuildFile)) {
      // Compose symbol name
      std::string symbolName =
        cmSystemTools::GetFilenameWithoutLastExtension(rccInputFile);
      if (!unique_n) {
        symbolName += "_";
        symbolName += FPathChecksum.getPart(rccInputFile);
      }
      // Replace '-' with '_'. The former is valid for
      // file names but not for symbol names.
      std::replace(symbolName.begin(), symbolName.end(), '-', '_');

      // Compose rcc command
      std::vector<std::string> cmd;
      cmd.push_back(this->RccExecutable);
      {
        std::map<std::string, std::string>::const_iterator optionIt =
          this->RccOptions.find(rccInputFile);
        if (optionIt != this->RccOptions.end()) {
          cmSystemTools::ExpandListArgument(optionIt->second, cmd);
        }
      }
      cmd.push_back("-name");
      cmd.push_back(symbolName);
      cmd.push_back("-o");
      cmd.push_back(rccBuildFile);
      cmd.push_back(rccInputFile);

      std::string output;
      if (this->RunCommand(cmd, output)) {
        // Success
        rccGenerated = true;
      } else {
        // Command failed
        {
          std::ostringstream ost;
          ost << "AutoRcc: Error: rcc process failed for\n";
          ost << Quoted(rccOutputFile) << "\n";
          ost << "AutoRcc: Command:\n" << QuotedCommand(cmd) << "\n";
          ost << "AutoRcc: Command output:\n" << output << "\n";
          this->LogError(ost.str());
        }
        cmSystemTools::RemoveFile(rccBuildFile);
        this->RccRunFailed = true;
      }
    } else {
      // Parent directory creation failed
      this->RccRunFailed = true;
    }
  }
  // For a multi configuration generator generate a wrapper file
  if (!this->ConfigSuffix.empty() && !this->RccRunFailed) {
    // Wrapper file name
    const std::string cppSuffix = ".cpp";
    const size_t suffixLength = this->ConfigSuffix.size() + cppSuffix.size();
    const std::string wrapperFileRel =
      rccOutputFile.substr(0, rccOutputFile.size() - suffixLength) + cppSuffix;
    const std::string wrapperFileAbs = cmSystemTools::CollapseCombinedPath(
      this->AutogenBuildDir, wrapperFileRel);
    // Wrapper file content
    std::string content =
      "// This is an autogenerated configuration wrapper file. Do not edit.\n"
      "#include \"";
    content += cmSystemTools::GetFilenameName(rccBuildFile);
    content += "\"\n";
    // Write content to file
    if (this->FileDiffers(wrapperFileAbs, content)) {
      // Write new wrapper file if the content differs
      if (this->Verbose) {
        this->LogBold("Generating RCC wrapper " + wrapperFileRel);
      }
      if (!this->FileWrite(cmQtAutoGen::RCC, wrapperFileAbs, content)) {
        // Error
        rccGenerated = false;
        this->RccRunFailed = true;
      }
    } else if (rccGenerated) {
      // Only touch wrapper file if the content matches
      if (this->Verbose) {
        this->LogInfo("Touching RCC wrapper " + wrapperFileRel);
      }
      cmSystemTools::Touch(wrapperFileAbs, false);
    }
  }

  return rccGenerated;
}

void cmQtAutoGenerators::LogErrorNameCollision(
  const std::string& message,
  const std::multimap<std::string, std::string>& collisions) const
{
  std::ostringstream ost;
  // Add message
  if (!message.empty()) {
    ost << message;
    if (message[message.size() - 1] != '\n') {
      ost << '\n';
    }
  }
  // Append collision list
  for (const auto& item : collisions) {
    ost << "  " << item.first << " : " << item.second << '\n';
  }
  this->LogError(ost.str());
}

void cmQtAutoGenerators::LogBold(const std::string& message) const
{
  cmSystemTools::MakefileColorEcho(cmsysTerminal_Color_ForegroundBlue |
                                     cmsysTerminal_Color_ForegroundBold,
                                   message.c_str(), true, this->ColorOutput);
}

void cmQtAutoGenerators::LogInfo(const std::string& message) const
{
  std::string msg(message);
  if (!msg.empty()) {
    if (msg[msg.size() - 1] != '\n') {
      msg.push_back('\n');
    }
    cmSystemTools::Stdout(msg.c_str(), msg.size());
  }
}

void cmQtAutoGenerators::LogWarning(const std::string& message) const
{
  std::string msg(message);
  if (!msg.empty()) {
    if (msg[msg.size() - 1] != '\n') {
      msg.push_back('\n');
    }
    // Append empty line
    msg.push_back('\n');
    cmSystemTools::Stdout(msg.c_str(), msg.size());
  }
}

void cmQtAutoGenerators::LogError(const std::string& message) const
{
  std::string msg(message);
  if (!msg.empty()) {
    if (msg[msg.size() - 1] != '\n') {
      msg.push_back('\n');
    }
    // Append empty line
    msg.push_back('\n');
    cmSystemTools::Stderr(msg.c_str(), msg.size());
  }
}

/**
 * @brief Collects name collisions as output/input pairs
 * @return True if there were collisions
 */
bool cmQtAutoGenerators::NameCollisionTest(
  const std::map<std::string, std::string>& genFiles,
  std::multimap<std::string, std::string>& collisions) const
{
  typedef std::map<std::string, std::string>::const_iterator Iter;
  typedef std::map<std::string, std::string>::value_type VType;
  for (Iter ait = genFiles.begin(); ait != genFiles.end(); ++ait) {
    bool first_match(true);
    for (Iter bit = (++Iter(ait)); bit != genFiles.end(); ++bit) {
      if (ait->second == bit->second) {
        if (first_match) {
          if (collisions.find(ait->second) != collisions.end()) {
            // We already know of this collision from before
            break;
          }
          collisions.insert(VType(ait->second, ait->first));
          first_match = false;
        }
        collisions.insert(VType(bit->second, bit->first));
      }
    }
  }

  return !collisions.empty();
}

/**
 * @brief Generates a file path based on the checksum of the source file path
 * @return The path
 */
std::string cmQtAutoGenerators::ChecksumedPath(
  const std::string& sourceFile, const std::string& basePrefix,
  const std::string& baseSuffix) const
{
  std::string res = FPathChecksum.getPart(sourceFile);
  res += "/";
  res += basePrefix;
  res += cmSystemTools::GetFilenameWithoutLastExtension(sourceFile);
  res += baseSuffix;
  return res;
}

/**
 * @brief Generates the parent directory of the given file on demand
 * @return True on success
 */
bool cmQtAutoGenerators::MakeParentDirectory(
  cmQtAutoGen::GeneratorType genType, const std::string& filename) const
{
  bool success = true;
  const std::string dirName = cmSystemTools::GetFilenamePath(filename);
  if (!dirName.empty()) {
    success = cmSystemTools::MakeDirectory(dirName);
    if (!success) {
      std::string error = cmQtAutoGen::GeneratorName(genType);
      error += ": Error: Parent directory creation failed for ";
      error += Quoted(filename);
      this->LogError(error);
    }
  }
  return success;
}

bool cmQtAutoGenerators::FileDiffers(const std::string& filename,
                                     const std::string& content)
{
  bool differs = true;
  {
    std::string oldContents;
    if (ReadAll(oldContents, filename)) {
      differs = (oldContents != content);
    }
  }
  return differs;
}

bool cmQtAutoGenerators::FileWrite(cmQtAutoGen::GeneratorType genType,
                                   const std::string& filename,
                                   const std::string& content)
{
  std::string error;
  // Make sure the parent directory exists
  if (this->MakeParentDirectory(genType, filename)) {
    cmsys::ofstream outfile;
    outfile.open(filename.c_str(), std::ios::trunc);
    if (outfile) {
      outfile << content;
      // Check for write errors
      if (!outfile.good()) {
        error = cmQtAutoGen::GeneratorName(genType);
        error += ": Error: File writing failed\n";
        error += "  ";
        error += Quoted(filename);
      }
    } else {
      error = cmQtAutoGen::GeneratorName(genType);
      error += ": Error: Opening file for writing failed\n";
      error += "  ";
      error += Quoted(filename);
    }
  }
  if (!error.empty()) {
    this->LogError(error);
    return false;
  }
  return true;
}

/**
 * @brief Runs a command and returns true on success
 * @return True on success
 */
bool cmQtAutoGenerators::RunCommand(const std::vector<std::string>& command,
                                    std::string& output, bool verbose) const
{
  // Log command
  if (this->Verbose) {
    this->LogInfo(QuotedCommand(command));
  }
  // Execute command
  int retVal = 0;
  bool res = cmSystemTools::RunSingleCommand(
    command, &output, &output, &retVal, nullptr,
    verbose ? cmSystemTools::OUTPUT_MERGE : cmSystemTools::OUTPUT_NONE);
  return (res && (retVal == 0));
}

/**
 * @brief Tries to find the header file to the given file base path by
 * appending different header extensions
 * @return True on success
 */
bool cmQtAutoGenerators::FindHeader(std::string& header,
                                    const std::string& testBasePath) const
{
  for (const std::string& ext : this->HeaderExtensions) {
    std::string testFilePath(testBasePath);
    testFilePath.push_back('.');
    testFilePath += ext;
    if (cmSystemTools::FileExists(testFilePath.c_str())) {
      header = testFilePath;
      return true;
    }
  }
  return false;
}

std::string cmQtAutoGenerators::MocFindHeader(
  const std::string& sourcePath, const std::string& includeBase) const
{
  std::string header;
  // Search in vicinity of the source
  if (!this->FindHeader(header, sourcePath + includeBase)) {
    // Search in include directories
    for (const std::string& path : this->MocIncludePaths) {
      std::string fullPath = path;
      fullPath.push_back('/');
      fullPath += includeBase;
      if (FindHeader(header, fullPath)) {
        break;
      }
    }
  }
  // Sanitize
  if (!header.empty()) {
    header = cmSystemTools::GetRealPath(header);
  }
  return header;
}

bool cmQtAutoGenerators::MocFindIncludedFile(
  std::string& absFile, const std::string& sourcePath,
  const std::string& includeString) const
{
  bool success = false;
  // Search in vicinity of the source
  {
    std::string testPath = sourcePath;
    testPath += includeString;
    if (cmSystemTools::FileExists(testPath.c_str())) {
      absFile = cmSystemTools::GetRealPath(testPath);
      success = true;
    }
  }
  // Search in include directories
  if (!success) {
    for (const std::string& path : this->MocIncludePaths) {
      std::string fullPath = path;
      fullPath.push_back('/');
      fullPath += includeString;
      if (cmSystemTools::FileExists(fullPath.c_str())) {
        absFile = cmSystemTools::GetRealPath(fullPath);
        success = true;
        break;
      }
    }
  }
  return success;
}
