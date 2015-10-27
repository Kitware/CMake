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

#include "cmQtAutoGeneratorInitializer.h"

#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"

#include <sys/stat.h>

#include <cmsys/FStream.hxx>

#if defined(_WIN32) && !defined(__CYGWIN__)
# include "cmGlobalVisualStudioGenerator.h"
#endif

std::string cmQtAutoGeneratorInitializer::GetAutogenTargetName(
    cmTarget const* target)
{
  std::string autogenTargetName = target->GetName();
  autogenTargetName += "_automoc";
  return autogenTargetName;
}

std::string cmQtAutoGeneratorInitializer::GetAutogenTargetDir(
    cmTarget const* target)
{
  cmMakefile* makefile = target->GetMakefile();
  std::string targetDir = makefile->GetCurrentBinaryDirectory();
  targetDir += makefile->GetCMakeInstance()->GetCMakeFilesDirectory();
  targetDir += "/";
  targetDir += cmQtAutoGeneratorInitializer::GetAutogenTargetName(target);
  targetDir += ".dir/";
  return targetDir;
}

static void copyTargetProperty(cmTarget* destinationTarget,
                               cmTarget* sourceTarget,
                               const std::string& propertyName)
{
  const char* propertyValue = sourceTarget->GetProperty(propertyName);
  if (propertyValue)
    {
    destinationTarget->SetProperty(propertyName, propertyValue);
    }
}

static std::string cmQtAutoGeneratorsStripCR(std::string const& line)
{
  // Strip CR characters rcc may have printed (possibly more than one!).
  std::string::size_type cr = line.find('\r');
  if (cr != line.npos)
    {
    return line.substr(0, cr);
    }
  return line;
}

static std::string ReadAll(const std::string& filename)
{
  cmsys::ifstream file(filename.c_str());
  std::stringstream stream;
  stream << file.rdbuf();
  file.close();
  return stream.str();
}

std::string cmQtAutoGeneratorInitializer::ListQt5RccInputs(cmSourceFile* sf,
                                            cmTarget const* target,
                                            std::vector<std::string>& depends)
{
  std::string rccCommand
      = cmQtAutoGeneratorInitializer::GetRccExecutable(target);
  std::vector<std::string> qrcEntries;

  std::vector<std::string> command;
  command.push_back(rccCommand);
  command.push_back("--list");

  std::string absFile = cmsys::SystemTools::GetRealPath(
                                              sf->GetFullPath());

  command.push_back(absFile);

  std::string rccStdOut;
  std::string rccStdErr;
  int retVal = 0;
  bool result = cmSystemTools::RunSingleCommand(
    command, &rccStdOut, &rccStdErr,
    &retVal, 0, cmSystemTools::OUTPUT_NONE);
  if (!result || retVal)
    {
    std::cerr << "AUTOGEN: error: Rcc list process for " << sf->GetFullPath()
              << " failed:\n" << rccStdOut << "\n" << rccStdErr << std::endl;
    return std::string();
    }

  {
  std::istringstream ostr(rccStdOut);
  std::string oline;
  while(std::getline(ostr, oline))
    {
    oline = cmQtAutoGeneratorsStripCR(oline);
    if(!oline.empty())
      {
      qrcEntries.push_back(oline);
      }
    }
  }

  {
  std::istringstream estr(rccStdErr);
  std::string eline;
  while(std::getline(estr, eline))
    {
    eline = cmQtAutoGeneratorsStripCR(eline);
    if (cmHasLiteralPrefix(eline, "RCC: Error in"))
      {
      static std::string searchString = "Cannot find file '";

      std::string::size_type pos = eline.find(searchString);
      if (pos == std::string::npos)
        {
        std::cerr << "AUTOGEN: error: Rcc lists unparsable output "
                  << eline << std::endl;
        return std::string();
        }
      pos += searchString.length();
      std::string::size_type sz = eline.size() - pos - 1;
      qrcEntries.push_back(eline.substr(pos, sz));
      }
    }
  }

  depends.insert(depends.end(), qrcEntries.begin(), qrcEntries.end());
  return cmJoin(qrcEntries, "@list_sep@");
}

std::string cmQtAutoGeneratorInitializer::ListQt4RccInputs(cmSourceFile* sf,
                                            std::vector<std::string>& depends)
{
  const std::string qrcContents = ReadAll(sf->GetFullPath());

  cmsys::RegularExpression fileMatchRegex("(<file[^<]+)");

  std::string entriesList;
  const char* sep = "";

  size_t offset = 0;
  while (fileMatchRegex.find(qrcContents.c_str() + offset))
    {
    std::string qrcEntry = fileMatchRegex.match(1);

    offset += qrcEntry.size();

    cmsys::RegularExpression fileReplaceRegex("(^<file[^>]*>)");
    fileReplaceRegex.find(qrcEntry);
    std::string tag = fileReplaceRegex.match(1);

    qrcEntry = qrcEntry.substr(tag.size());

    if (!cmSystemTools::FileIsFullPath(qrcEntry.c_str()))
      {
      qrcEntry = sf->GetLocation().GetDirectory() + "/" + qrcEntry;
      }

    entriesList += sep;
    entriesList += qrcEntry;
    sep = "@list_sep@";
    depends.push_back(qrcEntry);
    }
  return entriesList;
}


void cmQtAutoGeneratorInitializer::InitializeAutogenSources(cmTarget* target)
{
  cmMakefile* makefile = target->GetMakefile();

  if (target->GetPropertyAsBool("AUTOMOC"))
    {
    std::string automocTargetName =
        cmQtAutoGeneratorInitializer::GetAutogenTargetName(target);
    std::string mocCppFile = makefile->GetCurrentBinaryDirectory();
    mocCppFile += "/";
    mocCppFile += automocTargetName;
    mocCppFile += ".cpp";
    makefile->GetOrCreateSource(mocCppFile, true);
    makefile->AppendProperty("ADDITIONAL_MAKE_CLEAN_FILES",
                            mocCppFile.c_str(), false);

    target->AddSource(mocCppFile);
    }
}

void cmQtAutoGeneratorInitializer::InitializeAutogenTarget(
                                                 cmLocalGenerator* lg,
                                                 cmTarget* target)
{
  cmMakefile* makefile = target->GetMakefile();

  std::string qtMajorVersion = makefile->GetSafeDefinition("QT_VERSION_MAJOR");
  if (qtMajorVersion == "")
    {
    qtMajorVersion = makefile->GetSafeDefinition("Qt5Core_VERSION_MAJOR");
    }

  // create a custom target for running generators at buildtime:
  std::string autogenTargetName =
      cmQtAutoGeneratorInitializer::GetAutogenTargetName(target);

  std::string targetDir =
      cmQtAutoGeneratorInitializer::GetAutogenTargetDir(target);

  cmCustomCommandLine currentLine;
  currentLine.push_back(cmSystemTools::GetCMakeCommand());
  currentLine.push_back("-E");
  currentLine.push_back("cmake_autogen");
  currentLine.push_back(targetDir);
  currentLine.push_back("$<CONFIGURATION>");

  cmCustomCommandLines commandLines;
  commandLines.push_back(currentLine);

  std::string workingDirectory = cmSystemTools::CollapseFullPath(
                                    "", makefile->GetCurrentBinaryDirectory());

  std::vector<std::string> depends;
  if (const char *autogenDepends =
                                target->GetProperty("AUTOGEN_TARGET_DEPENDS"))
    {
    cmSystemTools::ExpandListArgument(autogenDepends, depends);
    }
  std::vector<std::string> toolNames;
  if (target->GetPropertyAsBool("AUTOMOC"))
    {
    toolNames.push_back("moc");
    }
  if (target->GetPropertyAsBool("AUTOUIC"))
    {
    toolNames.push_back("uic");
    }
  if (target->GetPropertyAsBool("AUTORCC"))
    {
    toolNames.push_back("rcc");
    }

  std::string tools = toolNames[0];
  toolNames.erase(toolNames.begin());
  while (toolNames.size() > 1)
    {
    tools += ", " + toolNames[0];
    toolNames.erase(toolNames.begin());
    }
  if (toolNames.size() == 1)
    {
    tools += " and " + toolNames[0];
    }
  std::string autogenComment = "Automatic " + tools + " for target ";
  autogenComment += target->GetName();

#if defined(_WIN32) && !defined(__CYGWIN__)
  bool usePRE_BUILD = false;
  cmGlobalGenerator* gg = lg->GetGlobalGenerator();
  if(gg->GetName().find("Visual Studio") != std::string::npos)
    {
    cmGlobalVisualStudioGenerator* vsgg =
      static_cast<cmGlobalVisualStudioGenerator*>(gg);
    // Under VS >= 7 use a PRE_BUILD event instead of a separate target to
    // reduce the number of targets loaded into the IDE.
    // This also works around a VS 11 bug that may skip updating the target:
    //  https://connect.microsoft.com/VisualStudio/feedback/details/769495
    usePRE_BUILD = vsgg->GetVersion() >= cmGlobalVisualStudioGenerator::VS7;
    if(usePRE_BUILD)
      {
      for (std::vector<std::string>::iterator it = depends.begin();
            it != depends.end(); ++it)
        {
        if(!makefile->FindTargetToUse(it->c_str()))
          {
          usePRE_BUILD = false;
          break;
          }
        }
      }
    }
#endif

  std::vector<std::string> rcc_output;
  bool const isNinja =
    lg->GetGlobalGenerator()->GetName() == "Ninja";
  if(isNinja
#if defined(_WIN32) && !defined(__CYGWIN__)
        || usePRE_BUILD
#endif
        )
    {
    std::vector<cmSourceFile*> srcFiles;
    cmGeneratorTarget* gtgt =
        lg->GetGlobalGenerator()->GetGeneratorTarget(target);
    gtgt->GetConfigCommonSourceFiles(srcFiles);
    for(std::vector<cmSourceFile*>::const_iterator fileIt = srcFiles.begin();
        fileIt != srcFiles.end();
        ++fileIt)
      {
      cmSourceFile* sf = *fileIt;
      std::string absFile = cmsys::SystemTools::GetRealPath(
                                                sf->GetFullPath());

      std::string ext = sf->GetExtension();

      if (target->GetPropertyAsBool("AUTORCC"))
        {
        if (ext == "qrc"
            && !cmSystemTools::IsOn(sf->GetPropertyForUser("SKIP_AUTORCC")))
          {
          std::string basename = cmsys::SystemTools::
                                  GetFilenameWithoutLastExtension(absFile);

          std::string rcc_output_dir = target->GetSupportDirectory();
          cmSystemTools::MakeDirectory(rcc_output_dir.c_str());
          std::string rcc_output_file = rcc_output_dir;
          rcc_output_file += "/qrc_" + basename + ".cpp";
          rcc_output.push_back(rcc_output_file);

          if (!cmSystemTools::IsOn(sf->GetPropertyForUser("GENERATED")))
            {
            if (qtMajorVersion == "5")
              {
              cmQtAutoGeneratorInitializer::ListQt5RccInputs(sf, target,
                                                              depends);
              }
            else
              {
              cmQtAutoGeneratorInitializer::ListQt4RccInputs(sf, depends);
              }
#if defined(_WIN32) && !defined(__CYGWIN__)
            // Cannot use PRE_BUILD because the resource files themselves
            // may not be sources within the target so VS may not know the
            // target needs to re-build at all.
            usePRE_BUILD = false;
#endif
            }
          }
        }
      }
    }

#if defined(_WIN32) && !defined(__CYGWIN__)
  if(usePRE_BUILD)
    {
    // Add the pre-build command directly to bypass the OBJECT_LIBRARY
    // rejection in cmMakefile::AddCustomCommandToTarget because we know
    // PRE_BUILD will work for an OBJECT_LIBRARY in this specific case.
    std::vector<std::string> no_output;
    std::vector<std::string> no_byproducts;
    cmCustomCommand cc(makefile, no_output, no_byproducts, depends,
                       commandLines, autogenComment.c_str(),
                       workingDirectory.c_str());
    cc.SetEscapeOldStyle(false);
    cc.SetEscapeAllowMakeVars(true);
    target->AddPreBuildCommand(cc);
    }
  else
#endif
    {
    cmTarget* autogenTarget = makefile->AddUtilityCommand(
                                autogenTargetName, true,
                                workingDirectory.c_str(),
                                /*byproducts=*/rcc_output, depends,
                                commandLines, false, autogenComment.c_str());

    cmGeneratorTarget* gt = new cmGeneratorTarget(autogenTarget, lg);
    makefile->AddGeneratorTarget(autogenTarget, gt);

    // Set target folder
    const char* autogenFolder = makefile->GetState()
                                ->GetGlobalProperty("AUTOMOC_TARGETS_FOLDER");
    if (!autogenFolder)
      {
      autogenFolder = makefile->GetState()
                                ->GetGlobalProperty("AUTOGEN_TARGETS_FOLDER");
      }
    if (autogenFolder && *autogenFolder)
      {
      autogenTarget->SetProperty("FOLDER", autogenFolder);
      }
    else
      {
      // inherit FOLDER property from target (#13688)
      copyTargetProperty(autogenTarget, target, "FOLDER");
      }

    target->AddUtility(autogenTargetName);
    }
}

static void GetCompileDefinitionsAndDirectories(cmTarget const* target,
                                                const std::string& config,
                                                std::string &incs,
                                                std::string &defs)
{
  cmMakefile* makefile = target->GetMakefile();
  cmGlobalGenerator* globalGen = makefile->GetGlobalGenerator();
  std::vector<std::string> includeDirs;
  cmGeneratorTarget *gtgt = globalGen->GetGeneratorTarget(target);
  cmLocalGenerator *localGen = gtgt->GetLocalGenerator();
  // Get the include dirs for this target, without stripping the implicit
  // include dirs off, see http://public.kitware.com/Bug/view.php?id=13667
  localGen->GetIncludeDirectories(includeDirs, gtgt, "CXX", config, false);

  incs = cmJoin(includeDirs, ";");

  std::set<std::string> defines;
  localGen->AddCompileDefinitions(defines, target, config, "CXX");

  defs += cmJoin(defines, ";");
}

void cmQtAutoGeneratorInitializer::SetupAutoGenerateTarget(
    cmTarget const* target)
{
  cmMakefile* makefile = target->GetMakefile();

  // forget the variables added here afterwards again:
  cmMakefile::ScopePushPop varScope(makefile);
  static_cast<void>(varScope);

  // create a custom target for running generators at buildtime:
  std::string autogenTargetName =
      cmQtAutoGeneratorInitializer::GetAutogenTargetName(target);

  makefile->AddDefinition("_moc_target_name",
          cmOutputConverter::EscapeForCMake(autogenTargetName).c_str());
  makefile->AddDefinition("_origin_target_name",
          cmOutputConverter::EscapeForCMake(target->GetName()).c_str());

  std::string targetDir =
      cmQtAutoGeneratorInitializer::GetAutogenTargetDir(target);

  const char *qtVersion = makefile->GetDefinition("Qt5Core_VERSION_MAJOR");
  if (!qtVersion)
    {
    qtVersion = makefile->GetDefinition("QT_VERSION_MAJOR");
    }
  cmGeneratorTarget *gtgt = target->GetMakefile()
                                  ->GetGlobalGenerator()
                                  ->GetGeneratorTarget(target);
  if (const char *targetQtVersion =
      gtgt->GetLinkInterfaceDependentStringProperty("QT_MAJOR_VERSION", ""))
    {
    qtVersion = targetQtVersion;
    }
  if (qtVersion)
    {
    makefile->AddDefinition("_target_qt_version", qtVersion);
    }

  std::vector<std::string> skipUic;
  std::vector<std::string> skipMoc;
  std::vector<std::string> mocSources;
  std::vector<std::string> mocHeaders;
  std::map<std::string, std::string> configIncludes;
  std::map<std::string, std::string> configDefines;
  std::map<std::string, std::string> configUicOptions;

  if (target->GetPropertyAsBool("AUTOMOC")
      || target->GetPropertyAsBool("AUTOUIC")
      || target->GetPropertyAsBool("AUTORCC"))
    {
    cmQtAutoGeneratorInitializer::SetupSourceFiles(target, skipMoc,
                                         mocSources, mocHeaders, skipUic);
    }
  makefile->AddDefinition("_cpp_files",
          cmOutputConverter::EscapeForCMake(cmJoin(mocSources, ";")).c_str());
  if (target->GetPropertyAsBool("AUTOMOC"))
    {
    cmQtAutoGeneratorInitializer::SetupAutoMocTarget(target, autogenTargetName,
                             skipMoc, mocHeaders,
                             configIncludes, configDefines);
    }
  if (target->GetPropertyAsBool("AUTOUIC"))
    {
    cmQtAutoGeneratorInitializer::SetupAutoUicTarget(target, skipUic,
                                                      configUicOptions);
    }
  if (target->GetPropertyAsBool("AUTORCC"))
    {
    cmQtAutoGeneratorInitializer::SetupAutoRccTarget(target);
    }

  const char* cmakeRoot = makefile->GetSafeDefinition("CMAKE_ROOT");
  std::string inputFile = cmakeRoot;
  inputFile += "/Modules/AutogenInfo.cmake.in";
  std::string outputFile = targetDir;
  outputFile += "/AutogenInfo.cmake";
  makefile->AddDefinition("_qt_rcc_inputs",
              makefile->GetDefinition("_qt_rcc_inputs_" + target->GetName()));
  makefile->ConfigureFile(inputFile.c_str(), outputFile.c_str(),
                          false, true, false);

  // Ensure we have write permission in case .in was read-only.
  mode_t perm = 0;
#if defined(_WIN32) && !defined(__CYGWIN__)
  mode_t mode_write = S_IWRITE;
#else
  mode_t mode_write = S_IWUSR;
#endif
  cmSystemTools::GetPermissions(outputFile, perm);
  if (!(perm & mode_write))
    {
    cmSystemTools::SetPermissions(outputFile, perm | mode_write);
    }
  if (!configDefines.empty()
      || !configIncludes.empty()
      || !configUicOptions.empty())
    {
    cmsys::ofstream infoFile(outputFile.c_str(), std::ios::app);
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
    if (!configUicOptions.empty())
      {
      for (std::map<std::string, std::string>::iterator
            it = configUicOptions.begin(), end = configUicOptions.end();
            it != end; ++it)
        {
        infoFile << "set(AM_UIC_TARGET_OPTIONS_" << it->first <<
          " " << it->second << ")\n";
        }
      }
    }
}

void cmQtAutoGeneratorInitializer::SetupSourceFiles(cmTarget const* target,
                                   std::vector<std::string>& skipMoc,
                                   std::vector<std::string>& mocSources,
                                   std::vector<std::string>& mocHeaders,
                                   std::vector<std::string>& skipUic)
{
  cmMakefile* makefile = target->GetMakefile();

  std::vector<cmSourceFile*> srcFiles;
  cmGeneratorTarget *gtgt = target->GetMakefile()
                                  ->GetGlobalGenerator()
                                  ->GetGeneratorTarget(target);
  gtgt->GetConfigCommonSourceFiles(srcFiles);

  std::vector<std::string> newRccFiles;

  for(std::vector<cmSourceFile*>::const_iterator fileIt = srcFiles.begin();
      fileIt != srcFiles.end();
      ++fileIt)
    {
    cmSourceFile* sf = *fileIt;
    std::string absFile = cmsys::SystemTools::GetRealPath(
                                                    sf->GetFullPath());
    bool skipFileForMoc =
        cmSystemTools::IsOn(sf->GetPropertyForUser("SKIP_AUTOMOC"));
    bool generated = cmSystemTools::IsOn(sf->GetPropertyForUser("GENERATED"));

    if(cmSystemTools::IsOn(sf->GetPropertyForUser("SKIP_AUTOUIC")))
      {
      skipUic.push_back(absFile);
      }

    std::string ext = sf->GetExtension();

    if (target->GetPropertyAsBool("AUTORCC"))
      {
      if (ext == "qrc"
          && !cmSystemTools::IsOn(sf->GetPropertyForUser("SKIP_AUTORCC")))
        {
        std::string basename = cmsys::SystemTools::
                                      GetFilenameWithoutLastExtension(absFile);

        std::string rcc_output_dir = target->GetSupportDirectory();
        cmSystemTools::MakeDirectory(rcc_output_dir.c_str());
        std::string rcc_output_file = rcc_output_dir;
        rcc_output_file += "/qrc_" + basename + ".cpp";
        makefile->AppendProperty("ADDITIONAL_MAKE_CLEAN_FILES",
                                rcc_output_file.c_str(), false);
        makefile->GetOrCreateSource(rcc_output_file, true);
        newRccFiles.push_back(rcc_output_file);
        }
      }

    if (!generated)
      {
      if (skipFileForMoc)
        {
        skipMoc.push_back(absFile);
        }
      else
        {
        cmSystemTools::FileFormat fileType = cmSystemTools::GetFileFormat(
                                                                ext.c_str());
        if (fileType == cmSystemTools::CXX_FILE_FORMAT)
          {
          mocSources.push_back(absFile);
          }
        else if (fileType == cmSystemTools::HEADER_FILE_FORMAT)
          {
          mocHeaders.push_back(absFile);
          }
        }
      }
    }

  for(std::vector<std::string>::const_iterator fileIt = newRccFiles.begin();
      fileIt != newRccFiles.end();
      ++fileIt)
    {
    const_cast<cmTarget*>(target)->AddSource(*fileIt);
    }
}

void cmQtAutoGeneratorInitializer::SetupAutoMocTarget(cmTarget const* target,
                          const std::string &autogenTargetName,
                          std::vector<std::string> const& skipMoc,
                          std::vector<std::string> const& mocHeaders,
                          std::map<std::string, std::string> &configIncludes,
                          std::map<std::string, std::string> &configDefines)
{
  cmMakefile* makefile = target->GetMakefile();

  const char* tmp = target->GetProperty("AUTOMOC_MOC_OPTIONS");
  std::string _moc_options = (tmp!=0 ? tmp : "");
  makefile->AddDefinition("_moc_options",
          cmOutputConverter::EscapeForCMake(_moc_options).c_str());
  makefile->AddDefinition("_skip_moc",
          cmOutputConverter::EscapeForCMake(cmJoin(skipMoc, ";")).c_str());
  makefile->AddDefinition("_moc_headers",
          cmOutputConverter::EscapeForCMake(cmJoin(mocHeaders, ";")).c_str());
  bool relaxedMode = makefile->IsOn("CMAKE_AUTOMOC_RELAXED_MODE");
  makefile->AddDefinition("_moc_relaxed_mode", relaxedMode ? "TRUE" : "FALSE");

  std::string _moc_incs;
  std::string _moc_compile_defs;
  std::vector<std::string> configs;
  const std::string& config = makefile->GetConfigurations(configs);
  GetCompileDefinitionsAndDirectories(target, config,
                                      _moc_incs, _moc_compile_defs);

  makefile->AddDefinition("_moc_incs",
          cmOutputConverter::EscapeForCMake(_moc_incs).c_str());
  makefile->AddDefinition("_moc_compile_defs",
          cmOutputConverter::EscapeForCMake(_moc_compile_defs).c_str());

  for (std::vector<std::string>::const_iterator li = configs.begin();
       li != configs.end(); ++li)
    {
    std::string config_moc_incs;
    std::string config_moc_compile_defs;
    GetCompileDefinitionsAndDirectories(target, *li,
                                        config_moc_incs,
                                        config_moc_compile_defs);
    if (config_moc_incs != _moc_incs)
      {
      configIncludes[*li] =
                    cmOutputConverter::EscapeForCMake(config_moc_incs);
      if(_moc_incs.empty())
        {
        _moc_incs = config_moc_incs;
        }
      }
    if (config_moc_compile_defs != _moc_compile_defs)
      {
      configDefines[*li] =
            cmOutputConverter::EscapeForCMake(config_moc_compile_defs);
      if(_moc_compile_defs.empty())
        {
        _moc_compile_defs = config_moc_compile_defs;
        }
      }
    }

  const char *qtVersion = makefile->GetDefinition("_target_qt_version");
  if (strcmp(qtVersion, "5") == 0)
    {
    cmTarget *qt5Moc = makefile->FindTargetToUse("Qt5::moc");
    if (!qt5Moc)
      {
      cmSystemTools::Error("Qt5::moc target not found ",
                          autogenTargetName.c_str());
      return;
      }
    makefile->AddDefinition("_qt_moc_executable",
                            qt5Moc->ImportedGetLocation(""));
    }
  else if (strcmp(qtVersion, "4") == 0)
    {
    cmTarget *qt4Moc = makefile->FindTargetToUse("Qt4::moc");
    if (!qt4Moc)
      {
      cmSystemTools::Error("Qt4::moc target not found ",
                          autogenTargetName.c_str());
      return;
      }
    makefile->AddDefinition("_qt_moc_executable",
                            qt4Moc->ImportedGetLocation(""));
    }
  else
    {
    cmSystemTools::Error("The CMAKE_AUTOMOC feature supports only Qt 4 and "
                        "Qt 5 ", autogenTargetName.c_str());
    }
}

static void GetUicOpts(cmTarget const* target, const std::string& config,
                       std::string &optString)
{
  cmGeneratorTarget *gtgt = target->GetMakefile()
                                  ->GetGlobalGenerator()
                                  ->GetGeneratorTarget(target);
  std::vector<std::string> opts;
  gtgt->GetAutoUicOptions(opts, config);
  optString = cmJoin(opts, ";");
}

void cmQtAutoGeneratorInitializer::SetupAutoUicTarget(cmTarget const* target,
                          std::vector<std::string> const& skipUic,
                          std::map<std::string, std::string> &configUicOptions)
{
  cmMakefile *makefile = target->GetMakefile();

  std::set<std::string> skipped;
  skipped.insert(skipUic.begin(), skipUic.end());

  makefile->AddDefinition("_skip_uic",
          cmOutputConverter::EscapeForCMake(cmJoin(skipUic, ";")).c_str());

  std::vector<cmSourceFile*> uiFilesWithOptions
                                        = makefile->GetQtUiFilesWithOptions();

  const char *qtVersion = makefile->GetDefinition("_target_qt_version");

  std::string _uic_opts;
  std::vector<std::string> configs;
  const std::string& config = makefile->GetConfigurations(configs);
  GetUicOpts(target, config, _uic_opts);

  if (!_uic_opts.empty())
    {
    _uic_opts = cmOutputConverter::EscapeForCMake(_uic_opts);
    makefile->AddDefinition("_uic_target_options", _uic_opts.c_str());
    }
  for (std::vector<std::string>::const_iterator li = configs.begin();
       li != configs.end(); ++li)
    {
    std::string config_uic_opts;
    GetUicOpts(target, *li, config_uic_opts);
    if (config_uic_opts != _uic_opts)
      {
      configUicOptions[*li] =
                    cmOutputConverter::EscapeForCMake(config_uic_opts);
      if(_uic_opts.empty())
        {
        _uic_opts = config_uic_opts;
        }
      }
    }

  std::string uiFileFiles;
  std::string uiFileOptions;
  const char* sep = "";

  for(std::vector<cmSourceFile*>::const_iterator fileIt =
      uiFilesWithOptions.begin();
      fileIt != uiFilesWithOptions.end();
      ++fileIt)
    {
    cmSourceFile* sf = *fileIt;
    std::string absFile = cmsys::SystemTools::GetRealPath(
                                                    sf->GetFullPath());

    if (!skipped.insert(absFile).second)
      {
      continue;
      }
    uiFileFiles += sep;
    uiFileFiles += absFile;
    uiFileOptions += sep;
    std::string opts = sf->GetProperty("AUTOUIC_OPTIONS");
    cmSystemTools::ReplaceString(opts, ";", "@list_sep@");
    uiFileOptions += opts;
    sep = ";";
    }

  makefile->AddDefinition("_qt_uic_options_files",
              cmOutputConverter::EscapeForCMake(uiFileFiles).c_str());
  makefile->AddDefinition("_qt_uic_options_options",
            cmOutputConverter::EscapeForCMake(uiFileOptions).c_str());

  std::string targetName = target->GetName();
  if (strcmp(qtVersion, "5") == 0)
    {
    cmTarget *qt5Uic = makefile->FindTargetToUse("Qt5::uic");
    if (!qt5Uic)
      {
      // Project does not use Qt5Widgets, but has AUTOUIC ON anyway
      }
    else
      {
      makefile->AddDefinition("_qt_uic_executable",
                              qt5Uic->ImportedGetLocation(""));
      }
    }
  else if (strcmp(qtVersion, "4") == 0)
    {
    cmTarget *qt4Uic = makefile->FindTargetToUse("Qt4::uic");
    if (!qt4Uic)
      {
      cmSystemTools::Error("Qt4::uic target not found ",
                          targetName.c_str());
      return;
      }
    makefile->AddDefinition("_qt_uic_executable",
                            qt4Uic->ImportedGetLocation(""));
    }
  else
    {
    cmSystemTools::Error("The CMAKE_AUTOUIC feature supports only Qt 4 and "
                        "Qt 5 ", targetName.c_str());
    }
}

void cmQtAutoGeneratorInitializer::MergeRccOptions(
                         std::vector<std::string> &opts,
                         const std::vector<std::string> &fileOpts,
                         bool isQt5)
{
  static const char* valueOptions[] = {
    "name",
    "root",
    "compress",
    "threshold"
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

void cmQtAutoGeneratorInitializer::SetupAutoRccTarget(cmTarget const* target)
{
  std::string _rcc_files;
  const char* sepRccFiles = "";
  cmMakefile *makefile = target->GetMakefile();

  std::vector<cmSourceFile*> srcFiles;
  cmGeneratorTarget *gtgt = target->GetMakefile()
                                  ->GetGlobalGenerator()
                                  ->GetGeneratorTarget(target);
  gtgt->GetConfigCommonSourceFiles(srcFiles);

  std::string qrcInputs;
  const char* qrcInputsSep = "";

  std::string rccFileFiles;
  std::string rccFileOptions;
  const char *optionSep = "";

  const char *qtVersion = makefile->GetDefinition("_target_qt_version");

  std::vector<std::string> rccOptions;
  if (const char* opts = target->GetProperty("AUTORCC_OPTIONS"))
    {
    cmSystemTools::ExpandListArgument(opts, rccOptions);
    }
  std::string qtMajorVersion = makefile->GetSafeDefinition("QT_VERSION_MAJOR");
  if (qtMajorVersion == "")
    {
    qtMajorVersion = makefile->GetSafeDefinition("Qt5Core_VERSION_MAJOR");
    }

  for(std::vector<cmSourceFile*>::const_iterator fileIt = srcFiles.begin();
      fileIt != srcFiles.end();
      ++fileIt)
    {
    cmSourceFile* sf = *fileIt;
    std::string ext = sf->GetExtension();
    if (ext == "qrc")
      {
      std::string absFile = cmsys::SystemTools::GetRealPath(
                                                  sf->GetFullPath());
      bool skip = cmSystemTools::IsOn(sf->GetPropertyForUser("SKIP_AUTORCC"));

      if (!skip)
        {
        _rcc_files += sepRccFiles;
        _rcc_files += absFile;
        sepRccFiles = ";";

        if (const char *prop = sf->GetProperty("AUTORCC_OPTIONS"))
          {
          std::vector<std::string> optsVec;
          cmSystemTools::ExpandListArgument(prop, optsVec);
          cmQtAutoGeneratorInitializer::MergeRccOptions(rccOptions, optsVec,
                                strcmp(qtVersion, "5") == 0);
          }

        if (!rccOptions.empty())
          {
          rccFileFiles += optionSep;
          rccFileFiles += absFile;
          rccFileOptions += optionSep;
          }
        const char *listSep = "";
        for(std::vector<std::string>::const_iterator it = rccOptions.begin();
            it != rccOptions.end();
            ++it)
          {
          rccFileOptions += listSep;
          rccFileOptions += *it;
          listSep = "@list_sep@";
          }
        optionSep = ";";

        std::vector<std::string> depends;

        std::string entriesList;
        if (!cmSystemTools::IsOn(sf->GetPropertyForUser("GENERATED")))
          {
          if (qtMajorVersion == "5")
            {
            entriesList = cmQtAutoGeneratorInitializer::ListQt5RccInputs(sf,
                                                               target,
                                                               depends);
            }
          else
            {
            entriesList =
                cmQtAutoGeneratorInitializer::ListQt4RccInputs(sf, depends);
            }
          if (entriesList.empty())
            {
            return;
            }
          }
        qrcInputs += qrcInputsSep;
        qrcInputs += entriesList;
        qrcInputsSep = ";";
        }
      }
    }
  makefile->AddDefinition("_qt_rcc_inputs_" + target->GetName(),
                      cmOutputConverter::EscapeForCMake(qrcInputs).c_str());

  makefile->AddDefinition("_rcc_files",
          cmOutputConverter::EscapeForCMake(_rcc_files).c_str());

  makefile->AddDefinition("_qt_rcc_options_files",
              cmOutputConverter::EscapeForCMake(rccFileFiles).c_str());
  makefile->AddDefinition("_qt_rcc_options_options",
            cmOutputConverter::EscapeForCMake(rccFileOptions).c_str());

  makefile->AddDefinition("_qt_rcc_executable",
              cmQtAutoGeneratorInitializer::GetRccExecutable(target).c_str());
}

std::string cmQtAutoGeneratorInitializer::GetRccExecutable(
    cmTarget const* target)
{
  cmGeneratorTarget *gtgt = target->GetMakefile()
                                  ->GetGlobalGenerator()
                                  ->GetGeneratorTarget(target);
  cmMakefile *makefile = target->GetMakefile();
  const char *qtVersion = makefile->GetDefinition("_target_qt_version");
  if (!qtVersion)
    {
    qtVersion = makefile->GetDefinition("Qt5Core_VERSION_MAJOR");
    if (!qtVersion)
      {
      qtVersion = makefile->GetDefinition("QT_VERSION_MAJOR");
      }
    if (const char *targetQtVersion =
        gtgt->GetLinkInterfaceDependentStringProperty("QT_MAJOR_VERSION", ""))
      {
      qtVersion = targetQtVersion;
      }
    }

  std::string targetName = target->GetName();
  if (strcmp(qtVersion, "5") == 0)
    {
    cmTarget *qt5Rcc = makefile->FindTargetToUse("Qt5::rcc");
    if (!qt5Rcc)
      {
      cmSystemTools::Error("Qt5::rcc target not found ",
                          targetName.c_str());
      return std::string();
      }
    return qt5Rcc->ImportedGetLocation("");
    }
  else if (strcmp(qtVersion, "4") == 0)
    {
    cmTarget *qt4Rcc = makefile->FindTargetToUse("Qt4::rcc");
    if (!qt4Rcc)
      {
      cmSystemTools::Error("Qt4::rcc target not found ",
                          targetName.c_str());
      return std::string();
      }
    return qt4Rcc->ImportedGetLocation("");
    }

  cmSystemTools::Error("The CMAKE_AUTORCC feature supports only Qt 4 and "
                      "Qt 5 ", targetName.c_str());
  return std::string();
}
