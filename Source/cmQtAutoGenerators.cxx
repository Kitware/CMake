/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmQtAutoGenerators.h"

#include <algorithm>
#include <assert.h>
#include <cmConfigure.h>
#include <cmsys/FStream.hxx>
#include <cmsys/Terminal.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <utility>

#include "cmAlgorithms.h"
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

static std::string findMatchingHeader(
  const std::string& absPath, const std::string& mocSubDir,
  const std::string& basename,
  const std::vector<std::string>& headerExtensions)
{
  std::string header;
  for (std::vector<std::string>::const_iterator ext = headerExtensions.begin();
       ext != headerExtensions.end(); ++ext) {
    std::string sourceFilePath = absPath + basename + "." + (*ext);
    if (cmsys::SystemTools::FileExists(sourceFilePath.c_str())) {
      header = sourceFilePath;
      break;
    }
    // Try subdirectory instead
    if (!mocSubDir.empty()) {
      sourceFilePath = mocSubDir + basename + "." + (*ext);
      if (cmsys::SystemTools::FileExists(sourceFilePath.c_str())) {
        header = sourceFilePath;
        break;
      }
    }
  }

  return header;
}

static std::string extractSubDir(const std::string& absPath,
                                 const std::string& currentMoc)
{
  std::string subDir;
  if (currentMoc.find_first_of('/') != std::string::npos) {
    subDir = absPath + cmsys::SystemTools::GetFilenamePath(currentMoc) + '/';
  }
  return subDir;
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

static bool ListContains(const std::vector<std::string>& list,
                         const std::string& entry)
{
  return (std::find(list.begin(), list.end(), entry) != list.end());
}

cmQtAutoGenerators::cmQtAutoGenerators()
  : Verbose(cmsys::SystemTools::HasEnv("VERBOSE"))
  , ColorOutput(true)
  , RunMocFailed(false)
  , RunUicFailed(false)
  , RunRccFailed(false)
  , GenerateAll(false)
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

  // Precompile regular expressions
  this->RegExpQObject.compile("[\n][ \t]*Q_OBJECT[^a-zA-Z0-9_]");
  this->RegExpQGadget.compile("[\n][ \t]*Q_GADGET[^a-zA-Z0-9_]");
  this->RegExpMocInclude.compile(
    "[\n][ \t]*#[ \t]*include[ \t]+"
    "[\"<](([^ \">]+/)?moc_[^ \">/]+\\.cpp|[^ \">]+\\.moc)[\">]");
  this->RegExpUicInclude.compile("[\n][ \t]*#[ \t]*include[ \t]+"
                                 "[\"<](([^ \">]+/)?ui_[^ \">/]+\\.h)[\">]");
}

void cmQtAutoGenerators::MergeUicOptions(
  std::vector<std::string>& opts, const std::vector<std::string>& fileOpts,
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

  if (!this->ReadAutogenInfoFile(mf.get(), targetDirectory, config)) {
    return false;
  }
  this->ReadOldMocDefinitionsFile(mf.get(), targetDirectory);
  this->Init();

  if (this->QtMajorVersion == "4" || this->QtMajorVersion == "5") {
    if (!this->RunAutogen(mf.get())) {
      return false;
    }
  }
  return this->WriteOldMocDefinitionsFile(targetDirectory);
}

bool cmQtAutoGenerators::ReadAutogenInfoFile(
  cmMakefile* makefile, const std::string& targetDirectory,
  const std::string& config)
{
  std::string filename(cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutogenInfo.cmake";

  if (!makefile->ReadListFile(filename.c_str())) {
    std::ostringstream err;
    err << "AUTOGEN: error processing file: " << filename << std::endl;
    this->LogError(err.str());
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
  this->MocExecutable = makefile->GetSafeDefinition("AM_QT_MOC_EXECUTABLE");
  this->UicExecutable = makefile->GetSafeDefinition("AM_QT_UIC_EXECUTABLE");
  this->RccExecutable = makefile->GetSafeDefinition("AM_QT_RCC_EXECUTABLE");

  // - File Lists
  cmSystemTools::ExpandListArgument(makefile->GetSafeDefinition("AM_SOURCES"),
                                    this->Sources);
  cmSystemTools::ExpandListArgument(makefile->GetSafeDefinition("AM_HEADERS"),
                                    this->Headers);

  // - Moc
  cmSystemTools::ExpandListArgument(makefile->GetSafeDefinition("AM_SKIP_MOC"),
                                    this->SkipMoc);
  {
    std::string compileDefsPropOrig = "AM_MOC_COMPILE_DEFINITIONS";
    std::string compileDefsProp = compileDefsPropOrig;
    if (!config.empty()) {
      compileDefsProp += "_";
      compileDefsProp += config;
    }
    const char* compileDefs = makefile->GetDefinition(compileDefsProp);
    this->MocCompileDefinitionsStr = compileDefs
      ? compileDefs
      : makefile->GetSafeDefinition(compileDefsPropOrig);
  }
  {
    std::string includesPropOrig = "AM_MOC_INCLUDES";
    std::string includesProp = includesPropOrig;
    if (!config.empty()) {
      includesProp += "_";
      includesProp += config;
    }
    const char* includes = makefile->GetDefinition(includesProp);
    this->MocIncludesStr =
      includes ? includes : makefile->GetSafeDefinition(includesPropOrig);
  }
  this->MocOptionsStr = makefile->GetSafeDefinition("AM_MOC_OPTIONS");

  // - Uic
  cmSystemTools::ExpandListArgument(makefile->GetSafeDefinition("AM_SKIP_UIC"),
                                    this->SkipUic);
  {
    const char* uicOptionsFiles =
      makefile->GetSafeDefinition("AM_UIC_OPTIONS_FILES");
    std::string uicOptionsPropOrig = "AM_UIC_TARGET_OPTIONS";
    std::string uicOptionsProp = uicOptionsPropOrig;
    if (!config.empty()) {
      uicOptionsProp += "_";
      uicOptionsProp += config;
    }
    const char* uicTargetOptions = makefile->GetSafeDefinition(uicOptionsProp);
    cmSystemTools::ExpandListArgument(
      uicTargetOptions ? uicTargetOptions
                       : makefile->GetSafeDefinition(uicOptionsPropOrig),
      this->UicTargetOptions);
    const char* uicOptionsOptions =
      makefile->GetSafeDefinition("AM_UIC_OPTIONS_OPTIONS");
    std::vector<std::string> uicFilesVec;
    cmSystemTools::ExpandListArgument(uicOptionsFiles, uicFilesVec);
    std::vector<std::string> uicOptionsVec;
    cmSystemTools::ExpandListArgument(uicOptionsOptions, uicOptionsVec);
    if (uicFilesVec.size() != uicOptionsVec.size()) {
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
  {
    std::string rccSources = makefile->GetSafeDefinition("AM_RCC_SOURCES");
    cmSystemTools::ExpandListArgument(rccSources, this->RccSources);
  }
  {
    const char* rccOptionsFiles =
      makefile->GetSafeDefinition("AM_RCC_OPTIONS_FILES");
    const char* rccOptionsOptions =
      makefile->GetSafeDefinition("AM_RCC_OPTIONS_OPTIONS");
    std::vector<std::string> rccFilesVec;
    cmSystemTools::ExpandListArgument(rccOptionsFiles, rccFilesVec);
    std::vector<std::string> rccOptionsVec;
    cmSystemTools::ExpandListArgument(rccOptionsOptions, rccOptionsVec);
    if (rccFilesVec.size() != rccOptionsVec.size()) {
      return false;
    }
    for (std::vector<std::string>::iterator fileIt = rccFilesVec.begin(),
                                            optionIt = rccOptionsVec.begin();
         fileIt != rccFilesVec.end(); ++fileIt, ++optionIt) {
      cmSystemTools::ReplaceString(*optionIt, "@list_sep@", ";");
      this->RccOptions[*fileIt] = *optionIt;
    }

    const char* rccInputs = makefile->GetSafeDefinition("AM_RCC_INPUTS");
    std::vector<std::string> rccInputLists;
    cmSystemTools::ExpandListArgument(rccInputs, rccInputLists);

    // qrc files in the end of the list may have been empty
    if (rccInputLists.size() < this->RccSources.size()) {
      rccInputLists.resize(this->RccSources.size());
    }
    if (this->RccSources.size() != rccInputLists.size()) {
      std::ostringstream err;
      err << "AUTOGEN: RCC sources lists size missmatch in: " << filename;
      err << std::endl;
      this->LogError(err.str());
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

  // - Settings
  this->CurrentCompileSettingsStr = this->MakeCompileSettingsString(makefile);

  // - Flags
  this->IncludeProjectDirsBefore =
    makefile->IsOn("AM_CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE");
  this->MocRelaxedMode = makefile->IsOn("AM_MOC_RELAXED_MODE");

  return true;
}

std::string cmQtAutoGenerators::MakeCompileSettingsString(cmMakefile* makefile)
{
  std::string s;
  s += makefile->GetSafeDefinition("AM_MOC_COMPILE_DEFINITIONS");
  s += " ~~~ ";
  s += makefile->GetSafeDefinition("AM_MOC_INCLUDES");
  s += " ~~~ ";
  s += makefile->GetSafeDefinition("AM_MOC_OPTIONS");
  s += " ~~~ ";
  s += makefile->IsOn("AM_CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE") ? "TRUE"
                                                                     : "FALSE";
  s += " ~~~ ";

  return s;
}

void cmQtAutoGenerators::ReadOldMocDefinitionsFile(
  cmMakefile* makefile, const std::string& targetDirectory)
{
  std::string filename(cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutomocOldMocDefinitions.cmake";

  if (makefile->ReadListFile(filename.c_str())) {
    this->OldCompileSettingsStr =
      makefile->GetSafeDefinition("AM_OLD_COMPILE_SETTINGS");
  }
}

bool cmQtAutoGenerators::WriteOldMocDefinitionsFile(
  const std::string& targetDirectory)
{
  bool success = true;

  std::string filename(cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutomocOldMocDefinitions.cmake";

  {
    cmsys::ofstream outfile;
    outfile.open(filename.c_str(), std::ios::trunc);
    if (outfile) {
      outfile << "set(AM_OLD_COMPILE_SETTINGS "
              << cmOutputConverter::EscapeForCMake(
                   this->CurrentCompileSettingsStr)
              << ")\n";
      success = outfile.good();
    } else {
      success = false;
    }
  }

  return success;
}

void cmQtAutoGenerators::Init()
{
  this->AutogenBuildSubDir = this->AutogenTargetName;
  this->AutogenBuildSubDir += "/";

  this->OutMocCppFilenameRel = this->AutogenBuildSubDir;
  this->OutMocCppFilenameRel += "moc_compilation.cpp";

  this->OutMocCppFilenameAbs =
    this->CurrentBinaryDir + this->OutMocCppFilenameRel;

  // Init file path checksum generator
  fpathCheckSum.setupParentDirs(this->CurrentSourceDir, this->CurrentBinaryDir,
                                this->ProjectSourceDir,
                                this->ProjectBinaryDir);

  std::vector<std::string> cdefList;
  cmSystemTools::ExpandListArgument(this->MocCompileDefinitionsStr, cdefList);
  for (std::vector<std::string>::const_iterator it = cdefList.begin();
       it != cdefList.end(); ++it) {
    this->MocDefinitions.push_back("-D" + (*it));
  }

  cmSystemTools::ExpandListArgument(this->MocOptionsStr, this->MocOptions);

  std::vector<std::string> incPaths;
  cmSystemTools::ExpandListArgument(this->MocIncludesStr, incPaths);

  std::set<std::string> frameworkPaths;
  for (std::vector<std::string>::const_iterator it = incPaths.begin();
       it != incPaths.end(); ++it) {
    const std::string& path = *it;
    this->MocIncludes.push_back("-I" + path);
    if (cmHasLiteralSuffix(path, ".framework/Headers")) {
      // Go up twice to get to the framework root
      std::vector<std::string> pathComponents;
      cmsys::SystemTools::SplitPath(path, pathComponents);
      std::string frameworkPath = cmsys::SystemTools::JoinPath(
        pathComponents.begin(), pathComponents.end() - 2);
      frameworkPaths.insert(frameworkPath);
    }
  }

  for (std::set<std::string>::const_iterator it = frameworkPaths.begin();
       it != frameworkPaths.end(); ++it) {
    this->MocIncludes.push_back("-F");
    this->MocIncludes.push_back(*it);
  }

  if (this->IncludeProjectDirsBefore) {
    const std::string binDir = "-I" + this->ProjectBinaryDir;

    const std::string srcDir = "-I" + this->ProjectSourceDir;

    std::list<std::string> sortedMocIncludes;
    std::list<std::string>::iterator it = this->MocIncludes.begin();
    while (it != this->MocIncludes.end()) {
      if (cmsys::SystemTools::StringStartsWith(*it, binDir.c_str())) {
        sortedMocIncludes.push_back(*it);
        it = this->MocIncludes.erase(it);
      } else {
        ++it;
      }
    }
    it = this->MocIncludes.begin();
    while (it != this->MocIncludes.end()) {
      if (cmsys::SystemTools::StringStartsWith(*it, srcDir.c_str())) {
        sortedMocIncludes.push_back(*it);
        it = this->MocIncludes.erase(it);
      } else {
        ++it;
      }
    }
    sortedMocIncludes.insert(sortedMocIncludes.end(),
                             this->MocIncludes.begin(),
                             this->MocIncludes.end());
    this->MocIncludes = sortedMocIncludes;
  }
}

bool cmQtAutoGenerators::RunAutogen(cmMakefile* makefile)
{
  // If settings changed everything needs to be re-generated.
  if (this->OldCompileSettingsStr != this->CurrentCompileSettingsStr) {
    this->GenerateAll = true;
  }

  // the program goes through all .cpp files to see which moc files are
  // included. It is not really interesting how the moc file is named, but
  // what file the moc is created from. Once a moc is included the same moc
  // may not be included in the moc_compilation.cpp file anymore. OTOH if
  // there's a header containing Q_OBJECT where no corresponding moc file
  // is included anywhere a moc_<filename>.cpp file is created and included in
  // the moc_compilation.cpp file.

  // key = moc source filepath, value = moc output filepath
  std::map<std::string, std::string> includedMocs;
  std::map<std::string, std::string> notIncludedMocs;
  std::map<std::string, std::vector<std::string> > includedUis;
  // collects all headers which may need to be mocced
  std::set<std::string> headerFilesMoc;
  std::set<std::string> headerFilesUic;

  // Parse sources
  {
    const std::vector<std::string>& headerExtensions =
      makefile->GetCMakeInstance()->GetHeaderExtensions();

    for (std::vector<std::string>::const_iterator it = this->Sources.begin();
         it != this->Sources.end(); ++it) {
      const std::string& absFilename = *it;
      // Parse source file for MOC/UIC
      if (!this->ParseSourceFile(absFilename, headerExtensions, includedMocs,
                                 includedUis, this->MocRelaxedMode)) {
        return false;
      }
      // Find additional headers
      this->SearchHeadersForSourceFile(absFilename, headerExtensions,
                                       headerFilesMoc, headerFilesUic);
    }
  }

  // Parse headers
  headerFilesMoc.insert(this->Headers.begin(), this->Headers.end());
  headerFilesUic.insert(this->Headers.begin(), this->Headers.end());
  this->ParseHeaders(headerFilesMoc, headerFilesUic, includedMocs,
                     notIncludedMocs, includedUis);

  // Generate files
  if (!this->MocExecutable.empty()) {
    if (!this->GenerateMocFiles(includedMocs, notIncludedMocs)) {
      return false;
    }
  }
  if (!this->UicExecutable.empty()) {
    if (!this->GenerateUiFiles(includedUis)) {
      return false;
    }
  }
  if (!this->RccExecutable.empty()) {
    if (!this->GenerateQrcFiles()) {
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
  const std::vector<std::string>& headerExtensions,
  std::map<std::string, std::string>& includedMocs,
  std::map<std::string, std::vector<std::string> >& includedUis, bool relaxed)
{
  bool success = true;
  const std::string contentsString = ReadAll(absFilename);
  if (contentsString.empty()) {
    std::ostringstream err;
    err << "AUTOGEN: warning: " << absFilename << "\n"
        << "The file is empty\n";
    this->LogWarning(err.str());
  } else {
    // Parse source contents for MOC
    success = this->ParseContentForMoc(
      absFilename, contentsString, headerExtensions, includedMocs, relaxed);
    // Parse source contents for UIC
    if (success) {
      this->ParseContentForUic(absFilename, contentsString, includedUis);
    }
  }
  return success;
}

bool cmQtAutoGenerators::requiresMocing(const std::string& text,
                                        std::string& macroName)
{
  // Run a simple check before an expensive regular expression check
  if (strstr(text.c_str(), "Q_OBJECT") != CM_NULLPTR) {
    if (this->RegExpQObject.find(text)) {
      macroName = "Q_OBJECT";
      return true;
    }
  }
  if (strstr(text.c_str(), "Q_GADGET") != CM_NULLPTR) {
    if (this->RegExpQGadget.find(text)) {
      macroName = "Q_GADGET";
      return true;
    }
  }
  return false;
}

void cmQtAutoGenerators::ParseContentForUic(
  const std::string& absFilename, const std::string& contentsString,
  std::map<std::string, std::vector<std::string> >& includedUis)
{
  if (this->UicExecutable.empty() || this->UicSkipTest(absFilename)) {
    return;
  }

  // Process
  if (this->Verbose) {
    std::ostringstream err;
    err << "AUTOUIC: Checking " << absFilename << "\n";
    this->LogInfo(err.str());
  }

  const std::string realName = cmsys::SystemTools::GetRealPath(absFilename);
  const char* contentChars = contentsString.c_str();
  if (strstr(contentChars, "ui_") != CM_NULLPTR) {
    while (this->RegExpUicInclude.find(contentChars)) {
      const std::string currentUi = this->RegExpUicInclude.match(1);
      const std::string basename =
        cmsys::SystemTools::GetFilenameWithoutLastExtension(currentUi);
      // basename should be the part of the ui filename used for
      // finding the correct header, so we need to remove the ui_ part
      includedUis[realName].push_back(basename.substr(3));
      contentChars += this->RegExpUicInclude.end();
    }
  }
}

/**
 * @return True on success
 */
bool cmQtAutoGenerators::ParseContentForMoc(
  const std::string& absFilename, const std::string& contentsString,
  const std::vector<std::string>& headerExtensions,
  std::map<std::string, std::string>& includedMocs, bool relaxed)
{
  if (this->MocExecutable.empty() || this->MocSkipTest(absFilename)) {
    return true;
  }

  // Process
  if (this->Verbose) {
    std::ostringstream err;
    err << "AUTOMOC: Checking " << absFilename << "\n";
    this->LogInfo(err.str());
  }

  const std::string scannedFileAbsPath =
    cmsys::SystemTools::GetFilenamePath(
      cmsys::SystemTools::GetRealPath(absFilename)) +
    '/';
  const std::string scannedFileBasename =
    cmsys::SystemTools::GetFilenameWithoutLastExtension(absFilename);

  std::string macroName;
  const bool requiresMoc = this->requiresMocing(contentsString, macroName);
  bool ownDotMocIncluded = false;
  bool ownMocUnderscoreIncluded = false;
  std::string ownMocUnderscoreFile;
  std::string ownMocHeaderFile;

  // first a simple string check for "moc" is *much* faster than the regexp,
  // and if the string search already fails, we don't have to try the
  // expensive regexp
  const char* contentChars = contentsString.c_str();
  if (strstr(contentChars, "moc") != CM_NULLPTR) {
    // Iterate over all included moc files
    while (this->RegExpMocInclude.find(contentChars)) {
      const std::string currentMoc = this->RegExpMocInclude.match(1);
      // Basename of the current moc include
      std::string basename =
        cmsys::SystemTools::GetFilenameWithoutLastExtension(currentMoc);

      // If the moc include is of the moc_foo.cpp style we expect
      // the Q_OBJECT class declaration in a header file.
      // If the moc include is of the foo.moc style we need to look for
      // a Q_OBJECT macro in the current source file, if it contains the
      // macro we generate the moc file from the source file.
      if (cmHasLiteralPrefix(basename, "moc_")) {
        // Include: moc_FOO.cxx
        // basename should be the part of the moc filename used for
        // finding the correct header, so we need to remove the moc_ part
        basename = basename.substr(4);
        const std::string mocSubDir =
          extractSubDir(scannedFileAbsPath, currentMoc);
        const std::string headerToMoc = findMatchingHeader(
          scannedFileAbsPath, mocSubDir, basename, headerExtensions);

        if (!headerToMoc.empty()) {
          includedMocs[headerToMoc] = currentMoc;
          if (relaxed && (basename == scannedFileBasename)) {
            ownMocUnderscoreIncluded = true;
            ownMocUnderscoreFile = currentMoc;
            ownMocHeaderFile = headerToMoc;
          }
        } else {
          std::ostringstream err;
          err << "AUTOGEN: error: " << absFilename << "\n"
              << "The file includes the moc file \"" << currentMoc
              << "\", but could not find header \"" << basename << '{'
              << this->JoinExts(headerExtensions) << "}\" ";
          if (mocSubDir.empty()) {
            err << "in " << scannedFileAbsPath << "\n";
          } else {
            err << "neither in " << scannedFileAbsPath << " nor in "
                << mocSubDir << "\n";
          }
          this->LogError(err.str());
          return false;
        }
      } else {
        // Include: FOO.moc
        std::string fileToMoc;
        if (relaxed) {
          // Mode: Relaxed
          if (!requiresMoc || basename != scannedFileBasename) {
            const std::string mocSubDir =
              extractSubDir(scannedFileAbsPath, currentMoc);
            const std::string headerToMoc = findMatchingHeader(
              scannedFileAbsPath, mocSubDir, basename, headerExtensions);
            if (!headerToMoc.empty()) {
              // This is for KDE4 compatibility:
              fileToMoc = headerToMoc;
              if (!requiresMoc && basename == scannedFileBasename) {
                std::ostringstream err;
                err << "AUTOMOC: warning: " << absFilename << "\n"
                    << "The file includes the moc file \"" << currentMoc
                    << "\", but does not contain a " << macroName
                    << " macro. Running moc on "
                    << "\"" << headerToMoc << "\" ! Include \"moc_" << basename
                    << ".cpp\" for a compatibility with "
                       "strict mode (see CMAKE_AUTOMOC_RELAXED_MODE).\n";
                this->LogWarning(err.str());
              } else {
                std::ostringstream err;
                err << "AUTOMOC: warning: " << absFilename << "\n"
                    << "The file includes the moc file \"" << currentMoc
                    << "\" instead of \"moc_" << basename
                    << ".cpp\". Running moc on "
                    << "\"" << headerToMoc << "\" ! Include \"moc_" << basename
                    << ".cpp\" for compatibility with "
                       "strict mode (see CMAKE_AUTOMOC_RELAXED_MODE).\n";
                this->LogWarning(err.str());
              }
            } else {
              std::ostringstream err;
              err << "AUTOMOC: error: " << absFilename << "\n"
                  << "The file includes the moc file \"" << currentMoc
                  << "\", which seems to be the moc file from a different "
                     "source file. CMake also could not find a matching "
                     "header.\n";
              this->LogError(err.str());
              return false;
            }
          } else {
            // Include self
            fileToMoc = absFilename;
            ownDotMocIncluded = true;
          }
        } else {
          // Mode: Strict
          if (basename != scannedFileBasename) {
            // Don't allow FOO.moc include other than self in strict mode
            std::ostringstream err;
            err << "AUTOMOC: error: " << absFilename << "\n"
                << "The file includes the moc file \"" << currentMoc
                << "\", which seems to be the moc file from a different "
                   "source file. This is not supported. Include \""
                << scannedFileBasename
                << ".moc\" to run moc on this source file.\n";
            this->LogError(err.str());
            return false;
          } else {
            // Include self
            fileToMoc = absFilename;
            ownDotMocIncluded = true;
          }
        }
        if (!fileToMoc.empty()) {
          includedMocs[fileToMoc] = currentMoc;
        }
      }
      // Forward content pointer
      contentChars += this->RegExpMocInclude.end();
    }
  }

  // In this case, check whether the scanned file itself contains a Q_OBJECT.
  // If this is the case, the moc_foo.cpp should probably be generated from
  // foo.cpp instead of foo.h, because otherwise it won't build.
  // But warn, since this is not how it is supposed to be used.
  if (requiresMoc && !ownDotMocIncluded) {
    if (relaxed && ownMocUnderscoreIncluded) {
      // This is for KDE4 compatibility:
      std::ostringstream err;
      err << "AUTOMOC: warning: " << absFilename << "\n"
          << "The file contains a " << macroName
          << " macro, but does not include "
          << "\"" << scannedFileBasename << ".moc\", but instead includes "
          << "\"" << ownMocUnderscoreFile << "\". Running moc on "
          << "\"" << absFilename << "\" ! Better include \""
          << scannedFileBasename
          << ".moc\" for compatibility with "
             "strict mode (see CMAKE_AUTOMOC_RELAXED_MODE).\n";
      this->LogWarning(err.str());

      // Use scanned source file instead of scanned header file as moc source
      includedMocs[absFilename] = ownMocUnderscoreFile;
      includedMocs.erase(ownMocHeaderFile);
    } else {
      // Otherwise always error out since it will not compile:
      std::ostringstream err;
      err << "AUTOMOC: error: " << absFilename << "\n"
          << "The file contains a " << macroName
          << " macro, but does not include "
          << "\"" << scannedFileBasename << ".moc\" !\n";
      this->LogError(err.str());
      return false;
    }
  }

  return true;
}

void cmQtAutoGenerators::SearchHeadersForSourceFile(
  const std::string& absFilename,
  const std::vector<std::string>& headerExtensions,
  std::set<std::string>& absHeadersMoc, std::set<std::string>& absHeadersUic)
{
  // search for header files and private header files we may need to moc:
  const std::string basename =
    cmsys::SystemTools::GetFilenameWithoutLastExtension(absFilename);
  const std::string absPath = cmsys::SystemTools::GetFilenamePath(
                                cmsys::SystemTools::GetRealPath(absFilename)) +
    '/';

  // Search for regular header
  for (std::vector<std::string>::const_iterator ext = headerExtensions.begin();
       ext != headerExtensions.end(); ++ext) {
    const std::string headerName = absPath + basename + "." + (*ext);
    if (cmsys::SystemTools::FileExists(headerName.c_str())) {
      // Moc headers
      if (!this->MocExecutable.empty() &&
          !ListContains(this->SkipMoc, absFilename)) {
        absHeadersMoc.insert(headerName);
      }
      // Uic headers
      if (!this->UicExecutable.empty() &&
          !ListContains(this->SkipUic, absFilename)) {
        absHeadersUic.insert(headerName);
      }
      break;
    }
  }
  // Search for private header
  for (std::vector<std::string>::const_iterator ext = headerExtensions.begin();
       ext != headerExtensions.end(); ++ext) {
    const std::string privateHeaderName = absPath + basename + "_p." + (*ext);
    if (cmsys::SystemTools::FileExists(privateHeaderName.c_str())) {
      // Moc headers
      if (!this->MocExecutable.empty() &&
          !ListContains(this->SkipMoc, absFilename)) {
        absHeadersMoc.insert(privateHeaderName);
      }
      // Uic headers
      if (!this->UicExecutable.empty() &&
          !ListContains(this->SkipUic, absFilename)) {
        absHeadersUic.insert(privateHeaderName);
      }
      break;
    }
  }
}

void cmQtAutoGenerators::ParseHeaders(
  const std::set<std::string>& absHeadersMoc,
  const std::set<std::string>& absHeadersUic,
  const std::map<std::string, std::string>& includedMocs,
  std::map<std::string, std::string>& notIncludedMocs,
  std::map<std::string, std::vector<std::string> >& includedUis)
{
  // Merged header files list to read files only once
  std::set<std::string> headerFiles;
  headerFiles.insert(absHeadersMoc.begin(), absHeadersMoc.end());
  headerFiles.insert(absHeadersUic.begin(), absHeadersUic.end());

  for (std::set<std::string>::const_iterator hIt = headerFiles.begin();
       hIt != headerFiles.end(); ++hIt) {
    const std::string& headerName = *hIt;
    const std::string contents = ReadAll(headerName);

    // Parse header content for MOC
    if (!this->MocExecutable.empty() &&
        (absHeadersMoc.find(headerName) != absHeadersMoc.end()) &&
        (includedMocs.find(headerName) == includedMocs.end())) {
      if (!this->MocSkipTest(headerName)) {
        // Process
        if (this->Verbose) {
          std::ostringstream err;
          err << "AUTOMOC: Checking " << headerName << "\n";
          this->LogInfo(err.str());
        }
        std::string macroName;
        if (this->requiresMocing(contents, macroName)) {
          notIncludedMocs[headerName] = fpathCheckSum.getPart(headerName) +
            "/moc_" +
            cmsys::SystemTools::GetFilenameWithoutLastExtension(headerName) +
            ".cpp";
        }
      }
    }

    // Parse header content for UIC
    if (absHeadersUic.find(headerName) != absHeadersUic.end()) {
      this->ParseContentForUic(headerName, contents, includedUis);
    }
  }
}

bool cmQtAutoGenerators::GenerateMocFiles(
  const std::map<std::string, std::string>& includedMocs,
  const std::map<std::string, std::string>& notIncludedMocs)
{
  // look for name collisions
  {
    std::multimap<std::string, std::string> collisions;
    // Test merged map of included and notIncluded
    std::map<std::string, std::string> mergedMocs(includedMocs);
    mergedMocs.insert(notIncludedMocs.begin(), notIncludedMocs.end());
    if (this->NameCollisionTest(mergedMocs, collisions)) {
      std::ostringstream err;
      err << "AUTOGEN: error: "
             "The same moc file will be generated "
             "from different sources."
          << std::endl
          << "To avoid this error either" << std::endl
          << "- rename the source files or" << std::endl
          << "- do not include the (moc_NAME.cpp|NAME.moc) file" << std::endl;
      this->LogErrorNameCollision(err.str(), collisions);
      return false;
    }
  }

  // generate moc files that are included by source files.
  {
    const std::string subDirPrefix = "include/";
    for (std::map<std::string, std::string>::const_iterator it =
           includedMocs.begin();
         it != includedMocs.end(); ++it) {
      if (!this->GenerateMoc(it->first, it->second, subDirPrefix)) {
        if (this->RunMocFailed) {
          return false;
        }
      }
    }
  }

  // generate moc files that are _not_ included by source files.
  bool automocCppChanged = false;
  {
    const std::string subDirPrefix;
    for (std::map<std::string, std::string>::const_iterator it =
           notIncludedMocs.begin();
         it != notIncludedMocs.end(); ++it) {
      if (this->GenerateMoc(it->first, it->second, subDirPrefix)) {
        automocCppChanged = true;
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
    std::ostringstream outStream;
    outStream << "/* This file is autogenerated, do not edit*/\n";
    if (notIncludedMocs.empty()) {
      // Dummy content
      outStream << "enum some_compilers { need_more_than_nothing };\n";
    } else {
      // Valid content
      for (std::map<std::string, std::string>::const_iterator it =
             notIncludedMocs.begin();
           it != notIncludedMocs.end(); ++it) {
        outStream << "#include \"" << it->second << "\"\n";
      }
    }
    outStream.flush();
    automocSource = outStream.str();
  }

  // Check if we even need to update moc_compilation.cpp
  if (!automocCppChanged) {
    // compare contents of the moc_compilation.cpp file
    const std::string oldContents = ReadAll(this->OutMocCppFilenameAbs);
    if (oldContents == automocSource) {
      // nothing changed: don't touch the moc_compilation.cpp file
      if (this->Verbose) {
        std::ostringstream err;
        err << "AUTOGEN: " << this->OutMocCppFilenameRel << " still up to date"
            << std::endl;
        this->LogInfo(err.str());
      }
      return true;
    }
  }

  // Actually write moc_compilation.cpp
  {
    std::string msg = "Generating MOC compilation ";
    msg += this->OutMocCppFilenameRel;
    this->LogBold(msg);
  }
  // Make sure the parent directory exists
  bool success = this->MakeParentDirectory(this->OutMocCppFilenameAbs);
  if (success) {
    cmsys::ofstream outfile;
    outfile.open(this->OutMocCppFilenameAbs.c_str(), std::ios::trunc);
    if (!outfile) {
      success = false;
      std::ostringstream err;
      err << "AUTOGEN: error opening " << this->OutMocCppFilenameAbs << "\n";
      this->LogError(err.str());
    } else {
      outfile << automocSource;
      // Check for write errors
      if (!outfile.good()) {
        success = false;
        std::ostringstream err;
        err << "AUTOGEN: error writing " << this->OutMocCppFilenameAbs << "\n";
        this->LogError(err.str());
      }
    }
  }
  return success;
}

/**
 * @return True if a moc file was created. False may indicate an error.
 */
bool cmQtAutoGenerators::GenerateMoc(const std::string& sourceFile,
                                     const std::string& mocFileName,
                                     const std::string& subDirPrefix)
{
  const std::string mocFileRel =
    this->AutogenBuildSubDir + subDirPrefix + mocFileName;
  const std::string mocFileAbs = this->CurrentBinaryDir + mocFileRel;
  int sourceNewerThanMoc = 0;
  bool success = cmsys::SystemTools::FileTimeCompare(sourceFile, mocFileAbs,
                                                     &sourceNewerThanMoc);
  if (this->GenerateAll || !success || sourceNewerThanMoc >= 0) {
    // Log
    this->LogBold("Generating MOC source " + mocFileRel);

    // Make sure the parent directory exists
    if (!this->MakeParentDirectory(mocFileAbs)) {
      this->RunMocFailed = true;
      return false;
    }

    std::vector<std::string> command;
    command.push_back(this->MocExecutable);
    command.insert(command.end(), this->MocIncludes.begin(),
                   this->MocIncludes.end());
    command.insert(command.end(), this->MocDefinitions.begin(),
                   this->MocDefinitions.end());
    command.insert(command.end(), this->MocOptions.begin(),
                   this->MocOptions.end());
#ifdef _WIN32
    command.push_back("-DWIN32");
#endif
    command.push_back("-o");
    command.push_back(mocFileAbs);
    command.push_back(sourceFile);

    if (this->Verbose) {
      this->LogCommand(command);
    }

    std::string output;
    int retVal = 0;
    bool result =
      cmSystemTools::RunSingleCommand(command, &output, &output, &retVal);
    if (!result || retVal) {
      {
        std::ostringstream err;
        err << "AUTOGEN: error: moc process for " << mocFileRel << " failed:\n"
            << output << std::endl;
        this->LogError(err.str());
      }
      cmSystemTools::RemoveFile(mocFileAbs);
      this->RunMocFailed = true;
      return false;
    }
    return true;
  }
  return false;
}

bool cmQtAutoGenerators::GenerateUiFiles(
  const std::map<std::string, std::vector<std::string> >& includedUis)
{
  // single map with input / output names
  std::map<std::string, std::map<std::string, std::string> > uiGenMap;
  std::map<std::string, std::string> testMap;
  for (std::map<std::string, std::vector<std::string> >::const_iterator it =
         includedUis.begin();
       it != includedUis.end(); ++it) {
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
      std::ostringstream err;
      err << "AUTOGEN: error: The same ui_NAME.h file will be generated "
             "from different sources."
          << std::endl
          << "To avoid this error rename the source files." << std::endl;
      this->LogErrorNameCollision(err.str(), collisions);
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
      if (!this->GenerateUi(it->first, sit->first, sit->second)) {
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
bool cmQtAutoGenerators::GenerateUi(const std::string& realName,
                                    const std::string& uiInputFile,
                                    const std::string& uiOutputFile)
{
  const std::string uicFileRel =
    this->AutogenBuildSubDir + "include/" + uiOutputFile;
  const std::string uicFileAbs = this->CurrentBinaryDir + uicFileRel;

  int sourceNewerThanUi = 0;
  bool success = cmsys::SystemTools::FileTimeCompare(uiInputFile, uicFileAbs,
                                                     &sourceNewerThanUi);
  if (this->GenerateAll || !success || sourceNewerThanUi >= 0) {
    // Log
    this->LogBold("Generating UIC header " + uicFileRel);

    // Make sure the parent directory exists
    if (!this->MakeParentDirectory(uicFileAbs)) {
      this->RunUicFailed = true;
      return false;
    }

    std::vector<std::string> command;
    command.push_back(this->UicExecutable);

    std::vector<std::string> opts = this->UicTargetOptions;
    std::map<std::string, std::string>::const_iterator optionIt =
      this->UicOptions.find(uiInputFile);
    if (optionIt != this->UicOptions.end()) {
      std::vector<std::string> fileOpts;
      cmSystemTools::ExpandListArgument(optionIt->second, fileOpts);
      cmQtAutoGenerators::MergeUicOptions(opts, fileOpts,
                                          this->QtMajorVersion == "5");
    }
    command.insert(command.end(), opts.begin(), opts.end());

    command.push_back("-o");
    command.push_back(uicFileAbs);
    command.push_back(uiInputFile);

    if (this->Verbose) {
      this->LogCommand(command);
    }
    std::string output;
    int retVal = 0;
    bool result =
      cmSystemTools::RunSingleCommand(command, &output, &output, &retVal);
    if (!result || retVal) {
      {
        std::ostringstream err;
        err << "AUTOUIC: error: uic process for " << uicFileRel
            << " needed by\n \"" << realName << "\"\nfailed:\n"
            << output << std::endl;
        this->LogError(err.str());
      }
      cmSystemTools::RemoveFile(uicFileAbs);
      this->RunUicFailed = true;
      return false;
    }
    return true;
  }
  return false;
}

bool cmQtAutoGenerators::InputFilesNewerThanQrc(const std::string& qrcFile,
                                                const std::string& rccOutput)
{
  std::vector<std::string> const& files = this->RccInputs[qrcFile];
  for (std::vector<std::string>::const_iterator it = files.begin();
       it != files.end(); ++it) {
    int inputNewerThanQrc = 0;
    bool success =
      cmsys::SystemTools::FileTimeCompare(*it, rccOutput, &inputNewerThanQrc);
    if (!success || inputNewerThanQrc >= 0) {
      return true;
    }
  }
  return false;
}

bool cmQtAutoGenerators::GenerateQrcFiles()
{
  // generate single map with input / output names
  std::map<std::string, std::string> qrcGenMap;
  for (std::vector<std::string>::const_iterator si = this->RccSources.begin();
       si != this->RccSources.end(); ++si) {
    const std::string ext = cmsys::SystemTools::GetFilenameLastExtension(*si);
    if (ext == ".qrc") {
      qrcGenMap[*si] = this->AutogenBuildSubDir + fpathCheckSum.getPart(*si) +
        "/qrc_" + cmsys::SystemTools::GetFilenameWithoutLastExtension(*si) +
        ".cpp";
    }
  }

  // look for name collisions
  {
    std::multimap<std::string, std::string> collisions;
    if (this->NameCollisionTest(qrcGenMap, collisions)) {
      std::ostringstream err;
      err << "AUTOGEN: error: The same qrc_NAME.cpp file"
             " will be generated from different sources."
          << std::endl
          << "To avoid this error rename the source .qrc files." << std::endl;
      this->LogErrorNameCollision(err.str(), collisions);
      return false;
    }
  }

  // generate qrc files
  for (std::map<std::string, std::string>::const_iterator si =
         qrcGenMap.begin();
       si != qrcGenMap.end(); ++si) {
    bool unique = FileNameIsUnique(si->first, qrcGenMap);
    if (!this->GenerateQrc(si->first, si->second, unique)) {
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
bool cmQtAutoGenerators::GenerateQrc(const std::string& qrcInputFile,
                                     const std::string& qrcOutputFile,
                                     bool unique_n)
{
  std::string symbolName =
    cmsys::SystemTools::GetFilenameWithoutLastExtension(qrcInputFile);
  if (!unique_n) {
    symbolName += "_";
    symbolName += fpathCheckSum.getPart(qrcInputFile);
  }
  // Replace '-' with '_'. The former is valid for
  // file names but not for symbol names.
  std::replace(symbolName.begin(), symbolName.end(), '-', '_');

  const std::string qrcBuildFile = this->CurrentBinaryDir + qrcOutputFile;

  int sourceNewerThanQrc = 0;
  bool generateQrc = !cmsys::SystemTools::FileTimeCompare(
    qrcInputFile, qrcBuildFile, &sourceNewerThanQrc);
  generateQrc = generateQrc || (sourceNewerThanQrc >= 0);
  generateQrc =
    generateQrc || this->InputFilesNewerThanQrc(qrcInputFile, qrcBuildFile);

  if (this->GenerateAll || generateQrc) {
    {
      std::string msg = "Generating RCC source ";
      msg += qrcOutputFile;
      this->LogBold(msg);
    }

    // Make sure the parent directory exists
    if (!this->MakeParentDirectory(qrcOutputFile)) {
      this->RunRccFailed = true;
      return false;
    }

    std::vector<std::string> command;
    command.push_back(this->RccExecutable);
    {
      std::map<std::string, std::string>::const_iterator optionIt =
        this->RccOptions.find(qrcInputFile);
      if (optionIt != this->RccOptions.end()) {
        cmSystemTools::ExpandListArgument(optionIt->second, command);
      }
    }
    command.push_back("-name");
    command.push_back(symbolName);
    command.push_back("-o");
    command.push_back(qrcBuildFile);
    command.push_back(qrcInputFile);

    if (this->Verbose) {
      this->LogCommand(command);
    }
    std::string output;
    int retVal = 0;
    bool result =
      cmSystemTools::RunSingleCommand(command, &output, &output, &retVal);
    if (!result || retVal) {
      {
        std::ostringstream err;
        err << "AUTORCC: error: rcc process for " << qrcOutputFile
            << " failed:\n"
            << output << std::endl;
        this->LogError(err.str());
      }
      cmSystemTools::RemoveFile(qrcBuildFile);
      this->RunRccFailed = true;
      return false;
    }
    return true;
  }
  return false;
}

/**
 * @brief Tests if the file name is in the skip list
 */
bool cmQtAutoGenerators::MocSkipTest(const std::string& absFilename)
{
  if (ListContains(this->SkipMoc, absFilename)) {
    if (this->Verbose) {
      std::ostringstream msg;
      msg << "AUTOMOC: Skipping " << absFilename << "\n";
      this->LogInfo(msg.str());
    }
    return true;
  }
  return false;
}

/**
 * @brief Tests if the file name is in the skip list
 */
bool cmQtAutoGenerators::UicSkipTest(const std::string& absFilename)
{
  if (ListContains(this->SkipUic, absFilename)) {
    if (this->Verbose) {
      std::ostringstream msg;
      msg << "AUTOUIC: Skipping " << absFilename << "\n";
      this->LogInfo(msg.str());
    }
    return true;
  }
  return false;
}

/**
 * @brief Collects name collisions as output/input pairs
 * @return True if there were collisions
 */
bool cmQtAutoGenerators::NameCollisionTest(
  const std::map<std::string, std::string>& genFiles,
  std::multimap<std::string, std::string>& collisions)
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

void cmQtAutoGenerators::LogErrorNameCollision(
  const std::string& message,
  const std::multimap<std::string, std::string>& collisions)
{
  typedef std::multimap<std::string, std::string>::const_iterator Iter;

  std::ostringstream err;
  // Add message
  err << message;
  // Append collision list
  for (Iter it = collisions.begin(); it != collisions.end(); ++it) {
    err << it->first << " : " << it->second << std::endl;
  }
  this->LogError(err.str());
}

void cmQtAutoGenerators::LogBold(const std::string& message)
{
  cmSystemTools::MakefileColorEcho(cmsysTerminal_Color_ForegroundBlue |
                                     cmsysTerminal_Color_ForegroundBold,
                                   message.c_str(), true, this->ColorOutput);
}

void cmQtAutoGenerators::LogInfo(const std::string& message)
{
  std::cout << message.c_str();
}

void cmQtAutoGenerators::LogWarning(const std::string& message)
{
  std::ostringstream ostr;
  ostr << message << "\n";
  std::cout << message.c_str();
}

void cmQtAutoGenerators::LogError(const std::string& message)
{
  std::ostringstream ostr;
  ostr << message << "\n\n";
  std::cerr << ostr.str();
}

void cmQtAutoGenerators::LogCommand(const std::vector<std::string>& command)
{
  std::ostringstream sbuf;
  for (std::vector<std::string>::const_iterator cmdIt = command.begin();
       cmdIt != command.end(); ++cmdIt) {
    if (cmdIt != command.begin()) {
      sbuf << " ";
    }
    sbuf << *cmdIt;
  }
  if (!sbuf.str().empty()) {
    sbuf << std::endl;
    this->LogInfo(sbuf.str());
  }
}

/**
 * @brief Generates the parent directory of the given file on demand
 * @return True on success
 */
bool cmQtAutoGenerators::MakeParentDirectory(const std::string& filename)
{
  bool success = true;
  const std::string dirName = cmSystemTools::GetFilenamePath(filename);
  if (!dirName.empty()) {
    success = cmsys::SystemTools::MakeDirectory(dirName);
    if (!success) {
      std::ostringstream err;
      err << "AUTOGEN: Directory creation failed: " << dirName << std::endl;
      this->LogError(err.str());
    }
  }
  return success;
}

std::string cmQtAutoGenerators::JoinExts(const std::vector<std::string>& lst)
{
  if (lst.empty()) {
    return "";
  }

  std::string result;
  std::string separator = ",";
  for (std::vector<std::string>::const_iterator it = lst.begin();
       it != lst.end(); ++it) {
    if (it != lst.begin()) {
      result += separator;
    }
    result += '.' + (*it);
  }
  result.erase(result.end() - 1);
  return result;
}
