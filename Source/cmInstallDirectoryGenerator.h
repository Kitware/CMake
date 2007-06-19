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
#ifndef cmInstallDirectoryGenerator_h
#define cmInstallDirectoryGenerator_h

#include "cmInstallGenerator.h"

/** \class cmInstallDirectoryGenerator
 * \brief Generate directory installation rules.
 */
class cmInstallDirectoryGenerator: public cmInstallGenerator
{
public:
  cmInstallDirectoryGenerator(std::vector<std::string> const& dirs,
                              const char* dest,
                              const char* file_permissions,
                              const char* dir_permissions,
                              std::vector<std::string> const& configurations,
                              const char* component,
                              const char* literal_args);
  virtual ~cmInstallDirectoryGenerator();

protected:
  virtual void GenerateScript(std::ostream& os);
  std::vector<std::string> Directories;
  std::string FilePermissions;
  std::string DirPermissions;
  std::vector<std::string> Configurations;
  std::string Component;
  std::string LiteralArguments;
};

#endif
