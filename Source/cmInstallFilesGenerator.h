/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmInstallFilesGenerator_h
#define cmInstallFilesGenerator_h

#include "cmInstallGenerator.h"

/** \class cmInstallFilesGenerator
 * \brief Generate file installation rules.
 */
class cmInstallFilesGenerator : public cmInstallGenerator
{
public:
  cmInstallFilesGenerator(std::vector<std::string> const& files,
                          const char* dest, bool programs,
                          const char* file_permissions,
                          std::vector<std::string> const& configurations,
                          const char* component, MessageLevel message,
                          bool exclude_from_all, const char* rename,
                          bool optional = false);
  virtual ~cmInstallFilesGenerator();

  void Compute(cmLocalGenerator* lg);

  std::string GetDestination(std::string const& config) const;

protected:
  virtual void GenerateScriptActions(std::ostream& os, Indent const& indent);
  virtual void GenerateScriptForConfig(std::ostream& os,
                                       const std::string& config,
                                       Indent const& indent);
  void AddFilesInstallRule(std::ostream& os, std::string const& config,
                           Indent const& indent,
                           std::vector<std::string> const& files);

  cmLocalGenerator* LocalGenerator;
  std::vector<std::string> Files;
  std::string FilePermissions;
  std::string Rename;
  bool Programs;
  bool Optional;
};

#endif
