/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalGhsMultiGenerator.h"

#include <algorithm>
#include <functional>
#include <map>
#include <sstream>
#include <utility>

#include <cm/memory>
#include <cm/string>
#include <cmext/algorithm>
#include <cmext/memory>

#include "cmCustomCommand.h"
#include "cmCustomCommandLines.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGhsMultiGpj.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmLocalGhsMultiGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSourceFile.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"
#include "cmVersion.h"
#include "cmake.h"

const char* cmGlobalGhsMultiGenerator::FILE_EXTENSION = ".gpj";
#ifdef __linux__
const char* cmGlobalGhsMultiGenerator::DEFAULT_BUILD_PROGRAM = "gbuild";
#elif defined(_WIN32)
const char* cmGlobalGhsMultiGenerator::DEFAULT_BUILD_PROGRAM = "gbuild.exe";
#endif
const char* cmGlobalGhsMultiGenerator::CHECK_BUILD_SYSTEM_TARGET =
  "RERUN_CMAKE";

cmGlobalGhsMultiGenerator::cmGlobalGhsMultiGenerator(cmake* cm)
  : cmGlobalGenerator(cm)
{
  cm->GetState()->SetGhsMultiIDE(true);
}

cmGlobalGhsMultiGenerator::~cmGlobalGhsMultiGenerator() = default;

std::unique_ptr<cmLocalGenerator>
cmGlobalGhsMultiGenerator::CreateLocalGenerator(cmMakefile* mf)
{
  return std::unique_ptr<cmLocalGenerator>(
    cm::make_unique<cmLocalGhsMultiGenerator>(this, mf));
}

cmDocumentationEntry cmGlobalGhsMultiGenerator::GetDocumentation()
{
  return {
    GetActualName(),
    "Generates Green Hills MULTI files (experimental, work-in-progress)."
  };
}

void cmGlobalGhsMultiGenerator::ComputeTargetObjectDirectory(
  cmGeneratorTarget* gt) const
{
  // Compute full path to object file directory for this target.
  std::string dir =
    cmStrCat(gt->LocalGenerator->GetCurrentBinaryDirectory(), '/',
             gt->LocalGenerator->GetTargetDirectory(gt), '/');
  gt->ObjectDirectory = dir;
}

bool cmGlobalGhsMultiGenerator::SetGeneratorToolset(std::string const& ts,
                                                    bool build, cmMakefile* mf)
{
  /* In build mode nothing to be done.
   * Toolset already determined and build tool absolute path is cached.
   */
  if (build) {
    return true;
  }

  /* Determine the absolute directory for the toolset */
  std::string tsp;
  this->GetToolset(mf, tsp, ts);

  /* no toolset was found */
  if (tsp.empty()) {
    return false;
  }

  /* set the build tool to use */
  std::string gbuild(tsp + ((tsp.back() == '/') ? "" : "/") +
                     DEFAULT_BUILD_PROGRAM);
  cmValue prevTool = mf->GetDefinition("CMAKE_MAKE_PROGRAM");

  /* check if the toolset changed from last generate */
  if (cmNonempty(prevTool) && !cmSystemTools::ComparePath(gbuild, *prevTool)) {
    std::string const& e = cmStrCat(
      "toolset build tool: ", gbuild, '\n',
      "Does not match the previously used build tool: ", *prevTool, '\n',
      "Either remove the CMakeCache.txt file and CMakeFiles "
      "directory or choose a different binary directory.");
    mf->IssueMessage(MessageType::FATAL_ERROR, e);
    return false;
  }

  /* store the toolset that is being used for this build */
  mf->AddCacheDefinition("CMAKE_MAKE_PROGRAM", gbuild, "build program to use",
                         cmStateEnums::INTERNAL, true);

  mf->AddDefinition("CMAKE_SYSTEM_VERSION", tsp);

  return true;
}

bool cmGlobalGhsMultiGenerator::SetGeneratorPlatform(std::string const& p,
                                                     cmMakefile* mf)
{
  /* set primary target */
  cmValue t = mf->GetDefinition("GHS_PRIMARY_TARGET");
  if (cmIsOff(t)) {
    /* Use the value from `-A` or use `arm` */
    std::string arch = "arm";
    if (!cmIsOff(p)) {
      arch = p;
    }
    cmValue platform = mf->GetDefinition("GHS_TARGET_PLATFORM");
    std::string tgt = cmStrCat(arch, '_', platform, ".tgt");

    /* update the primary target name*/
    mf->AddDefinition("GHS_PRIMARY_TARGET", tgt);
  }
  return true;
}

void cmGlobalGhsMultiGenerator::EnableLanguage(
  std::vector<std::string> const& l, cmMakefile* mf, bool optional)
{
  mf->AddDefinition("CMAKE_SYSTEM_NAME", "GHS-MULTI");

  mf->AddDefinition("GHSMULTI", "1"); // identifier for user CMake files

  this->cmGlobalGenerator::EnableLanguage(l, mf, optional);
}

bool cmGlobalGhsMultiGenerator::FindMakeProgram(cmMakefile* /*mf*/)
{
  // The GHS generator only knows how to lookup its build tool
  // during generation of the project files, but this
  // can only be done after the toolset is specified.

  return true;
}

void cmGlobalGhsMultiGenerator::GetToolset(cmMakefile* mf, std::string& tsp,
                                           const std::string& ts)
{
  /* Determine tsp - full path of the toolset from ts (toolset hint via -T) */

  std::string root = mf->GetSafeDefinition("GHS_TOOLSET_ROOT");

  // Check if `-T` was set by user
  if (ts.empty()) {
    // Enter toolset search mode
    std::vector<std::string> output;

    // Make sure root exists...
    if (!cmSystemTools::PathExists(root)) {
      std::string msg =
        "GHS_TOOLSET_ROOT directory \"" + root + "\" does not exist.";
      mf->IssueMessage(MessageType::FATAL_ERROR, msg);
      tsp = "";
      return;
    }

    // Add a directory separator
    if (root.back() != '/') {
      root += "/";
    }

    // Get all compiler directories in toolset root
    cmSystemTools::Glob(root, "comp_[^;]+", output);

    if (output.empty()) {
      // No compiler directories found
      std::string msg =
        "No GHS toolsets found in GHS_TOOLSET_ROOT \"" + root + "\".";
      mf->IssueMessage(MessageType::FATAL_ERROR, msg);
      tsp = "";
    } else {
      // Use latest? version
      tsp = root + output.back();
    }

  } else {
    // Toolset was provided by user
    std::string tryPath;

    // NOTE: CollapseFullPath() will determine if user toolset was full path or
    //       or relative path.
    tryPath = cmSystemTools::CollapseFullPath(ts, root);
    if (!cmSystemTools::FileExists(tryPath)) {
      std::string msg = "GHS toolset \"" + tryPath + "\" does not exist.";
      mf->IssueMessage(MessageType::FATAL_ERROR, msg);
      tsp = "";
    } else {
      tsp = tryPath;
    }
  }
}

void cmGlobalGhsMultiGenerator::WriteFileHeader(std::ostream& fout)
{
  /* clang-format off */
  fout << "#!gbuild\n"
          "#\n"
          "# CMAKE generated file: DO NOT EDIT!\n"
          "# Generated by \"" << GetActualName() << "\""
          " Generator, CMake Version " << cmVersion::GetMajorVersion() << '.'
       << cmVersion::GetMinorVersion() << "\n"
          "#\n\n";
  /* clang-format on */
}

void cmGlobalGhsMultiGenerator::WriteCustomRuleBOD(std::ostream& fout)
{
  fout << "Commands {\n"
          "  Custom_Rule_Command {\n"
          "    name = \"Custom Rule Command\"\n"
          "    exec = \""
#ifdef _WIN32
          "cmd.exe"
#else
          "/bin/sh"
#endif
          "\"\n"
          "    options = {\"SpecialOptions\"}\n"
          "  }\n"
          "}\n"

          "\n\n"
          "FileTypes {\n"
          "  CmakeRule {\n"
          "    name = \"Custom Rule\"\n"
          "    action = \"&Run\"\n"
          "    extensions = {\""
#ifdef _WIN32
          "bat"
#else
          "sh"
#endif
          "\"}\n"
          "    grepable = false\n"
          "    command = \"Custom Rule Command\"\n"
          "    commandLine = \"$COMMAND "
#ifdef _WIN32
          "/c"
#endif
          " $INPUTFILE\"\n"
          "    progress = \"Processing Custom Rule\"\n"
          "    promoteToFirstPass = true\n"
          "    outputType = \"None\"\n"
          "    color = \"#800080\"\n"
          "  }\n"
          "}\n";
}

void cmGlobalGhsMultiGenerator::WriteCustomTargetBOD(std::ostream& fout)
{
  fout << "FileTypes {\n"
          "  CmakeTarget {\n"
          "    name = \"Custom Target\"\n"
          "    action = \"&Execute\"\n"
          "    grepable = false\n"
          "    outputType = \"None\"\n"
          "    color = \"#800080\"\n"
          "  }\n"
          "}\n";
}

void cmGlobalGhsMultiGenerator::WriteTopLevelProject(std::ostream& fout,
                                                     cmLocalGenerator* root)
{
  this->WriteFileHeader(fout);
  this->WriteMacros(fout, root);
  this->WriteHighLevelDirectives(fout, root);
  GhsMultiGpj::WriteGpjTag(GhsMultiGpj::PROJECT, fout);

  fout << "# Top Level Project File\n";

  // Specify BSP option if supplied by user
  // -- not all platforms require this entry in the project file
  cmValue bspName = root->GetMakefile()->GetDefinition("GHS_BSP_NAME");
  if (!cmIsOff(bspName)) {
    fout << "    -bsp " << *bspName << '\n';
  }

  // Specify OS DIR if supplied by user
  // -- not all platforms require this entry in the project file
  cmValue osDir = root->GetMakefile()->GetDefinition("GHS_OS_DIR");
  if (!cmIsOff(osDir)) {
    cmValue osDirOption =
      root->GetMakefile()->GetDefinition("GHS_OS_DIR_OPTION");
    fout << "    ";
    if (cmIsOff(osDirOption)) {
      fout << "";
    } else {
      fout << *osDirOption;
    }
    fout << "\"" << osDir << "\"\n";
  }
}

void cmGlobalGhsMultiGenerator::WriteSubProjects(std::ostream& fout,
                                                 bool filterPredefined)
{
  std::set<std::string> predefinedTargets;
  predefinedTargets.insert(this->GetInstallTargetName());
  predefinedTargets.insert(this->GetAllTargetName());
  predefinedTargets.insert(std::string(CHECK_BUILD_SYSTEM_TARGET));

  // All known targets
  for (cmGeneratorTarget const* target : this->ProjectTargets) {
    if (target->GetType() == cmStateEnums::INTERFACE_LIBRARY ||
        target->GetType() == cmStateEnums::MODULE_LIBRARY ||
        target->GetType() == cmStateEnums::SHARED_LIBRARY ||
        (target->GetType() == cmStateEnums::GLOBAL_TARGET &&
         target->GetName() != this->GetInstallTargetName())) {
      continue;
    }
    /* Check if the current target is a predefined CMake target */
    bool predefinedTarget =
      predefinedTargets.find(target->GetName()) != predefinedTargets.end();
    if ((filterPredefined && predefinedTarget) ||
        (!filterPredefined && !predefinedTarget)) {
      fout << target->GetName() + ".tgt" + FILE_EXTENSION << " [Project]\n";
    }
  }
}

void cmGlobalGhsMultiGenerator::WriteProjectLine(
  std::ostream& fout, cmGeneratorTarget const* target,
  std::string& rootBinaryDir)
{
  cmValue projFile = target->GetProperty("GENERATOR_FILE_NAME");
  cmValue projType = target->GetProperty("GENERATOR_FILE_NAME_EXT");
  /* If either value is not valid then this particular target is an
   * unsupported target type and should be skipped.
   */
  if (projFile && projType) {
    std::string path = cmSystemTools::RelativePath(rootBinaryDir, *projFile);

    fout << path;
    fout << ' ' << *projType << '\n';
  }
}

void cmGlobalGhsMultiGenerator::WriteTargets(cmLocalGenerator* root)
{
  std::string rootBinaryDir = root->GetCurrentBinaryDirectory();

  // All known targets
  for (cmGeneratorTarget const* target : this->ProjectTargets) {
    if (target->GetType() == cmStateEnums::INTERFACE_LIBRARY ||
        target->GetType() == cmStateEnums::MODULE_LIBRARY ||
        target->GetType() == cmStateEnums::SHARED_LIBRARY ||
        (target->GetType() == cmStateEnums::GLOBAL_TARGET &&
         target->GetName() != this->GetInstallTargetName())) {
      continue;
    }

    // create target build file
    std::string name = cmStrCat(target->GetName(), ".tgt", FILE_EXTENSION);
    std::string fname = cmStrCat(rootBinaryDir, "/", name);
    cmGeneratedFileStream fbld(fname);
    fbld.SetCopyIfDifferent(true);
    this->WriteFileHeader(fbld);
    GhsMultiGpj::WriteGpjTag(GhsMultiGpj::PROJECT, fbld);
    std::vector<cmGeneratorTarget const*> build;
    if (this->ComputeTargetBuildOrder(target, build)) {
      cmSystemTools::Error(
        cmStrCat("The inter-target dependency graph for target [",
                 target->GetName(), "] had a cycle.\n"));
    } else {
      for (auto& tgt : build) {
        this->WriteProjectLine(fbld, tgt, rootBinaryDir);
      }
    }
    fbld.Close();
  }
}

void cmGlobalGhsMultiGenerator::Generate()
{
  std::string fname;

  // first do the superclass method
  this->cmGlobalGenerator::Generate();

  // output top-level projects
  for (auto& it : this->ProjectMap) {
    this->OutputTopLevelProject(it.second[0], it.second);
  }

  // create custom rule BOD file
  fname = this->GetCMakeInstance()->GetHomeOutputDirectory() +
    "/CMakeFiles/custom_rule.bod";
  cmGeneratedFileStream frule(fname);
  frule.SetCopyIfDifferent(true);
  this->WriteFileHeader(frule);
  this->WriteCustomRuleBOD(frule);
  frule.Close();

  // create custom target BOD file
  fname = this->GetCMakeInstance()->GetHomeOutputDirectory() +
    "/CMakeFiles/custom_target.bod";
  cmGeneratedFileStream ftarget(fname);
  ftarget.SetCopyIfDifferent(true);
  this->WriteFileHeader(ftarget);
  this->WriteCustomTargetBOD(ftarget);
  ftarget.Close();
}

void cmGlobalGhsMultiGenerator::OutputTopLevelProject(
  cmLocalGenerator* root, std::vector<cmLocalGenerator*>& generators)
{
  std::string fname;

  if (generators.empty()) {
    return;
  }

  // Collect all targets under this root generator and the transitive
  // closure of their dependencies.
  TargetDependSet projectTargets;
  TargetDependSet originalTargets;
  this->GetTargetSets(projectTargets, originalTargets, root, generators);
  OrderedTargetDependSet sortedProjectTargets(projectTargets, "");
  this->ProjectTargets.clear();
  for (cmGeneratorTarget const* t : sortedProjectTargets) {
    /* save list of all targets in sorted order */
    this->ProjectTargets.push_back(t);
  }

  /* Name top-level projects as filename.top.gpj to avoid name clashes
   * with target projects.  This avoid the issue where the project has
   * the same name as the executable target.
   */
  fname = cmStrCat(root->GetCurrentBinaryDirectory(), '/',
                   root->GetProjectName(), ".top", FILE_EXTENSION);

  cmGeneratedFileStream top(fname);
  top.SetCopyIfDifferent(true);
  this->WriteTopLevelProject(top, root);
  this->WriteTargets(root);
  this->WriteSubProjects(top, true);
  this->WriteSubProjects(top, false);
  top.Close();
}

std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalGhsMultiGenerator::GenerateBuildCommand(
  const std::string& makeProgram, const std::string& projectName,
  const std::string& projectDir, std::vector<std::string> const& targetNames,
  const std::string& /*config*/, int jobs, bool verbose,
  const cmBuildOptions& /*buildOptions*/,
  std::vector<std::string> const& makeOptions)
{
  GeneratedMakeCommand makeCommand;

  makeCommand.Add(this->SelectMakeProgram(makeProgram));

  if (jobs != cmake::NO_BUILD_PARALLEL_LEVEL) {
    if (jobs == cmake::DEFAULT_BUILD_PARALLEL_LEVEL) {
      makeCommand.Add("-parallel");
    } else {
      makeCommand.Add(std::string("-parallel=") + std::to_string(jobs));
    }
  }

  /* determine the top-project file in the project directory */
  std::string proj = projectName + ".top" + FILE_EXTENSION;
  std::vector<std::string> files;
  cmSystemTools::Glob(projectDir, ".*\\.top\\.gpj", files);
  if (!files.empty()) {
    /* use the real top-level project in the directory */
    proj = files.at(0);
  }
  makeCommand.Add("-top", proj);

  /* determine targets to build */
  bool build_all = false;
  if (!targetNames.empty()) {
    for (const auto& tname : targetNames) {
      if (!tname.empty()) {
        if (tname == "clean") {
          makeCommand.Add("-clean");
        } else {
          makeCommand.Add(tname + ".tgt.gpj");
        }
      } else {
        build_all = true;
      }
    }
  } else {
    build_all = true;
  }

  if (build_all) {
    /* transform name to default build */;
    std::string all = std::string(this->GetAllTargetName()) + ".tgt.gpj";
    makeCommand.Add(all);
  }

  if (verbose) {
    makeCommand.Add("-commands");
  }
  makeCommand.Add(makeOptions.begin(), makeOptions.end());

  return { std::move(makeCommand) };
}

void cmGlobalGhsMultiGenerator::WriteMacros(std::ostream& fout,
                                            cmLocalGenerator* root)
{
  fout << "macro PROJ_NAME=" << root->GetProjectName() << '\n';
  cmValue ghsGpjMacros = root->GetMakefile()->GetDefinition("GHS_GPJ_MACROS");
  if (ghsGpjMacros) {
    cmList expandedList{ *ghsGpjMacros };
    for (std::string const& arg : expandedList) {
      fout << "macro " << arg << '\n';
    }
  }
}

void cmGlobalGhsMultiGenerator::WriteHighLevelDirectives(
  std::ostream& fout, cmLocalGenerator* root)
{
  /* put primary target and customization files into project file */
  cmValue const tgt = root->GetMakefile()->GetDefinition("GHS_PRIMARY_TARGET");

  /* clang-format off */
  fout << "primaryTarget=" << tgt << "\n"
          "customization=" << root->GetBinaryDirectory()
       << "/CMakeFiles/custom_rule.bod\n"
          "customization=" << root->GetBinaryDirectory()
       << "/CMakeFiles/custom_target.bod" << '\n';
  /* clang-format on */

  cmValue const customization =
    root->GetMakefile()->GetDefinition("GHS_CUSTOMIZATION");
  if (cmNonempty(customization)) {
    fout << "customization="
         << cmGlobalGhsMultiGenerator::TrimQuotes(*customization) << '\n';
    this->GetCMakeInstance()->MarkCliAsUsed("GHS_CUSTOMIZATION");
  }
}

std::string cmGlobalGhsMultiGenerator::TrimQuotes(std::string str)
{
  cm::erase(str, '"');
  return str;
}

bool cmGlobalGhsMultiGenerator::TargetCompare::operator()(
  cmGeneratorTarget const* l, cmGeneratorTarget const* r) const
{
  // Make sure a given named target is ordered first,
  // e.g. to set ALL_BUILD as the default active project.
  // When the empty string is named this is a no-op.
  if (r->GetName() == this->First) {
    return false;
  }
  if (l->GetName() == this->First) {
    return true;
  }
  return l->GetName() < r->GetName();
}

cmGlobalGhsMultiGenerator::OrderedTargetDependSet::OrderedTargetDependSet(
  TargetDependSet const& targets, std::string const& first)
  : derived(TargetCompare(first))
{
  this->insert(targets.begin(), targets.end());
}

bool cmGlobalGhsMultiGenerator::ComputeTargetBuildOrder(
  cmGeneratorTarget const* tgt, std::vector<cmGeneratorTarget const*>& build)
{
  std::vector<cmGeneratorTarget const*> t{ tgt };
  return this->ComputeTargetBuildOrder(t, build);
}

bool cmGlobalGhsMultiGenerator::ComputeTargetBuildOrder(
  std::vector<cmGeneratorTarget const*>& tgt,
  std::vector<cmGeneratorTarget const*>& build)
{
  std::set<cmGeneratorTarget const*> temp;
  std::set<cmGeneratorTarget const*> perm;

  for (const auto* const ti : tgt) {
    bool r = this->VisitTarget(temp, perm, build, ti);
    if (r) {
      return r;
    }
  }
  return false;
}

bool cmGlobalGhsMultiGenerator::VisitTarget(
  std::set<cmGeneratorTarget const*>& temp,
  std::set<cmGeneratorTarget const*>& perm,
  std::vector<cmGeneratorTarget const*>& order, cmGeneratorTarget const* ti)
{
  /* check if permanent mark is set*/
  if (perm.find(ti) == perm.end()) {
    /* set temporary mark; check if revisit*/
    if (temp.insert(ti).second) {
      /* sort targets lexicographically to ensure that nodes are always visited
       * in the same order */
      OrderedTargetDependSet sortedTargets(this->GetTargetDirectDepends(ti),
                                           "");
      for (auto const& di : sortedTargets) {
        if (this->VisitTarget(temp, perm, order, di)) {
          return true;
        }
      }
      /* mark as complete; insert into beginning of list*/
      perm.insert(ti);
      order.push_back(ti);
      return false;
    }
    /* revisiting item - not a DAG */
    return true;
  }
  /* already complete */
  return false;
}

bool cmGlobalGhsMultiGenerator::AddCheckTarget()
{
  // Skip the target if no regeneration is to be done.
  if (this->GlobalSettingIsOn("CMAKE_SUPPRESS_REGENERATION")) {
    return false;
  }

  // Get the generators.
  std::vector<std::unique_ptr<cmLocalGenerator>> const& generators =
    this->LocalGenerators;
  auto& lg =
    cm::static_reference_cast<cmLocalGhsMultiGenerator>(generators[0]);

  // The name of the output file for the custom command.
  this->StampFile = lg.GetBinaryDirectory() + std::string("/CMakeFiles/") +
    CHECK_BUILD_SYSTEM_TARGET;

  // Add a custom rule to re-run CMake if any input files changed.
  {
    // Collect the input files used to generate all targets in this
    // project.
    std::vector<std::string> listFiles;
    for (const auto& gen : generators) {
      cm::append(listFiles, gen->GetMakefile()->GetListFiles());
    }

    // Add the cache file.
    listFiles.push_back(cmStrCat(
      this->GetCMakeInstance()->GetHomeOutputDirectory(), "/CMakeCache.txt"));

    // Print not implemented warning.
    if (this->GetCMakeInstance()->DoWriteGlobVerifyTarget()) {
      std::ostringstream msg;
      msg << "Any pre-check scripts, such as those generated for file(GLOB "
             "CONFIGURE_DEPENDS), will not be run by gbuild.";
      this->GetCMakeInstance()->IssueMessage(MessageType::AUTHOR_WARNING,
                                             msg.str());
    }

    // Sort the list of input files and remove duplicates.
    std::sort(listFiles.begin(), listFiles.end(), std::less<std::string>());
    auto newEnd = std::unique(listFiles.begin(), listFiles.end());
    listFiles.erase(newEnd, listFiles.end());

    // Create a rule to re-run CMake and create output file.
    cmCustomCommandLines commandLines;
    commandLines.emplace_back(
      cmMakeCommandLine({ cmSystemTools::GetCMakeCommand(), "-E", "rm", "-f",
                          this->StampFile }));
    std::string argS = cmStrCat("-S", lg.GetSourceDirectory());
    std::string argB = cmStrCat("-B", lg.GetBinaryDirectory());
    commandLines.emplace_back(
      cmMakeCommandLine({ cmSystemTools::GetCMakeCommand(), argS, argB }));
    commandLines.emplace_back(cmMakeCommandLine(
      { cmSystemTools::GetCMakeCommand(), "-E", "touch", this->StampFile }));

    /* Create the target(Exclude from ALL_BUILD).
     *
     * The build tool, currently, does not support rereading the project files
     * if they get updated. So do not run this target as part of ALL_BUILD.
     */
    auto cc = cm::make_unique<cmCustomCommand>();
    cmTarget* tgt =
      lg.AddUtilityCommand(CHECK_BUILD_SYSTEM_TARGET, true, std::move(cc));
    auto ptr = cm::make_unique<cmGeneratorTarget>(tgt, &lg);
    auto* gt = ptr.get();
    lg.AddGeneratorTarget(std::move(ptr));

    // Add the rule.
    cc = cm::make_unique<cmCustomCommand>();
    cc->SetOutputs(this->StampFile);
    cc->SetDepends(listFiles);
    cc->SetCommandLines(commandLines);
    cc->SetComment("Checking Build System");
    cc->SetEscapeOldStyle(false);
    cc->SetStdPipesUTF8(true);

    if (cmSourceFile* file =
          lg.AddCustomCommandToOutput(std::move(cc), true)) {
      gt->AddSource(file->ResolveFullPath());
    } else {
      cmSystemTools::Error("Error adding rule for " + this->StampFile);
    }
    // Organize in the "predefined targets" folder:
    if (this->UseFolderProperty()) {
      tgt->SetProperty("FOLDER", this->GetPredefinedTargetsFolder());
    }
  }

  return true;
}

void cmGlobalGhsMultiGenerator::AddAllTarget()
{
  // Add a special target that depends on ALL projects for easy build
  // of one configuration only.
  for (auto const& it : this->ProjectMap) {
    std::vector<cmLocalGenerator*> const& gen = it.second;
    // add the ALL_BUILD to the first local generator of each project
    if (!gen.empty()) {
      // Use no actual command lines so that the target itself is not
      // considered always out of date.
      auto cc = cm::make_unique<cmCustomCommand>();
      cc->SetEscapeOldStyle(false);
      cc->SetComment("Build all projects");
      cmTarget* allBuild = gen[0]->AddUtilityCommand(this->GetAllTargetName(),
                                                     true, std::move(cc));

      gen[0]->AddGeneratorTarget(
        cm::make_unique<cmGeneratorTarget>(allBuild, gen[0]));

      // Organize in the "predefined targets" folder:
      if (this->UseFolderProperty()) {
        allBuild->SetProperty("FOLDER", this->GetPredefinedTargetsFolder());
      }

      // Now make all targets depend on the ALL_BUILD target
      for (cmLocalGenerator const* i : gen) {
        for (const auto& tgt : i->GetGeneratorTargets()) {
          // Skip global or imported targets
          if (tgt->GetType() == cmStateEnums::GLOBAL_TARGET ||
              tgt->IsImported()) {
            continue;
          }
          // Skip Exclude From All Targets
          if (!this->IsExcluded(gen[0], tgt.get())) {
            allBuild->AddUtility(tgt->GetName(), false);
          }
        }
      }
    }
  }
}

void cmGlobalGhsMultiGenerator::AddExtraIDETargets()
{
  // Add a special target that depends on ALL projects.
  this->AddAllTarget();

  /* Add Custom Target to check if CMake needs to be rerun.
   *
   * The build tool, currently, does not support rereading the project files
   * if they get updated.  So do not make the other targets dependent on this
   * check.
   */
  this->AddCheckTarget();
}
