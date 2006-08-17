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
#ifndef cmInstallTargetGenerator_h
#define cmInstallTargetGenerator_h

#include "cmInstallGenerator.h"

class cmTarget;

/** \class cmInstallTargetGenerator
 * \brief Generate target installation rules.
 */
class cmInstallTargetGenerator: public cmInstallGenerator
{
public:
  cmInstallTargetGenerator(
    cmTarget& t, const char* dest, bool implib,
    const char* file_permissions = "",
    std::vector<std::string> const& configurations 
    = std::vector<std::string>(),
    const char* component = ""
    );
  virtual ~cmInstallTargetGenerator();

protected:
  virtual void GenerateScript(std::ostream& os);
  void PrepareScriptReference(std::ostream& os, cmTarget* target,
                              const char* place, bool useConfigDir,
                              bool useSOName);
  std::string GetScriptReference(cmTarget* target, const char* place,
                                 bool useSOName);
  void AddInstallNamePatchRule(std::ostream& os, const char* destination);
  cmTarget* Target;
  std::string Destination;
  bool ImportLibrary;
  std::string FilePermissions;
  std::vector<std::string> Configurations;
  std::string Component;
};

#endif
