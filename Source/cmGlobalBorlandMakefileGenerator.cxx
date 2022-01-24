/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalBorlandMakefileGenerator.h"

#include <ostream>
#include <utility>

#include <cm/memory>

#include "cmDocumentationEntry.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmake.h"

cmGlobalBorlandMakefileGenerator::cmGlobalBorlandMakefileGenerator(cmake* cm)
  : cmGlobalUnixMakefileGenerator3(cm)
{
  this->EmptyRuleHackDepends = "NUL";
  this->FindMakeProgramFile = "CMakeBorlandFindMake.cmake";
  this->ForceUnixPaths = false;
  this->ToolSupportsColor = true;
  this->UseLinkScript = false;
  cm->GetState()->SetWindowsShell(true);
  this->IncludeDirective = "!include";
  this->DefineWindowsNULL = true;
  this->PassMakeflags = true;
  this->UnixCD = false;

  /*
   * Borland Make does not support long line depend rule, as we have tested
   * generate one source file includes 40000 header files, and generate
   * depend.make in one line(use line continued tag), and error occurred:
   * ** Fatal CMakeFiles\main.dir\depend.make 1224: Rule line too long **
   * we disable long line dependencies rule generation for Borland make
   */
  this->ToolSupportsLongLineDependencies = false;
}

void cmGlobalBorlandMakefileGenerator::EnableLanguage(
  std::vector<std::string> const& l, cmMakefile* mf, bool optional)
{
  std::string outdir = this->CMakeInstance->GetHomeOutputDirectory();
  mf->AddDefinition("BORLAND", "1");
  mf->AddDefinition("CMAKE_GENERATOR_CC", "bcc32");
  mf->AddDefinition("CMAKE_GENERATOR_CXX", "bcc32");
  this->cmGlobalUnixMakefileGenerator3::EnableLanguage(l, mf, optional);
}

//! Create a local generator appropriate to this Global Generator
std::unique_ptr<cmLocalGenerator>
cmGlobalBorlandMakefileGenerator::CreateLocalGenerator(cmMakefile* mf)
{
  auto lg = cm::make_unique<cmLocalUnixMakefileGenerator3>(this, mf);
  lg->SetMakefileVariableSize(32);
  lg->SetMakeCommandEscapeTargetTwice(true);
  lg->SetBorlandMakeCurlyHack(true);
  return std::unique_ptr<cmLocalGenerator>(std::move(lg));
}

void cmGlobalBorlandMakefileGenerator::GetDocumentation(
  cmDocumentationEntry& entry)
{
  entry.Name = cmGlobalBorlandMakefileGenerator::GetActualName();
  entry.Brief = "Generates Borland makefiles.";
}

std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalBorlandMakefileGenerator::GenerateBuildCommand(
  const std::string& makeProgram, const std::string& projectName,
  const std::string& projectDir, std::vector<std::string> const& targetNames,
  const std::string& config, int /*jobs*/, bool verbose,
  const cmBuildOptions& buildOptions,
  std::vector<std::string> const& makeOptions)
{
  return this->cmGlobalUnixMakefileGenerator3::GenerateBuildCommand(
    makeProgram, projectName, projectDir, targetNames, config,
    cmake::NO_BUILD_PARALLEL_LEVEL, verbose, buildOptions, makeOptions);
}

void cmGlobalBorlandMakefileGenerator::PrintBuildCommandAdvice(
  std::ostream& os, int jobs) const
{
  if (jobs != cmake::NO_BUILD_PARALLEL_LEVEL) {
    // Borland's make does not support parallel builds
    // see http://docwiki.embarcadero.com/RADStudio/Tokyo/en/Make

    /* clang-format off */
    os <<
      "Warning: Borland's make does not support parallel builds. "
      "Ignoring parallel build command line option.\n";
    /* clang-format on */
  }

  this->cmGlobalUnixMakefileGenerator3::PrintBuildCommandAdvice(
    os, cmake::NO_BUILD_PARALLEL_LEVEL);
}
