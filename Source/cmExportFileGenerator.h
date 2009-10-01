/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmExportFileGenerator_h
#define cmExportFileGenerator_h

#include "cmCommand.h"

/** \class cmExportFileGenerator
 * \brief Generate a file exporting targets from a build or install tree.
 *
 * cmExportFileGenerator is the superclass for
 * cmExportBuildFileGenerator and cmExportInstallFileGenerator.  It
 * contains common code generation routines for the two kinds of
 * export implementations.
 */
class cmExportFileGenerator
{
public:
  cmExportFileGenerator();
  virtual ~cmExportFileGenerator() {}

  /** Set the full path to the export file to generate.  */
  void SetExportFile(const char* mainFile);

  /** Set the namespace in which to place exported target names.  */
  void SetNamespace(const char* ns) { this->Namespace = ns; }

  /** Add a configuration to be exported.  */
  void AddConfiguration(const char* config);

  /** Actually generate the export file.  Returns whether there was an
      error.  */
  bool GenerateImportFile();
protected:

  typedef std::map<cmStdString, cmStdString> ImportPropertyMap;

  // Generate per-configuration target information to the given output
  // stream.
  void GenerateImportConfig(std::ostream& os, const char* config);

  // Methods to implement export file code generation.
  void GenerateImportHeaderCode(std::ostream& os, const char* config = 0);
  void GenerateImportFooterCode(std::ostream& os);
  void GenerateImportVersionCode(std::ostream& os);
  void GenerateImportTargetCode(std::ostream& os, cmTarget* target);
  void GenerateImportPropertyCode(std::ostream& os, const char* config,
                                  cmTarget* target,
                                  ImportPropertyMap const& properties);

  // Collect properties with detailed information about targets beyond
  // their location on disk.
  void SetImportDetailProperties(const char* config,
                                 std::string const& suffix, cmTarget* target,
                                 ImportPropertyMap& properties);
  void SetImportLinkProperty(std::string const& suffix,
                             cmTarget* target, const char* propName,
                             std::vector<std::string> const& libs,
                             ImportPropertyMap& properties);

  /** Each subclass knows how to generate its kind of export file.  */
  virtual bool GenerateMainFile(std::ostream& os) = 0;

  /** Each subclass knows where the target files are located.  */
  virtual void GenerateImportTargetsConfig(std::ostream& os,
                                           const char* config,
                                           std::string const& suffix) = 0;

  /** Each subclass knows how to complain about a target that is
      missing from an export set.  */
  virtual void ComplainAboutMissingTarget(cmTarget* depender,
                                          cmTarget* dependee) = 0;

  // The namespace in which the exports are placed in the generated file.
  std::string Namespace;

  // The set of configurations to export.
  std::vector<std::string> Configurations;

  // The file to generate.
  std::string MainImportFile;
  std::string FileDir;
  std::string FileBase;
  std::string FileExt;
  bool AppendMode;

  // The set of targets included in the export.
  std::set<cmTarget*> ExportedTargets;
};

#endif
