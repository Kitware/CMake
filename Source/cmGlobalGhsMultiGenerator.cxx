/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalGhsMultiGenerator.h"

#include "cmDocumentationEntry.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGhsMultiGpj.h"
#include "cmLocalGenerator.h"
#include "cmLocalGhsMultiGenerator.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"
#include "cmVersion.h"
#include "cmake.h"

#include <algorithm>
#include <map>
#include <ostream>
#include <string.h>
#include <utility>

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
  }
  if (ts.empty()) {
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
  if (prevTool != nullptr && (gbuild != prevTool)) {
    std::string message = "toolset build tool: ";
    message += gbuild;
    message += "\nDoes not match the previously used build tool: ";
    message += prevTool;
    message += "\nEither remove the CMakeCache.txt file and CMakeFiles "
               "directory or choose a different binary directory.";
    cmSystemTools::Error(message);
    return false;
  }

  /* store the toolset that is being used for this build */
  mf->AddCacheDefinition("CMAKE_MAKE_PROGRAM", gbuild.c_str(),
                         "build program to use", cmStateEnums::INTERNAL, true);

  mf->AddDefinition("CMAKE_SYSTEM_VERSION", tsp.c_str());

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
  if (cmSystemTools::IsOff(osdir.c_str()) &&
      platform.find("integrity") != std::string::npos) {
    if (!this->CMakeInstance->GetIsInTryCompile()) {
      /* required OS location is not found */
      std::string m =
        "Green Hills MULTI: GHS_OS_DIR not specified; No OS found in \"";
      m += mf->GetSafeDefinition("GHS_OS_ROOT");
      m += "\"";
      cmSystemTools::Message(m);
    }
    osdir = "GHS_OS_DIR-NOT-SPECIFIED";
  } else if (!this->CMakeInstance->GetIsInTryCompile() &&
             cmSystemTools::IsOff(this->OsDir) &&
             !cmSystemTools::IsOff(osdir)) {
    /* OS location was updated by auto-selection */
    std::string m = "Green Hills MULTI: GHS_OS_DIR not specified; found \"";
    m += osdir;
    m += "\"";
    cmSystemTools::Message(m);
  }
  this->OsDir = osdir;

  // Determine GHS_BSP_NAME
  std::string bspName = mf->GetSafeDefinition("GHS_BSP_NAME");

  if (cmSystemTools::IsOff(bspName.c_str()) &&
      platform.find("integrity") != std::string::npos) {
    bspName = "sim" + arch;
    /* write back the calculate name for next time */
    mf->AddCacheDefinition("GHS_BSP_NAME", bspName.c_str(),
                           "Name of GHS target platform.",
                           cmStateEnums::STRING, true);
    std::string m =
      "Green Hills MULTI: GHS_BSP_NAME not specified; defaulting to \"";
    m += bspName;
    m += "\"";
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
  const char* bspName =
    this->GetCMakeInstance()->GetCacheDefinition("GHS_BSP_NAME");
  if (!cmSystemTools::IsOff(bspName)) {
    fout << "    -bsp " << bspName << std::endl;
  }

  // Specify OS DIR if supplied by user
  // -- not all platforms require this entry in the project file
  if (!cmSystemTools::IsOff(this->OsDir.c_str())) {
    const char* osDirOption =
      this->GetCMakeInstance()->GetCacheDefinition("GHS_OS_DIR_OPTION");
    std::replace(this->OsDir.begin(), this->OsDir.end(), '\\', '/');
    fout << "    ";
    if (cmSystemTools::IsOff(osDirOption)) {
      fout << "";
    } else {
      fout << osDirOption;
    }
    fout << "\"" << this->OsDir << "\"" << std::endl;
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
      dir = root->MaybeConvertToRelativePath(rootBinaryDir, dir);
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

        cmGeneratedFileStream fref(fname);
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

  cmGeneratedFileStream fout(fname);
  fout.SetCopyIfDifferent(true);

  this->WriteTopLevelProject(fout, root, generators);

  fout.Close();
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
    auto p = std::find(files.begin(), files.end(), proj);
    if (p == files.end()) {
      proj = files.at(0);
    }
  }

  makeCommand.Add("-top", proj);
  if (!targetNames.empty()) {
    if (std::find(targetNames.begin(), targetNames.end(), "clean") !=
        targetNames.end()) {
      makeCommand.Add("-clean");
    } else {
      for (const auto& tname : targetNames) {
        if (!tname.empty()) {
          if (tname.compare(tname.size() - 4, 4, ".gpj") == 0) {
            makeCommand.Add(tname);
          } else {
            makeCommand.Add(tname + ".gpj");
          }
        }
      }
    }
  }
  return { makeCommand };
}

void cmGlobalGhsMultiGenerator::WriteMacros(std::ostream& fout)
{
  char const* ghsGpjMacros =
    this->GetCMakeInstance()->GetCacheDefinition("GHS_GPJ_MACROS");
  if (nullptr != ghsGpjMacros) {
    std::vector<std::string> expandedList;
    cmSystemTools::ExpandListArgument(std::string(ghsGpjMacros), expandedList);
    for (std::string const& arg : expandedList) {
      fout << "macro " << arg << std::endl;
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
  if (nullptr != customization && strlen(customization) > 0) {
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
