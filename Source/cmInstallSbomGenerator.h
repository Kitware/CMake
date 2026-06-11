/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cmInstallGenerator.h"
#include "cmSbomArguments.h"

class cmDiagnosticContext;
class cmExportSet;
class cmGeneratorTarget;
class cmInstallSbomBuilder;
class cmLocalGenerator;

/** \class cmInstallSbomGenerator
 * \brief Generate installation rules for SBOM files.
 *
 * Thin cmInstallGenerator subclass that owns a cmInstallSbomBuilder.
 * At generate time it writes the SBOM into a temporary file in the build tree;
 * the cmake_install.cmake script then copies that file to the install
 * destination.
 */
class cmInstallSbomGenerator : public cmInstallGenerator
{
public:
  cmInstallSbomGenerator(std::vector<cmExportSet*> exportSets,
                         std::string destination, std::string filePermissions,
                         std::vector<std::string> const& configurations,
                         std::string component, MessageLevel message,
                         bool excludeFromAll, cmSbomArguments args,
                         cmDiagnosticContext context);
  cmInstallSbomGenerator(cmInstallSbomGenerator const&) = delete;
  ~cmInstallSbomGenerator() override;

  cmInstallSbomGenerator& operator=(cmInstallSbomGenerator const&) = delete;

  bool Compute(cmLocalGenerator* lg) override;

  std::string const& GetInstallFile() const { return this->SbomFilePath; }

  /** True if this SBOM directly describes `target`.  Used by peer SBOMs to
   *  attribute cross-references when install(export) provenance is absent. */
  bool CoversTarget(cmGeneratorTarget const* target) const;
  std::string const& GetPackageName() const;

  /** True if `set` is one of the export sets this SBOM was built from.
   *  Used by the autogen path to skip sets already explicitly tied to an
   *  install(SBOM). */
  bool CoversExportSet(cmExportSet const* set) const;

protected:
  void GenerateScript(std::ostream& os) override;
  void GenerateScriptActions(std::ostream& os, Indent indent) override;

private:
  std::map<std::string, std::string> TempSbomFiles;
  std::string const FilePermissions;
  std::string const SbomFileName;
  std::string const SbomFilePath;
  cmSbomArguments::SbomFormat SbomFormat;
  cmLocalGenerator* LocalGenerator = nullptr;
  std::unique_ptr<cmInstallSbomBuilder> Builder;
};
