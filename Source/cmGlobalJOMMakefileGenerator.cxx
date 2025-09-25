/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGlobalJOMMakefileGenerator.h"

#include <ostream>

#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmState.h"
#include "cmake.h"

cmGlobalJOMMakefileGenerator::cmGlobalJOMMakefileGenerator(cmake* cm)
  : cmGlobalUnixMakefileGenerator3(cm)
{
  this->FindMakeProgramFile = "CMakeJOMFindMake.cmake";
  this->ForceUnixPaths = false;
  this->ToolSupportsColor = true;
  this->UseLinkScript = false;
  cm->GetState()->SetWindowsShell(true);
  cm->GetState()->SetNMake(true);
  this->DefineWindowsNULL = true;
  this->PassMakeflags = true;
  this->UnixCD = false;
  this->MakeSilentFlag = "/nologo";
}

void cmGlobalJOMMakefileGenerator::EnableLanguage(
  std::vector<std::string> const& l, cmMakefile* mf, bool optional)
{
  // pick a default
  mf->AddDefinition("CMAKE_GENERATOR_CC", "cl");
  mf->AddDefinition("CMAKE_GENERATOR_CXX", "cl");
  this->cmGlobalUnixMakefileGenerator3::EnableLanguage(l, mf, optional);
}

cmDocumentationEntry cmGlobalJOMMakefileGenerator::GetDocumentation()
{
  return { cmGlobalJOMMakefileGenerator::GetActualName(),
           "Generates JOM makefiles." };
}

void cmGlobalJOMMakefileGenerator::PrintCompilerAdvice(std::ostream& os,
                                                       std::string const& lang,
                                                       cmValue envVar) const
{
  if (lang == "CXX"_s || lang == "C"_s) {
    /* clang-format off */
    os <<
      "To use the JOM generator with Visual C++, cmake must be run from a "
      "shell that can use the compiler cl from the command line. This "
      "environment is unable to invoke the cl compiler. To fix this problem, "
      "run cmake from the Visual Studio Command Prompt (vcvarsall.bat).\n";
    /* clang-format on */
  }
  this->cmGlobalUnixMakefileGenerator3::PrintCompilerAdvice(os, lang, envVar);
}

std::vector<cmGlobalGenerator::GeneratedMakeCommand>
cmGlobalJOMMakefileGenerator::GenerateBuildCommand(
  std::string const& makeProgram, std::string const& projectName,
  std::string const& projectDir, std::vector<std::string> const& targetNames,
  std::string const& config, int jobs, bool verbose,
  cmBuildOptions const& buildOptions,
  std::vector<std::string> const& makeOptions)
{
  std::vector<std::string> jomMakeOptions;

  // Since we have full control over the invocation of JOM, let us
  // make it quiet.
  jomMakeOptions.push_back(this->MakeSilentFlag);
  cm::append(jomMakeOptions, makeOptions);

  // JOM does parallel builds by default, the -j is only needed if a specific
  // number is given
  // see https://github.com/qt-labs/jom/blob/v1.1.2/src/jomlib/options.cpp
  if (jobs == cmake::DEFAULT_BUILD_PARALLEL_LEVEL) {
    jobs = cmake::NO_BUILD_PARALLEL_LEVEL;
  }

  return cmGlobalUnixMakefileGenerator3::GenerateBuildCommand(
    makeProgram, projectName, projectDir, targetNames, config, jobs, verbose,
    buildOptions, jomMakeOptions);
}
