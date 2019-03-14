/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalGhsMultiGenerator.h"

#include "cmsys/SystemTools.hxx"

#include "cmAlgorithms.h"
#include "cmDocumentationEntry.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGhsMultiTargetGenerator.h"
#include "cmLocalGhsMultiGenerator.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmVersion.h"
#include "cmake.h"

const char* cmGlobalGhsMultiGenerator::FILE_EXTENSION = ".gpj";
const char* cmGlobalGhsMultiGenerator::DEFAULT_BUILD_PROGRAM = "gbuild.exe";
const char* cmGlobalGhsMultiGenerator::DEFAULT_TOOLSET_ROOT = "C:/ghs";

cmGlobalGhsMultiGenerator::cmGlobalGhsMultiGenerator(cmake* cm)
  : cmGlobalGenerator(cm)
{
  cm->GetState()->SetGhsMultiIDE(true);
}

cmGlobalGhsMultiGenerator::~cmGlobalGhsMultiGenerator()
{
}

cmLocalGenerator* cmGlobalGhsMultiGenerator::CreateLocalGenerator(
  cmMakefile* mf)
{
  return new cmLocalGhsMultiGenerator(this, mf);
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
  std::string dir;
  dir += gt->LocalGenerator->GetCurrentBinaryDirectory();
  dir += "/";
  dir += gt->LocalGenerator->GetTargetDirectory(gt);
  dir += "/";
  gt->ObjectDirectory = dir;
}

bool cmGlobalGhsMultiGenerator::SetGeneratorToolset(std::string const& ts,
                                                    cmMakefile* mf)
{
  std::string tsp; /* toolset path */

  this->GetToolset(mf, tsp, ts);

  /* no toolset was found */
  if (tsp.empty()) {
    return false;
  } else if (ts.empty()) {
    std::string message;
    message =
      "Green Hills MULTI: -T <toolset> not specified; defaulting to \"";
    message += tsp;
    message += "\"";
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
  if (prevTool != NULL && (gbuild != prevTool)) {
    std::string message = "toolset build tool: ";
    message += gbuild;
    message += "\nDoes not match the previously used build tool: ";
    message += prevTool;
    message += "\nEither remove the CMakeCache.txt file and CMakeFiles "
               "directory or choose a different binary directory.";
    cmSystemTools::Error(message);
    return false;
  } else {
    /* store the toolset that is being used for this build */
    mf->AddCacheDefinition("CMAKE_MAKE_PROGRAM", gbuild.c_str(),
                           "build program to use", cmStateEnums::INTERNAL,
                           true);
  }

  mf->AddDefinition("CMAKE_SYSTEM_VERSION", tsp.c_str());

  return true;
}

bool cmGlobalGhsMultiGenerator::SetGeneratorPlatform(std::string const& p,
                                                     cmMakefile* mf)
{
  if (p == "") {
    cmSystemTools::Message(
      "Green Hills MULTI: -A <arch> not specified; defaulting to \"arm\"");
    std::string arch = "arm";

    /* store the platform name for later use
     * -- already done if -A<arch> was specified
     */
    mf->AddCacheDefinition("CMAKE_GENERATOR_PLATFORM", arch.c_str(),
                           "Name of generator platform.",
                           cmStateEnums::INTERNAL);
  }

  const char* tgtPlatform = mf->GetDefinition("GHS_TARGET_PLATFORM");
  if (tgtPlatform == nullptr) {
    cmSystemTools::Message("Green Hills MULTI: GHS_TARGET_PLATFORM not "
                           "specified; defaulting to \"integrity\"");
    tgtPlatform = "integrity";
  }

  /* store the platform name for later use */
  mf->AddCacheDefinition("GHS_TARGET_PLATFORM", tgtPlatform,
                         "Name of GHS target platform.",
                         cmStateEnums::INTERNAL);

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
    /* CollapseCombinedPath will check if ts is an absolute path */
    tryPath = cmSystemTools::CollapseCombinedPath(tsd, ts);
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
       << "# Generated by \"" << this->GetActualName() << "\""
       << " Generator, CMake Version " << cmVersion::GetMajorVersion() << "."
       << cmVersion::GetMinorVersion() << std::endl
       << "#" << std::endl
       << std::endl;
}

void cmGlobalGhsMultiGenerator::WriteTopLevelProject(
  std::ostream& fout, cmLocalGenerator* root,
  std::vector<cmLocalGenerator*>& generators)
{
  WriteFileHeader(fout);

  this->WriteMacros(fout);
  this->WriteHighLevelDirectives(fout);
  GhsMultiGpj::WriteGpjTag(GhsMultiGpj::PROJECT, fout);

  fout << "# Top Level Project File" << std::endl;

  // Specify BSP option if supplied by user
  // -- not all platforms require this entry in the project file
  //    integrity platforms require this field; use default if needed
  std::string platform;
  if (const char* p =
        this->GetCMakeInstance()->GetCacheDefinition("GHS_TARGET_PLATFORM")) {
    platform = p;
  }

  std::string bspName;
  if (char const* bspCache =
        this->GetCMakeInstance()->GetCacheDefinition("GHS_BSP_NAME")) {
    bspName = bspCache;
    this->GetCMakeInstance()->MarkCliAsUsed("GHS_BSP_NAME");
  } else {
    bspName = "IGNORE";
  }

  if (platform.find("integrity") != std::string::npos &&
      cmSystemTools::IsOff(bspName.c_str())) {
    const char* a =
      this->GetCMakeInstance()->GetCacheDefinition("CMAKE_GENERATOR_PLATFORM");
    bspName = "sim";
    bspName += (a ? a : "");
  }

  if (!cmSystemTools::IsOff(bspName.c_str())) {
    fout << "    -bsp " << bspName << std::endl;
  }

  // Specify OS DIR if supplied by user
  // -- not all platforms require this entry in the project file
  std::string osDir;
  std::string osDirOption;
  if (char const* osDirCache =
        this->GetCMakeInstance()->GetCacheDefinition("GHS_OS_DIR")) {
    osDir = osDirCache;
  }

  if (char const* osDirOptionCache =
        this->GetCMakeInstance()->GetCacheDefinition("GHS_OS_DIR_OPTION")) {
    osDirOption = osDirOptionCache;
  }

  if (!cmSystemTools::IsOff(osDir.c_str()) ||
      platform.find("integrity") != std::string::npos) {
    std::replace(osDir.begin(), osDir.end(), '\\', '/');
    fout << "    " << osDirOption << "\"" << osDir << "\"" << std::endl;
  }

  WriteSubProjects(fout, root, generators);
}

void cmGlobalGhsMultiGenerator::WriteSubProjects(
  std::ostream& fout, cmLocalGenerator* root,
  std::vector<cmLocalGenerator*>& generators)
{
  // Collect all targets under this root generator and the transitive
  // closure of their dependencies.
  TargetDependSet projectTargets;
  TargetDependSet originalTargets;
  this->GetTargetSets(projectTargets, originalTargets, root, generators);
  OrderedTargetDependSet orderedProjectTargets(projectTargets, "");

  // write out all the sub-projects
  std::string rootBinaryDir = root->GetCurrentBinaryDirectory();
  for (cmGeneratorTarget const* target : orderedProjectTargets) {
    if (target->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
      continue;
    }

    const char* projName = target->GetProperty("GENERATOR_FILE_NAME");
    const char* projType = target->GetProperty("GENERATOR_FILE_NAME_EXT");
    if (projName && projType) {
      cmLocalGenerator* lg = target->GetLocalGenerator();
      std::string dir = lg->GetCurrentBinaryDirectory();
      dir = root->MaybeConvertToRelativePath(rootBinaryDir, dir.c_str());
      if (dir == ".") {
        dir.clear();
      } else {
        if (dir.back() != '/') {
          dir += "/";
        }
      }

      if (cmSystemTools::IsOn(target->GetProperty("EXCLUDE_FROM_ALL"))) {
        fout << "{comment} ";
      }
      std::string projFile = dir + projName + FILE_EXTENSION;
      fout << projFile;
      fout << " " << projType << std::endl;

      if (cmSystemTools::IsOn(target->GetProperty("GHS_REFERENCE_PROJECT"))) {
        // create reference project
        std::string fname = dir;
        fname += target->GetName();
        fname += "REF";
        fname += FILE_EXTENSION;

        cmGeneratedFileStream fref(fname.c_str());
        fref.SetCopyIfDifferent(true);

        this->WriteFileHeader(fref);
        GhsMultiGpj::WriteGpjTag(GhsMultiGpj::REFERENCE, fref);
        fref << "    :reference=" << projFile << std::endl;

        fref.Close();
      }
    }
  }
}

void cmGlobalGhsMultiGenerator::Generate()
{
  // first do the superclass method
  this->cmGlobalGenerator::Generate();

  // output top-level projects
  for (auto& it : this->ProjectMap) {
    this->OutputTopLevelProject(it.second[0], it.second);
  }
}

void cmGlobalGhsMultiGenerator::OutputTopLevelProject(
  cmLocalGenerator* root, std::vector<cmLocalGenerator*>& generators)
{
  if (generators.empty()) {
    return;
  }

  /* Name top-level projects as filename.top.gpj to avoid name clashes
   * with target projects.  This avoid the issue where the project has
   * the same name as the executable target.
   */
  std::string fname = root->GetCurrentBinaryDirectory();
  fname += "/";
  fname += root->GetProjectName();
  fname += ".top";
  fname += FILE_EXTENSION;

  cmGeneratedFileStream fout(fname.c_str());
  fout.SetCopyIfDifferent(true);

  this->WriteTopLevelProject(fout, root, generators);

  fout.Close();
}

void cmGlobalGhsMultiGenerator::GenerateBuildCommand(
  GeneratedMakeCommand& makeCommand, const std::string& makeProgram,
  const std::string& projectName, const std::string& projectDir,
  const std::string& targetName, const std::string& /*config*/, bool /*fast*/,
  int jobs, bool /*verbose*/, std::vector<std::string> const& makeOptions)
{
  const char* gbuild =
    this->CMakeInstance->GetCacheDefinition("CMAKE_MAKE_PROGRAM");
  makeCommand.add(this->SelectMakeProgram(makeProgram, (std::string)gbuild));

  if (jobs != cmake::NO_BUILD_PARALLEL_LEVEL) {
    makeCommand.add("-parallel");
    if (jobs != cmake::DEFAULT_BUILD_PARALLEL_LEVEL) {
      makeCommand.add(std::to_string(jobs));
    }
  }

  makeCommand.add(makeOptions.begin(), makeOptions.end());

  /* determine which top-project file to use */
  std::string proj = projectName + ".top" + FILE_EXTENSION;
  std::vector<std::string> files;
  cmSystemTools::Glob(projectDir, ".*\\.top\\.gpj", files);
  if (!files.empty()) {
    auto p = std::find(files.begin(), files.end(), proj);
    if (p == files.end()) {
      proj = files.at(0);
    }
  }

  makeCommand.add("-top", proj);
  if (!targetName.empty()) {
    if (targetName == "clean") {
      makeCommand.add("-clean");
    } else {
      if (targetName.compare(targetName.size() - 4, 4, ".gpj") == 0) {
        makeCommand.add(targetName);
      } else {
        makeCommand.add(targetName + ".gpj");
      }
    }
  }
}

void cmGlobalGhsMultiGenerator::WriteMacros(std::ostream& fout)
{
  char const* ghsGpjMacros =
    this->GetCMakeInstance()->GetCacheDefinition("GHS_GPJ_MACROS");
  if (NULL != ghsGpjMacros) {
    std::vector<std::string> expandedList;
    cmSystemTools::ExpandListArgument(std::string(ghsGpjMacros), expandedList);
    for (std::vector<std::string>::const_iterator expandedListI =
           expandedList.begin();
         expandedListI != expandedList.end(); ++expandedListI) {
      fout << "macro " << *expandedListI << std::endl;
    }
  }
}

void cmGlobalGhsMultiGenerator::WriteHighLevelDirectives(std::ostream& fout)
{
  /* set primary target */
  std::string tgt;
  const char* t =
    this->GetCMakeInstance()->GetCacheDefinition("GHS_PRIMARY_TARGET");
  if (t) {
    tgt = t;
    this->GetCMakeInstance()->MarkCliAsUsed("GHS_PRIMARY_TARGET");
  } else {
    const char* a =
      this->GetCMakeInstance()->GetCacheDefinition("CMAKE_GENERATOR_PLATFORM");
    const char* p =
      this->GetCMakeInstance()->GetCacheDefinition("GHS_TARGET_PLATFORM");
    tgt = (a ? a : "");
    tgt += "_";
    tgt += (p ? p : "");
    tgt += ".tgt";
  }

  fout << "primaryTarget=" << tgt << std::endl;

  char const* const customization =
    this->GetCMakeInstance()->GetCacheDefinition("GHS_CUSTOMIZATION");
  if (NULL != customization && strlen(customization) > 0) {
    fout << "customization=" << trimQuotes(customization) << std::endl;
    this->GetCMakeInstance()->MarkCliAsUsed("GHS_CUSTOMIZATION");
  }
}

std::string cmGlobalGhsMultiGenerator::trimQuotes(std::string const& str)
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
