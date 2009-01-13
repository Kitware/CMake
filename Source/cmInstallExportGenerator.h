/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmInstallExportGenerator_h
#define cmInstallExportGenerator_h

#include "cmInstallGenerator.h"

class cmExportInstallFileGenerator;
class cmInstallFilesGenerator;
class cmInstallTargetGenerator;
class cmTarget;
class cmTargetExport;
class cmMakefile;

/** \class cmInstallExportGenerator
 * \brief Generate rules for creating an export files.
 */
class cmInstallExportGenerator: public cmInstallGenerator
{
public:
  cmInstallExportGenerator(const char* name,
                           const char* dest, const char* file_permissions,
                           const std::vector<std::string>& configurations,
                           const char* component,
                           const char* filename, const char* name_space,
                           cmMakefile* mf);
  ~cmInstallExportGenerator();
protected:
  typedef std::vector<cmTargetExport*> ExportSet;

  virtual void GenerateScript(std::ostream& os);
  virtual void GenerateScriptConfigs(std::ostream& os, Indent const& indent);
  virtual void GenerateScriptActions(std::ostream& os, Indent const& indent);
  void GenerateImportFile(ExportSet const* exportSet);
  void GenerateImportFile(const char* config, ExportSet const* exportSet);
  void ComputeTempDir();

  std::string Name;
  std::string FilePermissions;
  std::string FileName;
  std::string Namespace;
  cmMakefile* Makefile;

  std::string TempDir;
  std::string MainImportFile;
  cmExportInstallFileGenerator* EFGen;
};

#endif
