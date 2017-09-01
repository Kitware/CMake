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

static std::string QuotedCommand(const std::vector<std::string>& command)
{
  std::string res;
  for (const std::string& item : command) {
    if (!res.empty()) {
      res.push_back(' ');
    }
    const std::string cesc = cmQtAutoGen::Quoted(item);
    if (item.empty() || (cesc.size() > (item.size() + 2)) ||
        (cesc.find(' ') != std::string::npos)) {
      res += cesc;
    } else {
      res += item;
    }
  }
  return res;
}

static std::string SubDirPrefix(const std::string& fileName)
{
  std::string res(cmSystemTools::GetFilenamePath(fileName));
  if (!res.empty()) {
    res += '/';
  }
  return res;
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
  std::string error;
  if (!key.empty()) {
    if (!regExp.empty()) {
      KeyRegExp filter;
      filter.Key = key;
      if (filter.RegExp.compile(regExp)) {
        this->MocDependFilters.push_back(std::move(filter));
      } else {
        error = "Regular expression compiling failed";
      }
    } else {
      error = "Regular expression is empty";
    }
  } else {
    error = "Key is empty";
  }
  if (!error.empty()) {
    std::string emsg = "AUTOMOC_DEPEND_FILTERS: ";
    emsg += error;
    emsg += "\n";
    emsg += "  Key:    ";
    emsg += cmQtAutoGen::Quoted(key);
    emsg += "\n";
    emsg += "  RegExp: ";
    emsg += cmQtAutoGen::Quoted(regExp);
    emsg += "\n";
    this->LogError(cmQtAutoGen::MOC, emsg);
    return false;
  }
  return true;
}

bool cmQtAutoGenerators::ReadAutogenInfoFile(
  cmMakefile* makefile, const std::string& targetDirectory,
  const std::string& config)
{
  // Lambdas
  auto InfoGet = [makefile](const char* key) {
    return makefile->GetSafeDefinition(key);
  };
  auto InfoGetBool = [makefile](const char* key) {
    return makefile->IsOn(key);
  };
  auto InfoGetList = [makefile](const char* key) -> std::vector<std::string> {
    std::vector<std::string> list;
    cmSystemTools::ExpandListArgument(makefile->GetSafeDefinition(key), list);
    return list;
  };
  auto InfoGetLists =
    [makefile](const char* key) -> std::vector<std::vector<std::string>> {
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
  };
  auto InfoGetConfig = [makefile, &config](const char* key) -> std::string {
    const char* valueConf = nullptr;
    {
      std::string keyConf = key;
      if (!config.empty()) {
        keyConf += '_';
        keyConf += config;
      }
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

  std::string filename(cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutogenInfo.cmake";

  if (!makefile->ReadListFile(filename.c_str())) {
    this->LogFileError(cmQtAutoGen::GEN, filename, "File processing failed");
    return false;
  }

  // -- Meta
  this->ConfigSuffix = InfoGetConfig("AM_CONFIG_SUFFIX");

  // - Old settings file
  {
    this->SettingsFile = cmSystemTools::CollapseFullPath(targetDirectory);
    cmSystemTools::ConvertToUnixSlashes(this->SettingsFile);
    this->SettingsFile += "/AutogenOldSettings";
    this->SettingsFile += this->ConfigSuffix;
    this->SettingsFile += ".cmake";
  }

  // - Files and directories
  this->ProjectSourceDir = InfoGet("AM_CMAKE_SOURCE_DIR");
  this->ProjectBinaryDir = InfoGet("AM_CMAKE_BINARY_DIR");
  this->CurrentSourceDir = InfoGet("AM_CMAKE_CURRENT_SOURCE_DIR");
  this->CurrentBinaryDir = InfoGet("AM_CMAKE_CURRENT_BINARY_DIR");
  this->IncludeProjectDirsBefore =
    InfoGetBool("AM_CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE");
  this->AutogenBuildDir = InfoGet("AM_BUILD_DIR");
  if (this->AutogenBuildDir.empty()) {
    this->LogFileError(cmQtAutoGen::GEN, filename,
                       "Autogen build directory missing");
    return false;
  }
  this->Sources = InfoGetList("AM_SOURCES");
  this->Headers = InfoGetList("AM_HEADERS");

  // - Qt environment
  this->QtMajorVersion = InfoGet("AM_QT_VERSION_MAJOR");
  this->QtMinorVersion = InfoGet("AM_QT_VERSION_MINOR");
  this->MocExecutable = InfoGet("AM_QT_MOC_EXECUTABLE");
  this->UicExecutable = InfoGet("AM_QT_UIC_EXECUTABLE");
  this->RccExecutable = InfoGet("AM_QT_RCC_EXECUTABLE");

  // Check Qt version
  if ((this->QtMajorVersion != "4") && (this->QtMajorVersion != "5")) {
    this->LogFileError(cmQtAutoGen::GEN, filename, "Unsupported Qt version: " +
                         cmQtAutoGen::Quoted(this->QtMajorVersion));
    return false;
  }

  // - Moc
  if (this->MocEnabled()) {
    this->MocSkipList = InfoGetList("AM_MOC_SKIP");
    this->MocDefinitions = InfoGetConfigList("AM_MOC_DEFINITIONS");
#ifdef _WIN32
    {
      const std::string win32("WIN32");
      if (!ListContains(this->MocDefinitions, win32)) {
        this->MocDefinitions.push_back(win32);
      }
    }
#endif
    this->MocIncludePaths = InfoGetConfigList("AM_MOC_INCLUDES");
    this->MocOptions = InfoGetList("AM_MOC_OPTIONS");
    this->MocRelaxedMode = InfoGetBool("AM_MOC_RELAXED_MODE");
    {
      const std::vector<std::string> MocMacroNames =
        InfoGetList("AM_MOC_MACRO_NAMES");
      for (const std::string& item : MocMacroNames) {
        this->MocMacroFilters.emplace_back(
          item, ("[^a-zA-Z0-9_]" + item).append("[^a-zA-Z0-9_]"));
      }
    }
    {
      const std::vector<std::string> mocDependFilters =
        InfoGetList("AM_MOC_DEPEND_FILTERS");
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
        this->LogFileError(
          cmQtAutoGen::MOC, filename,
          "AUTOMOC_DEPEND_FILTERS list size is not a multiple of 2");
        return false;
      }
    }
    this->MocPredefsCmd = InfoGetList("AM_MOC_PREDEFS_CMD");
  }

  // - Uic
  if (this->UicEnabled()) {
    this->UicSkipList = InfoGetList("AM_UIC_SKIP");
    this->UicSearchPaths = InfoGetList("AM_UIC_SEARCH_PATHS");
    this->UicTargetOptions = InfoGetConfigList("AM_UIC_TARGET_OPTIONS");
    {
      auto sources = InfoGetList("AM_UIC_OPTIONS_FILES");
      auto options = InfoGetLists("AM_UIC_OPTIONS_OPTIONS");
      // Compare list sizes
      if (sources.size() == options.size()) {
        auto fitEnd = sources.cend();
        auto fit = sources.begin();
        auto oit = options.begin();
        while (fit != fitEnd) {
          this->UicOptions[*fit] = std::move(*oit);
          ++fit;
          ++oit;
        }
      } else {
        std::ostringstream ost;
        ost << "files/options lists sizes missmatch (" << sources.size() << "/"
            << options.size() << ")";
        this->LogFileError(cmQtAutoGen::UIC, filename, ost.str());
        return false;
      }
    }
  }

  // - Rcc
  if (this->RccEnabled()) {
    // File lists
    auto sources = InfoGetList("AM_RCC_SOURCES");
    auto builds = InfoGetList("AM_RCC_BUILDS");
    auto options = InfoGetLists("AM_RCC_OPTIONS");
    auto inputs = InfoGetLists("AM_RCC_INPUTS");

    if (sources.size() != builds.size()) {
      std::ostringstream ost;
      ost << "sources, builds lists sizes missmatch (" << sources.size() << "/"
          << builds.size() << ")";
      this->LogFileError(cmQtAutoGen::RCC, filename, ost.str());
      return false;
    }
    if (sources.size() != options.size()) {
      std::ostringstream ost;
      ost << "sources, options lists sizes missmatch (" << sources.size()
          << "/" << options.size() << ")";
      this->LogFileError(cmQtAutoGen::RCC, filename, ost.str());
      return false;
    }
    if (sources.size() != inputs.size()) {
      std::ostringstream ost;
      ost << "sources, inputs lists sizes missmatch (" << sources.size() << "/"
          << inputs.size() << ")";
      this->LogFileError(cmQtAutoGen::RCC, filename, ost.str());
      return false;
    }

    {
      auto srcItEnd = sources.end();
      auto srcIt = sources.begin();
      auto bldIt = builds.begin();
      auto optIt = options.begin();
      auto inpIt = inputs.begin();
      while (srcIt != srcItEnd) {
        this->RccJobs.push_back(RccJob{ std::move(*srcIt), std::move(*bldIt),
                                        std::move(*optIt),
                                        std::move(*inpIt) });
        ++srcIt;
        ++bldIt;
        ++optIt;
        ++inpIt;
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
      str += cmJoin(this->MocDefinitions, ";");
      str += sep;
      str += cmJoin(this->MocIncludePaths, ";");
      str += sep;
      str += cmJoin(this->MocOptions, ";");
      str += sep;
      str += this->IncludeProjectDirsBefore ? "TRUE" : "FALSE";
      str += sep;
      str += cmJoin(this->MocPredefsCmd, ";");
      str += sep;
      this->SettingsStringMoc = crypt.HashString(str);
    }
    if (this->UicEnabled()) {
      std::string str;
      str += this->UicExecutable;
      str += sep;
      str += cmJoin(this->UicTargetOptions, ";");
      for (const auto& item : this->UicOptions) {
        str += sep;
        str += item.first;
        str += sep;
        str += cmJoin(item.second, ";");
      }
      str += sep;
      this->SettingsStringUic = crypt.HashString(str);
    }
    if (this->RccEnabled()) {
      std::string str;
      str += this->RccExecutable;
      for (const RccJob& rccJob : this->RccJobs) {
        str += sep;
        str += rccJob.QrcFile;
        str += sep;
        str += rccJob.RccFile;
        str += sep;
        str += cmJoin(rccJob.Options, ";");
      }
      str += sep;
      this->SettingsStringRcc = crypt.HashString(str);
    }
  }

  // Read old settings
  if (makefile->ReadListFile(this->SettingsFile.c_str())) {
    {
      auto SMatch = [makefile](const char* key, const std::string& value) {
        return (value == makefile->GetSafeDefinition(key));
      };
      if (!SMatch(SettingsKeyMoc, this->SettingsStringMoc)) {
        this->MocSettingsChanged = true;
      }
      if (!SMatch(SettingsKeyUic, this->SettingsStringUic)) {
        this->UicSettingsChanged = true;
      }
      if (!SMatch(SettingsKeyRcc, this->SettingsStringRcc)) {
        this->RccSettingsChanged = true;
      }
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
      this->LogInfo(cmQtAutoGen::GEN, "Writing settings file " +
                      cmQtAutoGen::Quoted(this->SettingsFile));
    }
    // Compose settings file content
    std::string settings;
    {
      auto SettingAppend = [&settings](const char* key,
                                       const std::string& value) {
        settings += "set(";
        settings += key;
        settings += " ";
        settings += cmOutputConverter::EscapeForCMake(value);
        settings += ")\n";
      };
      SettingAppend(SettingsKeyMoc, this->SettingsStringMoc);
      SettingAppend(SettingsKeyUic, this->SettingsStringUic);
      SettingAppend(SettingsKeyRcc, this->SettingsStringRcc);
    }
    // Write settings file
    if (!this->FileWrite(cmQtAutoGen::GEN, this->SettingsFile, settings)) {
      this->LogFileError(cmQtAutoGen::GEN, this->SettingsFile,
                         "File writing failed");
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
      this->LogFileError(cmQtAutoGen::GEN, incDirAbs,
                         "Could not create directory");
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
              this->LogInfo(cmQtAutoGen::MOC, "Found dependency:\n  " +
                              cmQtAutoGen::Quoted(absFilename) + "\n  " +
                              cmQtAutoGen::Quoted(incFile));
            }
          } else {
            this->LogFileWarning(cmQtAutoGen::MOC, absFilename,
                                 "Could not find dependency file " +
                                   cmQtAutoGen::Quoted(match));
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
      this->LogFileWarning(cmQtAutoGen::GEN, absFilename, "The file is empty");
    }
  } else {
    this->LogFileError(cmQtAutoGen::GEN, absFilename, "Could not read file");
  }
  return success;
}

void cmQtAutoGenerators::UicParseContent(
  const std::string& absFilename, const std::string& contentText,
  std::map<std::string, std::vector<std::string>>& uisIncluded)
{
  if (this->Verbose) {
    this->LogInfo(cmQtAutoGen::UIC, "Checking " + absFilename);
  }

  const char* contentChars = contentText.c_str();
  if (strstr(contentChars, "ui_") != nullptr) {
    while (this->UicRegExpInclude.find(contentChars)) {
      uisIncluded[absFilename].push_back(this->UicRegExpInclude.match(1));
      contentChars += this->UicRegExpInclude.end();
    }
  }
}

std::string cmQtAutoGenerators::MocMacroNamesString() const
{
  std::string res;
  const auto itB = this->MocMacroFilters.cbegin();
  const auto itE = this->MocMacroFilters.cend();
  const auto itL = itE - 1;
  auto itC = itB;
  for (; itC != itE; ++itC) {
    // Separator
    if (itC != itB) {
      if (itC != itL) {
        res += ", ";
      } else {
        res += " or ";
      }
    }
    // Key
    res += itC->Key;
  }
  return res;
}

std::string cmQtAutoGenerators::MocHeaderSuffixesString() const
{
  std::string res = ".{";
  res += cmJoin(this->HeaderExtensions, ",");
  res += "}";
  return res;
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
    this->LogInfo(cmQtAutoGen::MOC, "Checking " + absFilename);
  }

  const std::string scanFileDir = SubDirPrefix(absFilename);
  const std::string scanFileBase =
    cmSystemTools::GetFilenameWithoutLastExtension(absFilename);

  std::string selfMacroName;
  const bool selfRequiresMoc = this->MocRequired(contentText, &selfMacroName);
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
      const std::string incDir(SubDirPrefix(incString));
      const std::string incBase =
        cmSystemTools::GetFilenameWithoutLastExtension(incString);

      // If the moc include is of the moc_foo.cpp style we expect
      // the Q_OBJECT class declaration in a header file.
      // If the moc include is of the foo.moc style we need to look for
      // a Q_OBJECT macro in the current source file, if it contains the
      // macro we generate the moc file from the source file.
      if (cmHasLiteralPrefix(incBase, "moc_")) {
        // Include: moc_FOO.cxx
        // Remove the moc_ part
        const std::string incRealBase = incBase.substr(4);
        const std::string headerToMoc =
          this->MocFindHeader(scanFileDir, incDir + incRealBase);
        if (!headerToMoc.empty()) {
          if (!this->MocSkip(headerToMoc)) {
            // Register moc job
            mocsIncluded[headerToMoc] = incString;
            this->MocFindDepends(headerToMoc, contentText, mocDepends);
            // Store meta information for relaxed mode
            if (relaxed && (incRealBase == scanFileBase)) {
              ownMocUnderscoreInclude = incString;
              ownMocUnderscoreHeader = headerToMoc;
            }
          }
        } else {
          std::ostringstream ost;
          ost << "The file includes the moc file "
              << cmQtAutoGen::Quoted(incString)
              << ", but could not find header ";
          ost << cmQtAutoGen::Quoted(incRealBase +
                                     this->MocHeaderSuffixesString());
          this->LogFileError(cmQtAutoGen::MOC, absFilename, ost.str());
          return false;
        }
      } else {
        // Include: FOO.moc
        bool ownDotMoc = (incBase == scanFileBase);
        std::string fileToMoc;
        if (relaxed) {
          // Mode: Relaxed
          if (selfRequiresMoc && ownDotMoc) {
            // Include self
            fileToMoc = absFilename;
            ownDotMocIncluded = true;
          } else {
            // In relaxed mode try to find a header instead but issue a warning
            const std::string headerToMoc =
              this->MocFindHeader(scanFileDir, incDir + incBase);
            if (!headerToMoc.empty()) {
              if (!this->MocSkip(headerToMoc)) {
                // This is for KDE4 compatibility:
                fileToMoc = headerToMoc;

                auto quoted_inc = cmQtAutoGen::Quoted(incString);
                auto quoted_header = cmQtAutoGen::Quoted(headerToMoc);
                auto quoted_base =
                  cmQtAutoGen::Quoted("moc_" + incBase + ".cpp");
                if (!selfRequiresMoc) {
                  if (ownDotMoc) {
                    std::ostringstream ost;
                    ost << "The file includes the moc file " << quoted_inc
                        << ", but does not contain a "
                        << this->MocMacroNamesString() << " macro.\n"
                        << "Running moc on\n"
                        << "  " << quoted_header << "!\n"
                        << "Better include " << quoted_base
                        << " for a compatibility with strict mode.\n"
                           "(CMAKE_AUTOMOC_RELAXED_MODE warning)\n";
                    this->LogFileWarning(cmQtAutoGen::MOC, absFilename,
                                         ost.str());
                  } else if (!ownDotMoc) {
                    std::ostringstream ost;
                    ost << "The file includes the moc file " << quoted_inc
                        << " instead of " << quoted_base << ".\n";
                    ost << "Running moc on\n"
                        << "  " << quoted_header << "!\n"
                        << "Better include " << quoted_base
                        << " for compatibility with strict mode.\n"
                           "(CMAKE_AUTOMOC_RELAXED_MODE warning)\n";
                    this->LogFileWarning(cmQtAutoGen::MOC, absFilename,
                                         ost.str());
                  }
                } else {
                  if (!ownDotMoc) {
                    // Handled further down
                  }
                }
              }
            } else {
              std::ostringstream ost;
              ost << "The file includes the moc file "
                  << cmQtAutoGen::Quoted(incString)
                  << ", which seems to be the moc file from a different "
                     "source file. CMake also could not find a matching "
                     "header.";
              this->LogFileError(cmQtAutoGen::MOC, absFilename, ost.str());
              return false;
            }
          }
        } else {
          // Mode: Strict
          if (ownDotMoc) {
            // Include self
            fileToMoc = absFilename;
            ownDotMocIncluded = true;
            // Accept but issue a warning if moc isn't required
            if (!selfRequiresMoc) {
              std::ostringstream ost;
              ost << "The file includes the moc file "
                  << cmQtAutoGen::Quoted(incString)
                  << ", but does not contain a " << this->MocMacroNamesString()
                  << " macro.";
              this->LogFileWarning(cmQtAutoGen::MOC, absFilename, ost.str());
            }
          } else {
            // Don't allow FOO.moc include other than self in strict mode
            std::ostringstream ost;
            ost << "The file includes the moc file "
                << cmQtAutoGen::Quoted(incString)
                << ", which seems to be the moc file from a different "
                   "source file. This is not supported. Include "
                << cmQtAutoGen::Quoted(scanFileBase + ".moc")
                << " to run moc on this source file.";
            this->LogFileError(cmQtAutoGen::MOC, absFilename, ost.str());
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

  if (selfRequiresMoc && !ownDotMocIncluded) {
    // In this case, check whether the scanned file itself contains a
    // Q_OBJECT.
    // If this is the case, the moc_foo.cpp should probably be generated from
    // foo.cpp instead of foo.h, because otherwise it won't build.
    // But warn, since this is not how it is supposed to be used.
    if (relaxed && !ownMocUnderscoreInclude.empty()) {
      // This is for KDE4 compatibility:
      std::ostringstream ost;
      ost << "The file contains a " << selfMacroName
          << " macro, but does not include "
          << cmQtAutoGen::Quoted(scanFileBase + ".moc")
          << ". Instead it includes "
          << cmQtAutoGen::Quoted(ownMocUnderscoreInclude) << ".\n"
          << "Running moc on\n"
          << "  " << cmQtAutoGen::Quoted(absFilename) << "!\n"
          << "Better include " << cmQtAutoGen::Quoted(scanFileBase + ".moc")
          << " for compatibility with strict mode.\n"
             "(CMAKE_AUTOMOC_RELAXED_MODE warning)";
      this->LogFileWarning(cmQtAutoGen::MOC, absFilename, ost.str());

      // Use scanned source file instead of scanned header file as moc source
      mocsIncluded[absFilename] = ownMocUnderscoreInclude;
      this->MocFindDepends(absFilename, contentText, mocDepends);
      // Remove
      mocsIncluded.erase(ownMocUnderscoreHeader);
    } else {
      // Otherwise always error out since it will not compile:
      std::ostringstream ost;
      ost << "The file contains a " << selfMacroName
          << " macro, but does not include "
          << cmQtAutoGen::Quoted(scanFileBase + ".moc") << "!\n"
          << "Consider adding the include or enabling SKIP_AUTOMOC for this "
             "file.";
      this->LogFileError(cmQtAutoGen::MOC, absFilename, ost.str());
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
    this->LogInfo(cmQtAutoGen::MOC, "Checking " + absFilename);
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
      this->LogFileError(cmQtAutoGen::GEN, headerName,
                         "Could not read header file");
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
      std::string msg = "The same moc file will be generated "
                        "from different sources.\n"
                        "To avoid this error either\n"
                        " - rename the source files or\n"
                        " - do not include the (moc_NAME.cpp|NAME.moc) file";
      this->LogNameCollisionError(cmQtAutoGen::MOC, msg, collisions);
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
        if (!this->RunCommand(cmd, output)) {
          this->LogCommandError(cmQtAutoGen::MOC,
                                "moc_predefs generation failed", cmd, output);
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
      } else {
        // Touch to update the time stamp
        if (this->Verbose) {
          this->LogInfo(cmQtAutoGen::MOC,
                        "Touching moc_predefs " + this->MocPredefsFileRel);
        }
        cmSystemTools::Touch(this->MocPredefsFileAbs, false);
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
      this->LogInfo(cmQtAutoGen::MOC,
                    "Touching mocs compilation " + this->MocCompFileRel);
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
        this->LogCommandError(cmQtAutoGen::MOC, "moc failed for\n  " +
                                cmQtAutoGen::Quoted(sourceFile),
                              cmd, output);
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
    ost << "Could not find " << cmQtAutoGen::Quoted(searchFile) << " in\n";
    for (const std::string& testFile : testFiles) {
      ost << "  " << cmQtAutoGen::Quoted(testFile) << "\n";
    }
    this->LogFileError(cmQtAutoGen::UIC, sourceFile, ost.str());
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
        std::string msg = "The same ui_NAME.h file will be generated "
                          "from different sources.\n"
                          "To avoid this error rename the source files.\n";
        this->LogNameCollisionError(cmQtAutoGen::UIC, msg, collisions);
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
        auto optionIt = this->UicOptions.find(uiInputFile);
        if (optionIt != this->UicOptions.end()) {
          cmQtAutoGen::UicMergeOptions(allOpts, optionIt->second,
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
          ost << "uic failed for\n"
              << "  " << cmQtAutoGen::Quoted(uiInputFile) << "\n"
              << "needed by\n"
              << "  " << cmQtAutoGen::Quoted(realName);
          this->LogCommandError(cmQtAutoGen::UIC, ost.str(), cmd, output);
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

  // Generate qrc files
  for (const RccJob& rccJob : this->RccJobs) {
    if (!this->RccGenerateFile(rccJob)) {
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
bool cmQtAutoGenerators::RccGenerateFile(const RccJob& rccJob)
{
  bool rccGenerated = false;
  bool generateRcc = this->RccSettingsChanged;

  std::string rccFileAbs;
  if (this->ConfigSuffix.empty()) {
    rccFileAbs = rccJob.RccFile;
  } else {
    rccFileAbs = SubDirPrefix(rccJob.RccFile);
    rccFileAbs +=
      cmSystemTools::GetFilenameWithoutLastExtension(rccJob.RccFile);
    rccFileAbs += this->ConfigSuffix;
    rccFileAbs += cmSystemTools::GetFilenameLastExtension(rccJob.RccFile);
  }
  const std::string rccFileRel = cmSystemTools::RelativePath(
    this->AutogenBuildDir.c_str(), rccFileAbs.c_str());

  // Check if regeneration is required
  if (!generateRcc) {
    // Test if the resources list file is newer than build file
    generateRcc = FileAbsentOrOlder(rccFileAbs, rccJob.QrcFile);
    if (!generateRcc) {
      // Acquire input file list
      std::vector<std::string> readFiles;
      const std::vector<std::string>* files = &rccJob.Inputs;
      if (files->empty()) {
        // Read input file list from qrc file
        std::string error;
        if (cmQtAutoGen::RccListInputs(this->QtMajorVersion,
                                       this->RccExecutable, rccJob.QrcFile,
                                       readFiles, &error)) {
          files = &readFiles;
        } else {
          files = nullptr;
          this->LogFileError(cmQtAutoGen::RCC, rccJob.QrcFile, error);
          this->RccRunFailed = true;
        }
      }
      // Test if any input file is newer than the build file
      if (files != nullptr) {
        for (const std::string& file : *files) {
          if (FileAbsentOrOlder(rccFileAbs, file)) {
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
      this->LogBold("Generating RCC source " + rccFileRel);
    }

    // Make sure the parent directory exists
    if (this->MakeParentDirectory(cmQtAutoGen::RCC, rccFileAbs)) {
      // Compose rcc command
      std::vector<std::string> cmd;
      cmd.push_back(this->RccExecutable);
      cmd.insert(cmd.end(), rccJob.Options.begin(), rccJob.Options.end());
      cmd.push_back("-o");
      cmd.push_back(rccFileAbs);
      cmd.push_back(rccJob.QrcFile);

      std::string output;
      if (this->RunCommand(cmd, output)) {
        // Success
        rccGenerated = true;
      } else {
        this->LogCommandError(cmQtAutoGen::RCC, "rcc failed for\n  " +
                                cmQtAutoGen::Quoted(rccJob.QrcFile),
                              cmd, output);
        cmSystemTools::RemoveFile(rccFileAbs);
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
    const std::string& wrapperFileAbs = rccJob.RccFile;
    const std::string wrapperFileRel = cmSystemTools::RelativePath(
      this->AutogenBuildDir.c_str(), wrapperFileAbs.c_str());
    // Wrapper file content
    std::string content = "// This is an autogenerated configuration "
                          "wrapper file. Do not edit.\n"
                          "#include \"";
    content += cmSystemTools::GetFilenameName(rccFileRel);
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
        this->LogInfo(cmQtAutoGen::RCC,
                      "Touching RCC wrapper " + wrapperFileRel);
      }
      cmSystemTools::Touch(wrapperFileAbs, false);
    }
  }

  return rccGenerated;
}

void cmQtAutoGenerators::LogBold(const std::string& message) const
{
  cmSystemTools::MakefileColorEcho(cmsysTerminal_Color_ForegroundBlue |
                                     cmsysTerminal_Color_ForegroundBold,
                                   message.c_str(), true, this->ColorOutput);
}

void cmQtAutoGenerators::LogInfo(cmQtAutoGen::GeneratorType genType,
                                 const std::string& message) const
{
  std::string msg = cmQtAutoGen::GeneratorName(genType);
  msg += ": ";
  msg += message;
  if (msg.back() != '\n') {
    msg.push_back('\n');
  }
  cmSystemTools::Stdout(msg.c_str(), msg.size());
}

void cmQtAutoGenerators::LogWarning(cmQtAutoGen::GeneratorType genType,
                                    const std::string& message) const
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
  msg += "\n";
  cmSystemTools::Stdout(msg.c_str(), msg.size());
}

void cmQtAutoGenerators::LogFileWarning(cmQtAutoGen::GeneratorType genType,
                                        const std::string& filename,
                                        const std::string& message) const
{
  std::string emsg = "  ";
  emsg += cmQtAutoGen::Quoted(filename);
  emsg += "\n";
  // Message
  emsg += message;
  this->LogWarning(genType, emsg);
}

void cmQtAutoGenerators::LogError(cmQtAutoGen::GeneratorType genType,
                                  const std::string& message) const
{
  std::string msg;
  msg.push_back('\n');
  msg = cmQtAutoGen::GeneratorName(genType);
  msg += " error:";
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
  msg += "\n";
  cmSystemTools::Stderr(msg.c_str(), msg.size());
}

void cmQtAutoGenerators::LogFileError(cmQtAutoGen::GeneratorType genType,
                                      const std::string& filename,
                                      const std::string& message) const
{
  std::string emsg = "  ";
  emsg += cmQtAutoGen::Quoted(filename);
  emsg += "\n";
  // Message
  emsg += message;
  this->LogError(genType, emsg);
}

void cmQtAutoGenerators::LogNameCollisionError(
  cmQtAutoGen::GeneratorType genType, const std::string& message,
  const std::multimap<std::string, std::string>& collisions) const
{
  std::string emsg;
  // Add message
  if (!message.empty()) {
    emsg += message;
    if (emsg.back() != '\n') {
      emsg.push_back('\n');
    }
  }
  // Append collision list
  for (const auto& item : collisions) {
    emsg += "  ";
    emsg += item.first;
    emsg += " -> ";
    emsg += item.second;
    emsg += "\n";
  }
  this->LogError(genType, emsg);
}

void cmQtAutoGenerators::LogCommandError(
  cmQtAutoGen::GeneratorType genType, const std::string& message,
  const std::vector<std::string>& command, const std::string& output) const
{
  std::string msg;
  msg.push_back('\n');
  msg += cmQtAutoGen::GeneratorName(genType);
  msg += " subprocess error: ";
  msg += message;
  if (msg.back() != '\n') {
    msg.push_back('\n');
  }
  msg.push_back('\n');
  msg += "Command\n";
  msg += "-------\n";
  msg += QuotedCommand(command);
  if (msg.back() != '\n') {
    msg.push_back('\n');
  }
  msg.push_back('\n');
  msg += "Output\n";
  msg += "------\n";
  msg += output;
  if (msg.back() != '\n') {
    msg.push_back('\n');
  }
  msg += "\n";
  cmSystemTools::Stderr(msg.c_str(), msg.size());
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
      this->LogFileError(genType, filename,
                         "Could not create parent directory");
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
bool cmQtAutoGenerators::RunCommand(const std::vector<std::string>& command,
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
