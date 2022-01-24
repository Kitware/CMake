/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalGhsMultiGenerator.h"

#include <map>
#include <ostream>
#include <utility>

#include <cm/memory>
#include <cm/string>
#include <cmext/algorithm>

#include "cmDocumentationEntry.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGhsMultiGpj.h"
#include "cmLocalGenerator.h"
#include "cmLocalGhsMultiGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmVersion.h"
#include "cmake.h"

const char* cmGlobalGhsMultiGenerator::FILE_EXTENSION = ".gpj";
#ifdef __linux__
const char* cmGlobalGhsMultiGenerator::DEFAULT_BUILD_PROGRAM = "gbuild";
#elif defined(_WIN32)
const char* cmGlobalGhsMultiGenerator::DEFAULT_BUILD_PROGRAM = "gbuild.exe";
#endif

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

void cmGlobalGhsMultiGenerator::GetDocumentation(cmDocumentationEntry& entry)
{
  entry.Name = GetActualName();
  entry.Brief =
    "Generates Green Hills MULTI files (experimental, work-in-progress).";
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
  if (cmNonempty(prevTool) && !cmSystemTools::ComparePath(gbuild, prevTool)) {
    std::string const& e =
      cmStrCat("toolset build tool: ", gbuild,
               "\nDoes not match the previously used build tool: ", prevTool,
               "\nEither remove the CMakeCache.txt file and CMakeFiles "
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
  this->WriteHighLevelDirectives(root, fout);
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
                                                 std::string& all_target)
{
  fout << "CMakeFiles/" << all_target << " [Project]\n";
  // All known targets
  for (cmGeneratorTarget const* target : this->ProjectTargets) {
    if (target->GetType() == cmStateEnums::INTERFACE_LIBRARY ||
        target->GetType() == cmStateEnums::MODULE_LIBRARY ||
        target->GetType() == cmStateEnums::SHARED_LIBRARY ||
        (target->GetType() == cmStateEnums::GLOBAL_TARGET &&
         target->GetName() != this->GetInstallTargetName())) {
      continue;
    }
    fout << "CMakeFiles/" << target->GetName() + ".tgt" + FILE_EXTENSION
         << " [Project]\n";
  }
}

void cmGlobalGhsMultiGenerator::WriteProjectLine(
  std::ostream& fout, cmGeneratorTarget const* target,
  std::string& rootBinaryDir)
{
  cmValue projName = target->GetProperty("GENERATOR_FILE_NAME");
  cmValue projType = target->GetProperty("GENERATOR_FILE_NAME_EXT");
  if (projName && projType) {
    cmLocalGenerator* lg = target->GetLocalGenerator();
    std::string dir = lg->GetCurrentBinaryDirectory();
    dir = cmSystemTools::ForceToRelativePath(rootBinaryDir, dir);
    if (dir == ".") {
      dir.clear();
    } else {
      if (dir.back() != '/') {
        dir += "/";
      }
    }

    std::string projFile = dir + *projName + FILE_EXTENSION;
    fout << projFile;
    fout << ' ' << *projType << '\n';
  } else {
    /* Should never happen */
    std::string message =
      "The project file for target [" + target->GetName() + "] is missing.\n";
    cmSystemTools::Error(message);
    fout << "{comment} " << target->GetName() << " [missing project file]\n";
  }
}

void cmGlobalGhsMultiGenerator::WriteTargets(cmLocalGenerator* root)
{
  std::string rootBinaryDir =
    cmStrCat(root->GetCurrentBinaryDirectory(), "/CMakeFiles");

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

void cmGlobalGhsMultiGenerator::WriteAllTarget(
  cmLocalGenerator* root, std::vector<cmLocalGenerator*>& generators,
  std::string& all_target)
{
  this->ProjectTargets.clear();

  // create target build file
  all_target = root->GetProjectName() + "." + this->GetAllTargetName() +
    ".tgt" + FILE_EXTENSION;
  std::string fname =
    root->GetCurrentBinaryDirectory() + "/CMakeFiles/" + all_target;
  cmGeneratedFileStream fbld(fname);
  fbld.SetCopyIfDifferent(true);
  this->WriteFileHeader(fbld);
  GhsMultiGpj::WriteGpjTag(GhsMultiGpj::PROJECT, fbld);

  // Collect all targets under this root generator and the transitive
  // closure of their dependencies.
  TargetDependSet projectTargets;
  TargetDependSet originalTargets;
  this->GetTargetSets(projectTargets, originalTargets, root, generators);
  OrderedTargetDependSet sortedProjectTargets(projectTargets, "");
  std::vector<cmGeneratorTarget const*> defaultTargets;
  for (cmGeneratorTarget const* t : sortedProjectTargets) {
    /* save list of all targets in sorted order */
    this->ProjectTargets.push_back(t);
  }
  for (cmGeneratorTarget const* t : sortedProjectTargets) {
    if (!t->IsInBuildSystem()) {
      continue;
    }
    if (!this->IsExcluded(t->GetLocalGenerator(), t)) {
      defaultTargets.push_back(t);
    }
  }
  std::vector<cmGeneratorTarget const*> build;
  if (this->ComputeTargetBuildOrder(defaultTargets, build)) {
    std::string message = "The inter-target dependency graph for project [" +
      root->GetProjectName() + "] had a cycle.\n";
    cmSystemTools::Error(message);
  } else {
    // determine the targets for ALL target
    std::string rootBinaryDir =
      cmStrCat(root->GetCurrentBinaryDirectory(), "/CMakeFiles");
    for (cmGeneratorTarget const* target : build) {
      if (target->GetType() == cmStateEnums::INTERFACE_LIBRARY ||
          target->GetType() == cmStateEnums::MODULE_LIBRARY ||
          target->GetType() == cmStateEnums::SHARED_LIBRARY) {
        continue;
      }
      this->WriteProjectLine(fbld, target, rootBinaryDir);
    }
  }
  fbld.Close();
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
  std::string all_target;

  if (generators.empty()) {
    return;
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

  this->WriteAllTarget(root, generators, all_target);
  this->WriteTargets(root);

  this->WriteSubProjects(top, all_target);
  top.Close();
}

std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalGhsMultiGenerator::GenerateBuildCommand(
  const std::string& makeProgram, const std::string& projectName,
  const std::string& projectDir, std::vector<std::string> const& targetNames,
  const std::string& /*config*/, int jobs, bool /*verbose*/,
  const cmBuildOptions& /*buildOptions*/,
  std::vector<std::string> const& makeOptions)
{
  GeneratedMakeCommand makeCommand = {};
  std::string gbuild;
  if (cmValue gbuildCached =
        this->CMakeInstance->GetCacheDefinition("CMAKE_MAKE_PROGRAM")) {
    gbuild = *gbuildCached;
  }
  makeCommand.Add(this->SelectMakeProgram(makeProgram, gbuild));

  if (jobs != cmake::NO_BUILD_PARALLEL_LEVEL) {
    makeCommand.Add("-parallel");
    if (jobs != cmake::DEFAULT_BUILD_PARALLEL_LEVEL) {
      makeCommand.Add(std::to_string(jobs));
    }
  }

  makeCommand.Add(makeOptions.begin(), makeOptions.end());

  /* determine which top-project file to use */
  std::string proj = projectName + ".top" + FILE_EXTENSION;
  std::vector<std::string> files;
  cmSystemTools::Glob(projectDir, ".*\\.top\\.gpj", files);
  if (!files.empty()) {
    /* if multiple top-projects are found in build directory
     * then prefer projectName top-project.
     */
    if (!cm::contains(files, proj)) {
      proj = files.at(0);
    }
  }

  makeCommand.Add("-top", proj);
  if (!targetNames.empty()) {
    if (cm::contains(targetNames, "clean")) {
      makeCommand.Add("-clean");
    } else {
      for (const auto& tname : targetNames) {
        if (!tname.empty()) {
          makeCommand.Add(tname + ".tgt.gpj");
        }
      }
    }
  } else {
    /* transform name to default build */;
    std::string all = proj;
    all.replace(all.end() - 7, all.end(),
                std::string(this->GetAllTargetName()) + ".tgt.gpj");
    makeCommand.Add(all);
  }
  return { makeCommand };
}

void cmGlobalGhsMultiGenerator::WriteMacros(std::ostream& fout,
                                            cmLocalGenerator* root)
{
  fout << "macro PROJ_NAME=" << root->GetProjectName() << '\n';
  cmValue ghsGpjMacros = root->GetMakefile()->GetDefinition("GHS_GPJ_MACROS");
  if (ghsGpjMacros) {
    std::vector<std::string> expandedList = cmExpandedList(*ghsGpjMacros);
    for (std::string const& arg : expandedList) {
      fout << "macro " << arg << '\n';
    }
  }
}

void cmGlobalGhsMultiGenerator::WriteHighLevelDirectives(
  cmLocalGenerator* root, std::ostream& fout)
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
