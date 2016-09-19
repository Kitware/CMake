/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExportBuildAndroidMKGenerator_h
#define cmExportBuildAndroidMKGenerator_h

#include "cmExportBuildFileGenerator.h"
#include "cmListFileCache.h"

class cmExportSet;

/** \class cmExportBuildAndroidMKGenerator
 * \brief Generate a file exporting targets from a build tree.
 *
 * cmExportBuildAndroidMKGenerator generates a file exporting targets from
 * a build tree.  This exports the targets to the Android ndk build tool
 * makefile format for prebuilt libraries.
 *
 * This is used to implement the EXPORT() command.
 */
class cmExportBuildAndroidMKGenerator : public cmExportBuildFileGenerator
{
public:
  cmExportBuildAndroidMKGenerator();
  // this is so cmExportInstallAndroidMKGenerator can share this
  // function as they are almost the same
  enum GenerateType
  {
    BUILD,
    INSTALL
  };
  static void GenerateInterfaceProperties(cmGeneratorTarget const* target,
                                          std::ostream& os,
                                          const ImportPropertyMap& properties,
                                          GenerateType type,
                                          std::string const& config);

protected:
  // Implement virtual methods from the superclass.
  void GeneratePolicyHeaderCode(std::ostream&) CM_OVERRIDE {}
  void GeneratePolicyFooterCode(std::ostream&) CM_OVERRIDE {}
  void GenerateImportHeaderCode(std::ostream& os,
                                const std::string& config = "") CM_OVERRIDE;
  void GenerateImportFooterCode(std::ostream& os) CM_OVERRIDE;
  void GenerateImportTargetCode(std::ostream& os,
                                const cmGeneratorTarget* target) CM_OVERRIDE;
  void GenerateExpectedTargetsCode(
    std::ostream& os, const std::string& expectedTargets) CM_OVERRIDE;
  void GenerateImportPropertyCode(std::ostream& os, const std::string& config,
                                  cmGeneratorTarget const* target,
                                  ImportPropertyMap const& properties)
    CM_OVERRIDE;
  void GenerateMissingTargetsCheckCode(
    std::ostream& os,
    const std::vector<std::string>& missingTargets) CM_OVERRIDE;
  void GenerateInterfaceProperties(
    cmGeneratorTarget const* target, std::ostream& os,
    const ImportPropertyMap& properties) CM_OVERRIDE;
};

#endif
