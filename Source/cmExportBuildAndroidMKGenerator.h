/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <string>

#include "cmExportAndroidMKGenerator.h"
#include "cmExportBuildFileGenerator.h"
#include "cmStateTypes.h"

class cmGeneratorTarget;

/** \class cmExportBuildAndroidMKGenerator
 * \brief Generate a file exporting targets from a build tree.
 *
 * cmExportBuildAndroidMKGenerator generates a file exporting targets from
 * a build tree.  This exports the targets to the Android ndk build tool
 * makefile format for prebuilt libraries.
 *
 * This is used to implement the export() command.
 */
class cmExportBuildAndroidMKGenerator
  : public cmExportBuildFileGenerator
  , public cmExportAndroidMKGenerator
{
public:
  cmExportBuildAndroidMKGenerator();

  /** Set whether to append generated code to the output file.  */
  void SetAppendMode(bool append) { this->AppendMode = append; }

protected:
  GenerateType GetGenerateType() const override { return BUILD; }

  // Implement virtual methods from the superclass.
  bool GenerateMainFile(std::ostream& os) override;
  void GenerateImportHeaderCode(std::ostream& os,
                                std::string const& config = "") override;
  void GenerateImportTargetCode(
    std::ostream& os, cmGeneratorTarget const* target,
    cmStateEnums::TargetType /*targetType*/) override;

  std::string GetCxxModulesDirectory() const override { return {}; }
};
