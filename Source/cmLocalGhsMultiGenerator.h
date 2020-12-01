/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <map>
#include <string>

#include "cmLocalGenerator.h"

class cmGeneratorTarget;
class cmGlobalGenerator;
class cmMakefile;
class cmSourceFile;

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
    cmGeneratorTarget const* target) const override;

  void ComputeObjectFilenames(
    std::map<cmSourceFile const*, std::string>& mapping,
    cmGeneratorTarget const* gt = nullptr) override;
};
