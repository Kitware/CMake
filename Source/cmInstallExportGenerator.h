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

class cmTarget;


class cmInstallTargetGenerator;

/* cmInstallExportTarget is used in cmGlobalGenerator to collect the 
install generators for the exported targets. These are then used by the 
cmInstallExportGenerator.
*/
class cmTargetExport
{
public:
  cmTargetExport(cmTarget* tgt, 
                 cmInstallTargetGenerator* archive, 
                 cmInstallTargetGenerator* runtime, 
                 cmInstallTargetGenerator* library
                ) : Target(tgt), ArchiveGenerator(archive),
                    RuntimeGenerator(runtime), LibraryGenerator(library) {}

  cmTarget* Target;
  cmInstallTargetGenerator* ArchiveGenerator;
  cmInstallTargetGenerator* RuntimeGenerator;
  cmInstallTargetGenerator* LibraryGenerator;
private:
  cmTargetExport();
};


/** \class cmInstallExportGenerator
 * \brief Generate rules for creating an export files.
 */
class cmInstallExportGenerator: public cmInstallGenerator
{
public:
  cmInstallExportGenerator(const char* dest, const char* file_permissions,
                           const std::vector<std::string>& configurations,
                           const char* filename, const char* prefix, 
                           const char* tempOutputDir);

  bool SetExportSet(const char* name, 
                    const std::vector<cmTargetExport*>* exportSet);
protected:
  // internal class which collects all the properties which will be set
  // in the export file for the target
  class cmTargetWithProperties
  {
  public:
    cmTargetWithProperties(cmTarget* target):Target(target) {}
    cmTarget* Target;
    std::map<std::string, std::string> Properties;
  private:
    cmTargetWithProperties();
  };

  virtual void GenerateScript(std::ostream& os);
  static bool AddInstallLocations(cmTargetWithProperties *twp, 
                                  cmInstallTargetGenerator* generator,
                                  const char* prefix);

  std::string Name;
  std::string FilePermissions;
  const std::vector<std::string> Configurations;
  std::string Filename;
  std::string Prefix;
  std::string TempOutputDir;
  std::string ExportFilename;

  std::vector<cmTargetWithProperties*> Targets;
};

#endif
