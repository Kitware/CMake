/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExportTryCompileFileGenerator_h
#define cmExportTryCompileFileGenerator_h

#include "cmExportFileGenerator.h"

class cmInstallExportGenerator;
class cmInstallTargetGenerator;

class cmExportTryCompileFileGenerator: public cmExportFileGenerator
{
public:
  cmExportTryCompileFileGenerator(cmGlobalGenerator* gg,
                                  std::vector<std::string> const& targets,
                                  cmMakefile* mf);

  /** Set the list of targets to export.  */
  void SetConfig(const std::string& config) { this->Config = config; }
protected:

  // Implement virtual methods from the superclass.
  virtual bool GenerateMainFile(std::ostream& os);

  virtual void GenerateImportTargetsConfig(std::ostream&,
                                           const std::string&,
                                           std::string const&,
                            std::vector<std::string>&) {}
  virtual void HandleMissingTarget(std::string&,
                                   std::vector<std::string>&,
                                   cmGeneratorTarget*,
                                   cmGeneratorTarget*) {}

  void PopulateProperties(cmGeneratorTarget const* target,
                          ImportPropertyMap& properties,
                          std::set<const cmGeneratorTarget*>& emitted);

  std::string InstallNameDir(cmGeneratorTarget* target,
                             const std::string& config);
private:
  std::string FindTargets(const std::string& prop,
                          const cmGeneratorTarget* tgt,
                          std::set<const cmGeneratorTarget*>& emitted);


  std::vector<cmGeneratorTarget const*> Exports;
  std::string Config;
};

#endif
