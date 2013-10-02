/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2011 Kitware, Inc.
  Copyright 2011 Alexander Neundorf (neundorf@kde.org)

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmSystemTools.h"

#if defined(_WIN32) && !defined(__CYGWIN__)
# include "cmLocalVisualStudioGenerator.h"
#endif

#include <cmsys/Terminal.h>
#include <cmsys/ios/sstream>

#include <string.h>
#if defined(__APPLE__)
#include <unistd.h>
#endif

#include "cmQtAutoGenerators.h"


static bool requiresMocing(const std::string& text, std::string &macroName)
{
  // this simple check is much much faster than the regexp
  if (strstr(text.c_str(), "Q_OBJECT") == NULL
      && strstr(text.c_str(), "Q_GADGET") == NULL)
    {
    return false;
    }

  cmsys::RegularExpression qObjectRegExp("[\n][ \t]*Q_OBJECT[^a-zA-Z0-9_]");
  if (qObjectRegExp.find(text))
    {
    macroName = "Q_OBJECT";
    return true;
    }
  cmsys::RegularExpression qGadgetRegExp("[\n][ \t]*Q_GADGET[^a-zA-Z0-9_]");
  if (qGadgetRegExp.find(text))
    {
    macroName = "Q_GADGET";
    return true;
    }
  return false;
}


static std::string findMatchingHeader(const std::string& absPath,
                                      const std::string& mocSubDir,
                                      const std::string& basename,
                              const std::vector<std::string>& headerExtensions)
{
  std::string header;
  for(std::vector<std::string>::const_iterator ext = headerExtensions.begin();
      ext != headerExtensions.end();
      ++ext)
    {
    std::string sourceFilePath = absPath + basename + "." + (*ext);
    if (cmsys::SystemTools::FileExists(sourceFilePath.c_str()))
      {
      header = sourceFilePath;
      break;
      }
    if (!mocSubDir.empty())
      {
      sourceFilePath = mocSubDir + basename + "." + (*ext);
      if (cmsys::SystemTools::FileExists(sourceFilePath.c_str()))
        {
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
  if (currentMoc.find_first_of('/') != std::string::npos)
    {
    subDir = absPath
                  + cmsys::SystemTools::GetFilenamePath(currentMoc) + '/';
    }
  return subDir;
}


static void copyTargetProperty(cmTarget* destinationTarget,
                               cmTarget* sourceTarget,
                               const char* propertyName)
{
  const char* propertyValue = sourceTarget->GetProperty(propertyName);
  if (propertyValue)
    {
    destinationTarget->SetProperty(propertyName, propertyValue);
    }
}


cmQtAutoGenerators::cmQtAutoGenerators()
:Verbose(cmsys::SystemTools::GetEnv("VERBOSE") != 0)
,ColorOutput(true)
,RunMocFailed(false)
,GenerateAll(false)
{

  std::string colorEnv = "";
  cmsys::SystemTools::GetEnv("COLOR", colorEnv);
  if(!colorEnv.empty())
    {
    if(cmSystemTools::IsOn(colorEnv.c_str()))
      {
      this->ColorOutput = true;
      }
    else
      {
      this->ColorOutput = false;
      }
    }
}

bool cmQtAutoGenerators::InitializeMocSourceFile(cmTarget* target)
{
  cmMakefile* makefile = target->GetMakefile();
  // don't do anything if there is no Qt4 or Qt5Core (which contains moc):
  std::string qtMajorVersion = makefile->GetSafeDefinition("QT_VERSION_MAJOR");
  if (qtMajorVersion == "")
    {
    qtMajorVersion = makefile->GetSafeDefinition("Qt5Core_VERSION_MAJOR");
    }
  if (qtMajorVersion != "4" && qtMajorVersion != "5")
    {
    return false;
    }

  std::string automocTargetName = target->GetName();
  automocTargetName += "_automoc";
  std::string mocCppFile = makefile->GetCurrentOutputDirectory();
  mocCppFile += "/";
  mocCppFile += automocTargetName;
  mocCppFile += ".cpp";
  cmSourceFile* mocCppSource = makefile->GetOrCreateSource(mocCppFile.c_str(),
                                                         true);
  makefile->AppendProperty("ADDITIONAL_MAKE_CLEAN_FILES",
                           mocCppFile.c_str(), false);

  target->AddSourceFile(mocCppSource);
  return true;
}

static void GetCompileDefinitionsAndDirectories(cmTarget *target,
                                                const char * config,
                                                std::string &incs,
                                                std::string &defs)
{
  cmMakefile* makefile = target->GetMakefile();
  cmLocalGenerator* localGen = makefile->GetLocalGenerator();
  std::vector<std::string> includeDirs;
  cmGeneratorTarget gtgt(target);
  // Get the include dirs for this target, without stripping the implicit
  // include dirs off, see http://public.kitware.com/Bug/view.php?id=13667
  localGen->GetIncludeDirectories(includeDirs, &gtgt, "CXX", config, false);
  const char* sep = "";
  incs = "";
  for(std::vector<std::string>::const_iterator incDirIt = includeDirs.begin();
      incDirIt != includeDirs.end();
      ++incDirIt)
    {
    incs += sep;
    sep = ";";
    incs += *incDirIt;
    }

  std::set<std::string> defines;
  localGen->AddCompileDefinitions(defines, target, config);

  sep = "";
  for(std::set<std::string>::const_iterator defIt = defines.begin();
      defIt != defines.end();
      ++defIt)
    {
    defs += sep;
    sep = ";";
    defs += *defIt;
    }
}

void cmQtAutoGenerators::SetupAutoGenerateTarget(cmTarget* target)
{
  cmMakefile* makefile = target->GetMakefile();
  const char* targetName = target->GetName();

  bool relaxedMode = makefile->IsOn("CMAKE_AUTOMOC_RELAXED_MODE");

  // create a custom target for running automoc at buildtime:
  std::string automocTargetName = targetName;
  automocTargetName += "_automoc";

  std::string targetDir = makefile->GetCurrentOutputDirectory();
  targetDir += makefile->GetCMakeInstance()->GetCMakeFilesDirectory();
  targetDir += "/";
  targetDir += automocTargetName;
  targetDir += ".dir/";

  cmCustomCommandLine currentLine;
  currentLine.push_back(makefile->GetSafeDefinition("CMAKE_COMMAND"));
  currentLine.push_back("-E");
  currentLine.push_back("cmake_autogen");
  currentLine.push_back(targetDir);
  currentLine.push_back("$<CONFIGURATION>");

  cmCustomCommandLines commandLines;
  commandLines.push_back(currentLine);

  std::string workingDirectory = cmSystemTools::CollapseFullPath(
                                    "", makefile->GetCurrentOutputDirectory());

  std::vector<std::string> depends;
  std::string automocComment = "Automoc for target ";
  automocComment += targetName;

#if defined(_WIN32) && !defined(__CYGWIN__)
  bool usePRE_BUILD = false;
  cmLocalGenerator* localGen = makefile->GetLocalGenerator();
  cmGlobalGenerator* gg = localGen->GetGlobalGenerator();
  if(strstr(gg->GetName(), "Visual Studio"))
    {
    cmLocalVisualStudioGenerator* vslg =
      static_cast<cmLocalVisualStudioGenerator*>(localGen);
    // Under VS >= 7 use a PRE_BUILD event instead of a separate target to
    // reduce the number of targets loaded into the IDE.
    // This also works around a VS 11 bug that may skip updating the target:
    //  https://connect.microsoft.com/VisualStudio/feedback/details/769495
    usePRE_BUILD = vslg->GetVersion() >= cmLocalVisualStudioGenerator::VS7;
    }
  if(usePRE_BUILD)
    {
    // Add the pre-build command directly to bypass the OBJECT_LIBRARY
    // rejection in cmMakefile::AddCustomCommandToTarget because we know
    // PRE_BUILD will work for an OBJECT_LIBRARY in this specific case.
    std::vector<std::string> no_output;
    cmCustomCommand cc(makefile, no_output, depends,
                       commandLines, automocComment.c_str(),
                       workingDirectory.c_str());
    cc.SetEscapeOldStyle(false);
    cc.SetEscapeAllowMakeVars(true);
    target->GetPreBuildCommands().push_back(cc);
    }
  else
#endif
    {
    cmTarget* automocTarget = makefile->AddUtilityCommand(
                                automocTargetName.c_str(), true,
                                workingDirectory.c_str(), depends,
                                commandLines, false, automocComment.c_str());
    // Set target folder
    const char* automocFolder = makefile->GetCMakeInstance()->GetProperty(
                                                     "AUTOMOC_TARGETS_FOLDER");
    if (automocFolder && *automocFolder)
      {
      automocTarget->SetProperty("FOLDER", automocFolder);
      }
    else
      {
      // inherit FOLDER property from target (#13688)
      copyTargetProperty(automocTarget, target, "FOLDER");
      }

    target->AddUtility(automocTargetName.c_str());
    }

  // configure a file to get all information to automoc at buildtime:
  std::string _moc_files;
  std::string _moc_headers;
  const char* sepFiles = "";
  const char* sepHeaders = "";

  const std::vector<cmSourceFile*>& srcFiles = target->GetSourceFiles();

  for(std::vector<cmSourceFile*>::const_iterator fileIt = srcFiles.begin();
      fileIt != srcFiles.end();
      ++fileIt)
    {
    cmSourceFile* sf = *fileIt;
    std::string absFile = cmsys::SystemTools::GetRealPath(
                                                    sf->GetFullPath().c_str());
    bool skip = cmSystemTools::IsOn(sf->GetPropertyForUser("SKIP_AUTOMOC"));
    bool generated = cmSystemTools::IsOn(sf->GetPropertyForUser("GENERATED"));

    if ((skip==false) && (generated == false))
      {
      std::string ext = sf->GetExtension();
      cmSystemTools::FileFormat fileType = cmSystemTools::GetFileFormat(
                                                                  ext.c_str());
      if (fileType == cmSystemTools::CXX_FILE_FORMAT)
        {
        _moc_files += sepFiles;
        _moc_files += absFile;
        sepFiles = ";";
        }
      else if (fileType == cmSystemTools::HEADER_FILE_FORMAT)
        {
        _moc_headers += sepHeaders;
        _moc_headers += absFile;
        sepHeaders = ";";
        }
      }
    }

  const char* tmp = target->GetProperty("AUTOMOC_MOC_OPTIONS");
  std::string _moc_options = (tmp!=0 ? tmp : "");

  // forget the variables added here afterwards again:
  cmMakefile::ScopePushPop varScope(makefile);
  static_cast<void>(varScope);

  makefile->AddDefinition("_moc_target_name",
          cmLocalGenerator::EscapeForCMake(automocTargetName.c_str()).c_str());
  makefile->AddDefinition("_moc_options",
          cmLocalGenerator::EscapeForCMake(_moc_options.c_str()).c_str());
  makefile->AddDefinition("_moc_files",
          cmLocalGenerator::EscapeForCMake(_moc_files.c_str()).c_str());
  makefile->AddDefinition("_moc_headers",
          cmLocalGenerator::EscapeForCMake(_moc_headers.c_str()).c_str());
  makefile->AddDefinition("_moc_relaxed_mode", relaxedMode ? "TRUE" : "FALSE");

  std::string _moc_incs;
  std::string _moc_compile_defs;
  std::vector<std::string> configs;
  const char *config = makefile->GetConfigurations(configs);
  GetCompileDefinitionsAndDirectories(target, config,
                                      _moc_incs, _moc_compile_defs);

  makefile->AddDefinition("_moc_incs",
          cmLocalGenerator::EscapeForCMake(_moc_incs.c_str()).c_str());
  makefile->AddDefinition("_moc_compile_defs",
          cmLocalGenerator::EscapeForCMake(_moc_compile_defs.c_str()).c_str());

  std::map<std::string, std::string> configIncludes;
  std::map<std::string, std::string> configDefines;

  for (std::vector<std::string>::const_iterator li = configs.begin();
       li != configs.end(); ++li)
    {
    std::string config_moc_incs;
    std::string config_moc_compile_defs;
    GetCompileDefinitionsAndDirectories(target, li->c_str(),
                                        config_moc_incs,
                                        config_moc_compile_defs);
    if (config_moc_incs != _moc_incs)
      {
      configIncludes["_moc_incs_" + *li] =
                    cmLocalGenerator::EscapeForCMake(config_moc_incs.c_str());
      if(_moc_incs.empty())
        {
        _moc_incs = config_moc_incs;
        }
      }
    if (config_moc_compile_defs != _moc_compile_defs)
      {
      configDefines["_moc_compile_defs_" + *li] =
            cmLocalGenerator::EscapeForCMake(config_moc_compile_defs.c_str());
      if(_moc_compile_defs.empty())
        {
        _moc_compile_defs = config_moc_compile_defs;
        }
      }
    }

  const char *qtVersion = makefile->GetDefinition("Qt5Core_VERSION_MAJOR");
  if (!qtVersion)
    {
    qtVersion = makefile->GetDefinition("QT_VERSION_MAJOR");
    }
  if (const char *targetQtVersion =
      target->GetLinkInterfaceDependentStringProperty("QT_MAJOR_VERSION", 0))
    {
    qtVersion = targetQtVersion;
    }
  if (qtVersion)
    {
    makefile->AddDefinition("_target_qt_version", qtVersion);
    }

  {
  const char *qtMoc = makefile->GetSafeDefinition("QT_MOC_EXECUTABLE");
  makefile->AddDefinition("_qt_moc_executable", qtMoc);
  }

  if (strcmp(qtVersion, "5") == 0)
    {
    cmTarget *qt5Moc = makefile->FindTargetToUse("Qt5::moc");
    if (!qt5Moc)
      {
      cmSystemTools::Error("Qt5::moc target not found ",
                          automocTargetName.c_str());
      return;
      }
    makefile->AddDefinition("_qt_moc_executable", qt5Moc->GetLocation(0));
    }
  else
    {
    if (strcmp(qtVersion, "4") != 0)
      {
      cmSystemTools::Error("The CMAKE_AUTOMOC feature supports only Qt 4 and "
                          "Qt 5 ", automocTargetName.c_str());
      }
    }

  const char* cmakeRoot = makefile->GetSafeDefinition("CMAKE_ROOT");
  std::string inputFile = cmakeRoot;
  inputFile += "/Modules/AutomocInfo.cmake.in";
  std::string outputFile = targetDir;
  outputFile += "/AutomocInfo.cmake";
  makefile->ConfigureFile(inputFile.c_str(), outputFile.c_str(),
                          false, true, false);

  if (!configDefines.empty() || !configIncludes.empty())
    {
    std::ofstream infoFile(outputFile.c_str(), std::ios::app);
    if ( !infoFile )
      {
      std::string error = "Internal CMake error when trying to open file: ";
      error += outputFile.c_str();
      error += " for writing.";
      cmSystemTools::Error(error.c_str());
      return;
      }
    if (!configDefines.empty())
      {
      for (std::map<std::string, std::string>::iterator
            it = configDefines.begin(), end = configDefines.end();
            it != end; ++it)
        {
        infoFile << "set(AM_MOC_COMPILE_DEFINITIONS_" << it->first <<
          " " << it->second << ")\n";
        }
      }
    if (!configIncludes.empty())
      {
      for (std::map<std::string, std::string>::iterator
            it = configIncludes.begin(), end = configIncludes.end();
            it != end; ++it)
        {
        infoFile << "set(AM_MOC_INCLUDES_" << it->first <<
          " " << it->second << ")\n";
        }
      }
    }
}


bool cmQtAutoGenerators::Run(const char* targetDirectory, const char *config)
{
  bool success = true;
  cmake cm;
  cmGlobalGenerator* gg = this->CreateGlobalGenerator(&cm, targetDirectory);
  cmMakefile* makefile = gg->GetCurrentLocalGenerator()->GetMakefile();

  this->ReadAutomocInfoFile(makefile, targetDirectory, config);
  this->ReadOldMocDefinitionsFile(makefile, targetDirectory);

  this->Init();

  if (this->QtMajorVersion == "4" || this->QtMajorVersion == "5")
    {
    success = this->RunAutomoc(makefile);
    }

  this->WriteOldMocDefinitionsFile(targetDirectory);

  delete gg;
  gg = NULL;
  makefile = NULL;
  return success;
}


cmGlobalGenerator* cmQtAutoGenerators::CreateGlobalGenerator(cmake* cm,
                                                  const char* targetDirectory)
{
  cmGlobalGenerator* gg = new cmGlobalGenerator();
  gg->SetCMakeInstance(cm);

  cmLocalGenerator* lg = gg->CreateLocalGenerator();
  lg->GetMakefile()->SetHomeOutputDirectory(targetDirectory);
  lg->GetMakefile()->SetStartOutputDirectory(targetDirectory);
  lg->GetMakefile()->SetHomeDirectory(targetDirectory);
  lg->GetMakefile()->SetStartDirectory(targetDirectory);
  gg->SetCurrentLocalGenerator(lg);

  return gg;
}


bool cmQtAutoGenerators::ReadAutomocInfoFile(cmMakefile* makefile,
                                      const char* targetDirectory,
                                      const char *config)
{
  std::string filename(cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutomocInfo.cmake";

  if (!makefile->ReadListFile(0, filename.c_str()))
    {
    cmSystemTools::Error("Error processing file: ", filename.c_str());
    return false;
    }

  this->QtMajorVersion = makefile->GetSafeDefinition("AM_QT_VERSION_MAJOR");
  if (this->QtMajorVersion == "")
    {
    this->QtMajorVersion = makefile->GetSafeDefinition(
                                     "AM_Qt5Core_VERSION_MAJOR");
    }
  this->Sources = makefile->GetSafeDefinition("AM_SOURCES");
  this->Headers = makefile->GetSafeDefinition("AM_HEADERS");
  this->IncludeProjectDirsBefore = makefile->IsOn(
                                "AM_CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE");
  this->Srcdir = makefile->GetSafeDefinition("AM_CMAKE_CURRENT_SOURCE_DIR");
  this->Builddir = makefile->GetSafeDefinition("AM_CMAKE_CURRENT_BINARY_DIR");
  this->MocExecutable = makefile->GetSafeDefinition("AM_QT_MOC_EXECUTABLE");
  std::string compileDefsPropOrig = "AM_MOC_COMPILE_DEFINITIONS";
  std::string compileDefsProp = compileDefsPropOrig;
  if(config)
    {
    compileDefsProp += "_";
    compileDefsProp += config;
    }
  const char *compileDefs = makefile->GetDefinition(compileDefsProp.c_str());
  this->MocCompileDefinitionsStr = compileDefs ? compileDefs
                  : makefile->GetSafeDefinition(compileDefsPropOrig.c_str());
  std::string includesPropOrig = "AM_MOC_INCLUDES";
  std::string includesProp = includesPropOrig;
  if(config)
    {
    includesProp += "_";
    includesProp += config;
    }
  const char *includes = makefile->GetDefinition(includesProp.c_str());
  this->MocIncludesStr = includes ? includes
                      : makefile->GetSafeDefinition(includesPropOrig.c_str());
  this->MocOptionsStr = makefile->GetSafeDefinition("AM_MOC_OPTIONS");
  this->ProjectBinaryDir = makefile->GetSafeDefinition("AM_CMAKE_BINARY_DIR");
  this->ProjectSourceDir = makefile->GetSafeDefinition("AM_CMAKE_SOURCE_DIR");
  this->TargetName = makefile->GetSafeDefinition("AM_TARGET_NAME");

  this->CurrentCompileSettingsStr = this->MakeCompileSettingsString(makefile);

  this->RelaxedMode = makefile->IsOn("AM_RELAXED_MODE");

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


bool cmQtAutoGenerators::ReadOldMocDefinitionsFile(cmMakefile* makefile,
                                            const char* targetDirectory)
{
  std::string filename(cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutomocOldMocDefinitions.cmake";

  if (makefile->ReadListFile(0, filename.c_str()))
    {
    this->OldCompileSettingsStr =
                        makefile->GetSafeDefinition("AM_OLD_COMPILE_SETTINGS");
    }
  return true;
}


void cmQtAutoGenerators::WriteOldMocDefinitionsFile(const char* targetDirectory)
{
  std::string filename(cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutomocOldMocDefinitions.cmake";

  std::fstream outfile;
  outfile.open(filename.c_str(),
               std::ios::out | std::ios::trunc);
  outfile << "set(AM_OLD_COMPILE_SETTINGS "
              << cmLocalGenerator::EscapeForCMake(
                 this->CurrentCompileSettingsStr.c_str()) << ")\n";

  outfile.close();
}


void cmQtAutoGenerators::Init()
{
  this->OutMocCppFilename = this->Builddir;
  this->OutMocCppFilename += this->TargetName;
  this->OutMocCppFilename += ".cpp";

  std::vector<std::string> cdefList;
  cmSystemTools::ExpandListArgument(this->MocCompileDefinitionsStr, cdefList);
  for(std::vector<std::string>::const_iterator it = cdefList.begin();
      it != cdefList.end();
      ++it)
    {
    this->MocDefinitions.push_back("-D" + (*it));
    }

  cmSystemTools::ExpandListArgument(this->MocOptionsStr, this->MocOptions);

  std::vector<std::string> incPaths;
  cmSystemTools::ExpandListArgument(this->MocIncludesStr, incPaths);

  std::set<std::string> frameworkPaths;
  for(std::vector<std::string>::const_iterator it = incPaths.begin();
      it != incPaths.end();
      ++it)
    {
    const std::string &path = *it;
    this->MocIncludes.push_back("-I" + path);
    if (this->EndsWith(path, ".framework/Headers"))
      {
      // Go up twice to get to the framework root
      std::vector<std::string> pathComponents;
      cmsys::SystemTools::SplitPath(path.c_str(), pathComponents);
      std::string frameworkPath =cmsys::SystemTools::JoinPath(
                             pathComponents.begin(), pathComponents.end() - 2);
      frameworkPaths.insert(frameworkPath);
      }
    }

  for (std::set<std::string>::const_iterator it = frameworkPaths.begin();
         it != frameworkPaths.end(); ++it)
    {
    this->MocIncludes.push_back("-F");
    this->MocIncludes.push_back(*it);
    }


    if (this->IncludeProjectDirsBefore)
      {
      const std::string &binDir = "-I" + this->ProjectBinaryDir;

      const std::string srcDir = "-I" + this->ProjectSourceDir;

      std::list<std::string> sortedMocIncludes;
      std::list<std::string>::iterator it = this->MocIncludes.begin();
      while (it != this->MocIncludes.end())
        {
        if (this->StartsWith(*it, binDir))
          {
          sortedMocIncludes.push_back(*it);
          it = this->MocIncludes.erase(it);
          }
        else
          {
          ++it;
          }
        }
      it = this->MocIncludes.begin();
      while (it != this->MocIncludes.end())
        {
        if (this->StartsWith(*it, srcDir))
          {
          sortedMocIncludes.push_back(*it);
          it = this->MocIncludes.erase(it);
          }
        else
          {
          ++it;
          }
        }
      sortedMocIncludes.insert(sortedMocIncludes.end(),
                           this->MocIncludes.begin(), this->MocIncludes.end());
      this->MocIncludes = sortedMocIncludes;
    }

}


bool cmQtAutoGenerators::RunAutomoc(cmMakefile* makefile)
{
  if (!cmsys::SystemTools::FileExists(this->OutMocCppFilename.c_str())
    || (this->OldCompileSettingsStr != this->CurrentCompileSettingsStr))
    {
    this->GenerateAll = true;
    }

  // the program goes through all .cpp files to see which moc files are
  // included. It is not really interesting how the moc file is named, but
  // what file the moc is created from. Once a moc is included the same moc
  // may not be included in the _automoc.cpp file anymore. OTOH if there's a
  // header containing Q_OBJECT where no corresponding moc file is included
  // anywhere a moc_<filename>.cpp file is created and included in
  // the _automoc.cpp file.

  // key = moc source filepath, value = moc output filepath
  std::map<std::string, std::string> includedMocs;
  // collect all headers which may need to be mocced
  std::set<std::string> headerFiles;

  std::vector<std::string> sourceFiles;
  cmSystemTools::ExpandListArgument(this->Sources, sourceFiles);

  const std::vector<std::string>& headerExtensions =
                                               makefile->GetHeaderExtensions();

  for (std::vector<std::string>::const_iterator it = sourceFiles.begin();
       it != sourceFiles.end();
       ++it)
    {
    const std::string &absFilename = *it;
    if (this->Verbose)
      {
      std::cout << "AUTOMOC: Checking " << absFilename << std::endl;
      }
    if (this->RelaxedMode)
      {
      this->ParseCppFile(absFilename, headerExtensions, includedMocs);
      }
    else
      {
      this->StrictParseCppFile(absFilename, headerExtensions, includedMocs);
      }
    this->SearchHeadersForCppFile(absFilename, headerExtensions, headerFiles);
    }

  std::vector<std::string> headerFilesVec;
  cmSystemTools::ExpandListArgument(this->Headers, headerFilesVec);
  for (std::vector<std::string>::const_iterator it = headerFilesVec.begin();
       it != headerFilesVec.end();
       ++it)
    {
    headerFiles.insert(*it);
    }

  // key = moc source filepath, value = moc output filename
  std::map<std::string, std::string> notIncludedMocs;
  this->ParseHeaders(headerFiles, includedMocs, notIncludedMocs);

  // run moc on all the moc's that are #included in source files
  for(std::map<std::string, std::string>::const_iterator
                                                     it = includedMocs.begin();
      it != includedMocs.end();
      ++it)
    {
    this->GenerateMoc(it->first, it->second);
    }

  cmsys_ios::stringstream outStream;
  outStream << "/* This file is autogenerated, do not edit*/\n";

  bool automocCppChanged = false;
  if (notIncludedMocs.empty())
    {
    outStream << "enum some_compilers { need_more_than_nothing };\n";
    }
  else
    {
    // run moc on the remaining headers and include them in
    // the _automoc.cpp file
    for(std::map<std::string, std::string>::const_iterator
                                                  it = notIncludedMocs.begin();
        it != notIncludedMocs.end();
        ++it)
      {
      bool mocSuccess = this->GenerateMoc(it->first, it->second);
      if (mocSuccess)
        {
        automocCppChanged = true;
        }
      outStream << "#include \"" << it->second << "\"\n";
      }
    }

  if (this->RunMocFailed)
    {
    std::cerr << "moc failed..."<< std::endl;
    return false;
    }
  outStream.flush();
  std::string automocSource = outStream.str();
  if (!automocCppChanged)
    {
    // compare contents of the _automoc.cpp file
    const std::string oldContents = this->ReadAll(this->OutMocCppFilename);
    if (oldContents == automocSource)
      {
      // nothing changed: don't touch the _automoc.cpp file
      return true;
      }
    }

  // source file that includes all remaining moc files (_automoc.cpp file)
  std::fstream outfile;
  outfile.open(this->OutMocCppFilename.c_str(),
               std::ios::out | std::ios::trunc);
  outfile << automocSource;
  outfile.close();

  return true;
}


void cmQtAutoGenerators::ParseCppFile(const std::string& absFilename,
                              const std::vector<std::string>& headerExtensions,
                              std::map<std::string, std::string>& includedMocs)
{
  cmsys::RegularExpression mocIncludeRegExp(
              "[\n][ \t]*#[ \t]*include[ \t]+"
              "[\"<](([^ \">]+/)?moc_[^ \">/]+\\.cpp|[^ \">]+\\.moc)[\">]");

  const std::string contentsString = this->ReadAll(absFilename);
  if (contentsString.empty())
    {
    std::cerr << "AUTOMOC: warning: " << absFilename << ": file is empty\n"
              << std::endl;
    return;
    }
  const std::string absPath = cmsys::SystemTools::GetFilenamePath(
                   cmsys::SystemTools::GetRealPath(absFilename.c_str())) + '/';
  const std::string scannedFileBasename = cmsys::SystemTools::
                                  GetFilenameWithoutLastExtension(absFilename);
  std::string macroName;
  const bool requiresMoc = requiresMocing(contentsString, macroName);
  bool dotMocIncluded = false;
  bool mocUnderscoreIncluded = false;
  std::string ownMocUnderscoreFile;
  std::string ownDotMocFile;
  std::string ownMocHeaderFile;

  std::string::size_type matchOffset = 0;
  // first a simple string check for "moc" is *much* faster than the regexp,
  // and if the string search already fails, we don't have to try the
  // expensive regexp
  if ((strstr(contentsString.c_str(), "moc") != NULL)
                                    && (mocIncludeRegExp.find(contentsString)))
    {
    // for every moc include in the file
    do
      {
      const std::string currentMoc = mocIncludeRegExp.match(1);
      //std::cout << "found moc include: " << currentMoc << std::endl;

      std::string basename = cmsys::SystemTools::
                                   GetFilenameWithoutLastExtension(currentMoc);
      const bool moc_style = this->StartsWith(basename, "moc_");

      // If the moc include is of the moc_foo.cpp style we expect
      // the Q_OBJECT class declaration in a header file.
      // If the moc include is of the foo.moc style we need to look for
      // a Q_OBJECT macro in the current source file, if it contains the
      // macro we generate the moc file from the source file.
      // Q_OBJECT
      if (moc_style)
        {
        // basename should be the part of the moc filename used for
        // finding the correct header, so we need to remove the moc_ part
        basename = basename.substr(4);
        std::string mocSubDir = extractSubDir(absPath, currentMoc);
        std::string headerToMoc = findMatchingHeader(
                               absPath, mocSubDir, basename, headerExtensions);

        if (!headerToMoc.empty())
          {
          includedMocs[headerToMoc] = currentMoc;
          if (basename == scannedFileBasename)
            {
            mocUnderscoreIncluded = true;
            ownMocUnderscoreFile = currentMoc;
            ownMocHeaderFile = headerToMoc;
            }
          }
        else
          {
          std::cerr << "AUTOMOC: error: " << absFilename << " The file "
                    << "includes the moc file \"" << currentMoc << "\", "
                    << "but could not find header \"" << basename
                    << '{' << this->Join(headerExtensions, ',') << "}\" ";
          if (mocSubDir.empty())
            {
            std::cerr << "in " << absPath << "\n" << std::endl;
            }
          else
            {
            std::cerr << "neither in " << absPath
                      << " nor in " << mocSubDir << "\n" << std::endl;
            }

          ::exit(EXIT_FAILURE);
          }
        }
      else
        {
        std::string fileToMoc = absFilename;
        if ((basename != scannedFileBasename) || (requiresMoc==false))
          {
          std::string mocSubDir = extractSubDir(absPath, currentMoc);
          std::string headerToMoc = findMatchingHeader(
                              absPath, mocSubDir, basename, headerExtensions);
          if (!headerToMoc.empty())
            {
            // this is for KDE4 compatibility:
            fileToMoc = headerToMoc;
            if ((requiresMoc==false) &&(basename==scannedFileBasename))
              {
              std::cerr << "AUTOMOC: warning: " << absFilename << ": The file "
                            "includes the moc file \"" << currentMoc <<
                            "\", but does not contain a " << macroName
                            << " macro. Running moc on "
                        << "\"" << headerToMoc << "\" ! Include \"moc_"
                        << basename << ".cpp\" for a compatiblity with "
                           "strict mode (see CMAKE_AUTOMOC_RELAXED_MODE).\n"
                        << std::endl;
              }
            else
              {
              std::cerr << "AUTOMOC: warning: " << absFilename << ": The file "
                            "includes the moc file \"" << currentMoc <<
                            "\" instead of \"moc_" << basename << ".cpp\". "
                            "Running moc on "
                        << "\"" << headerToMoc << "\" ! Include \"moc_"
                        << basename << ".cpp\" for compatiblity with "
                           "strict mode (see CMAKE_AUTOMOC_RELAXED_MODE).\n"
                        << std::endl;
              }
            }
          else
            {
            std::cerr <<"AUTOMOC: error: " << absFilename << ": The file "
                        "includes the moc file \"" << currentMoc <<
                        "\", which seems to be the moc file from a different "
                        "source file. CMake also could not find a matching "
                        "header.\n" << std::endl;
            ::exit(EXIT_FAILURE);
            }
          }
        else
          {
          dotMocIncluded = true;
          ownDotMocFile = currentMoc;
          }
        includedMocs[fileToMoc] = currentMoc;
        }
      matchOffset += mocIncludeRegExp.end();
      } while(mocIncludeRegExp.find(contentsString.c_str() + matchOffset));
    }

  // In this case, check whether the scanned file itself contains a Q_OBJECT.
  // If this is the case, the moc_foo.cpp should probably be generated from
  // foo.cpp instead of foo.h, because otherwise it won't build.
  // But warn, since this is not how it is supposed to be used.
  if ((dotMocIncluded == false) && (requiresMoc == true))
    {
    if (mocUnderscoreIncluded == true)
      {
      // this is for KDE4 compatibility:
      std::cerr << "AUTOMOC: warning: " << absFilename << ": The file "
                << "contains a " << macroName << " macro, but does not "
                "include "
                << "\"" << scannedFileBasename << ".moc\", but instead "
                   "includes "
                << "\"" << ownMocUnderscoreFile  << "\". Running moc on "
                << "\"" << absFilename << "\" ! Better include \""
                << scannedFileBasename << ".moc\" for compatiblity with "
                   "strict mode (see CMAKE_AUTOMOC_RELAXED_MODE).\n"
                << std::endl;
      includedMocs[absFilename] = ownMocUnderscoreFile;
      includedMocs.erase(ownMocHeaderFile);
      }
    else
      {
      // otherwise always error out since it will not compile:
      std::cerr << "AUTOMOC: error: " << absFilename << ": The file "
                << "contains a " << macroName << " macro, but does not "
                "include "
                << "\"" << scannedFileBasename << ".moc\" !\n"
                << std::endl;
      ::exit(EXIT_FAILURE);
      }
    }

}


void cmQtAutoGenerators::StrictParseCppFile(const std::string& absFilename,
                              const std::vector<std::string>& headerExtensions,
                              std::map<std::string, std::string>& includedMocs)
{
  cmsys::RegularExpression mocIncludeRegExp(
              "[\n][ \t]*#[ \t]*include[ \t]+"
              "[\"<](([^ \">]+/)?moc_[^ \">/]+\\.cpp|[^ \">]+\\.moc)[\">]");

  const std::string contentsString = this->ReadAll(absFilename);
  if (contentsString.empty())
    {
    std::cerr << "AUTOMOC: warning: " << absFilename << ": file is empty\n"
              << std::endl;
    return;
    }
  const std::string absPath = cmsys::SystemTools::GetFilenamePath(
                   cmsys::SystemTools::GetRealPath(absFilename.c_str())) + '/';
  const std::string scannedFileBasename = cmsys::SystemTools::
                                  GetFilenameWithoutLastExtension(absFilename);

  bool dotMocIncluded = false;

  std::string::size_type matchOffset = 0;
  // first a simple string check for "moc" is *much* faster than the regexp,
  // and if the string search already fails, we don't have to try the
  // expensive regexp
  if ((strstr(contentsString.c_str(), "moc") != NULL)
                                    && (mocIncludeRegExp.find(contentsString)))
    {
    // for every moc include in the file
    do
      {
      const std::string currentMoc = mocIncludeRegExp.match(1);

      std::string basename = cmsys::SystemTools::
                                   GetFilenameWithoutLastExtension(currentMoc);
      const bool mocUnderscoreStyle = this->StartsWith(basename, "moc_");

      // If the moc include is of the moc_foo.cpp style we expect
      // the Q_OBJECT class declaration in a header file.
      // If the moc include is of the foo.moc style we need to look for
      // a Q_OBJECT macro in the current source file, if it contains the
      // macro we generate the moc file from the source file.
      if (mocUnderscoreStyle)
        {
        // basename should be the part of the moc filename used for
        // finding the correct header, so we need to remove the moc_ part
        basename = basename.substr(4);
        std::string mocSubDir = extractSubDir(absPath, currentMoc);
        std::string headerToMoc = findMatchingHeader(
                               absPath, mocSubDir, basename, headerExtensions);

        if (!headerToMoc.empty())
          {
          includedMocs[headerToMoc] = currentMoc;
          }
        else
          {
          std::cerr << "AUTOMOC: error: " << absFilename << " The file "
                    << "includes the moc file \"" << currentMoc << "\", "
                    << "but could not find header \"" << basename
                    << '{' << this->Join(headerExtensions, ',') << "}\" ";
          if (mocSubDir.empty())
            {
            std::cerr << "in " << absPath << "\n" << std::endl;
            }
          else
            {
            std::cerr << "neither in " << absPath
                      << " nor in " << mocSubDir << "\n" << std::endl;
            }

          ::exit(EXIT_FAILURE);
          }
        }
      else
        {
        if (basename != scannedFileBasename)
          {
          std::cerr <<"AUTOMOC: error: " << absFilename << ": The file "
                      "includes the moc file \"" << currentMoc <<
                      "\", which seems to be the moc file from a different "
                      "source file. This is not supported. "
                      "Include \"" << scannedFileBasename << ".moc\" to run "
                      "moc on this source file.\n" << std::endl;
          ::exit(EXIT_FAILURE);
          }
        dotMocIncluded = true;
        includedMocs[absFilename] = currentMoc;
        }
      matchOffset += mocIncludeRegExp.end();
      } while(mocIncludeRegExp.find(contentsString.c_str() + matchOffset));
    }

  // In this case, check whether the scanned file itself contains a Q_OBJECT.
  // If this is the case, the moc_foo.cpp should probably be generated from
  // foo.cpp instead of foo.h, because otherwise it won't build.
  // But warn, since this is not how it is supposed to be used.
  std::string macroName;
  if ((dotMocIncluded == false) && (requiresMocing(contentsString,
                                                     macroName)))
    {
    // otherwise always error out since it will not compile:
    std::cerr << "AUTOMOC: error: " << absFilename << ": The file "
              << "contains a " << macroName << " macro, but does not include "
              << "\"" << scannedFileBasename << ".moc\" !\n"
              << std::endl;
    ::exit(EXIT_FAILURE);
    }

}


void cmQtAutoGenerators::SearchHeadersForCppFile(const std::string& absFilename,
                              const std::vector<std::string>& headerExtensions,
                              std::set<std::string>& absHeaders)
{
  // search for header files and private header files we may need to moc:
  const std::string basename =
              cmsys::SystemTools::GetFilenameWithoutLastExtension(absFilename);
  const std::string absPath = cmsys::SystemTools::GetFilenamePath(
                   cmsys::SystemTools::GetRealPath(absFilename.c_str())) + '/';

  for(std::vector<std::string>::const_iterator ext = headerExtensions.begin();
      ext != headerExtensions.end();
      ++ext)
    {
    const std::string headerName = absPath + basename + "." + (*ext);
    if (cmsys::SystemTools::FileExists(headerName.c_str()))
      {
      absHeaders.insert(headerName);
      break;
      }
    }
  for(std::vector<std::string>::const_iterator ext = headerExtensions.begin();
      ext != headerExtensions.end();
      ++ext)
    {
    const std::string privateHeaderName = absPath+basename+"_p."+(*ext);
    if (cmsys::SystemTools::FileExists(privateHeaderName.c_str()))
      {
      absHeaders.insert(privateHeaderName);
      break;
      }
    }

}


void cmQtAutoGenerators::ParseHeaders(const std::set<std::string>& absHeaders,
                        const std::map<std::string, std::string>& includedMocs,
                        std::map<std::string, std::string>& notIncludedMocs)
{
  for(std::set<std::string>::const_iterator hIt=absHeaders.begin();
      hIt!=absHeaders.end();
      ++hIt)
    {
    const std::string& headerName = *hIt;

    if (includedMocs.find(headerName) == includedMocs.end())
      {
      if (this->Verbose)
        {
        std::cout << "AUTOMOC: Checking " << headerName << std::endl;
        }

      const std::string basename = cmsys::SystemTools::
                                   GetFilenameWithoutLastExtension(headerName);

      const std::string currentMoc = "moc_" + basename + ".cpp";
      const std::string contents = this->ReadAll(headerName);
      std::string macroName;
      if (requiresMocing(contents, macroName))
        {
        //std::cout << "header contains Q_OBJECT macro";
        notIncludedMocs[headerName] = currentMoc;
        }
      }
    }

}


bool cmQtAutoGenerators::GenerateMoc(const std::string& sourceFile,
                              const std::string& mocFileName)
{
  const std::string mocFilePath = this->Builddir + mocFileName;
  int sourceNewerThanMoc = 0;
  bool success = cmsys::SystemTools::FileTimeCompare(sourceFile.c_str(),
                                                     mocFilePath.c_str(),
                                                     &sourceNewerThanMoc);
  if (this->GenerateAll || !success || sourceNewerThanMoc >= 0)
    {
    // make sure the directory for the resulting moc file exists
    std::string mocDir = mocFilePath.substr(0, mocFilePath.rfind('/'));
    if (!cmsys::SystemTools::FileExists(mocDir.c_str(), false))
      {
      cmsys::SystemTools::MakeDirectory(mocDir.c_str());
      }

    std::string msg = "Generating ";
    msg += mocFileName;
    cmSystemTools::MakefileColorEcho(cmsysTerminal_Color_ForegroundBlue
                                           |cmsysTerminal_Color_ForegroundBold,
                                     msg.c_str(), true, this->ColorOutput);

    std::vector<cmStdString> command;
    command.push_back(this->MocExecutable);
    for (std::list<std::string>::const_iterator it = this->MocIncludes.begin();
         it != this->MocIncludes.end();
         ++it)
      {
      command.push_back(*it);
      }
    for(std::list<std::string>::const_iterator it=this->MocDefinitions.begin();
        it != this->MocDefinitions.end();
        ++it)
      {
      command.push_back(*it);
      }
    for(std::vector<std::string>::const_iterator it=this->MocOptions.begin();
        it != this->MocOptions.end();
        ++it)
      {
      command.push_back(*it);
      }
#ifdef _WIN32
    command.push_back("-DWIN32");
#endif
    command.push_back("-o");
    command.push_back(mocFilePath);
    command.push_back(sourceFile);

    if (this->Verbose)
      {
      for(std::vector<cmStdString>::const_iterator cmdIt = command.begin();
          cmdIt != command.end();
          ++cmdIt)
        {
        std::cout << *cmdIt << " ";
        }
      std::cout << std::endl;
      }

    std::string output;
    int retVal = 0;
    bool result = cmSystemTools::RunSingleCommand(command, &output, &retVal);
    if (!result || retVal)
      {
      std::cerr << "AUTOMOC: error: process for " << mocFilePath <<" failed:\n"
                << output << std::endl;
      this->RunMocFailed = true;
      cmSystemTools::RemoveFile(mocFilePath.c_str());
      }
    return true;
    }
  return false;
}


std::string cmQtAutoGenerators::Join(const std::vector<std::string>& lst,
                              char separator)
{
    if (lst.empty())
      {
      return "";
      }

    std::string result;
    for (std::vector<std::string>::const_iterator it = lst.begin();
         it != lst.end();
         ++it)
      {
      result += "." + (*it) + separator;
      }
    result.erase(result.end() - 1);
    return result;
}


bool cmQtAutoGenerators::StartsWith(const std::string& str, const std::string& with)
{
  return (str.substr(0, with.length()) == with);
}


bool cmQtAutoGenerators::EndsWith(const std::string& str, const std::string& with)
{
  if (with.length() > (str.length()))
    {
    return false;
    }
  return (str.substr(str.length() - with.length(), with.length()) == with);
}


std::string cmQtAutoGenerators::ReadAll(const std::string& filename)
{
  std::ifstream file(filename.c_str());
  cmsys_ios::stringstream stream;
  stream << file.rdbuf();
  file.close();
  return stream.str();
}
