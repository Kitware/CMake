/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2011 Peter Collingbourne <peter@pcc.me.uk>
  Copyright 2011 Nicolas Despres <nicolas.despres@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmNinjaNormalTargetGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmSourceFile.h"
#include "cmGeneratedFileStream.h"
#include "cmMakefile.h"

#include <assert.h>

cmNinjaNormalTargetGenerator::
cmNinjaNormalTargetGenerator(cmTarget* target)
  : cmNinjaTargetGenerator(target)
  , TargetNameOut()
  , TargetNameSO()
  , TargetNameReal()
  , TargetNameImport()
  , TargetNamePDB()
{
  this->TargetLinkLanguage = target->GetLinkerLanguage(this->GetConfigName());
  if (target->GetType() == cmTarget::EXECUTABLE)
    target->GetExecutableNames(this->TargetNameOut,
                               this->TargetNameReal,
                               this->TargetNameImport,
                               this->TargetNamePDB,
                               GetLocalGenerator()->GetConfigName());
  else
    target->GetLibraryNames(this->TargetNameOut,
                            this->TargetNameSO,
                            this->TargetNameReal,
                            this->TargetNameImport,
                            this->TargetNamePDB,
                            GetLocalGenerator()->GetConfigName());

  if(target->GetType() != cmTarget::OBJECT_LIBRARY)
    {
    // on Windows the output dir is already needed at compile time
    // ensure the directory exists (OutDir test)
    std::string outpath = target->GetDirectory(this->GetConfigName());
    cmSystemTools::MakeDirectory(outpath.c_str());
    }
}

cmNinjaNormalTargetGenerator::~cmNinjaNormalTargetGenerator()
{
}

void cmNinjaNormalTargetGenerator::Generate()
{
  if (!this->TargetLinkLanguage) {
    cmSystemTools::Error("CMake can not determine linker language for target:",
                         this->GetTarget()->GetName());
    return;
  }

  // Write the rules for each language.
  this->WriteLanguagesRules();

  // Write the build statements
  this->WriteObjectBuildStatements();

  if(this->GetTarget()->GetType() == cmTarget::OBJECT_LIBRARY)
    {
    this->WriteObjectLibStatement();
    }
  else
    {
    this->WriteLinkRule();
    this->WriteLinkStatement();
    }

  this->GetBuildFileStream() << "\n";
  this->GetRulesFileStream() << "\n";
}

void cmNinjaNormalTargetGenerator::WriteLanguagesRules()
{
  cmGlobalNinjaGenerator::WriteDivider(this->GetRulesFileStream());
  this->GetRulesFileStream()
    << "# Rules for each languages for "
    << cmTarget::GetTargetTypeName(this->GetTarget()->GetType())
    << " target "
    << this->GetTargetName()
    << "\n\n";

  std::set<cmStdString> languages;
  this->GetTarget()->GetLanguages(languages);
  for(std::set<cmStdString>::const_iterator l = languages.begin();
      l != languages.end();
      ++l)
    this->WriteLanguageRules(*l);
}

const char *cmNinjaNormalTargetGenerator::GetVisibleTypeName() const
{
  switch (this->GetTarget()->GetType()) {
    case cmTarget::STATIC_LIBRARY:
      return "static library";
    case cmTarget::SHARED_LIBRARY:
      return "shared library";
    case cmTarget::MODULE_LIBRARY:
      return "shared module";
    case cmTarget::EXECUTABLE:
      return "executable";
    default:
      return 0;
  }
}

std::string
cmNinjaNormalTargetGenerator
::LanguageLinkerRule() const
{
  return std::string(this->TargetLinkLanguage)
    + "_"
    + cmTarget::GetTargetTypeName(this->GetTarget()->GetType())
    + "_LINKER";
}

void
cmNinjaNormalTargetGenerator
::WriteLinkRule()
{
  cmTarget::TargetType targetType = this->GetTarget()->GetType();
  std::string ruleName = this->LanguageLinkerRule();

  if (!this->GetGlobalGenerator()->HasRule(ruleName)) {
    cmLocalGenerator::RuleVariables vars;
    vars.RuleLauncher = "RULE_LAUNCH_LINK";
    vars.CMTarget = this->GetTarget();
    vars.Language = this->TargetLinkLanguage;
    vars.Objects = "$in";
    std::string objdir =
      this->GetLocalGenerator()->GetHomeRelativeOutputPath();
    objdir += objdir.empty() ? "" : "/";
    objdir += cmake::GetCMakeFilesDirectoryPostSlash();
    objdir += this->GetTargetName();
    objdir += ".dir";
    objdir = this->GetLocalGenerator()->Convert(objdir.c_str(),
                                                cmLocalGenerator::START_OUTPUT,
                                                cmLocalGenerator::SHELL);
    vars.ObjectDir = objdir.c_str();
    vars.Target = "$out";
    vars.TargetSOName = "$SONAME";
    vars.TargetInstallNameDir = "$INSTALLNAME_DIR";
    vars.TargetPDB = "$TARGET_PDB";

    // Setup the target version.
    std::string targetVersionMajor;
    std::string targetVersionMinor;
    {
    cmOStringStream majorStream;
    cmOStringStream minorStream;
    int major;
    int minor;
    this->GetTarget()->GetTargetVersion(major, minor);
    majorStream << major;
    minorStream << minor;
    targetVersionMajor = majorStream.str();
    targetVersionMinor = minorStream.str();
    }
    vars.TargetVersionMajor = targetVersionMajor.c_str();
    vars.TargetVersionMinor = targetVersionMinor.c_str();

    vars.LinkLibraries = "$LINK_LIBRARIES";
    vars.Flags = "$FLAGS";
    vars.LinkFlags = "$LINK_FLAGS";

    std::string langFlags;
    this->GetLocalGenerator()->AddLanguageFlags(langFlags,
                                                this->TargetLinkLanguage,
                                                this->GetConfigName());
    if (targetType != cmTarget::EXECUTABLE)
      langFlags += " $ARCH_FLAGS";
    vars.LanguageCompileFlags = langFlags.c_str();

    // Rule for linking library.
    std::vector<std::string> linkCmds = this->ComputeLinkCmd();
    for(std::vector<std::string>::iterator i = linkCmds.begin();
        i != linkCmds.end();
        ++i)
      {
      this->GetLocalGenerator()->ExpandRuleVariables(*i, vars);
      }
    linkCmds.insert(linkCmds.begin(), "$PRE_LINK");
    linkCmds.push_back("$POST_BUILD");
    std::string linkCmd =
      this->GetLocalGenerator()->BuildCommandLine(linkCmds);

    // Write the linker rule.
    std::ostringstream comment;
    comment << "Rule for linking " << this->TargetLinkLanguage << " "
            << this->GetVisibleTypeName() << ".";
    std::ostringstream description;
    description << "Linking " << this->TargetLinkLanguage << " "
                << this->GetVisibleTypeName() << " $out";
    this->GetGlobalGenerator()->AddRule(ruleName,
                                        linkCmd,
                                        description.str(),
                                        comment.str());
  }

  if (this->TargetNameOut != this->TargetNameReal) {
    std::string cmakeCommand =
      this->GetLocalGenerator()->ConvertToOutputFormat(
        this->GetMakefile()->GetRequiredDefinition("CMAKE_COMMAND"),
        cmLocalGenerator::SHELL);
    if (targetType == cmTarget::EXECUTABLE)
      this->GetGlobalGenerator()->AddRule("CMAKE_SYMLINK_EXECUTABLE",
                                          cmakeCommand +
                                          " -E cmake_symlink_executable"
                                          " $in $out && $POST_BUILD",
                                          "Creating executable symlink $out",
                                      "Rule for creating executable symlink.");
    else
      this->GetGlobalGenerator()->AddRule("CMAKE_SYMLINK_LIBRARY",
                                          cmakeCommand +
                                          " -E cmake_symlink_library"
                                          " $in $SONAME $out && $POST_BUILD",
                                          "Creating library symlink $out",
                                         "Rule for creating library symlink.");
  }
}

std::vector<std::string>
cmNinjaNormalTargetGenerator
::ComputeLinkCmd()
{
  std::vector<std::string> linkCmds;
  cmTarget::TargetType targetType = this->GetTarget()->GetType();
  switch (targetType) {
    case cmTarget::STATIC_LIBRARY: {
      // Check if you have a non archive way to create the static library.
      {
      std::string linkCmdVar = "CMAKE_";
      linkCmdVar += this->TargetLinkLanguage;
      linkCmdVar += "_CREATE_STATIC_LIBRARY";
      if (const char *linkCmd =
            this->GetMakefile()->GetDefinition(linkCmdVar.c_str()))
        {
        cmSystemTools::ExpandListArgument(linkCmd, linkCmds);
        return linkCmds;
        }
      }

      // We have archive link commands set. First, delete the existing archive.
      std::string cmakeCommand =
        this->GetLocalGenerator()->ConvertToOutputFormat(
          this->GetMakefile()->GetRequiredDefinition("CMAKE_COMMAND"),
          cmLocalGenerator::SHELL);
      linkCmds.push_back(cmakeCommand + " -E remove $out");

      // TODO: Use ARCHIVE_APPEND for archives over a certain size.
      {
      std::string linkCmdVar = "CMAKE_";
      linkCmdVar += this->TargetLinkLanguage;
      linkCmdVar += "_ARCHIVE_CREATE";
      const char *linkCmd =
        this->GetMakefile()->GetRequiredDefinition(linkCmdVar.c_str());
      cmSystemTools::ExpandListArgument(linkCmd, linkCmds);
      }
      {
      std::string linkCmdVar = "CMAKE_";
      linkCmdVar += this->TargetLinkLanguage;
      linkCmdVar += "_ARCHIVE_FINISH";
      const char *linkCmd =
        this->GetMakefile()->GetRequiredDefinition(linkCmdVar.c_str());
      cmSystemTools::ExpandListArgument(linkCmd, linkCmds);
      }
      return linkCmds;
    }
    case cmTarget::SHARED_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
    case cmTarget::EXECUTABLE: {
      std::string linkCmdVar = "CMAKE_";
      linkCmdVar += this->TargetLinkLanguage;
      switch (targetType) {
      case cmTarget::SHARED_LIBRARY:
        linkCmdVar += "_CREATE_SHARED_LIBRARY";
        break;
      case cmTarget::MODULE_LIBRARY:
        linkCmdVar += "_CREATE_SHARED_MODULE";
        break;
      case cmTarget::EXECUTABLE:
        linkCmdVar += "_LINK_EXECUTABLE";
        break;
      default:
        assert(0 && "Unexpected target type");
      }

      const char *linkCmd =
        this->GetMakefile()->GetRequiredDefinition(linkCmdVar.c_str());
      cmSystemTools::ExpandListArgument(linkCmd, linkCmds);
      return linkCmds;
    }
    default:
      assert(0 && "Unexpected target type");
  }
  return std::vector<std::string>();
}

void cmNinjaNormalTargetGenerator::WriteLinkStatement()
{
  cmTarget::TargetType targetType = this->GetTarget()->GetType();

  // Write comments.
  cmGlobalNinjaGenerator::WriteDivider(this->GetBuildFileStream());
  this->GetBuildFileStream()
    << "# Link build statements for "
    << cmTarget::GetTargetTypeName(targetType)
    << " target "
    << this->GetTargetName()
    << "\n\n";

  cmNinjaDeps emptyDeps;
  cmNinjaVars vars;

  std::string targetOutput = ConvertToNinjaPath(
    this->GetTarget()->GetFullPath(this->GetConfigName()).c_str());
  std::string targetOutputReal = ConvertToNinjaPath(
    this->GetTarget()->GetFullPath(this->GetConfigName(),
                                   /*implib=*/false,
                                   /*realpath=*/true).c_str());
  std::string targetOutputImplib = ConvertToNinjaPath(
    this->GetTarget()->GetFullPath(this->GetConfigName(),
                                   /*implib=*/true).c_str());

  // Compute the comment.
  std::ostringstream comment;
  comment << "Link the " << this->GetVisibleTypeName() << " "
          << targetOutputReal;

  // Compute outputs.
  cmNinjaDeps outputs;
  outputs.push_back(targetOutputReal);

  // Compute specific libraries to link with.
  cmNinjaDeps explicitDeps = this->GetObjects(),
              implicitDeps = this->ComputeLinkDeps();

  this->GetLocalGenerator()->GetTargetFlags(vars["LINK_LIBRARIES"],
                                            vars["FLAGS"],
                                            vars["LINK_FLAGS"],
                                            *this->GetTarget());

  this->AddModuleDefinitionFlag(vars["LINK_FLAGS"]);

  // Compute architecture specific link flags.  Yes, these go into a different
  // variable for executables, probably due to a mistake made when duplicating
  // code between the Makefile executable and library generators.
  this->GetLocalGenerator()
      ->AddArchitectureFlags(targetType == cmTarget::EXECUTABLE
                               ? vars["FLAGS"]
                               : vars["ARCH_FLAGS"],
                             this->GetTarget(),
                             this->TargetLinkLanguage,
                             this->GetConfigName());
  vars["SONAME"] = this->TargetNameSO;

  if (targetType == cmTarget::SHARED_LIBRARY) {
    std::string install_name_dir =
      this->GetTarget()->GetInstallNameDirForBuildTree(this->GetConfigName());

    if (!install_name_dir.empty()) {
      vars["INSTALLNAME_DIR"] =
        this->GetLocalGenerator()->Convert(install_name_dir.c_str(),
            cmLocalGenerator::NONE,
            cmLocalGenerator::SHELL, false);
    }
  }

  if (!this->TargetNameImport.empty()) {
    vars["TARGET_IMPLIB"] = this->GetLocalGenerator()->ConvertToOutputFormat(
      targetOutputImplib.c_str(), cmLocalGenerator::SHELL);
  }

  vars["TARGET_PDB"] = this->GetLocalGenerator()->ConvertToOutputFormat(
    this->GetTargetPDB().c_str(), cmLocalGenerator::SHELL);

  std::vector<cmCustomCommand> *cmdLists[3] = {
    &this->GetTarget()->GetPreBuildCommands(),
    &this->GetTarget()->GetPreLinkCommands(),
    &this->GetTarget()->GetPostBuildCommands()
  };

  std::vector<std::string> preLinkCmdLines, postBuildCmdLines;
  std::vector<std::string> *cmdLineLists[3] = {
    &preLinkCmdLines,
    &preLinkCmdLines,
    &postBuildCmdLines
  };

  for (unsigned i = 0; i != 3; ++i) {
    for (std::vector<cmCustomCommand>::const_iterator
         ci = cmdLists[i]->begin();
         ci != cmdLists[i]->end(); ++ci) {
      this->GetLocalGenerator()->AppendCustomCommandLines(&*ci,
                                                          *cmdLineLists[i]);
    }
  }

  // If we have any PRE_LINK commands, we need to go back to HOME_OUTPUT for
  // the link commands.
  if (!preLinkCmdLines.empty()) {
    std::string path = this->GetLocalGenerator()->ConvertToOutputFormat(
      this->GetMakefile()->GetHomeOutputDirectory(),
      cmLocalGenerator::SHELL);
    preLinkCmdLines.push_back("cd " + path);
  }

  vars["PRE_LINK"] =
    this->GetLocalGenerator()->BuildCommandLine(preLinkCmdLines);
  std::string postBuildCmdLine =
    this->GetLocalGenerator()->BuildCommandLine(postBuildCmdLines);

  cmNinjaVars symlinkVars;
  if (targetOutput == targetOutputReal) {
    vars["POST_BUILD"] = postBuildCmdLine;
  } else {
    vars["POST_BUILD"] = ":";
    symlinkVars["POST_BUILD"] = postBuildCmdLine;
  }

  // Write the build statement for this target.
  cmGlobalNinjaGenerator::WriteBuild(this->GetBuildFileStream(),
                                     comment.str(),
                                     this->LanguageLinkerRule(),
                                     outputs,
                                     explicitDeps,
                                     implicitDeps,
                                     emptyDeps,
                                     vars);

  if (targetOutput != targetOutputReal) {
    if (targetType == cmTarget::EXECUTABLE) {
      cmGlobalNinjaGenerator::WriteBuild(this->GetBuildFileStream(),
                                  "Create executable symlink " + targetOutput,
                                         "CMAKE_SYMLINK_EXECUTABLE",
                                         cmNinjaDeps(1, targetOutput),
                                         cmNinjaDeps(1, targetOutputReal),
                                         emptyDeps,
                                         emptyDeps,
                                         symlinkVars);
    } else {
      symlinkVars["SONAME"] = this->GetTargetFilePath(this->TargetNameSO);
      cmGlobalNinjaGenerator::WriteBuild(this->GetBuildFileStream(),
                                      "Create library symlink " + targetOutput,
                                         "CMAKE_SYMLINK_LIBRARY",
                                         cmNinjaDeps(1, targetOutput),
                                         cmNinjaDeps(1, targetOutputReal),
                                         emptyDeps,
                                         emptyDeps,
                                         symlinkVars);
    }
  }

  if (!this->TargetNameImport.empty()) {
    // Since using multiple outputs would mess up the $out variable, use an
    // alias for the import library.
    cmGlobalNinjaGenerator::WritePhonyBuild(this->GetBuildFileStream(),
                                            "Alias for import library.",
                                            cmNinjaDeps(1, targetOutputImplib),
                                            cmNinjaDeps(1, targetOutputReal));
  }

  // Add aliases for the file name and the target name.
  this->GetGlobalGenerator()->AddTargetAlias(this->TargetNameOut,
                                             this->GetTarget());
  this->GetGlobalGenerator()->AddTargetAlias(this->GetTargetName(),
                                             this->GetTarget());
}

//----------------------------------------------------------------------------
void cmNinjaNormalTargetGenerator::WriteObjectLibStatement()
{
  // Write a phony output that depends on all object files.
  cmNinjaDeps outputs;
  this->GetLocalGenerator()->AppendTargetOutputs(this->GetTarget(), outputs);
  cmNinjaDeps depends = this->GetObjects();
  cmGlobalNinjaGenerator::WritePhonyBuild(this->GetBuildFileStream(),
                                          "Object library "
                                          + this->GetTargetName(),
                                          outputs,
                                          depends);

  // Add aliases for the target name.
  this->GetGlobalGenerator()->AddTargetAlias(this->GetTargetName(),
                                             this->GetTarget());
}
