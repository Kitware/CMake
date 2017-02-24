/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGenerators.h"

#include <algorithm>
#include <assert.h>
#include <cmConfigure.h>
#include <cmsys/FStream.hxx>
#include <cmsys/Terminal.h>
#include <list>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <utility>

#include "cmAlgorithms.h"
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

static const char* SettingsKeyMoc = "AM_MOC_OLD_SETTINGS";
static const char* SettingsKeyUic = "AM_UIC_OLD_SETTINGS";
static const char* SettingsKeyRcc = "AM_RCC_OLD_SETTINGS";

// -- Static functions

/**
 * @brief Returns a the string escaped and enclosed in quotes
 */
static std::string Quoted(const std::string& text)
{
  static const char* rep[18] = { "\\", "\\\\", "\"", "\\\"", "\a", "\\a",
                                 "\b", "\\b",  "\f", "\\f",  "\n", "\\n",
                                 "\r", "\\r",  "\t", "\\t",  "\v", "\\v" };

  std::string res = text;
  for (const char* const* it = cmArrayBegin(rep); it != cmArrayEnd(rep);
       it += 2) {
    cmSystemTools::ReplaceString(res, *it, *(it + 1));
  }
  res = '"' + res;
  res += '"';
  return res;
}

static std::string GetConfigDefinition(cmMakefile* makefile,
                                       const std::string& key,
                                       const std::string& config)
{
  std::string keyConf = key;
  if (!config.empty()) {
    keyConf += "_";
    keyConf += config;
  }
  const char* valueConf = makefile->GetDefinition(keyConf);
  if (valueConf != CM_NULLPTR) {
    return valueConf;
  }
  return makefile->GetSafeDefinition(key);
}

static std::string SettingsFile(const std::string& targetDirectory)
{
  std::string filename(cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutogenOldSettings.cmake";
  return filename;
}

inline static bool SettingsMatch(cmMakefile* makefile, const char* key,
                                 const std::string& value)
{
  return (value == makefile->GetSafeDefinition(key));
}

static void SettingWrite(std::ostream& ostr, const char* key,
                         const std::string& value)
{
  if (!value.empty()) {
    ostr << "set(" << key << " " << cmOutputConverter::EscapeForCMake(value)
         << ")\n";
  }
}

static bool FileNameIsUnique(const std::string& filePath,
                             const std::map<std::string, std::string>& fileMap)
{
  size_t count(0);
  const std::string fileName = cmsys::SystemTools::GetFilenameName(filePath);
  for (std::map<std::string, std::string>::const_iterator si = fileMap.begin();
       si != fileMap.end(); ++si) {
    if (cmsys::SystemTools::GetFilenameName(si->first) == fileName) {
      ++count;
      if (count > 1) {
        return false;
      }
    }
  }
  return true;
}

static std::string ReadAll(const std::string& filename)
{
  cmsys::ifstream file(filename.c_str());
  std::ostringstream stream;
  stream << file.rdbuf();
  file.close();
  return stream.str();
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
    cmsys::SystemTools::FileTimeCompare(buildFile, sourceFile, &result);
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
  for (std::map<std::string, std::string>::const_iterator it = opts.begin();
       it != opts.end(); ++it) {
    if (it != opts.begin()) {
      result += "@list_sep@";
    }
    result += it->first;
    result += "===";
    result += it->second;
  }
  return result;
}

static std::string JoinExts(const std::vector<std::string>& lst)
{
  std::string result;
  if (!lst.empty()) {
    const std::string separator = ",";
    for (std::vector<std::string>::const_iterator it = lst.begin();
         it != lst.end(); ++it) {
      if (it != lst.begin()) {
        result += separator;
      }
      result += '.';
      result += *it;
    }
  }
  return result;
}

static void UicMergeOptions(std::vector<std::string>& opts,
                            const std::vector<std::string>& fileOpts,
                            bool isQt5)
{
  static const char* valueOptions[] = { "tr",      "translate",
                                        "postfix", "generator",
                                        "include", // Since Qt 5.3
                                        "g" };
  std::vector<std::string> extraOpts;
  for (std::vector<std::string>::const_iterator it = fileOpts.begin();
       it != fileOpts.end(); ++it) {
    std::vector<std::string>::iterator existingIt =
      std::find(opts.begin(), opts.end(), *it);
    if (existingIt != opts.end()) {
      const char* o = it->c_str();
      if (*o == '-') {
        ++o;
      }
      if (isQt5 && *o == '-') {
        ++o;
      }
      if (std::find_if(cmArrayBegin(valueOptions), cmArrayEnd(valueOptions),
                       cmStrCmp(*it)) != cmArrayEnd(valueOptions)) {
        assert(existingIt + 1 != opts.end());
        *(existingIt + 1) = *(it + 1);
        ++it;
      }
    } else {
      extraOpts.push_back(*it);
    }
  }
  opts.insert(opts.end(), extraOpts.begin(), extraOpts.end());
}

// -- Class methods

cmQtAutoGenerators::cmQtAutoGenerators()
  : Verbose(cmsys::SystemTools::HasEnv("VERBOSE"))
  , ColorOutput(true)
  , RunMocFailed(false)
  , RunUicFailed(false)
  , RunRccFailed(false)
  , GenerateAllMoc(false)
  , GenerateAllUic(false)
  , GenerateAllRcc(false)
{

  std::string colorEnv;
  cmsys::SystemTools::GetEnv("COLOR", colorEnv);
  if (!colorEnv.empty()) {
    if (cmSystemTools::IsOn(colorEnv.c_str())) {
      this->ColorOutput = true;
    } else {
      this->ColorOutput = false;
    }
  }

  this->MacroFilters[0].first = "Q_OBJECT";
  this->MacroFilters[0].second.compile("[\n][ \t]*Q_OBJECT[^a-zA-Z0-9_]");
  this->MacroFilters[1].first = "Q_GADGET";
  this->MacroFilters[1].second.compile("[\n][ \t]*Q_GADGET[^a-zA-Z0-9_]");

  // Precompile regular expressions
  this->RegExpMocInclude.compile(
    "[\n][ \t]*#[ \t]*include[ \t]+"
    "[\"<](([^ \">]+/)?moc_[^ \">/]+\\.cpp|[^ \">]+\\.moc)[\">]");
  this->RegExpUicInclude.compile("[\n][ \t]*#[ \t]*include[ \t]+"
                                 "[\"<](([^ \">]+/)?ui_[^ \">/]+\\.h)[\">]");
}

bool cmQtAutoGenerators::Run(const std::string& targetDirectory,
                             const std::string& config)
{
  cmake cm;
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
    this->SettingsFileRead(mf.get(), targetDirectory);
    // Init and run
    this->Init(mf.get());
    if (this->RunAutogen()) {
      // Write current settings
      if (this->SettingsFileWrite(targetDirectory)) {
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
      MocDependFilter filter;
      filter.key = key;
      if (filter.regExp.compile(regExp)) {
        this->MocDependFilters.push_back(filter);
        success = true;
      } else {
        this->LogError("AutoMoc: Error in AUTOMOC_DEPEND_FILTERS: Compiling "
                       "regular expression failed.\nKey:  " +
                       Quoted(key) + "\nExp.: " + Quoted(regExp));
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

  // - Target names
  this->OriginTargetName =
    makefile->GetSafeDefinition("AM_ORIGIN_TARGET_NAME");
  this->AutogenTargetName = makefile->GetSafeDefinition("AM_TARGET_NAME");

  // - Directories
  this->ProjectSourceDir = makefile->GetSafeDefinition("AM_CMAKE_SOURCE_DIR");
  this->ProjectBinaryDir = makefile->GetSafeDefinition("AM_CMAKE_BINARY_DIR");
  this->CurrentSourceDir =
    makefile->GetSafeDefinition("AM_CMAKE_CURRENT_SOURCE_DIR");
  this->CurrentBinaryDir =
    makefile->GetSafeDefinition("AM_CMAKE_CURRENT_BINARY_DIR");

  // - Qt environment
  this->QtMajorVersion = makefile->GetSafeDefinition("AM_QT_VERSION_MAJOR");
  if (this->QtMajorVersion == "") {
    this->QtMajorVersion =
      makefile->GetSafeDefinition("AM_Qt5Core_VERSION_MAJOR");
  }
  // Check Qt version
  if ((this->QtMajorVersion != "4") && (this->QtMajorVersion != "5")) {
    this->LogError("AutoGen: Error: Unsupported Qt version: " +
                   Quoted(this->QtMajorVersion));
    return false;
  }

  this->MocExecutable = makefile->GetSafeDefinition("AM_QT_MOC_EXECUTABLE");
  this->UicExecutable = makefile->GetSafeDefinition("AM_QT_UIC_EXECUTABLE");
  this->RccExecutable = makefile->GetSafeDefinition("AM_QT_RCC_EXECUTABLE");

  // - File Lists
  cmSystemTools::ExpandListArgument(makefile->GetSafeDefinition("AM_SOURCES"),
                                    this->Sources);
  cmSystemTools::ExpandListArgument(makefile->GetSafeDefinition("AM_HEADERS"),
                                    this->Headers);

  // - Moc
  cmSystemTools::ExpandListArgument(makefile->GetSafeDefinition("AM_MOC_SKIP"),
                                    this->MocSkipList);
  cmSystemTools::ExpandListArgument(
    GetConfigDefinition(makefile, "AM_MOC_COMPILE_DEFINITIONS", config),
    this->MocDefinitions);
  cmSystemTools::ExpandListArgument(
    GetConfigDefinition(makefile, "AM_MOC_INCLUDES", config),
    this->MocIncludePaths);
  cmSystemTools::ExpandListArgument(
    makefile->GetSafeDefinition("AM_MOC_OPTIONS"), this->MocOptions);
  {
    std::vector<std::string> mocDependFilters;
    cmSystemTools::ExpandListArgument(
      makefile->GetSafeDefinition("AM_MOC_DEPEND_FILTERS"), mocDependFilters);
    // Insert Q_PLUGIN_METADATA dependency filter
    if (this->QtMajorVersion != "4") {
      this->MocDependFilterPush("Q_PLUGIN_METADATA",
                                "[\n][ \t]*Q_PLUGIN_METADATA[ \t]*\\("
                                "[^\\)]*FILE[ \t]*\"([^\"]+)\"");
    }
    // Insert user defined dependency filters
    if ((mocDependFilters.size() % 2) == 0) {
      for (std::vector<std::string>::const_iterator dit =
             mocDependFilters.begin();
           dit != mocDependFilters.end(); dit += 2) {
        if (!this->MocDependFilterPush(*dit, *(dit + 1))) {
          return false;
        }
      }
    } else {
      this->LogError("AutoMoc: Error: AUTOMOC_DEPEND_FILTERS list size is not "
                     "a multiple of 2");
    }
  }

  // - Uic
  cmSystemTools::ExpandListArgument(makefile->GetSafeDefinition("AM_UIC_SKIP"),
                                    this->UicSkipList);
  cmSystemTools::ExpandListArgument(
    GetConfigDefinition(makefile, "AM_UIC_TARGET_OPTIONS", config),
    this->UicTargetOptions);
  {
    std::vector<std::string> uicFilesVec;
    std::vector<std::string> uicOptionsVec;
    cmSystemTools::ExpandListArgument(
      makefile->GetSafeDefinition("AM_UIC_OPTIONS_FILES"), uicFilesVec);
    cmSystemTools::ExpandListArgument(
      makefile->GetSafeDefinition("AM_UIC_OPTIONS_OPTIONS"), uicOptionsVec);
    if (uicFilesVec.size() != uicOptionsVec.size()) {
      this->LogError(
        "AutoGen: Error: Uic files/options lists size missmatch in: " +
        filename);
      return false;
    }
    for (std::vector<std::string>::iterator fileIt = uicFilesVec.begin(),
                                            optionIt = uicOptionsVec.begin();
         fileIt != uicFilesVec.end(); ++fileIt, ++optionIt) {
      cmSystemTools::ReplaceString(*optionIt, "@list_sep@", ";");
      this->UicOptions[*fileIt] = *optionIt;
    }
  }

  // - Rcc
  cmSystemTools::ExpandListArgument(
    makefile->GetSafeDefinition("AM_RCC_SOURCES"), this->RccSources);
  {
    std::vector<std::string> rccFilesVec;
    std::vector<std::string> rccOptionsVec;
    cmSystemTools::ExpandListArgument(
      makefile->GetSafeDefinition("AM_RCC_OPTIONS_FILES"), rccFilesVec);
    cmSystemTools::ExpandListArgument(
      makefile->GetSafeDefinition("AM_RCC_OPTIONS_OPTIONS"), rccOptionsVec);
    if (rccFilesVec.size() != rccOptionsVec.size()) {
      this->LogError(
        "AutoGen: Error: RCC files/options lists size missmatch in: " +
        filename);
      return false;
    }
    for (std::vector<std::string>::iterator fileIt = rccFilesVec.begin(),
                                            optionIt = rccOptionsVec.begin();
         fileIt != rccFilesVec.end(); ++fileIt, ++optionIt) {
      cmSystemTools::ReplaceString(*optionIt, "@list_sep@", ";");
      this->RccOptions[*fileIt] = *optionIt;
    }
  }
  {
    std::vector<std::string> rccInputLists;
    cmSystemTools::ExpandListArgument(
      makefile->GetSafeDefinition("AM_RCC_INPUTS"), rccInputLists);

    // qrc files in the end of the list may have been empty
    if (rccInputLists.size() < this->RccSources.size()) {
      rccInputLists.resize(this->RccSources.size());
    }
    if (this->RccSources.size() != rccInputLists.size()) {
      this->LogError(
        "AutoGen: Error: RCC sources/inputs lists size missmatch in: " +
        filename);
      return false;
    }
    for (std::vector<std::string>::iterator fileIt = this->RccSources.begin(),
                                            inputIt = rccInputLists.begin();
         fileIt != this->RccSources.end(); ++fileIt, ++inputIt) {
      cmSystemTools::ReplaceString(*inputIt, "@list_sep@", ";");
      std::vector<std::string> rccInputFiles;
      cmSystemTools::ExpandListArgument(*inputIt, rccInputFiles);
      this->RccInputs[*fileIt] = rccInputFiles;
    }
  }

  // - Flags
  this->IncludeProjectDirsBefore =
    makefile->IsOn("AM_CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE");
  this->MocRelaxedMode = makefile->IsOn("AM_MOC_RELAXED_MODE");

  return true;
}

void cmQtAutoGenerators::SettingsFileRead(cmMakefile* makefile,
                                          const std::string& targetDirectory)
{
  // Compose current settings strings
  if (this->MocEnabled()) {
    std::string& str = this->SettingsStringMoc;
    str += JoinOptionsList(this->MocDefinitions);
    str += " ~~~ ";
    str += JoinOptionsList(this->MocIncludePaths);
    str += " ~~~ ";
    str += JoinOptionsList(this->MocOptions);
    str += " ~~~ ";
    str += this->IncludeProjectDirsBefore ? "TRUE" : "FALSE";
    str += " ~~~ ";
  }
  if (this->UicEnabled()) {
    std::string& str = this->SettingsStringUic;
    str += JoinOptionsList(this->UicTargetOptions);
    str += " ~~~ ";
    str += JoinOptionsMap(this->UicOptions);
    str += " ~~~ ";
  }
  if (this->RccEnabled()) {
    std::string& str = this->SettingsStringRcc;
    str += JoinOptionsMap(this->RccOptions);
    str += " ~~~ ";
  }

  // Read old settings
  const std::string filename = SettingsFile(targetDirectory);
  if (makefile->ReadListFile(filename.c_str())) {
    if (!SettingsMatch(makefile, SettingsKeyMoc, this->SettingsStringMoc)) {
      this->GenerateAllMoc = true;
    }
    if (!SettingsMatch(makefile, SettingsKeyUic, this->SettingsStringUic)) {
      this->GenerateAllUic = true;
    }
    if (!SettingsMatch(makefile, SettingsKeyRcc, this->SettingsStringRcc)) {
      this->GenerateAllRcc = true;
    }
    // In case any setting changed remove the old settings file.
    // This triggers a full rebuild on the next run if the current
    // build is aborted before writing the current settings in the end.
    if (this->GenerateAllAny()) {
      cmSystemTools::RemoveFile(filename);
    }
  } else {
    // If the file could not be read re-generate everythiung.
    this->GenerateAllMoc = true;
    this->GenerateAllUic = true;
    this->GenerateAllRcc = true;
  }
}

bool cmQtAutoGenerators::SettingsFileWrite(const std::string& targetDirectory)
{
  bool success = true;
  // Only write if any setting changed
  if (this->GenerateAllAny()) {
    const std::string filename = SettingsFile(targetDirectory);
    if (this->Verbose) {
      this->LogInfo("AutoGen: Writing settings file " + filename);
    }
    cmsys::ofstream outfile;
    outfile.open(filename.c_str(), std::ios::trunc);
    if (outfile) {
      SettingWrite(outfile, SettingsKeyMoc, this->SettingsStringMoc);
      SettingWrite(outfile, SettingsKeyUic, this->SettingsStringUic);
      SettingWrite(outfile, SettingsKeyRcc, this->SettingsStringRcc);
      success = outfile.good();
      outfile.close();
    } else {
      success = false;
      // Remove old settings file to trigger full rebuild on next run
      cmSystemTools::RemoveFile(filename);
      this->LogError("AutoGen: Error: Writing old settings file failed: " +
                     filename);
    }
  }
  return success;
}

void cmQtAutoGenerators::Init(cmMakefile* makefile)
{
  this->AutogenBuildSubDir = this->AutogenTargetName;
  this->AutogenBuildSubDir += "/";

  this->MocCppFilenameRel = this->AutogenBuildSubDir;
  this->MocCppFilenameRel += "moc_compilation.cpp";

  this->MocCppFilenameAbs = this->CurrentBinaryDir + this->MocCppFilenameRel;

  // Init file path checksum generator
  fpathCheckSum.setupParentDirs(this->CurrentSourceDir, this->CurrentBinaryDir,
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
      const char* movePaths[2] = { this->ProjectBinaryDir.c_str(),
                                   this->ProjectSourceDir.c_str() };
      for (const char* const* mpit = cmArrayBegin(movePaths);
           mpit != cmArrayEnd(movePaths); ++mpit) {
        std::list<std::string>::iterator it = includes.begin();
        while (it != includes.end()) {
          const std::string& path = *it;
          if (cmsys::SystemTools::StringStartsWith(path, *mpit)) {
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
    for (std::vector<std::string>::const_iterator it =
           this->MocIncludePaths.begin();
         it != this->MocIncludePaths.end(); ++it) {
      const std::string& path = *it;
      this->MocIncludes.push_back("-I" + path);
      // Extract framework path
      if (cmHasLiteralSuffix(path, ".framework/Headers")) {
        // Go up twice to get to the framework root
        std::vector<std::string> pathComponents;
        cmsys::SystemTools::SplitPath(path, pathComponents);
        std::string frameworkPath = cmsys::SystemTools::JoinPath(
          pathComponents.begin(), pathComponents.end() - 2);
        frameworkPaths.insert(frameworkPath);
      }
    }
    // Append framework includes
    for (std::set<std::string>::const_iterator it = frameworkPaths.begin();
         it != frameworkPaths.end(); ++it) {
      this->MocIncludes.push_back("-F");
      this->MocIncludes.push_back(*it);
    }
  }
}

bool cmQtAutoGenerators::RunAutogen()
{
  // the program goes through all .cpp files to see which moc files are
  // included. It is not really interesting how the moc file is named, but
  // what file the moc is created from. Once a moc is included the same moc
  // may not be included in the moc_compilation.cpp file anymore. OTOH if
  // there's a header containing Q_OBJECT where no corresponding moc file
  // is included anywhere a moc_<filename>.cpp file is created and included
  // in the moc_compilation.cpp file.

  // key = moc source filepath, value = moc output filepath
  std::map<std::string, std::string> mocsIncluded;
  std::map<std::string, std::string> mocsNotIncluded;
  std::map<std::string, std::set<std::string> > mocDepends;
  std::map<std::string, std::vector<std::string> > uisIncluded;
  // collects all headers which may need to be mocced
  std::set<std::string> mocHeaderFiles;
  std::set<std::string> uicHeaderFiles;

  // Parse sources
  for (std::vector<std::string>::const_iterator it = this->Sources.begin();
       it != this->Sources.end(); ++it) {
    const std::string& absFilename = cmsys::SystemTools::GetRealPath(*it);
    // Parse source file for MOC/UIC
    if (!this->ParseSourceFile(absFilename, mocsIncluded, mocDepends,
                               uisIncluded, this->MocRelaxedMode)) {
      return false;
    }
    // Find additional headers
    this->SearchHeadersForSourceFile(absFilename, mocHeaderFiles,
                                     uicHeaderFiles);
  }

  // Parse headers
  for (std::vector<std::string>::const_iterator it = this->Headers.begin();
       it != this->Headers.end(); ++it) {
    const std::string& headerName = cmsys::SystemTools::GetRealPath(*it);
    if (!this->MocSkip(headerName)) {
      mocHeaderFiles.insert(headerName);
    }
    if (!this->UicSkip(headerName)) {
      uicHeaderFiles.insert(headerName);
    }
  }
  this->ParseHeaders(mocHeaderFiles, uicHeaderFiles, mocsIncluded,
                     mocsNotIncluded, mocDepends, uisIncluded);

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
  for (unsigned int ii = 0; ii != cmArraySize(this->MacroFilters); ++ii) {
    MacroFilter& filter = this->MacroFilters[ii];
    // Run a simple find string operation before the expensive
    // regular expression check
    if (contentText.find(filter.first) != std::string::npos) {
      if (filter.second.find(contentText)) {
        // Return macro name on demand
        if (macroName != CM_NULLPTR) {
          *macroName = filter.first;
        }
        return true;
      }
    }
  }
  return false;
}

void cmQtAutoGenerators::MocFindDepends(
  const std::string& absFilename, const std::string& contentText,
  std::map<std::string, std::set<std::string> >& mocDepends)
{
  for (std::vector<MocDependFilter>::iterator fit =
         this->MocDependFilters.begin();
       fit != this->MocDependFilters.end(); ++fit) {
    MocDependFilter& filter = *fit;
    // Run a simple find string operation before the expensive
    // regular expression check
    if (contentText.find(filter.key) != std::string::npos) {
      // Run regular expression check loop
      const char* contentChars = contentText.c_str();
      while (filter.regExp.find(contentChars)) {
        // Evaluate match
        const std::string match = filter.regExp.match(1);
        if (!match.empty()) {
          // Find the dependency file
          const std::string incFile =
            this->FindIncludedFile(absFilename, match);
          if (!incFile.empty()) {
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
        contentChars += filter.regExp.end();
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
  std::map<std::string, std::set<std::string> >& mocDepends,
  std::map<std::string, std::vector<std::string> >& uisIncluded, bool relaxed)
{
  bool success = true;
  const std::string contentText = ReadAll(absFilename);
  if (contentText.empty()) {
    std::ostringstream ost;
    ost << "AutoGen: Warning: " << absFilename << "\n"
        << "The file is empty\n";
    this->LogWarning(ost.str());
  } else {
    // Parse source contents for MOC
    if (success && !this->MocSkip(absFilename)) {
      success = this->MocParseSourceContent(absFilename, contentText,
                                            mocsIncluded, mocDepends, relaxed);
    }
    // Parse source contents for UIC
    if (success && !this->UicSkip(absFilename)) {
      this->UicParseContent(absFilename, contentText, uisIncluded);
    }
  }
  return success;
}

void cmQtAutoGenerators::UicParseContent(
  const std::string& absFilename, const std::string& contentText,
  std::map<std::string, std::vector<std::string> >& uisIncluded)
{
  if (this->Verbose) {
    this->LogInfo("AutoUic: Checking " + absFilename);
  }

  const char* contentChars = contentText.c_str();
  if (strstr(contentChars, "ui_") != CM_NULLPTR) {
    while (this->RegExpUicInclude.find(contentChars)) {
      const std::string currentUi = this->RegExpUicInclude.match(1);
      const std::string basename =
        cmsys::SystemTools::GetFilenameWithoutLastExtension(currentUi);
      // basename should be the part of the ui filename used for
      // finding the correct header, so we need to remove the ui_ part
      uisIncluded[absFilename].push_back(basename.substr(3));
      contentChars += this->RegExpUicInclude.end();
    }
  }
}

/**
 * @return True on success
 */
bool cmQtAutoGenerators::MocParseSourceContent(
  const std::string& absFilename, const std::string& contentText,
  std::map<std::string, std::string>& mocsIncluded,
  std::map<std::string, std::set<std::string> >& mocDepends, bool relaxed)
{
  if (this->Verbose) {
    this->LogInfo("AutoMoc: Checking " + absFilename);
  }

  const std::string scannedFileAbsPath =
    cmsys::SystemTools::GetFilenamePath(absFilename) + '/';
  const std::string scannedFileBasename =
    cmsys::SystemTools::GetFilenameWithoutLastExtension(absFilename);

  std::string macroName;
  const bool requiresMoc = this->MocRequired(contentText, &macroName);
  bool ownDotMocIncluded = false;
  std::string ownMocUnderscoreInclude;
  std::string ownMocUnderscoreHeader;

  // first a simple string check for "moc" is *much* faster than the regexp,
  // and if the string search already fails, we don't have to try the
  // expensive regexp
  const char* contentChars = contentText.c_str();
  if (strstr(contentChars, "moc") != CM_NULLPTR) {
    // Iterate over all included moc files
    while (this->RegExpMocInclude.find(contentChars)) {
      const std::string incString = this->RegExpMocInclude.match(1);
      // Basename of the moc include
      const std::string incBasename =
        cmsys::SystemTools::GetFilenameWithoutLastExtension(incString);
      std::string incSubDir;
      if (incString.find_first_of('/') != std::string::npos) {
        incSubDir = cmsys::SystemTools::GetFilenamePath(incString);
        incSubDir += '/';
      }

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
          this->FindMocHeader(scannedFileAbsPath, incRealBasename, incSubDir);
        if (!headerToMoc.empty()) {
          // Register moc job
          mocsIncluded[headerToMoc] = incString;
          this->MocFindDepends(headerToMoc, contentText, mocDepends);
          // Store meta information for relaxed mode
          if (relaxed && (incRealBasename == scannedFileBasename)) {
            ownMocUnderscoreInclude = incString;
            ownMocUnderscoreHeader = headerToMoc;
          }
        } else {
          std::ostringstream ost;
          ost << "AutoMoc: Error: " << absFilename << "\n"
              << "The file includes the moc file " << Quoted(incString)
              << ", but could not find header "
              << Quoted(incRealBasename + "{" +
                        JoinExts(this->HeaderExtensions) + "}");
          ;
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
              this->FindMocHeader(scannedFileAbsPath, incBasename, incSubDir);
            if (!headerToMoc.empty()) {
              // This is for KDE4 compatibility:
              fileToMoc = headerToMoc;
              if (!requiresMoc && (incBasename == scannedFileBasename)) {
                std::ostringstream ost;
                ost << "AutoMoc: Warning: " << Quoted(absFilename) << "\n"
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
                    << " instead of " << Quoted("moc_" + incBasename + ".cpp")
                    << ".\n"
                    << "Running moc on " << Quoted(headerToMoc) << "!\n"
                    << "Include " << Quoted("moc_" + incBasename + ".cpp")
                    << " for compatibility with strict mode (see "
                       "CMAKE_AUTOMOC_RELAXED_MODE).\n";
                this->LogWarning(ost.str());
              }
            } else {
              std::ostringstream ost;
              ost << "AutoMoc: Error: " << Quoted(absFilename) << "\n"
                  << "The file includes the moc file " << Quoted(incString)
                  << ". which seems to be the moc file from a different "
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
              ost << "AutoMoc: Error: " << Quoted(absFilename) << "\n"
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
      contentChars += this->RegExpMocInclude.end();
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
  std::map<std::string, std::set<std::string> >& mocDepends)
{
  // Log
  if (this->Verbose) {
    this->LogInfo("AutoMoc: Checking " + absFilename);
  }
  if (this->MocRequired(contentText)) {
    // Register moc job
    mocsNotIncluded[absFilename] =
      this->ChecksumedPath(absFilename, "moc_", ".cpp");
    this->MocFindDepends(absFilename, contentText, mocDepends);
  }
}

void cmQtAutoGenerators::SearchHeadersForSourceFile(
  const std::string& absFilename, std::set<std::string>& mocHeaderFiles,
  std::set<std::string>& uicHeaderFiles) const
{
  std::string basepaths[2];
  {
    std::string bpath = cmsys::SystemTools::GetFilenamePath(absFilename);
    bpath += '/';
    bpath += cmsys::SystemTools::GetFilenameWithoutLastExtension(absFilename);
    // search for default header files and private header files
    basepaths[0] = bpath;
    basepaths[1] = bpath + "_p";
  }

  for (const std::string* bpit = cmArrayBegin(basepaths);
       bpit != cmArrayEnd(basepaths); ++bpit) {
    std::string headerName;
    if (this->FindHeader(headerName, *bpit)) {
      // Moc headers
      if (!this->MocSkip(absFilename) && !this->MocSkip(headerName)) {
        mocHeaderFiles.insert(headerName);
      }
      // Uic headers
      if (!this->UicSkip(absFilename) && !this->UicSkip(headerName)) {
        uicHeaderFiles.insert(headerName);
      }
      break;
    }
  }
}

void cmQtAutoGenerators::ParseHeaders(
  const std::set<std::string>& mocHeaderFiles,
  const std::set<std::string>& uicHeaderFiles,
  const std::map<std::string, std::string>& mocsIncluded,
  std::map<std::string, std::string>& mocsNotIncluded,
  std::map<std::string, std::set<std::string> >& mocDepends,
  std::map<std::string, std::vector<std::string> >& uisIncluded)
{
  // Merged header files list to read files only once
  std::set<std::string> headerFiles;
  headerFiles.insert(mocHeaderFiles.begin(), mocHeaderFiles.end());
  headerFiles.insert(uicHeaderFiles.begin(), uicHeaderFiles.end());

  for (std::set<std::string>::const_iterator hIt = headerFiles.begin();
       hIt != headerFiles.end(); ++hIt) {
    const std::string& headerName = *hIt;
    const std::string contentText = ReadAll(headerName);

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
  }
}

bool cmQtAutoGenerators::MocGenerateAll(
  const std::map<std::string, std::string>& mocsIncluded,
  const std::map<std::string, std::string>& mocsNotIncluded,
  const std::map<std::string, std::set<std::string> >& mocDepends)
{
  if (!this->MocEnabled()) {
    return true;
  }

  bool mocCompFileGenerated = false;
  bool mocCompChanged = false;

  // look for name collisions
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
  // Generate moc files that are included by source files.
  {
    const std::string subDir = "include/";
    for (std::map<std::string, std::string>::const_iterator it =
           mocsIncluded.begin();
         it != mocsIncluded.end(); ++it) {
      if (!this->MocGenerateFile(it->first, it->second, subDir, mocDepends)) {
        if (this->RunMocFailed) {
          return false;
        }
      }
    }
  }
  // Generate moc files that are _not_ included by source files.
  {
    const std::string subDir;
    for (std::map<std::string, std::string>::const_iterator it =
           mocsNotIncluded.begin();
         it != mocsNotIncluded.end(); ++it) {
      if (this->MocGenerateFile(it->first, it->second, subDir, mocDepends)) {
        mocCompFileGenerated = true;
      } else {
        if (this->RunMocFailed) {
          return false;
        }
      }
    }
  }

  // Compose moc_compilation.cpp content
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

  // Check if the content of moc_compilation.cpp changed
  {
    const std::string oldContents = ReadAll(this->MocCppFilenameAbs);
    mocCompChanged = (oldContents != automocSource);
  }

  bool success = true;
  if (mocCompChanged) {
    // Actually write moc_compilation.cpp
    this->LogBold("Generating MOC compilation " + this->MocCppFilenameRel);

    // Make sure the parent directory exists
    success = this->MakeParentDirectory(this->MocCppFilenameAbs);
    if (success) {
      cmsys::ofstream outfile;
      outfile.open(this->MocCppFilenameAbs.c_str(), std::ios::trunc);
      if (!outfile) {
        success = false;
        this->LogError("AutoMoc: Error opening " + this->MocCppFilenameAbs);
      } else {
        outfile << automocSource;
        // Check for write errors
        if (!outfile.good()) {
          success = false;
          this->LogError("AutoMoc: Error writing " + this->MocCppFilenameAbs);
        }
      }
    }
  } else if (mocCompFileGenerated) {
    // Only touch moc_compilation.cpp
    if (this->Verbose) {
      this->LogInfo("Touching MOC compilation " + this->MocCppFilenameRel);
    }
    cmSystemTools::Touch(this->MocCppFilenameAbs, false);
  }

  return success;
}

/**
 * @return True if a moc file was created. False may indicate an error.
 */
bool cmQtAutoGenerators::MocGenerateFile(
  const std::string& sourceFile, const std::string& mocFileName,
  const std::string& subDirPrefix,
  const std::map<std::string, std::set<std::string> >& mocDepends)
{
  bool mocGenerated = false;
  bool generateMoc = this->GenerateAllMoc;

  const std::string mocFileRel =
    this->AutogenBuildSubDir + subDirPrefix + mocFileName;
  const std::string mocFileAbs = this->CurrentBinaryDir + mocFileRel;

  if (!generateMoc) {
    // Test if the source file is newer that the build file
    generateMoc = FileAbsentOrOlder(mocFileAbs, sourceFile);
    if (!generateMoc) {
      // Test if a dependency file changed
      std::map<std::string, std::set<std::string> >::const_iterator dit =
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
    this->LogBold("Generating MOC source " + mocFileRel);

    // Make sure the parent directory exists
    if (this->MakeParentDirectory(mocFileAbs)) {
      // Compose moc command
      std::vector<std::string> cmd;
      cmd.push_back(this->MocExecutable);
      cmd.insert(cmd.end(), this->MocIncludes.begin(),
                 this->MocIncludes.end());
      // Add definitions
      for (std::vector<std::string>::const_iterator it =
             this->MocDefinitions.begin();
           it != this->MocDefinitions.end(); ++it) {
        cmd.push_back("-D" + (*it));
      }
      cmd.insert(cmd.end(), this->MocOptions.begin(), this->MocOptions.end());
#ifdef _WIN32
      cmd.push_back("-DWIN32");
#endif
      cmd.push_back("-o");
      cmd.push_back(mocFileAbs);
      cmd.push_back(sourceFile);

      // Log moc command
      if (this->Verbose) {
        this->LogCommand(cmd);
      }

      // Execute moc command
      bool res = false;
      int retVal = 0;
      std::string output;
      res = cmSystemTools::RunSingleCommand(cmd, &output, &output, &retVal);

      if (!res || (retVal != 0)) {
        // Command failed
        {
          std::ostringstream ost;
          ost << "AutoMoc: Error: moc process failed for\n";
          ost << Quoted(mocFileRel) << "\n";
          ost << "AutoMoc: Command:\n" << cmJoin(cmd, " ") << "\n";
          ost << "AutoMoc: Command output:\n" << output << "\n";
          this->LogError(ost.str());
        }
        cmSystemTools::RemoveFile(mocFileAbs);
        this->RunMocFailed = true;
      } else {
        // Success
        mocGenerated = true;
      }
    } else {
      // Parent directory creation failed
      this->RunMocFailed = true;
    }
  }
  return mocGenerated;
}

bool cmQtAutoGenerators::UicGenerateAll(
  const std::map<std::string, std::vector<std::string> >& uisIncluded)
{
  if (!this->UicEnabled()) {
    return true;
  }

  // single map with input / output names
  std::map<std::string, std::map<std::string, std::string> > uiGenMap;
  std::map<std::string, std::string> testMap;
  for (std::map<std::string, std::vector<std::string> >::const_iterator it =
         uisIncluded.begin();
       it != uisIncluded.end(); ++it) {
    // source file path
    std::string sourcePath = cmsys::SystemTools::GetFilenamePath(it->first);
    sourcePath += '/';
    // insert new map for source file an use new reference
    uiGenMap[it->first] = std::map<std::string, std::string>();
    std::map<std::string, std::string>& sourceMap = uiGenMap[it->first];
    for (std::vector<std::string>::const_iterator sit = it->second.begin();
         sit != it->second.end(); ++sit) {
      const std::string& uiFileName = *sit;
      const std::string uiInputFile = sourcePath + uiFileName + ".ui";
      const std::string uiOutputFile = "ui_" + uiFileName + ".h";
      sourceMap[uiInputFile] = uiOutputFile;
      testMap[uiInputFile] = uiOutputFile;
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
  testMap.clear();

  // generate ui files
  for (std::map<std::string,
                std::map<std::string, std::string> >::const_iterator it =
         uiGenMap.begin();
       it != uiGenMap.end(); ++it) {
    for (std::map<std::string, std::string>::const_iterator sit =
           it->second.begin();
         sit != it->second.end(); ++sit) {
      if (!this->UicGenerateFile(it->first, sit->first, sit->second)) {
        if (this->RunUicFailed) {
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
  bool generateUic = this->GenerateAllUic;

  const std::string uicFileRel =
    this->AutogenBuildSubDir + "include/" + uiOutputFile;
  const std::string uicFileAbs = this->CurrentBinaryDir + uicFileRel;

  if (!generateUic) {
    // Test if the source file is newer that the build file
    generateUic = FileAbsentOrOlder(uicFileAbs, uiInputFile);
  }
  if (generateUic) {
    // Log
    this->LogBold("Generating UIC header " + uicFileRel);

    // Make sure the parent directory exists
    if (this->MakeParentDirectory(uicFileAbs)) {
      // Compose uic command
      std::vector<std::string> cmd;
      cmd.push_back(this->UicExecutable);
      {
        std::vector<std::string> opts = this->UicTargetOptions;
        std::map<std::string, std::string>::const_iterator optionIt =
          this->UicOptions.find(uiInputFile);
        if (optionIt != this->UicOptions.end()) {
          std::vector<std::string> fileOpts;
          cmSystemTools::ExpandListArgument(optionIt->second, fileOpts);
          UicMergeOptions(opts, fileOpts, (this->QtMajorVersion == "5"));
        }
        cmd.insert(cmd.end(), opts.begin(), opts.end());
      }
      cmd.push_back("-o");
      cmd.push_back(uicFileAbs);
      cmd.push_back(uiInputFile);

      // Log command
      if (this->Verbose) {
        this->LogCommand(cmd);
      }

      // Execute command
      bool res = false;
      int retVal = 0;
      std::string output;
      res = cmSystemTools::RunSingleCommand(cmd, &output, &output, &retVal);

      if (!res || (retVal != 0)) {
        // Command failed
        {
          std::ostringstream ost;
          ost << "AutoUic: Error: uic process failed for\n";
          ost << Quoted(uicFileRel) << " needed by\n";
          ost << Quoted(realName) << "\n";
          ost << "AutoUic: Command:\n" << cmJoin(cmd, " ") << "\n";
          ost << "AutoUic: Command output:\n" << output << "\n";
          this->LogError(ost.str());
        }
        cmSystemTools::RemoveFile(uicFileAbs);
        this->RunUicFailed = true;
      } else {
        // Success
        uicGenerated = true;
      }
    } else {
      // Parent directory creation failed
      this->RunUicFailed = true;
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
  for (std::vector<std::string>::const_iterator si = this->RccSources.begin();
       si != this->RccSources.end(); ++si) {
    const std::string ext = cmsys::SystemTools::GetFilenameLastExtension(*si);
    if (ext == ".qrc") {
      qrcGenMap[*si] =
        this->AutogenBuildSubDir + this->ChecksumedPath(*si, "qrc_", ".cpp");
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
  for (std::map<std::string, std::string>::const_iterator si =
         qrcGenMap.begin();
       si != qrcGenMap.end(); ++si) {
    bool unique = FileNameIsUnique(si->first, qrcGenMap);
    if (!this->RccGenerateFile(si->first, si->second, unique)) {
      if (this->RunRccFailed) {
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
  bool generateRcc = this->GenerateAllRcc;

  const std::string rccBuildFile = this->CurrentBinaryDir + rccOutputFile;

  if (!generateRcc) {
    // Test if the resources list file is newer than build file
    generateRcc = FileAbsentOrOlder(rccBuildFile, rccInputFile);
    if (!generateRcc) {
      // Test if any resource file is newer than the build file
      const std::vector<std::string>& files = this->RccInputs[rccInputFile];
      for (std::vector<std::string>::const_iterator it = files.begin();
           it != files.end(); ++it) {
        if (FileAbsentOrOlder(rccBuildFile, *it)) {
          generateRcc = true;
          break;
        }
      }
    }
  }
  if (generateRcc) {
    // Log
    this->LogBold("Generating RCC source " + rccOutputFile);

    // Make sure the parent directory exists
    if (this->MakeParentDirectory(rccBuildFile)) {
      // Compose symbol name
      std::string symbolName =
        cmsys::SystemTools::GetFilenameWithoutLastExtension(rccInputFile);
      if (!unique_n) {
        symbolName += "_";
        symbolName += fpathCheckSum.getPart(rccInputFile);
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

      // Log command
      if (this->Verbose) {
        this->LogCommand(cmd);
      }

      // Execute command
      bool res = false;
      int retVal = 0;
      std::string output;
      res = cmSystemTools::RunSingleCommand(cmd, &output, &output, &retVal);
      if (!res || (retVal != 0)) {
        // Command failed
        {
          std::ostringstream ost;
          ost << "AutoRcc: Error: rcc process failed for\n";
          ost << Quoted(rccOutputFile) << "\n";
          ost << "AutoRcc: Command:\n" << cmJoin(cmd, " ") << "\n";
          ost << "AutoRcc: Command output:\n" << output << "\n";
          this->LogError(ost.str());
        }
        cmSystemTools::RemoveFile(rccBuildFile);
        this->RunRccFailed = true;
      } else {
        // Success
        rccGenerated = true;
      }
    } else {
      // Parent directory creation failed
      this->RunRccFailed = true;
    }
  }
  return rccGenerated;
}

void cmQtAutoGenerators::LogErrorNameCollision(
  const std::string& message,
  const std::multimap<std::string, std::string>& collisions) const
{
  typedef std::multimap<std::string, std::string>::const_iterator Iter;

  std::ostringstream ost;
  // Add message
  if (!message.empty()) {
    ost << message;
    if (message[message.size() - 1] != '\n') {
      ost << '\n';
    }
  }
  // Append collision list
  for (Iter it = collisions.begin(); it != collisions.end(); ++it) {
    ost << it->first << " : " << it->second << '\n';
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

void cmQtAutoGenerators::LogCommand(
  const std::vector<std::string>& command) const
{
  this->LogInfo(cmJoin(command, " "));
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
std::string cmQtAutoGenerators::ChecksumedPath(const std::string& sourceFile,
                                               const char* basePrefix,
                                               const char* baseSuffix) const
{
  std::string res = fpathCheckSum.getPart(sourceFile);
  res += "/";
  res += basePrefix;
  res += cmsys::SystemTools::GetFilenameWithoutLastExtension(sourceFile);
  res += baseSuffix;
  return res;
}

/**
 * @brief Tries to find the header file to the given file base path by
 * appending different header extensions
 * @return True on success
 */
bool cmQtAutoGenerators::FindHeader(std::string& header,
                                    const std::string& testBasePath) const
{
  for (std::vector<std::string>::const_iterator ext =
         this->HeaderExtensions.begin();
       ext != this->HeaderExtensions.end(); ++ext) {
    std::string testFilePath(testBasePath);
    testFilePath += '.';
    testFilePath += (*ext);
    if (cmsys::SystemTools::FileExists(testFilePath.c_str())) {
      header = testFilePath;
      return true;
    }
  }
  return false;
}

bool cmQtAutoGenerators::FindHeaderGlobal(
  std::string& header, const std::string& testBasePath) const
{
  for (std::vector<std::string>::const_iterator iit =
         this->MocIncludePaths.begin();
       iit != this->MocIncludePaths.end(); ++iit) {
    const std::string fullPath = ((*iit) + '/' + testBasePath);
    if (FindHeader(header, fullPath)) {
      return true;
    }
  }
  return false;
}

std::string cmQtAutoGenerators::FindMocHeader(const std::string& basePath,
                                              const std::string& baseName,
                                              const std::string& subDir) const
{
  std::string header;
  do {
    if (!subDir.empty()) {
      if (this->FindHeader(header, basePath + subDir + baseName)) {
        break;
      }
    }
    if (this->FindHeader(header, basePath + baseName)) {
      break;
    }
    // Try include directories
    if (this->FindHeaderGlobal(header, subDir + baseName)) {
      break;
    }
  } while (false);
  // Sanitize
  if (!header.empty()) {
    header = cmsys::SystemTools::GetRealPath(header);
  }
  return header;
}

std::string cmQtAutoGenerators::FindIncludedFile(
  const std::string& sourceFile, const std::string& includeString) const
{
  // Search in vicinity of the source
  {
    std::string testPath = cmSystemTools::GetFilenamePath(sourceFile);
    testPath += '/';
    testPath += includeString;
    if (cmsys::SystemTools::FileExists(testPath.c_str())) {
      return cmsys::SystemTools::GetRealPath(testPath);
    }
  }
  // Search globally
  return FindInIncludeDirectories(includeString);
}

/**
 * @brief Tries to find a file in the include directories
 * @return True on success
 */
std::string cmQtAutoGenerators::FindInIncludeDirectories(
  const std::string& includeString) const
{
  std::string res;
  for (std::vector<std::string>::const_iterator iit =
         this->MocIncludePaths.begin();
       iit != this->MocIncludePaths.end(); ++iit) {
    const std::string fullPath = ((*iit) + '/' + includeString);
    if (cmsys::SystemTools::FileExists(fullPath.c_str())) {
      res = cmsys::SystemTools::GetRealPath(fullPath);
      break;
    }
  }
  return res;
}

/**
 * @brief Generates the parent directory of the given file on demand
 * @return True on success
 */
bool cmQtAutoGenerators::MakeParentDirectory(const std::string& filename) const
{
  bool success = true;
  const std::string dirName = cmSystemTools::GetFilenamePath(filename);
  if (!dirName.empty()) {
    success = cmsys::SystemTools::MakeDirectory(dirName);
    if (!success) {
      this->LogError("AutoGen: Error: Directory creation failed: " + dirName);
    }
  }
  return success;
}
