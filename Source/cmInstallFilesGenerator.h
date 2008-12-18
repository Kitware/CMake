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
#ifndef cmInstallFilesGenerator_h
#define cmInstallFilesGenerator_h

#include "cmInstallGenerator.h"

/** \class cmInstallFilesGenerator
 * \brief Generate file installation rules.
 */
class cmInstallFilesGenerator: public cmInstallGenerator
{
public:
  cmInstallFilesGenerator(std::vector<std::string> const& files,
                          const char* dest, bool programs,
                          const char* file_permissions,
                          std::vector<std::string> const& configurations,
                          const char* component,
                          const char* rename,
                          bool optional = false);
  virtual ~cmInstallFilesGenerator();

protected:
  virtual void GenerateScriptActions(std::ostream& os, Indent const& indent);
  std::vector<std::string> Files;
  bool Programs;
  std::string FilePermissions;
  std::string Rename;
  bool Optional;
};

#endif
