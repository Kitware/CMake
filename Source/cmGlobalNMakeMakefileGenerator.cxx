/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalNMakeMakefileGenerator.h"

#include <ostream>

#include <cmext/algorithm>

#include "cmsys/RegularExpression.hxx"

#include "cmDuration.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

cmGlobalNMakeMakefileGenerator::cmGlobalNMakeMakefileGenerator(cmake* cm)
  : cmGlobalUnixMakefileGenerator3(cm)
{
  this->FindMakeProgramFile = "CMakeNMakeFindMake.cmake";
  this->ForceUnixPaths = false;
  this->ToolSupportsColor = true;
  this->UseLinkScript = false;
  cm->GetState()->SetWindowsShell(true);
  cm->GetState()->SetNMake(true);
  this->DefineWindowsNULL = true;
  this->PassMakeflags = true;
  this->UnixCD = false;
  this->MakeSilentFlag = "/nologo";
  // nmake breaks on '!' in long-line dependencies
  this->ToolSupportsLongLineDependencies = false;
}

void cmGlobalNMakeMakefileGenerator::EnableLanguage(
  std::vector<std::string> const& l, cmMakefile* mf, bool optional)
{
  // pick a default
  mf->AddDefinition("CMAKE_GENERATOR_CC", "cl");
  mf->AddDefinition("CMAKE_GENERATOR_CXX", "cl");
  this->cmGlobalUnixMakefileGenerator3::EnableLanguage(l, mf, optional);
}

bool cmGlobalNMakeMakefileGenerator::FindMakeProgram(cmMakefile* mf)
{
  if (!this->cmGlobalGenerator::FindMakeProgram(mf)) {
    return false;
  }
  if (cmValue nmakeCommand = mf->GetDefinition("CMAKE_MAKE_PROGRAM")) {
    std::vector<std::string> command{ *nmakeCommand, "-?" };
    std::string out;
    std::string err;
    if (!cmSystemTools::RunSingleCommand(command, &out, &err, nullptr, nullptr,
                                         cmSystemTools::OUTPUT_NONE,
                                         cmDuration(30))) {
      mf->IssueMessage(MessageType::FATAL_ERROR,
                       cmStrCat("Running\n '", cmJoin(command, "' '"),
                                "'\n"
                                "failed with:\n ",
                                err));
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }
    cmsys::RegularExpression regex(
      "Program Maintenance Utility Version ([1-9][0-9.]+)");
    if (regex.find(err)) {
      this->NMakeVersion = regex.match(1);
      this->CheckNMakeFeatures();
    }
  }
  return true;
}

void cmGlobalNMakeMakefileGenerator::CheckNMakeFeatures()
{
  this->NMakeSupportsUTF8 = !cmSystemTools::VersionCompare(
    cmSystemTools::OP_LESS, this->NMakeVersion, "9");
}

cmDocumentationEntry cmGlobalNMakeMakefileGenerator::GetDocumentation()
{
  return { cmGlobalNMakeMakefileGenerator::GetActualName(),
           "Generates NMake makefiles." };
}

void cmGlobalNMakeMakefileGenerator::PrintCompilerAdvice(
  std::ostream& os, std::string const& lang, cmValue envVar) const
{
  if (lang == "CXX" || lang == "C") {
    /* clang-format off */
    os <<
      "To use the NMake generator with Visual C++, cmake must be run from a "
      "shell that can use the compiler cl from the command line. This "
      "environment is unable to invoke the cl compiler. To fix this problem, "
      "run cmake from the Visual Studio Command Prompt (vcvarsall.bat).\n";
    /* clang-format on */
  }
  this->cmGlobalUnixMakefileGenerator3::PrintCompilerAdvice(os, lang, envVar);
}

std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalNMakeMakefileGenerator::GenerateBuildCommand(
  const std::string& makeProgram, const std::string& projectName,
  const std::string& projectDir, std::vector<std::string> const& targetNames,
  const std::string& config, int /*jobs*/, bool verbose,
  const cmBuildOptions& buildOptions,
  std::vector<std::string> const& makeOptions)
{
  std::vector<std::string> nmakeMakeOptions;

  // Since we have full control over the invocation of nmake, let us
  // make it quiet.
  nmakeMakeOptions.push_back(this->MakeSilentFlag);
  cm::append(nmakeMakeOptions, makeOptions);

  return this->cmGlobalUnixMakefileGenerator3::GenerateBuildCommand(
    makeProgram, projectName, projectDir, targetNames, config,
    cmake::NO_BUILD_PARALLEL_LEVEL, verbose, buildOptions, nmakeMakeOptions);
}

void cmGlobalNMakeMakefileGenerator::PrintBuildCommandAdvice(std::ostream& os,
                                                             int jobs) const
{
  if (jobs != cmake::NO_BUILD_PARALLEL_LEVEL) {
    // nmake does not support parallel build level
    // see https://msdn.microsoft.com/en-us/library/afyyse50.aspx
    os << "Warning: NMake does not support parallel builds. "
          "Ignoring parallel build command line option.\n";
  }

  this->cmGlobalUnixMakefileGenerator3::PrintBuildCommandAdvice(
    os, cmake::NO_BUILD_PARALLEL_LEVEL);
}
