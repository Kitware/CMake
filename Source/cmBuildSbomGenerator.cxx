/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmBuildSbomGenerator.h"

#include "cmGeneratedFileStream.h"
#include "cmStringAlgorithms.h"

void cmBuildSbomGenerator::Compute(cmLocalGenerator* lg)
{
  this->Builder->Compute(lg);
}

bool cmBuildSbomGenerator::GenerateForBuild(std::string const& config)
{
  cmGeneratedFileStream os(
    cmStrCat(this->OutputFile, "-", config, ".spdx.json"));
  return this->Builder->Generate(os, config);
}
