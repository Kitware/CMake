/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGhsMultiTargetGenerator.h"

#include "cmComputeLinkInformation.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGhsMultiGenerator.h"
#include "cmLinkLineComputer.h"
#include "cmLocalGhsMultiGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmSourceGroup.h"
#include "cmTarget.h"

cmGhsMultiTargetGenerator::cmGhsMultiTargetGenerator(cmGeneratorTarget* target)
  : GeneratorTarget(target)
  , LocalGenerator(
      static_cast<cmLocalGhsMultiGenerator*>(target->GetLocalGenerator()))
  , Makefile(target->Target->GetMakefile())
  , Name(target->GetName())
{
  // Store the configuration name that is being used
  if (const char* config = this->Makefile->GetDefinition("CMAKE_BUILD_TYPE")) {
    // Use the build type given by the user.
    this->ConfigName = config;
  } else {
    // No configuration type given.
    this->ConfigName.clear();
  }
}

cmGhsMultiTargetGenerator::~cmGhsMultiTargetGenerator()
{
}

void cmGhsMultiTargetGenerator::Generate()
{
  // Determine type of target for this project
  switch (this->GeneratorTarget->GetType()) {
    case cmStateEnums::EXECUTABLE: {
      // Get the name of the executable to generate.
      std::string targetName;
      std::string targetNameImport;
      std::string targetNamePDB;
      this->GeneratorTarget->GetExecutableNames(
        targetName, this->TargetNameReal, targetNameImport, targetNamePDB,
        this->ConfigName);
      if (cmGhsMultiTargetGenerator::DetermineIfIntegrityApp()) {
        this->TagType = GhsMultiGpj::INTERGRITY_APPLICATION;
      } else {
        this->TagType = GhsMultiGpj::PROGRAM;
      }
      break;
    }
    case cmStateEnums::STATIC_LIBRARY: {
      std::string targetName;
      std::string targetNameSO;
      std::string targetNameImport;
      std::string targetNamePDB;
      this->GeneratorTarget->GetLibraryNames(
        targetName, targetNameSO, this->TargetNameReal, targetNameImport,
        targetNamePDB, this->ConfigName);
      this->TagType = GhsMultiGpj::LIBRARY;
      break;
    }
    case cmStateEnums::SHARED_LIBRARY: {
      std::string msg = "add_library(<name> SHARED ...) not supported: ";
      msg += this->Name;
      cmSystemTools::Message(msg);
      return;
    }
    case cmStateEnums::OBJECT_LIBRARY: {
      std::string targetName;
      std::string targetNameSO;
      std::string targetNameImport;
      std::string targetNamePDB;
      this->GeneratorTarget->GetLibraryNames(
        targetName, targetNameSO, this->TargetNameReal, targetNameImport,
        targetNamePDB, this->ConfigName);
      this->TagType = GhsMultiGpj::SUBPROJECT;
      break;
    }
    case cmStateEnums::MODULE_LIBRARY: {
      std::string msg = "add_library(<name> MODULE ...) not supported: ";
      msg += this->Name;
      cmSystemTools::Message(msg);
      return;
    }
    case cmStateEnums::UTILITY: {
      std::string msg = "add_custom_target(<name> ...) not supported: ";
      msg += this->Name;
      cmSystemTools::Message(msg);
      return;
    }
    default:
      return;
  }

  // Tell the global generator the name of the project file
  this->GeneratorTarget->Target->SetProperty("GENERATOR_FILE_NAME",
                                             this->Name.c_str());
  this->GeneratorTarget->Target->SetProperty(
    "GENERATOR_FILE_NAME_EXT", GhsMultiGpj::GetGpjTag(this->TagType));

  this->GenerateTarget();
}

void cmGhsMultiTargetGenerator::GenerateTarget()
{
  // Open the filestream in copy-if-different mode.
  std::string fname = this->LocalGenerator->GetCurrentBinaryDirectory();
  fname += "/";
  fname += this->Name;
  fname += cmGlobalGhsMultiGenerator::FILE_EXTENSION;
  cmGeneratedFileStream fout(fname.c_str());
  fout.SetCopyIfDifferent(true);

  this->GetGlobalGenerator()->WriteFileHeader(fout);
  GhsMultiGpj::WriteGpjTag(this->TagType, fout);

  const std::string language(
    this->GeneratorTarget->GetLinkerLanguage(this->ConfigName));

  this->WriteTargetSpecifics(fout, this->ConfigName);
  this->SetCompilerFlags(this->ConfigName, language);
  this->WriteCompilerFlags(fout, this->ConfigName, language);
  this->WriteCompilerDefinitions(fout, this->ConfigName, language);
  this->WriteIncludes(fout, this->ConfigName, language);
  this->WriteTargetLinkLine(fout, this->ConfigName);
  this->WriteCustomCommands(fout);
  this->WriteSources(fout);
  this->WriteReferences(fout);
  fout.Close();
}

cmGlobalGhsMultiGenerator* cmGhsMultiTargetGenerator::GetGlobalGenerator()
  const
{
  return static_cast<cmGlobalGhsMultiGenerator*>(
    this->LocalGenerator->GetGlobalGenerator());
}

void cmGhsMultiTargetGenerator::WriteTargetSpecifics(std::ostream& fout,
                                                     const std::string& config)
{
  std::string outpath;
  std::string rootpath = this->LocalGenerator->GetCurrentBinaryDirectory();

  if (this->TagType != GhsMultiGpj::SUBPROJECT) {
    // set target binary file destination
    outpath = this->GeneratorTarget->GetDirectory(config);
    outpath =
      this->LocalGenerator->MaybeConvertToRelativePath(rootpath, outpath);
    fout << "    :binDirRelative=\"" << outpath << "\"" << std::endl;
    fout << "    -o \"" << this->TargetNameReal << "\"" << std::endl;
  }

  // set target object file destination
  outpath = this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  fout << "    :outputDirRelative=\"" << outpath << "\"" << std::endl;
}

void cmGhsMultiTargetGenerator::SetCompilerFlags(std::string const& config,
                                                 const std::string& language)
{
  std::map<std::string, std::string>::iterator i =
    this->FlagsByLanguage.find(language);
  if (i == this->FlagsByLanguage.end()) {
    std::string flags;
    const char* lang = language.c_str();

    this->LocalGenerator->AddLanguageFlags(flags, this->GeneratorTarget, lang,
                                           config);

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
      std::vector<std::string> ghsCompFlags =
        cmSystemTools::ParseArguments(flagsByLangI->second.c_str());
      for (auto& f : ghsCompFlags) {
        fout << "    " << f << std::endl;
      }
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

void cmGhsMultiTargetGenerator::WriteTargetLinkLine(std::ostream& fout,
                                                    std::string const& config)
{
  if (this->TagType == GhsMultiGpj::INTERGRITY_APPLICATION) {
    return;
  }

  std::string linkLibraries;
  std::string flags;
  std::string linkFlags;
  std::string frameworkPath;
  std::string linkPath;

  std::unique_ptr<cmLinkLineComputer> linkLineComputer(
    this->GetGlobalGenerator()->CreateLinkLineComputer(
      this->LocalGenerator,
      this->LocalGenerator->GetStateSnapshot().GetDirectory()));

  this->LocalGenerator->GetTargetFlags(
    linkLineComputer.get(), config, linkLibraries, flags, linkFlags,
    frameworkPath, linkPath, this->GeneratorTarget);

  // write out link options
  std::vector<std::string> lopts =
    cmSystemTools::ParseArguments(linkFlags.c_str());
  for (auto& l : lopts) {
    fout << "    " << l << std::endl;
  }

  // write out link search paths
  // must be quoted for paths that contain spaces
  std::vector<std::string> lpath =
    cmSystemTools::ParseArguments(linkPath.c_str());
  for (auto& l : lpath) {
    fout << "    -L\"" << l << "\"" << std::endl;
  }

  // write out link libs
  // must be quoted for filepaths that contains spaces
  std::string cbd = this->LocalGenerator->GetCurrentBinaryDirectory();

  std::vector<std::string> llibs =
    cmSystemTools::ParseArguments(linkLibraries.c_str());
  for (auto& l : llibs) {
    if (l.compare(0, 2, "-l") == 0) {
      fout << "    \"" << l << "\"" << std::endl;
    } else {
      std::string rl = cmSystemTools::CollapseCombinedPath(cbd, l);
      fout << "    -l\"" << rl << "\"" << std::endl;
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

void cmGhsMultiTargetGenerator::WriteSourceProperty(std::ostream& fout,
                                                    const cmSourceFile* sf,
                                                    std::string propName,
                                                    std::string propFlag)
{
  const char* prop = sf->GetProperty(propName);
  if (prop) {
    std::vector<std::string> list;
    cmSystemTools::ExpandListArgument(prop, list);
    for (auto& p : list) {
      fout << "    " << propFlag << p << std::endl;
    }
  }
}

void cmGhsMultiTargetGenerator::WriteSources(std::ostream& fout_proj)
{
  /* vector of all sources for this target */
  std::vector<cmSourceFile*> sources;
  this->GeneratorTarget->GetSourceFiles(sources, this->ConfigName);

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
        this->Makefile->GetDefinition("CMAKE_GHS_NO_SOURCE_GROUP_FILE"));
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
      this->GetGlobalGenerator()->WriteFileHeader(*f);
      GhsMultiGpj::WriteGpjTag(GhsMultiGpj::SUBPROJECT, *f);
      fout_proj << lpath << " ";
      GhsMultiGpj::WriteGpjTag(GhsMultiGpj::SUBPROJECT, fout_proj);
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

      // Convert filename to native system
      // WORKAROUND: GHS MULTI 6.1.4 and 6.1.6 are known to need backslash on
      // windows when opening some files from the search window.
      std::string fname(si->GetFullPath());
      cmSystemTools::ConvertToOutputSlashes(fname);
      *fout << fname << std::endl;

      if ("ld" != si->GetExtension() && "int" != si->GetExtension() &&
          "bsp" != si->GetExtension()) {
        this->WriteObjectLangOverride(*fout, si);
      }

      this->WriteSourceProperty(*fout, si, "INCLUDE_DIRECTORIES", "-I");
      this->WriteSourceProperty(*fout, si, "COMPILE_DEFINITIONS", "-D");
      this->WriteSourceProperty(*fout, si, "COMPILE_OPTIONS", "");

      /* to avoid clutter in the gui only print out the objectName if it has
       * been renamed */
      std::string objectName = this->GeneratorTarget->GetObjectName(si);
      if (!objectName.empty() &&
          this->GeneratorTarget->HasExplicitObjectName(si)) {
        *fout << "    -o " << objectName << std::endl;
      }
    }
  }

  for (cmGeneratedFileStream* f : gfiles) {
    f->Close();
  }
}

void cmGhsMultiTargetGenerator::WriteObjectLangOverride(
  std::ostream& fout, const cmSourceFile* sourceFile)
{
  const char* rawLangProp = sourceFile->GetProperty("LANGUAGE");
  if (NULL != rawLangProp) {
    std::string sourceLangProp(rawLangProp);
    std::string extension(sourceFile->GetExtension());
    if ("CXX" == sourceLangProp && ("c" == extension || "C" == extension)) {
      fout << "    -dotciscxx" << std::endl;
    }
  }
}

void cmGhsMultiTargetGenerator::WriteReferences(std::ostream& fout)
{
  // This only applies to INTEGRITY Applications
  if (this->TagType != GhsMultiGpj::INTERGRITY_APPLICATION) {
    return;
  }

  // Get the targets that this one depends upon
  cmTargetDependSet unordered =
    this->GetGlobalGenerator()->GetTargetDirectDepends(this->GeneratorTarget);
  cmGlobalGhsMultiGenerator::OrderedTargetDependSet ordered(unordered,
                                                            this->Name);
  for (auto& t : ordered) {
    std::string tname = t->GetName();
    std::string tpath = t->LocalGenerator->GetCurrentBinaryDirectory();
    std::string rootpath = this->LocalGenerator->GetCurrentBinaryDirectory();
    std::string outpath =
      this->LocalGenerator->MaybeConvertToRelativePath(rootpath, tpath) + "/" +
      tname + "REF" + cmGlobalGhsMultiGenerator::FILE_EXTENSION;

    fout << outpath;
    fout << "    ";
    GhsMultiGpj::WriteGpjTag(GhsMultiGpj::REFERENCE, fout);

    // Tell the global generator that a refernce project needs to be created
    t->Target->SetProperty("GHS_REFERENCE_PROJECT", "ON");
  }
}

bool cmGhsMultiTargetGenerator::DetermineIfIntegrityApp(void)
{
  const char* p = this->GeneratorTarget->GetProperty("ghs_integrity_app");
  if (p) {
    return cmSystemTools::IsOn(
      this->GeneratorTarget->GetProperty("ghs_integrity_app"));
  } else {
    std::vector<cmSourceFile*> sources;
    this->GeneratorTarget->GetSourceFiles(sources, this->ConfigName);
    for (auto& sf : sources) {
      if ("int" == sf->GetExtension()) {
        return true;
      }
    }
    return false;
  }
}
