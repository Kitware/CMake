/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalGhsMultiGenerator.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <ostream>
#include <utility>

#include <cm/memory>

#include "cmAlgorithms.h"
#include "cmDocumentationEntry.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGhsMultiGpj.h"
#include "cmLocalGenerator.h"
#include "cmLocalGhsMultiGenerator.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmVersion.h"
#include "cmake.h"

const char* cmGlobalGhsMultiGenerator::FILE_EXTENSION = ".gpj";
#ifdef __linux__
const char* cmGlobalGhsMultiGenerator::DEFAULT_BUILD_PROGRAM = "gbuild";
const char* cmGlobalGhsMultiGenerator::DEFAULT_TOOLSET_ROOT = "/usr/ghs";
#elif defined(_WIN32)
const char* cmGlobalGhsMultiGenerator::DEFAULT_BUILD_PROGRAM = "gbuild.exe";
const char* cmGlobalGhsMultiGenerator::DEFAULT_TOOLSET_ROOT = "C:/ghs";
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
  if (build) {
    return true;
  }
  std::string tsp; /* toolset path */

  this->GetToolset(mf, tsp, ts);

  /* no toolset was found */
  if (tsp.empty()) {
    return false;
  }
  if (ts.empty()) {
    std::string message;
    message = cmStrCat(
      "Green Hills MULTI: -T <toolset> not specified; defaulting to \"", tsp,
      '"');
    cmSystemTools::Message(message);

    /* store the full toolset for later use
     * -- already done if -T<toolset> was specified
     */
    mf->AddCacheDefinition("CMAKE_GENERATOR_TOOLSET", tsp.c_str(),
                           "Location of generator toolset.",
                           cmStateEnums::INTERNAL);
  }

  /* set the build tool to use */
  std::string gbuild(tsp + ((tsp.back() == '/') ? "" : "/") +
                     DEFAULT_BUILD_PROGRAM);
  const char* prevTool = mf->GetDefinition("CMAKE_MAKE_PROGRAM");

  /* check if the toolset changed from last generate */
  if (prevTool != nullptr && (gbuild != prevTool)) {
    std::string message =
      cmStrCat("toolset build tool: ", gbuild,
               "\nDoes not match the previously used build tool: ", prevTool,
               "\nEither remove the CMakeCache.txt file and CMakeFiles "
               "directory or choose a different binary directory.");
    cmSystemTools::Error(message);
    return false;
  }

  /* store the toolset that is being used for this build */
  mf->AddCacheDefinition("CMAKE_MAKE_PROGRAM", gbuild.c_str(),
                         "build program to use", cmStateEnums::INTERNAL, true);

  mf->AddDefinition("CMAKE_SYSTEM_VERSION", tsp);

  return true;
}

bool cmGlobalGhsMultiGenerator::SetGeneratorPlatform(std::string const& p,
                                                     cmMakefile* mf)
{
  std::string arch;
  if (p.empty()) {
    cmSystemTools::Message(
      "Green Hills MULTI: -A <arch> not specified; defaulting to \"arm\"");
    arch = "arm";

    /* store the platform name for later use
     * -- already done if -A<arch> was specified
     */
    mf->AddCacheDefinition("CMAKE_GENERATOR_PLATFORM", arch.c_str(),
                           "Name of generator platform.",
                           cmStateEnums::INTERNAL);
  } else {
    arch = p;
  }

  /* check if OS location has been updated by platform scripts */
  std::string platform = mf->GetSafeDefinition("GHS_TARGET_PLATFORM");
  std::string osdir = mf->GetSafeDefinition("GHS_OS_DIR");
  if (cmIsOff(osdir) && platform.find("integrity") != std::string::npos) {
    if (!this->CMakeInstance->GetIsInTryCompile()) {
      /* required OS location is not found */
      std::string m = cmStrCat(
        "Green Hills MULTI: GHS_OS_DIR not specified; No OS found in \"",
        mf->GetSafeDefinition("GHS_OS_ROOT"), '"');
      cmSystemTools::Message(m);
    }
    osdir = "GHS_OS_DIR-NOT-SPECIFIED";
  } else if (!this->CMakeInstance->GetIsInTryCompile() &&
             cmIsOff(this->OsDir) && !cmIsOff(osdir)) {
    /* OS location was updated by auto-selection */
    std::string m = cmStrCat(
      "Green Hills MULTI: GHS_OS_DIR not specified; found \"", osdir, '"');
    cmSystemTools::Message(m);
  }
  this->OsDir = osdir;

  // Determine GHS_BSP_NAME
  std::string bspName = mf->GetSafeDefinition("GHS_BSP_NAME");

  if (cmIsOff(bspName) && platform.find("integrity") != std::string::npos) {
    bspName = "sim" + arch;
    /* write back the calculate name for next time */
    mf->AddCacheDefinition("GHS_BSP_NAME", bspName.c_str(),
                           "Name of GHS target platform.",
                           cmStateEnums::STRING, true);
    std::string m = cmStrCat(
      "Green Hills MULTI: GHS_BSP_NAME not specified; defaulting to \"",
      bspName, '"');
    cmSystemTools::Message(m);
  }

  return true;
}

void cmGlobalGhsMultiGenerator::EnableLanguage(
  std::vector<std::string> const& l, cmMakefile* mf, bool optional)
{
  mf->AddDefinition("CMAKE_SYSTEM_NAME", "GHS-MULTI");

  mf->AddDefinition("GHSMULTI", "1"); // identifier for user CMake files

  const char* tgtPlatform = mf->GetDefinition("GHS_TARGET_PLATFORM");
  if (!tgtPlatform) {
    cmSystemTools::Message("Green Hills MULTI: GHS_TARGET_PLATFORM not "
                           "specified; defaulting to \"integrity\"");
    tgtPlatform = "integrity";
  }

  /* store the platform name for later use */
  mf->AddCacheDefinition("GHS_TARGET_PLATFORM", tgtPlatform,
                         "Name of GHS target platform.", cmStateEnums::STRING);

  /* store original OS location */
  this->OsDir = mf->GetSafeDefinition("GHS_OS_DIR");

  this->cmGlobalGenerator::EnableLanguage(l, mf, optional);
}

bool cmGlobalGhsMultiGenerator::FindMakeProgram(cmMakefile* /*mf*/)
{
  // The GHS generator only knows how to lookup its build tool
  // during generation of the project files, but this
  // can only be done after the toolset is specified.

  return true;
}

void cmGlobalGhsMultiGenerator::GetToolset(cmMakefile* mf, std::string& tsd,
                                           const std::string& ts)
{
  const char* ghsRoot = mf->GetDefinition("GHS_TOOLSET_ROOT");

  if (!ghsRoot || ghsRoot[0] == '\0') {
    ghsRoot = DEFAULT_TOOLSET_ROOT;
  }
  tsd = ghsRoot;

  if (ts.empty()) {
    std::vector<std::string> output;

    // Use latest? version
    if (tsd.back() != '/') {
      tsd += "/";
    }
    cmSystemTools::Glob(tsd, "comp_[^;]+", output);

    if (output.empty()) {
      std::string msg =
        "No GHS toolsets found in GHS_TOOLSET_ROOT \"" + tsd + "\".";
      cmSystemTools::Error(msg);
      tsd = "";
    } else {
      tsd += output.back();
    }
  } else {
    std::string tryPath;
    tryPath = cmSystemTools::CollapseFullPath(ts, tsd);
    if (!cmSystemTools::FileExists(tryPath)) {
      std::string msg = "GHS toolset \"" + tryPath + "\" not found.";
      cmSystemTools::Error(msg);
      tsd = "";
    } else {
      tsd = tryPath;
    }
  }
}

void cmGlobalGhsMultiGenerator::WriteFileHeader(std::ostream& fout)
{
  fout << "#!gbuild" << std::endl;
  fout << "#" << std::endl
       << "# CMAKE generated file: DO NOT EDIT!" << std::endl
       << "# Generated by \"" << GetActualName() << "\""
       << " Generator, CMake Version " << cmVersion::GetMajorVersion() << "."
       << cmVersion::GetMinorVersion() << std::endl
       << "#" << std::endl
       << std::endl;
}

void cmGlobalGhsMultiGenerator::WriteCustomRuleBOD(std::ostream& fout)
{
  fout << "Commands {\n"
          "  Custom_Rule_Command {\n"
          "    name = \"Custom Rule Command\"\n"
          "    exec = \"";
#ifdef _WIN32
  fout << "cmd.exe";
#else
  fout << "/bin/sh";
#endif
  fout << "\"\n"
          "    options = {\"SpecialOptions\"}\n"
          "  }\n"
          "}\n";

  fout << "\n\n";
  fout << "FileTypes {\n"
          "  CmakeRule {\n"
          "    name = \"Custom Rule\"\n"
          "    action = \"&Run\"\n"
          "    extensions = {\"";
#ifdef _WIN32
  fout << "bat";
#else
  fout << "sh";
#endif
  fout << "\"}\n"
          "    grepable = false\n"
          "    command = \"Custom Rule Command\"\n"
          "    commandLine = \"$COMMAND ";
#ifdef _WIN32
  fout << "/c";
#endif
  fout << " $INPUTFILE\"\n"
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

  fout << "# Top Level Project File" << std::endl;

  // Specify BSP option if supplied by user
  const char* bspName =
    this->GetCMakeInstance()->GetCacheDefinition("GHS_BSP_NAME");
  if (!cmIsOff(bspName)) {
    fout << "    -bsp " << bspName << std::endl;
  }

  // Specify OS DIR if supplied by user
  // -- not all platforms require this entry in the project file
  if (!cmIsOff(this->OsDir)) {
    const char* osDirOption =
      this->GetCMakeInstance()->GetCacheDefinition("GHS_OS_DIR_OPTION");
    std::replace(this->OsDir.begin(), this->OsDir.end(), '\\', '/');
    fout << "    ";
    if (cmIsOff(osDirOption)) {
      fout << "";
    } else {
      fout << osDirOption;
    }
    fout << "\"" << this->OsDir << "\"" << std::endl;
  }
}

void cmGlobalGhsMultiGenerator::WriteSubProjects(std::ostream& fout,
                                                 std::string& all_target)
{
  fout << "CMakeFiles/" << all_target << " [Project]" << std::endl;
  // All known targets
  for (cmGeneratorTarget const* target : this->ProjectTargets) {
    if (target->GetType() == cmStateEnums::INTERFACE_LIBRARY ||
        target->GetType() == cmStateEnums::MODULE_LIBRARY ||
        target->GetType() == cmStateEnums::SHARED_LIBRARY ||
        (target->GetType() == cmStateEnums::GLOBAL_TARGET &&
         target->GetName() != GetInstallTargetName())) {
      continue;
    }
    fout << "CMakeFiles/" << target->GetName() + ".tgt" + FILE_EXTENSION
         << " [Project]" << std::endl;
  }
}

void cmGlobalGhsMultiGenerator::WriteProjectLine(
  std::ostream& fout, cmGeneratorTarget const* target, cmLocalGenerator* root,
  std::string& rootBinaryDir)
{
  const char* projName = target->GetProperty("GENERATOR_FILE_NAME");
  const char* projType = target->GetProperty("GENERATOR_FILE_NAME_EXT");
  if (projName && projType) {
    cmLocalGenerator* lg = target->GetLocalGenerator();
    std::string dir = lg->GetCurrentBinaryDirectory();
    dir = root->MaybeConvertToRelativePath(rootBinaryDir, dir);
    if (dir == ".") {
      dir.clear();
    } else {
      if (dir.back() != '/') {
        dir += "/";
      }
    }

    std::string projFile = dir + projName + FILE_EXTENSION;
    fout << projFile;
    fout << " " << projType << std::endl;
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
         target->GetName() != GetInstallTargetName())) {
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
    if (ComputeTargetBuildOrder(target, build)) {
      cmSystemTools::Error(
        cmStrCat("The inter-target dependency graph for target [",
                 target->GetName(), "] had a cycle.\n"));
    } else {
      for (auto& tgt : build) {
        WriteProjectLine(fbld, tgt, root, rootBinaryDir);
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
    if (t->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
      continue;
    }
    if (!cmIsOn(t->GetProperty("EXCLUDE_FROM_ALL"))) {
      defaultTargets.push_back(t);
    }
  }
  std::vector<cmGeneratorTarget const*> build;
  if (ComputeTargetBuildOrder(defaultTargets, build)) {
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
      this->WriteProjectLine(fbld, target, root, rootBinaryDir);
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
  const std::string& /*config*/, bool /*fast*/, int jobs, bool /*verbose*/,
  std::vector<std::string> const& makeOptions)
{
  GeneratedMakeCommand makeCommand = {};
  std::string gbuild;
  if (const char* gbuildCached =
        this->CMakeInstance->GetCacheDefinition("CMAKE_MAKE_PROGRAM")) {
    gbuild = gbuildCached;
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
    if (!cmContains(files, proj)) {
      proj = files.at(0);
    }
  }

  makeCommand.Add("-top", proj);
  if (!targetNames.empty()) {
    if (cmContains(targetNames, "clean")) {
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
  fout << "macro PROJ_NAME=" << root->GetProjectName() << std::endl;
  char const* ghsGpjMacros =
    this->GetCMakeInstance()->GetCacheDefinition("GHS_GPJ_MACROS");
  if (nullptr != ghsGpjMacros) {
    std::vector<std::string> expandedList =
      cmExpandedList(std::string(ghsGpjMacros));
    for (std::string const& arg : expandedList) {
      fout << "macro " << arg << std::endl;
    }
  }
}

void cmGlobalGhsMultiGenerator::WriteHighLevelDirectives(
  cmLocalGenerator* root, std::ostream& fout)
{
  /* set primary target */
  std::string tgt;
  const char* t =
    this->GetCMakeInstance()->GetCacheDefinition("GHS_PRIMARY_TARGET");
  if (t && *t != '\0') {
    tgt = t;
    this->GetCMakeInstance()->MarkCliAsUsed("GHS_PRIMARY_TARGET");
  } else {
    const char* a =
      this->GetCMakeInstance()->GetCacheDefinition("CMAKE_GENERATOR_PLATFORM");
    const char* p =
      this->GetCMakeInstance()->GetCacheDefinition("GHS_TARGET_PLATFORM");
    tgt = cmStrCat((a ? a : ""), '_', (p ? p : ""), ".tgt");
  }

  fout << "primaryTarget=" << tgt << std::endl;
  fout << "customization=" << root->GetBinaryDirectory()
       << "/CMakeFiles/custom_rule.bod" << std::endl;
  fout << "customization=" << root->GetBinaryDirectory()
       << "/CMakeFiles/custom_target.bod" << std::endl;

  char const* const customization =
    this->GetCMakeInstance()->GetCacheDefinition("GHS_CUSTOMIZATION");
  if (nullptr != customization && strlen(customization) > 0) {
    fout << "customization=" << this->TrimQuotes(customization) << std::endl;
    this->GetCMakeInstance()->MarkCliAsUsed("GHS_CUSTOMIZATION");
  }
}

std::string cmGlobalGhsMultiGenerator::TrimQuotes(std::string const& str)
{
  std::string result;
  result.reserve(str.size());
  for (const char* ch = str.c_str(); *ch != '\0'; ++ch) {
    if (*ch != '"') {
      result += *ch;
    }
  }
  return result;
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
  return ComputeTargetBuildOrder(t, build);
}

bool cmGlobalGhsMultiGenerator::ComputeTargetBuildOrder(
  std::vector<cmGeneratorTarget const*>& tgt,
  std::vector<cmGeneratorTarget const*>& build)
{
  std::set<cmGeneratorTarget const*> temp;
  std::set<cmGeneratorTarget const*> perm;

  for (auto ti : tgt) {
    bool r = VisitTarget(temp, perm, build, ti);
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
      for (auto& di : sortedTargets) {
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
