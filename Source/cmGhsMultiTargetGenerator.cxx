/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGhsMultiTargetGenerator.h"

#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGhsMultiGenerator.h"
#include "cmLinkLineComputer.h"
#include "cmLocalGhsMultiGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmSourceGroup.h"
#include "cmTarget.h"

std::string const cmGhsMultiTargetGenerator::DDOption("-dynamic");

cmGhsMultiTargetGenerator::cmGhsMultiTargetGenerator(cmGeneratorTarget* target)
  : GeneratorTarget(target)
  , LocalGenerator(
      static_cast<cmLocalGhsMultiGenerator*>(target->GetLocalGenerator()))
  , Makefile(target->Target->GetMakefile())
  , TargetGroup(DetermineIfTargetGroup(target))
  , DynamicDownload(false)
  , Name(target->GetName())
{
  this->RelBuildFilePath = this->GetRelBuildFilePath(target);

  this->RelOutputFileName = this->RelBuildFilePath + target->GetName() + ".a";

  this->RelBuildFileName = this->RelBuildFilePath;
  this->RelBuildFileName += this->GetBuildFileName(target);

  std::string absPathToRoot = this->GetAbsPathToRoot(target);
  absPathToRoot = this->AddSlashIfNeededToPath(absPathToRoot);
  this->AbsBuildFilePath = absPathToRoot + this->RelBuildFilePath;
  this->AbsBuildFileName = absPathToRoot + this->RelBuildFileName;
  this->AbsOutputFileName = absPathToRoot + this->RelOutputFileName;
}

cmGhsMultiTargetGenerator::~cmGhsMultiTargetGenerator()
{
  cmDeleteAll(this->FolderBuildStreams);
}

std::string cmGhsMultiTargetGenerator::GetRelBuildFilePath(
  const cmGeneratorTarget* target)
{
  std::string output = target->GetEffectiveFolderName();
  cmSystemTools::ConvertToUnixSlashes(output);
  if (!output.empty()) {
    output += "/";
  }
  output += target->GetName() + "/";
  return output;
}

std::string cmGhsMultiTargetGenerator::GetAbsPathToRoot(
  const cmGeneratorTarget* target)
{
  return target->GetLocalGenerator()->GetBinaryDirectory();
}

std::string cmGhsMultiTargetGenerator::GetAbsBuildFilePath(
  const cmGeneratorTarget* target)
{
  std::string output;
  output = cmGhsMultiTargetGenerator::GetAbsPathToRoot(target);
  output = cmGhsMultiTargetGenerator::AddSlashIfNeededToPath(output);
  output += cmGhsMultiTargetGenerator::GetRelBuildFilePath(target);
  return output;
}

std::string cmGhsMultiTargetGenerator::GetRelBuildFileName(
  const cmGeneratorTarget* target)
{
  std::string output;
  output = cmGhsMultiTargetGenerator::GetRelBuildFilePath(target);
  output = cmGhsMultiTargetGenerator::AddSlashIfNeededToPath(output);
  output += cmGhsMultiTargetGenerator::GetBuildFileName(target);
  return output;
}

std::string cmGhsMultiTargetGenerator::GetBuildFileName(
  const cmGeneratorTarget* target)
{
  std::string output;
  output = target->GetName();
  output += cmGlobalGhsMultiGenerator::FILE_EXTENSION;
  return output;
}

std::string cmGhsMultiTargetGenerator::AddSlashIfNeededToPath(
  std::string const& input)
{
  std::string output(input);
  if (!cmHasLiteralSuffix(output, "/")) {
    output += "/";
  }
  return output;
}

void cmGhsMultiTargetGenerator::Generate()
{
  // Tell the global generator the name of the project file
  this->GeneratorTarget->Target->SetProperty("GENERATOR_FILE_NAME",
                                             this->Name.c_str());

  // Skip if empty or not included in build
  if (!this->GetSources().empty() && this->IncludeThisTarget()) {

    // Open the filestream in copy-if-different mode.
    std::string fname = this->LocalGenerator->GetCurrentBinaryDirectory();
    fname += "/";
    fname += this->Name;
    fname += cmGlobalGhsMultiGenerator::FILE_EXTENSION;
    cmGeneratedFileStream fout(fname.c_str());
    fout.SetCopyIfDifferent(true);

    cmGlobalGhsMultiGenerator::OpenBuildFileStream(&fout);

    std::string config = this->Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
    if (0 == config.length()) {
      config = "RELEASE";
    }
    const std::string language(
      this->GeneratorTarget->GetLinkerLanguage(config));
    config = cmSystemTools::UpperCase(config);
    this->DynamicDownload = this->DetermineIfDynamicDownload(config, language);
    if (this->DynamicDownload) {
      fout << "#component integrity_dynamic_download" << std::endl;
    }
    GhsMultiGpj::WriteGpjTag(this->GetGpjTag(), &fout);
    cmGlobalGhsMultiGenerator::WriteDisclaimer(&fout);

    bool const notKernel = this->IsNotKernel(config, language);
    this->WriteTypeSpecifics(fout, config, notKernel);
    this->SetCompilerFlags(config, language, notKernel);
    this->WriteCompilerFlags(fout, config, language);
    this->WriteCompilerDefinitions(fout, config, language);
    this->WriteIncludes(fout, config, language);
    if (this->GeneratorTarget->GetType() == cmStateEnums::EXECUTABLE) {
      this->WriteTargetLinkLibraries(fout, config, language);
    }
    this->WriteCustomCommands(fout);
    this->WriteSources(fout);

    fout.Close();
  }
}

bool cmGhsMultiTargetGenerator::IncludeThisTarget()
{
  bool output = true;
  char const* excludeFromAll =
    this->GeneratorTarget->GetProperty("EXCLUDE_FROM_ALL");
  if (NULL != excludeFromAll && '1' == excludeFromAll[0] &&
      '\0' == excludeFromAll[1]) {
    output = false;
  }
  return output;
}

std::vector<cmSourceFile*> cmGhsMultiTargetGenerator::GetSources() const
{
  std::vector<cmSourceFile*> output;
  std::string config = this->Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
  this->GeneratorTarget->GetSourceFiles(output, config);
  return output;
}

GhsMultiGpj::Types cmGhsMultiTargetGenerator::GetGpjTag() const
{
  return cmGhsMultiTargetGenerator::GetGpjTag(this->GeneratorTarget);
}

GhsMultiGpj::Types cmGhsMultiTargetGenerator::GetGpjTag(
  const cmGeneratorTarget* target)
{
  GhsMultiGpj::Types output;
  if (cmGhsMultiTargetGenerator::DetermineIfTargetGroup(target)) {
    output = GhsMultiGpj::INTERGRITY_APPLICATION;
  } else if (target->GetType() == cmStateEnums::STATIC_LIBRARY) {
    output = GhsMultiGpj::LIBRARY;
  } else {
    output = GhsMultiGpj::PROGRAM;
  }
  return output;
}

cmGlobalGhsMultiGenerator* cmGhsMultiTargetGenerator::GetGlobalGenerator()
  const
{
  return static_cast<cmGlobalGhsMultiGenerator*>(
    this->LocalGenerator->GetGlobalGenerator());
}

void cmGhsMultiTargetGenerator::WriteTypeSpecifics(std::ostream& fout,
                                                   const std::string& config,
                                                   bool const notKernel)
{
  std::string outputDir(this->GetOutputDirectory(config));
  std::string outputFilename(this->GetOutputFilename(config));

  if (this->GeneratorTarget->GetType() == cmStateEnums::STATIC_LIBRARY) {
    std::string const& static_library_suffix =
      this->Makefile->GetSafeDefinition("CMAKE_STATIC_LIBRARY_SUFFIX");
    fout << "    -o \"" << outputDir << outputFilename << static_library_suffix
         << "\"" << std::endl;
  } else if (this->GeneratorTarget->GetType() == cmStateEnums::EXECUTABLE) {
    if (notKernel && !this->IsTargetGroup()) {
      fout << "    -relprog" << std::endl;
    }
    if (this->IsTargetGroup()) {
      fout << "    -o \"" << outputDir << outputFilename << ".elf\""
           << std::endl;
      fout << "    :extraOutputFile=\"" << outputDir << outputFilename
           << ".elf.ael\"" << std::endl;
    } else {
      std::string const executable_suffix =
        this->Makefile->GetSafeDefinition("CMAKE_EXECUTABLE_SUFFIX");
      fout << "    -o \"" << outputDir << outputFilename << executable_suffix
           << "\"" << std::endl;
    }
  }
}

void cmGhsMultiTargetGenerator::SetCompilerFlags(std::string const& config,
                                                 const std::string& language,
                                                 bool const notKernel)
{
  std::map<std::string, std::string>::iterator i =
    this->FlagsByLanguage.find(language);
  if (i == this->FlagsByLanguage.end()) {
    std::string flags;
    const char* lang = language.c_str();

    if (notKernel) {
      this->LocalGenerator->AddLanguageFlags(flags, this->GeneratorTarget,
                                             lang, config);
    } else {
      this->LocalGenerator->AddLanguageFlags(flags, this->GeneratorTarget,
                                             lang + std::string("_GHS_KERNEL"),
                                             config);
    }
    this->LocalGenerator->AddCMP0018Flags(flags, this->GeneratorTarget, lang,
                                          config);
    this->LocalGenerator->AddVisibilityPresetFlags(
      flags, this->GeneratorTarget, lang);

    // Append old-style preprocessor definition flags.
    if (this->Makefile->GetDefineFlags() != " ") {
      this->LocalGenerator->AppendFlags(flags,
                                        this->Makefile->GetDefineFlags());
    }

    // Add target-specific flags.
    this->LocalGenerator->AddCompileOptions(flags, this->GeneratorTarget, lang,
                                            config);

    std::map<std::string, std::string>::value_type entry(language, flags);
    i = this->FlagsByLanguage.insert(entry).first;
  }
}

std::string cmGhsMultiTargetGenerator::GetDefines(const std::string& language,
                                                  std::string const& config)
{
  std::map<std::string, std::string>::iterator i =
    this->DefinesByLanguage.find(language);
  if (i == this->DefinesByLanguage.end()) {
    std::set<std::string> defines;
    const char* lang = language.c_str();
    // Add preprocessor definitions for this target and configuration.
    this->LocalGenerator->GetTargetDefines(this->GeneratorTarget, config,
                                           language, defines);

    std::string definesString;
    this->LocalGenerator->JoinDefines(defines, definesString, lang);

    std::map<std::string, std::string>::value_type entry(language,
                                                         definesString);
    i = this->DefinesByLanguage.insert(entry).first;
  }
  return i->second;
}

void cmGhsMultiTargetGenerator::WriteCompilerFlags(std::ostream& fout,
                                                   std::string const&,
                                                   const std::string& language)
{
  std::map<std::string, std::string>::iterator flagsByLangI =
    this->FlagsByLanguage.find(language);
  if (flagsByLangI != this->FlagsByLanguage.end()) {
    if (!flagsByLangI->second.empty()) {
      fout << "    " << flagsByLangI->second << std::endl;
    }
  }
}

void cmGhsMultiTargetGenerator::WriteCompilerDefinitions(
  std::ostream& fout, const std::string& config, const std::string& language)
{
  std::vector<std::string> compileDefinitions;
  this->GeneratorTarget->GetCompileDefinitions(compileDefinitions, config,
                                               language);
  for (std::vector<std::string>::const_iterator cdI =
         compileDefinitions.begin();
       cdI != compileDefinitions.end(); ++cdI) {
    fout << "    -D" << (*cdI) << std::endl;
  }
}

void cmGhsMultiTargetGenerator::WriteIncludes(std::ostream& fout,
                                              const std::string& config,
                                              const std::string& language)
{
  std::vector<std::string> includes;
  this->LocalGenerator->GetIncludeDirectories(includes, this->GeneratorTarget,
                                              language, config);

  for (std::vector<std::string>::const_iterator includes_i = includes.begin();
       includes_i != includes.end(); ++includes_i) {
    fout << "    -I\"" << *includes_i << "\"" << std::endl;
  }
}

void cmGhsMultiTargetGenerator::WriteTargetLinkLibraries(
  std::ostream& fout, std::string const& config, std::string const& language)
{
  // library directories
  cmTargetDependSet tds =
    this->GetGlobalGenerator()->GetTargetDirectDepends(this->GeneratorTarget);
  for (cmTargetDependSet::iterator tdsI = tds.begin(); tdsI != tds.end();
       ++tdsI) {
    const cmGeneratorTarget* tg = *tdsI;
    fout << "    -L\"" << GetAbsBuildFilePath(tg) << "\"" << std::endl;
  }
  // library targets
  cmTarget::LinkLibraryVectorType llv =
    this->GeneratorTarget->Target->GetOriginalLinkLibraries();
  for (cmTarget::LinkLibraryVectorType::const_iterator llvI = llv.begin();
       llvI != llv.end(); ++llvI) {
    std::string libName = llvI->first;
    // if it is a user defined target get the full path to the lib
    cmTarget* tg(GetGlobalGenerator()->FindTarget(libName));
    if (NULL != tg) {
      libName = tg->GetName() + ".a";
    }
    fout << "    -l\"" << libName << "\"" << std::endl;
  }

  if (!this->TargetGroup) {
    std::string linkLibraries;
    std::string flags;
    std::string linkFlags;
    std::string frameworkPath;
    std::string linkPath;
    std::string createRule =
      this->GeneratorTarget->GetCreateRuleVariable(language, config);
    bool useWatcomQuote =
      this->Makefile->IsOn(createRule + "_USE_WATCOM_QUOTE");
    std::unique_ptr<cmLinkLineComputer> linkLineComputer(
      this->GetGlobalGenerator()->CreateLinkLineComputer(
        this->LocalGenerator,
        this->LocalGenerator->GetStateSnapshot().GetDirectory()));
    linkLineComputer->SetUseWatcomQuote(useWatcomQuote);

    this->LocalGenerator->GetTargetFlags(
      linkLineComputer.get(), config, linkLibraries, flags, linkFlags,
      frameworkPath, linkPath, this->GeneratorTarget);
    linkFlags = cmSystemTools::TrimWhitespace(linkFlags);

    if (!linkPath.empty()) {
      linkPath = " " + linkPath.substr(0U, linkPath.size() - 1U);
      fout << linkPath;
    }

    if (!linkFlags.empty()) {
      fout << "    " << linkFlags << std::endl;
    }
  }
}

void cmGhsMultiTargetGenerator::WriteCustomCommands(std::ostream& fout)
{
  WriteCustomCommandsHelper(fout, this->GeneratorTarget->GetPreBuildCommands(),
                            cmTarget::PRE_BUILD);
  WriteCustomCommandsHelper(
    fout, this->GeneratorTarget->GetPostBuildCommands(), cmTarget::POST_BUILD);
}

void cmGhsMultiTargetGenerator::WriteCustomCommandsHelper(
  std::ostream& fout, std::vector<cmCustomCommand> const& commandsSet,
  cmTarget::CustomCommandType const commandType)
{
  for (std::vector<cmCustomCommand>::const_iterator commandsSetI =
         commandsSet.begin();
       commandsSetI != commandsSet.end(); ++commandsSetI) {
    cmCustomCommandLines const& commands = commandsSetI->GetCommandLines();
    for (cmCustomCommandLines::const_iterator commandI = commands.begin();
         commandI != commands.end(); ++commandI) {
      switch (commandType) {
        case cmTarget::PRE_BUILD:
          fout << "    :preexecShellSafe=";
          break;
        case cmTarget::POST_BUILD:
          fout << "    :postexecShellSafe=";
          break;
        default:
          assert("Only pre and post are supported");
      }
      cmCustomCommandLine const& command = *commandI;
      for (cmCustomCommandLine::const_iterator commandLineI = command.begin();
           commandLineI != command.end(); ++commandLineI) {
        std::string subCommandE =
          this->LocalGenerator->EscapeForShell(*commandLineI, true);
        if (!command.empty()) {
          fout << (command.begin() == commandLineI ? "'" : " ");
          // Need to double escape backslashes
          cmSystemTools::ReplaceString(subCommandE, "\\", "\\\\");
        }
        fout << subCommandE;
      }
      if (!command.empty()) {
        fout << "'" << std::endl;
      }
    }
  }
}

std::map<const cmSourceFile*, std::string>
cmGhsMultiTargetGenerator::GetObjectNames(
  std::vector<cmSourceFile*>* const objectSources,
  cmLocalGhsMultiGenerator* const localGhsMultiGenerator,
  cmGeneratorTarget* const generatorTarget)
{
  std::map<std::string, std::vector<cmSourceFile*>> filenameToSource;
  std::map<cmSourceFile*, std::string> sourceToFilename;
  for (std::vector<cmSourceFile*>::const_iterator sf = objectSources->begin();
       sf != objectSources->end(); ++sf) {
    const std::string filename =
      cmSystemTools::GetFilenameName((*sf)->GetFullPath());
    const std::string lower_filename = cmSystemTools::LowerCase(filename);
    filenameToSource[lower_filename].push_back(*sf);
    sourceToFilename[*sf] = lower_filename;
  }

  std::vector<cmSourceFile*> duplicateSources;
  for (std::map<std::string, std::vector<cmSourceFile*>>::const_iterator
         msvSourceI = filenameToSource.begin();
       msvSourceI != filenameToSource.end(); ++msvSourceI) {
    if (msvSourceI->second.size() > 1) {
      duplicateSources.insert(duplicateSources.end(),
                              msvSourceI->second.begin(),
                              msvSourceI->second.end());
    }
  }

  std::map<const cmSourceFile*, std::string> objectNamesCorrected;

  for (std::vector<cmSourceFile*>::const_iterator sf =
         duplicateSources.begin();
       sf != duplicateSources.end(); ++sf) {
    std::string const longestObjectDirectory(
      cmGhsMultiTargetGenerator::ComputeLongestObjectDirectory(
        localGhsMultiGenerator, generatorTarget, *sf));
    std::string objFilenameName =
      localGhsMultiGenerator->GetObjectFileNameWithoutTarget(
        **sf, longestObjectDirectory);
    cmsys::SystemTools::ReplaceString(objFilenameName, "/", "_");
    objectNamesCorrected[*sf] = objFilenameName;
  }

  return objectNamesCorrected;
}

void cmGhsMultiTargetGenerator::WriteSources(std::ostream& fout_proj)
{
  /* vector of all sources for this target */
  std::vector<cmSourceFile*> sources = this->GetSources();

  /* vector of all groups defined for this target
   * -- but the vector is not expanded with sub groups or in any useful order
   */
  std::vector<cmSourceGroup> sourceGroups = this->Makefile->GetSourceGroups();

  /* for each source file assign it to its group */
  std::map<std::string, std::vector<cmSourceFile*>> groupFiles;
  std::set<std::string> groupNames;
  for (auto& sf : sources) {
    cmSourceGroup* sourceGroup =
      this->Makefile->FindSourceGroup(sf->GetFullPath(), sourceGroups);
    std::string gn = sourceGroup->GetFullName();
    groupFiles[gn].push_back(sf);
    groupNames.insert(gn);
  }

  /* list of known groups and the order they are displayed in a project file */
  const std::vector<std::string> standardGroups = {
    "Header Files", "Source Files",     "CMake Rules",
    "Object Files", "Object Libraries", "Resources"
  };

  /* list of groups in the order they are displayed in a project file*/
  std::vector<std::string> groupFilesList(groupFiles.size());

  /* put the groups in the order they should be listed
   * - standard groups first, and then everything else
   *   in the order used by std::map.
   */
  int i = 0;
  for (const std::string& gn : standardGroups) {
    auto n = groupNames.find(gn);
    if (n != groupNames.end()) {
      groupFilesList[i] = *n;
      i += 1;
      groupNames.erase(gn);
    }
  }

  { /* catch-all group - is last item */
    std::string gn = "";
    auto n = groupNames.find(gn);
    if (n != groupNames.end()) {
      groupFilesList.back() = *n;
      groupNames.erase(gn);
    }
  }

  for (auto& n : groupNames) {
    groupFilesList[i] = n;
    i += 1;
  }

  /* sort the files within each group */
  for (auto& n : groupFilesList) {
    std::sort(groupFiles[n].begin(), groupFiles[n].end(),
              [](cmSourceFile* l, cmSourceFile* r) {
                return l->GetFullPath() < r->GetFullPath();
              });
  }

  /* get all the object names for these sources */
  std::map<const cmSourceFile*, std::string> objectNames =
    cmGhsMultiTargetGenerator::GetObjectNames(&sources, this->LocalGenerator,
                                              this->GeneratorTarget);

  /* list of open project files */
  std::vector<cmGeneratedFileStream*> gfiles;

  /* write files into the proper project file
   * -- groups go into main project file
   *    unless FOLDER property or variable is set.
   */
  for (auto& sg : groupFilesList) {
    std::ostream* fout;
    bool useProjectFile =
      cmSystemTools::IsOn(
        this->GeneratorTarget->GetProperty("GHS_NO_SOURCE_GROUP_FILE")) ||
      cmSystemTools::IsOn(
        this->Makefile->GetDefinition("GHS_NO_SOURCE_GROUP_FILE"));
    if (useProjectFile || sg.empty()) {
      fout = &fout_proj;
    } else {
      // Open the filestream in copy-if-different mode.
      std::string gname = sg;
      cmsys::SystemTools::ReplaceString(gname, "\\", "_");
      std::string lpath =
        this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
      lpath += "/";
      lpath += gname;
      lpath += cmGlobalGhsMultiGenerator::FILE_EXTENSION;
      std::string fpath = this->LocalGenerator->GetCurrentBinaryDirectory();
      fpath += "/";
      fpath += lpath;
      cmGeneratedFileStream* f = new cmGeneratedFileStream(fpath.c_str());
      f->SetCopyIfDifferent(true);
      gfiles.push_back(f);
      fout = f;
      cmGlobalGhsMultiGenerator::OpenBuildFileStream(f);
      *fout << "[Subproject]" << std::endl;
      cmGlobalGhsMultiGenerator::WriteDisclaimer(f);
      fout_proj << lpath << " ";
      fout_proj << "[Subproject]" << std::endl;
    }

    if (useProjectFile) {
      if (sg.empty()) {
        *fout << "{comment} Others" << std::endl;
      } else {
        *fout << "{comment} " << sg << std::endl;
      }
    }

    /* output rule for each source file */
    for (const cmSourceFile* si : groupFiles[sg]) {
      std::string fullSourcePath(si->GetFullPath());

      if (si->GetExtension() == "int" || si->GetExtension() == "bsp") {
        *fout << fullSourcePath << std::endl;
      } else {
        // WORKAROUND: GHS MULTI needs the path to use backslashes without
        // quotes
        //  to open files in search as of version 6.1.6
        cmsys::SystemTools::ReplaceString(fullSourcePath, "/", "\\");
        *fout << fullSourcePath << std::endl;
      }

      if ("ld" != si->GetExtension() && "int" != si->GetExtension() &&
          "bsp" != si->GetExtension()) {
        this->WriteObjectLangOverride(fout, si);
        if (objectNames.end() != objectNames.find(si)) {
          *fout << "    -o \"" << objectNames.find(si)->second << "\""
                << std::endl;
        }

        this->WriteObjectDir(*fout, this->AbsBuildFilePath);
      }
    }
  }

  for (cmGeneratedFileStream* f : gfiles) {
    f->Close();
  }
}

void cmGhsMultiTargetGenerator::WriteObjectLangOverride(
  std::ostream* fout, const cmSourceFile* sourceFile)
{
  const char* rawLangProp = sourceFile->GetProperty("LANGUAGE");
  if (NULL != rawLangProp) {
    std::string sourceLangProp(rawLangProp);
    std::string extension(sourceFile->GetExtension());
    if ("CXX" == sourceLangProp && ("c" == extension || "C" == extension)) {
      *fout << "    -dotciscxx" << std::endl;
    }
  }
}

void cmGhsMultiTargetGenerator::WriteObjectDir(std::ostream& fout,
                                               std::string const& dir)
{
  std::string workingDir(dir);
  cmSystemTools::ConvertToUnixSlashes(workingDir);
  if (!workingDir.empty()) {
    workingDir += "/";
  }
  workingDir += "Objs";
  fout << "    -object_dir=\"" << workingDir << "\"" << std::endl;
}

std::string cmGhsMultiTargetGenerator::GetOutputDirectory(
  const std::string& config) const
{
  std::string outputDir(AbsBuildFilePath);

  const char* runtimeOutputProp =
    this->GeneratorTarget->GetProperty("RUNTIME_OUTPUT_DIRECTORY");
  if (NULL != runtimeOutputProp) {
    outputDir = runtimeOutputProp;
  }

  std::string configCapped(cmSystemTools::UpperCase(config));
  const char* runtimeOutputSProp = this->GeneratorTarget->GetProperty(
    "RUNTIME_OUTPUT_DIRECTORY_" + configCapped);
  if (NULL != runtimeOutputSProp) {
    outputDir = runtimeOutputSProp;
  }
  cmSystemTools::ConvertToUnixSlashes(outputDir);

  if (!outputDir.empty()) {
    outputDir += "/";
  }

  return outputDir;
}

std::string cmGhsMultiTargetGenerator::GetOutputFilename(
  const std::string& config) const
{
  std::string outputFilename(this->GeneratorTarget->GetName());

  const char* outputNameProp =
    this->GeneratorTarget->GetProperty("OUTPUT_NAME");
  if (NULL != outputNameProp) {
    outputFilename = outputNameProp;
  }

  std::string configCapped(cmSystemTools::UpperCase(config));
  const char* outputNameSProp =
    this->GeneratorTarget->GetProperty(configCapped + "_OUTPUT_NAME");
  if (NULL != outputNameSProp) {
    outputFilename = outputNameSProp;
  }

  return outputFilename;
}

std::string cmGhsMultiTargetGenerator::ComputeLongestObjectDirectory(
  cmLocalGhsMultiGenerator const* localGhsMultiGenerator,
  cmGeneratorTarget* const generatorTarget, cmSourceFile* const sourceFile)
{
  std::string dir_max;
  dir_max +=
    localGhsMultiGenerator->GetMakefile()->GetCurrentBinaryDirectory();
  dir_max += "/";
  dir_max += generatorTarget->Target->GetName();
  dir_max += "/";
  std::vector<cmSourceGroup> sourceGroups(
    localGhsMultiGenerator->GetMakefile()->GetSourceGroups());
  std::string const& sourceFullPath = sourceFile->GetFullPath();
  cmSourceGroup* sourceGroup =
    localGhsMultiGenerator->GetMakefile()->FindSourceGroup(sourceFullPath,
                                                           sourceGroups);
  std::string const& sgPath = sourceGroup->GetFullName();
  dir_max += sgPath;
  dir_max += "/Objs/libs/";
  dir_max += generatorTarget->Target->GetName();
  dir_max += "/";
  return dir_max;
}

bool cmGhsMultiTargetGenerator::IsNotKernel(std::string const& config,
                                            const std::string& language)
{
  bool output;
  std::vector<std::string> options;
  this->GeneratorTarget->GetCompileOptions(options, config, language);
  output =
    options.end() == std::find(options.begin(), options.end(), "-kernel");
  return output;
}

bool cmGhsMultiTargetGenerator::DetermineIfTargetGroup(
  const cmGeneratorTarget* target)
{
  bool output = false;
  std::vector<cmSourceFile*> sources;
  std::string config =
    target->Target->GetMakefile()->GetSafeDefinition("CMAKE_BUILD_TYPE");
  target->GetSourceFiles(sources, config);
  for (std::vector<cmSourceFile*>::const_iterator sources_i = sources.begin();
       sources.end() != sources_i; ++sources_i) {
    if ("int" == (*sources_i)->GetExtension()) {
      output = true;
    }
  }
  return output;
}

bool cmGhsMultiTargetGenerator::DetermineIfDynamicDownload(
  std::string const& config, const std::string& language)
{
  std::vector<std::string> options;
  bool output = false;
  this->GeneratorTarget->GetCompileOptions(options, config, language);
  for (std::vector<std::string>::const_iterator options_i = options.begin();
       options_i != options.end(); ++options_i) {
    std::string option = *options_i;
    if (this->DDOption == option) {
      output = true;
    }
  }
  return output;
}
