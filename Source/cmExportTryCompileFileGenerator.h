/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExportInstallFileGenerator_h
#define cmExportInstallFileGenerator_h

#include "cmExportFileGenerator.h"

class cmInstallExportGenerator;
class cmInstallTargetGenerator;

class cmExportTryCompileFileGenerator: public cmExportFileGenerator
{
public:
  /** Set the list of targets to export.  */
  void SetExports(const std::vector<cmTarget*> &exports)
    { this->Exports = exports; }
  void SetConfig(const char *config) { this->Config = config; }
protected:

  // Implement virtual methods from the superclass.
  virtual bool GenerateMainFile(std::ostream& os);

  virtual void GenerateImportTargetsConfig(std::ostream&,
                                           const char*,
                                           std::string const&,
                            std::vector<std::string>&) {}
  virtual void HandleMissingTarget(std::string&,
                                   std::vector<std::string>&,
                                   cmMakefile*,
                                   cmTarget*,
                                   cmTarget*) {}

  void PopulateProperties(cmTarget* target,
                          ImportPropertyMap& properties,
                          std::set<cmTarget*> &emitted);

private:
  std::string FindTargets(const char *prop, cmTarget *tgt,
                   std::set<cmTarget*> &emitted);


  std::vector<cmTarget*> Exports;
  const char *Config;
};

#endif
