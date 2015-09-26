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
#include "cmOutputConverter.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmState.h"
#include "cmAlgorithms.h"

#include <sys/stat.h>

#include <cmsys/Terminal.h>
#include <cmsys/FStream.hxx>
#include <assert.h>

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

cmQtAutoGenerators::cmQtAutoGenerators()
:Verbose(cmsys::SystemTools::GetEnv("VERBOSE") != 0)
,ColorOutput(true)
,RunMocFailed(false)
,RunUicFailed(false)
,RunRccFailed(false)
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

void cmQtAutoGenerators::MergeUicOptions(std::vector<std::string> &opts,
                         const std::vector<std::string> &fileOpts,
                         bool isQt5)
{
  static const char* valueOptions[] = {
    "tr",
    "translate",
    "postfix",
    "generator",
    "include", // Since Qt 5.3
    "g"
  };
  std::vector<std::string> extraOpts;
  for(std::vector<std::string>::const_iterator it = fileOpts.begin();
      it != fileOpts.end(); ++it)
    {
    std::vector<std::string>::iterator existingIt
                                  = std::find(opts.begin(), opts.end(), *it);
    if (existingIt != opts.end())
      {
      const char *o = it->c_str();
      if (*o == '-')
        {
        ++o;
        }
      if (isQt5 && *o == '-')
        {
        ++o;
        }
      if (std::find_if(cmArrayBegin(valueOptions), cmArrayEnd(valueOptions),
                  cmStrCmp(*it)) != cmArrayEnd(valueOptions))
        {
        assert(existingIt + 1 != opts.end());
        *(existingIt + 1) = *(it + 1);
        ++it;
        }
      }
    else
      {
      extraOpts.push_back(*it);
      }
    }
  opts.insert(opts.end(), extraOpts.begin(), extraOpts.end());
}

bool cmQtAutoGenerators::Run(const std::string& targetDirectory,
                             const std::string& config)
{
  bool success = true;
  cmake cm;
  cm.SetHomeOutputDirectory(targetDirectory);
  cm.SetHomeDirectory(targetDirectory);
  cmGlobalGenerator gg(&cm);

  cmState::Snapshot snapshot = cm.GetCurrentSnapshot();
  cmsys::auto_ptr<cmMakefile> mf(new cmMakefile(&gg, snapshot));
  mf->SetCurrentBinaryDirectory(targetDirectory);
  mf->SetCurrentSourceDirectory(targetDirectory);
  gg.SetCurrentMakefile(mf.get());

  this->ReadAutogenInfoFile(mf.get(), targetDirectory, config);
  this->ReadOldMocDefinitionsFile(mf.get(), targetDirectory);

  this->Init();

  if (this->QtMajorVersion == "4" || this->QtMajorVersion == "5")
    {
    success = this->RunAutogen(mf.get());
    }

  this->WriteOldMocDefinitionsFile(targetDirectory);

  return success;
}

bool cmQtAutoGenerators::ReadAutogenInfoFile(cmMakefile* makefile,
                                      const std::string& targetDirectory,
                                      const std::string& config)
{
  std::string filename(
      cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutogenInfo.cmake";

  if (!makefile->ReadListFile(filename.c_str()))
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
  {
  std::string rccSources = makefile->GetSafeDefinition("AM_RCC_SOURCES");
  cmSystemTools::ExpandListArgument(rccSources, this->RccSources);
  }
  this->SkipMoc = makefile->GetSafeDefinition("AM_SKIP_MOC");
  this->SkipUic = makefile->GetSafeDefinition("AM_SKIP_UIC");
  this->Headers = makefile->GetSafeDefinition("AM_HEADERS");
  this->IncludeProjectDirsBefore = makefile->IsOn(
                                "AM_CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE");
  this->Srcdir = makefile->GetSafeDefinition("AM_CMAKE_CURRENT_SOURCE_DIR");
  this->Builddir = makefile->GetSafeDefinition("AM_CMAKE_CURRENT_BINARY_DIR");
  this->MocExecutable = makefile->GetSafeDefinition("AM_QT_MOC_EXECUTABLE");
  this->UicExecutable = makefile->GetSafeDefinition("AM_QT_UIC_EXECUTABLE");
  this->RccExecutable = makefile->GetSafeDefinition("AM_QT_RCC_EXECUTABLE");
  {
  std::string compileDefsPropOrig = "AM_MOC_COMPILE_DEFINITIONS";
  std::string compileDefsProp = compileDefsPropOrig;
  if(!config.empty())
    {
    compileDefsProp += "_";
    compileDefsProp += config;
    }
  const char *compileDefs = makefile->GetDefinition(compileDefsProp);
  this->MocCompileDefinitionsStr = compileDefs ? compileDefs
                  : makefile->GetSafeDefinition(compileDefsPropOrig);
  }
  {
  std::string includesPropOrig = "AM_MOC_INCLUDES";
  std::string includesProp = includesPropOrig;
  if(!config.empty())
    {
    includesProp += "_";
    includesProp += config;
    }
  const char *includes = makefile->GetDefinition(includesProp);
  this->MocIncludesStr = includes ? includes
                      : makefile->GetSafeDefinition(includesPropOrig);
  }
  this->MocOptionsStr = makefile->GetSafeDefinition("AM_MOC_OPTIONS");
  this->ProjectBinaryDir = makefile->GetSafeDefinition("AM_CMAKE_BINARY_DIR");
  this->ProjectSourceDir = makefile->GetSafeDefinition("AM_CMAKE_SOURCE_DIR");
  this->TargetName = makefile->GetSafeDefinition("AM_TARGET_NAME");
  this->OriginTargetName
                      = makefile->GetSafeDefinition("AM_ORIGIN_TARGET_NAME");

  {
  const char *uicOptionsFiles
                        = makefile->GetSafeDefinition("AM_UIC_OPTIONS_FILES");
  std::string uicOptionsPropOrig = "AM_UIC_TARGET_OPTIONS";
  std::string uicOptionsProp = uicOptionsPropOrig;
  if(!config.empty())
    {
    uicOptionsProp += "_";
    uicOptionsProp += config;
    }
  const char *uicTargetOptions
                        = makefile->GetSafeDefinition(uicOptionsProp);
  cmSystemTools::ExpandListArgument(
      uicTargetOptions ? uicTargetOptions
                    : makefile->GetSafeDefinition(uicOptionsPropOrig),
    this->UicTargetOptions);
  const char *uicOptionsOptions
                      = makefile->GetSafeDefinition("AM_UIC_OPTIONS_OPTIONS");
  std::vector<std::string> uicFilesVec;
  cmSystemTools::ExpandListArgument(uicOptionsFiles, uicFilesVec);
  std::vector<std::string> uicOptionsVec;
  cmSystemTools::ExpandListArgument(uicOptionsOptions, uicOptionsVec);
  if (uicFilesVec.size() != uicOptionsVec.size())
    {
    return false;
    }
  for (std::vector<std::string>::iterator fileIt = uicFilesVec.begin(),
                                            optionIt = uicOptionsVec.begin();
                                            fileIt != uicFilesVec.end();
                                            ++fileIt, ++optionIt)
    {
    cmSystemTools::ReplaceString(*optionIt, "@list_sep@", ";");
    this->UicOptions[*fileIt] = *optionIt;
    }
  }
  {
  const char *rccOptionsFiles
                        = makefile->GetSafeDefinition("AM_RCC_OPTIONS_FILES");
  const char *rccOptionsOptions
                      = makefile->GetSafeDefinition("AM_RCC_OPTIONS_OPTIONS");
  std::vector<std::string> rccFilesVec;
  cmSystemTools::ExpandListArgument(rccOptionsFiles, rccFilesVec);
  std::vector<std::string> rccOptionsVec;
  cmSystemTools::ExpandListArgument(rccOptionsOptions, rccOptionsVec);
  if (rccFilesVec.size() != rccOptionsVec.size())
    {
    return false;
    }
  for (std::vector<std::string>::iterator fileIt = rccFilesVec.begin(),
                                            optionIt = rccOptionsVec.begin();
                                            fileIt != rccFilesVec.end();
                                            ++fileIt, ++optionIt)
    {
    cmSystemTools::ReplaceString(*optionIt, "@list_sep@", ";");
    this->RccOptions[*fileIt] = *optionIt;
    }

  const char *rccInputs = makefile->GetSafeDefinition("AM_RCC_INPUTS");
  std::vector<std::string> rccInputLists;
  cmSystemTools::ExpandListArgument(rccInputs, rccInputLists);

  if (this->RccSources.size() != rccInputLists.size())
    {
    cmSystemTools::Error("Error processing file: ", filename.c_str());
    return false;
    }

  for (std::vector<std::string>::iterator fileIt = this->RccSources.begin(),
                                            inputIt = rccInputLists.begin();
                                            fileIt != this->RccSources.end();
                                            ++fileIt, ++inputIt)
    {
    cmSystemTools::ReplaceString(*inputIt, "@list_sep@", ";");
    std::vector<std::string> rccInputFiles;
    cmSystemTools::ExpandListArgument(*inputIt, rccInputFiles);

    this->RccInputs[*fileIt] = rccInputFiles;
    }
  }
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
                                            const std::string& targetDirectory)
{
  std::string filename(
      cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutomocOldMocDefinitions.cmake";

  if (makefile->ReadListFile(filename.c_str()))
    {
    this->OldCompileSettingsStr =
                        makefile->GetSafeDefinition("AM_OLD_COMPILE_SETTINGS");
    }
  return true;
}


void
cmQtAutoGenerators::WriteOldMocDefinitionsFile(
                                            const std::string& targetDirectory)
{
  std::string filename(
      cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutomocOldMocDefinitions.cmake";

  cmsys::ofstream outfile;
  outfile.open(filename.c_str(),
               std::ios::trunc);
  outfile << "set(AM_OLD_COMPILE_SETTINGS "
              << cmOutputConverter::EscapeForCMake(
                 this->CurrentCompileSettingsStr) << ")\n";

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
    if (cmHasLiteralSuffix(path, ".framework/Headers"))
      {
      // Go up twice to get to the framework root
      std::vector<std::string> pathComponents;
      cmsys::SystemTools::SplitPath(path, pathComponents);
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
      const std::string binDir = "-I" + this->ProjectBinaryDir;

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

static std::string ReadAll(const std::string& filename)
{
  cmsys::ifstream file(filename.c_str());
  std::stringstream stream;
  stream << file.rdbuf();
  file.close();
  return stream.str();
}

bool cmQtAutoGenerators::RunAutogen(cmMakefile* makefile)
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

  std::map<std::string, std::vector<std::string> > includedUis;
  std::map<std::string, std::vector<std::string> > skippedUis;
  std::vector<std::string> uicSkipped;
  cmSystemTools::ExpandListArgument(this->SkipUic, uicSkipped);

  for (std::vector<std::string>::const_iterator it = sourceFiles.begin();
       it != sourceFiles.end();
       ++it)
    {
    const bool skipUic = std::find(uicSkipped.begin(), uicSkipped.end(), *it)
        != uicSkipped.end();
    std::map<std::string, std::vector<std::string> >& uiFiles
                                          = skipUic ? skippedUis : includedUis;
    const std::string &absFilename = *it;
    if (this->Verbose)
      {
      std::cout << "AUTOGEN: Checking " << absFilename << std::endl;
      }
    if (this->RelaxedMode)
      {
      this->ParseCppFile(absFilename, headerExtensions, includedMocs,
                         uiFiles);
      }
    else
      {
      this->StrictParseCppFile(absFilename, headerExtensions, includedMocs,
                               uiFiles);
      }
    this->SearchHeadersForCppFile(absFilename, headerExtensions, headerFiles);
    }

  {
  std::vector<std::string> mocSkipped;
  cmSystemTools::ExpandListArgument(this->SkipMoc, mocSkipped);
  for (std::vector<std::string>::const_iterator it = mocSkipped.begin();
       it != mocSkipped.end();
       ++it)
    {
    if (std::find(uicSkipped.begin(), uicSkipped.end(), *it)
        != uicSkipped.end())
      {
      const std::string &absFilename = *it;
      if (this->Verbose)
        {
        std::cout << "AUTOGEN: Checking " << absFilename << std::endl;
        }
      this->ParseForUic(absFilename, includedUis);
      }
    }
  }

  std::vector<std::string> headerFilesVec;
  cmSystemTools::ExpandListArgument(this->Headers, headerFilesVec);
  headerFiles.insert(headerFilesVec.begin(), headerFilesVec.end());

  // key = moc source filepath, value = moc output filename
  std::map<std::string, std::string> notIncludedMocs;
  this->ParseHeaders(headerFiles, includedMocs, notIncludedMocs, includedUis);

  // run moc on all the moc's that are #included in source files
  for(std::map<std::string, std::string>::const_iterator
                                                     it = includedMocs.begin();
      it != includedMocs.end();
      ++it)
    {
    this->GenerateMoc(it->first, it->second);
    }
  for(std::map<std::string, std::vector<std::string> >::const_iterator
      it = includedUis.begin();
      it != includedUis.end();
      ++it)
    {
    for (std::vector<std::string>::const_iterator nit = it->second.begin();
        nit != it->second.end();
        ++nit)
      {
      this->GenerateUi(it->first, *nit);
      }
    }

  if(!this->RccExecutable.empty())
    {
    this->GenerateQrc();
    }

  std::stringstream outStream;
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
    std::cerr << "moc failed..." << std::endl;
    return false;
    }

  if (this->RunUicFailed)
    {
    std::cerr << "uic failed..." << std::endl;
    return false;
    }
  if (this->RunRccFailed)
    {
    std::cerr << "rcc failed..." << std::endl;
    return false;
    }
  outStream.flush();
  std::string automocSource = outStream.str();
  if (!automocCppChanged)
    {
    // compare contents of the _automoc.cpp file
    const std::string oldContents = ReadAll(this->OutMocCppFilename);
    if (oldContents == automocSource)
      {
      // nothing changed: don't touch the _automoc.cpp file
      return true;
      }
    }

  // source file that includes all remaining moc files (_automoc.cpp file)
  cmsys::ofstream outfile;
  outfile.open(this->OutMocCppFilename.c_str(),
               std::ios::trunc);
  outfile << automocSource;
  outfile.close();

  return true;
}


void cmQtAutoGenerators::ParseCppFile(const std::string& absFilename,
                const std::vector<std::string>& headerExtensions,
                std::map<std::string, std::string>& includedMocs,
                std::map<std::string, std::vector<std::string> > &includedUis)
{
  cmsys::RegularExpression mocIncludeRegExp(
              "[\n][ \t]*#[ \t]*include[ \t]+"
              "[\"<](([^ \">]+/)?moc_[^ \">/]+\\.cpp|[^ \">]+\\.moc)[\">]");

  const std::string contentsString = ReadAll(absFilename);
  if (contentsString.empty())
    {
    std::cerr << "AUTOGEN: warning: " << absFilename << ": file is empty\n"
              << std::endl;
    return;
    }
  this->ParseForUic(absFilename, contentsString, includedUis);
  if (this->MocExecutable.empty())
    {
    return;
    }

  const std::string absPath = cmsys::SystemTools::GetFilenamePath(
                   cmsys::SystemTools::GetRealPath(absFilename)) + '/';
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
      const bool moc_style = cmHasLiteralPrefix(basename, "moc_");

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
          std::cerr << "AUTOGEN: error: " << absFilename << ": The file "
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
              std::cerr << "AUTOGEN: warning: " << absFilename << ": The file "
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
              std::cerr << "AUTOGEN: warning: " << absFilename << ": The file "
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
            std::cerr <<"AUTOGEN: error: " << absFilename << ": The file "
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
      std::cerr << "AUTOGEN: warning: " << absFilename << ": The file "
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
      std::cerr << "AUTOGEN: error: " << absFilename << ": The file "
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
                std::map<std::string, std::string>& includedMocs,
                std::map<std::string, std::vector<std::string> >& includedUis)
{
  cmsys::RegularExpression mocIncludeRegExp(
              "[\n][ \t]*#[ \t]*include[ \t]+"
              "[\"<](([^ \">]+/)?moc_[^ \">/]+\\.cpp|[^ \">]+\\.moc)[\">]");

  const std::string contentsString = ReadAll(absFilename);
  if (contentsString.empty())
    {
    std::cerr << "AUTOGEN: warning: " << absFilename << ": file is empty\n"
              << std::endl;
    return;
    }
  this->ParseForUic(absFilename, contentsString, includedUis);
  if (this->MocExecutable.empty())
    {
    return;
    }

  const std::string absPath = cmsys::SystemTools::GetFilenamePath(
                   cmsys::SystemTools::GetRealPath(absFilename)) + '/';
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
      const bool mocUnderscoreStyle = cmHasLiteralPrefix(basename, "moc_");

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
          std::cerr << "AUTOGEN: error: " << absFilename << " The file "
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
          std::cerr <<"AUTOGEN: error: " << absFilename << ": The file "
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
    std::cerr << "AUTOGEN: error: " << absFilename << ": The file "
              << "contains a " << macroName << " macro, but does not include "
              << "\"" << scannedFileBasename << ".moc\" !\n"
              << std::endl;
    ::exit(EXIT_FAILURE);
    }

}


void cmQtAutoGenerators::ParseForUic(const std::string& absFilename,
                std::map<std::string, std::vector<std::string> >& includedUis)
{
  if (this->UicExecutable.empty())
    {
    return;
    }
  const std::string contentsString = ReadAll(absFilename);
  if (contentsString.empty())
    {
    std::cerr << "AUTOGEN: warning: " << absFilename << ": file is empty\n"
              << std::endl;
    return;
    }
  this->ParseForUic(absFilename, contentsString, includedUis);
}


void cmQtAutoGenerators::ParseForUic(const std::string& absFilename,
                const std::string& contentsString,
                std::map<std::string, std::vector<std::string> >& includedUis)
{
  if (this->UicExecutable.empty())
    {
    return;
    }
  cmsys::RegularExpression uiIncludeRegExp(
              "[\n][ \t]*#[ \t]*include[ \t]+"
              "[\"<](([^ \">]+/)?ui_[^ \">/]+\\.h)[\">]");

  std::string::size_type matchOffset = 0;

  const std::string realName =
                   cmsys::SystemTools::GetRealPath(absFilename);

  matchOffset = 0;
  if ((strstr(contentsString.c_str(), "ui_") != NULL)
                                    && (uiIncludeRegExp.find(contentsString)))
    {
    do
      {
      const std::string currentUi = uiIncludeRegExp.match(1);

      std::string basename = cmsys::SystemTools::
                                  GetFilenameWithoutLastExtension(currentUi);

      // basename should be the part of the ui filename used for
      // finding the correct header, so we need to remove the ui_ part
      basename = basename.substr(3);

      includedUis[realName].push_back(basename);

      matchOffset += uiIncludeRegExp.end();
      } while(uiIncludeRegExp.find(contentsString.c_str() + matchOffset));
    }
}


void
cmQtAutoGenerators::SearchHeadersForCppFile(const std::string& absFilename,
                              const std::vector<std::string>& headerExtensions,
                              std::set<std::string>& absHeaders)
{
  // search for header files and private header files we may need to moc:
  const std::string basename =
              cmsys::SystemTools::GetFilenameWithoutLastExtension(absFilename);
  const std::string absPath = cmsys::SystemTools::GetFilenamePath(
                   cmsys::SystemTools::GetRealPath(absFilename)) + '/';

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
                std::map<std::string, std::string>& notIncludedMocs,
                std::map<std::string, std::vector<std::string> >& includedUis)
{
  for(std::set<std::string>::const_iterator hIt=absHeaders.begin();
      hIt!=absHeaders.end();
      ++hIt)
    {
    const std::string& headerName = *hIt;
    const std::string contents = ReadAll(headerName);

    if (!this->MocExecutable.empty()
        && includedMocs.find(headerName) == includedMocs.end())
      {
      if (this->Verbose)
        {
        std::cout << "AUTOGEN: Checking " << headerName << std::endl;
        }

      const std::string basename = cmsys::SystemTools::
                                   GetFilenameWithoutLastExtension(headerName);

      const std::string currentMoc = "moc_" + basename + ".cpp";
      std::string macroName;
      if (requiresMocing(contents, macroName))
        {
        //std::cout << "header contains Q_OBJECT macro";
        notIncludedMocs[headerName] = currentMoc;
        }
      }
    this->ParseForUic(headerName, contents, includedUis);
    }
}

bool cmQtAutoGenerators::GenerateMoc(const std::string& sourceFile,
                              const std::string& mocFileName)
{
  const std::string mocFilePath = this->Builddir + mocFileName;
  int sourceNewerThanMoc = 0;
  bool success = cmsys::SystemTools::FileTimeCompare(sourceFile,
                                                     mocFilePath,
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

    std::vector<std::string> command;
    command.push_back(this->MocExecutable);
    command.insert(command.end(),
                   this->MocIncludes.begin(), this->MocIncludes.end());
    command.insert(command.end(),
                   this->MocDefinitions.begin(), this->MocDefinitions.end());
    command.insert(command.end(),
                   this->MocOptions.begin(), this->MocOptions.end());
#ifdef _WIN32
    command.push_back("-DWIN32");
#endif
    command.push_back("-o");
    command.push_back(mocFilePath);
    command.push_back(sourceFile);

    if (this->Verbose)
      {
      for(std::vector<std::string>::const_iterator cmdIt = command.begin();
          cmdIt != command.end();
          ++cmdIt)
        {
        std::cout << *cmdIt << " ";
        }
      std::cout << std::endl;
      }

    std::string output;
    int retVal = 0;
    bool result = cmSystemTools::RunSingleCommand(command, &output, &output,
                                                  &retVal);
    if (!result || retVal)
      {
      std::cerr << "AUTOGEN: error: process for " << mocFilePath <<" failed:\n"
                << output << std::endl;
      this->RunMocFailed = true;
      cmSystemTools::RemoveFile(mocFilePath);
      }
    return true;
    }
  return false;
}

bool cmQtAutoGenerators::GenerateUi(const std::string& realName,
                                    const std::string& uiFileName)
{
  if (!cmsys::SystemTools::FileExists(this->Builddir.c_str(), false))
    {
    cmsys::SystemTools::MakeDirectory(this->Builddir.c_str());
    }

  const std::string path = cmsys::SystemTools::GetFilenamePath(
                                                      realName) + '/';

  std::string ui_output_file = "ui_" + uiFileName + ".h";
  std::string ui_input_file = path + uiFileName + ".ui";

  int sourceNewerThanUi = 0;
  bool success = cmsys::SystemTools::FileTimeCompare(ui_input_file,
                                    this->Builddir + ui_output_file,
                                                     &sourceNewerThanUi);
  if (this->GenerateAll || !success || sourceNewerThanUi >= 0)
    {
    std::string msg = "Generating ";
    msg += ui_output_file;
    cmSystemTools::MakefileColorEcho(cmsysTerminal_Color_ForegroundBlue
                                          |cmsysTerminal_Color_ForegroundBold,
                                      msg.c_str(), true, this->ColorOutput);

    std::vector<std::string> command;
    command.push_back(this->UicExecutable);

    std::vector<std::string> opts = this->UicTargetOptions;
    std::map<std::string, std::string>::const_iterator optionIt
            = this->UicOptions.find(ui_input_file);
    if (optionIt != this->UicOptions.end())
      {
      std::vector<std::string> fileOpts;
      cmSystemTools::ExpandListArgument(optionIt->second, fileOpts);
      cmQtAutoGenerators::MergeUicOptions(opts, fileOpts,
                                          this->QtMajorVersion == "5");
      }
    command.insert(command.end(), opts.begin(), opts.end());

    command.push_back("-o");
    command.push_back(this->Builddir + ui_output_file);
    command.push_back(ui_input_file);

    if (this->Verbose)
      {
      for(std::vector<std::string>::const_iterator cmdIt = command.begin();
          cmdIt != command.end();
          ++cmdIt)
        {
        std::cout << *cmdIt << " ";
        }
      std::cout << std::endl;
      }
    std::string output;
    int retVal = 0;
    bool result = cmSystemTools::RunSingleCommand(command, &output, &output,
                                                  &retVal);
    if (!result || retVal)
      {
      std::cerr << "AUTOUIC: error: process for " << ui_output_file <<
                " needed by\n \"" << realName << "\"\nfailed:\n" << output
                << std::endl;
      this->RunUicFailed = true;
      cmSystemTools::RemoveFile(ui_output_file);
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
       it != files.end(); ++it)
    {
    int inputNewerThanQrc = 0;
    bool success = cmsys::SystemTools::FileTimeCompare(*it,
                                                      rccOutput,
                                                      &inputNewerThanQrc);
    if (!success || inputNewerThanQrc >= 0)
      {
      return true;
      }
    }
  return false;
}

bool cmQtAutoGenerators::GenerateQrc()
{
  for(std::vector<std::string>::const_iterator si = this->RccSources.begin();
      si != this->RccSources.end(); ++si)
    {
    std::string ext = cmsys::SystemTools::GetFilenameLastExtension(*si);

    if (ext != ".qrc")
      {
      continue;
      }
    std::vector<std::string> command;
    command.push_back(this->RccExecutable);

    std::string basename = cmsys::SystemTools::
                                  GetFilenameWithoutLastExtension(*si);

    std::string rcc_output_file = this->Builddir
                                + "CMakeFiles/" + this->OriginTargetName
                                + ".dir/qrc_" + basename + ".cpp";

    int sourceNewerThanQrc = 0;
    bool generateQrc = !cmsys::SystemTools::FileTimeCompare(*si,
                                                      rcc_output_file,
                                                      &sourceNewerThanQrc);
    generateQrc = generateQrc || (sourceNewerThanQrc >= 0);
    generateQrc = generateQrc || this->InputFilesNewerThanQrc(*si,
                                                          rcc_output_file);

    if (this->GenerateAll || generateQrc)
      {
      std::map<std::string, std::string>::const_iterator optionIt
              = this->RccOptions.find(*si);
      if (optionIt != this->RccOptions.end())
        {
        cmSystemTools::ExpandListArgument(optionIt->second, command);
        }

      command.push_back("-name");
      command.push_back(basename);
      command.push_back("-o");
      command.push_back(rcc_output_file);
      command.push_back(*si);

      if (this->Verbose)
        {
        for(std::vector<std::string>::const_iterator cmdIt = command.begin();
            cmdIt != command.end();
            ++cmdIt)
          {
          std::cout << *cmdIt << " ";
          }
        std::cout << std::endl;
        }
      std::string output;
      int retVal = 0;
      bool result = cmSystemTools::RunSingleCommand(command, &output, &output,
                                                    &retVal);
      if (!result || retVal)
        {
        std::cerr << "AUTORCC: error: process for " << rcc_output_file <<
                  " failed:\n" << output << std::endl;
        this->RunRccFailed = true;
        cmSystemTools::RemoveFile(rcc_output_file);
        return false;
        }
      }
    }
  return true;
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


bool cmQtAutoGenerators::StartsWith(const std::string& str,
                                    const std::string& with)
{
  return (str.substr(0, with.length()) == with);
}


bool cmQtAutoGenerators::EndsWith(const std::string& str,
                                  const std::string& with)
{
  if (with.length() > (str.length()))
    {
    return false;
    }
  return (str.substr(str.length() - with.length(), with.length()) == with);
}
