/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

#include "cmLocalGenerator.h"

class cmGlobalGenerator;
class cmMakefile;

/** \class cmLocalGhsMultiGenerator
 * \brief Write Green Hills MULTI project files.
 *
 * cmLocalGhsMultiGenerator produces a set of .gpj
 * file for each target in its mirrored directory.
 */
class cmLocalGhsMultiGenerator : public cmLocalGenerator
{
public:
  cmLocalGhsMultiGenerator(cmGlobalGenerator* gg, cmMakefile* mf);

  ~cmLocalGhsMultiGenerator() override;

  /**
   * Generate the makefile for this directory.
   */
  void Generate() override;

  std::string GetTargetDirectory(
    cmGeneratorTarget const* target,
    cmStateEnums::IntermediateDirKind kind) const override;

  void ComputeObjectFilenames(
    std::map<cmSourceFile const*, cmObjectLocations>& mapping,
    std::string const& config, cmGeneratorTarget const* gt = nullptr) override;
};
