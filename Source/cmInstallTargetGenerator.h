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
#include "cmTarget.h"

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
    const char* component = "",
    bool optional = false
    );
  virtual ~cmInstallTargetGenerator();

  std::string GetInstallFilename(const char* config) const;
  static std::string GetInstallFilename(cmTarget*target, const char* config, 
                                        bool implib, bool useSOName);

  const std::vector<std::string>& GetConfigurations() const {return this->Configurations;}
  
protected:
  virtual void GenerateScript(std::ostream& os);
  void GenerateScriptForConfig(std::ostream& os,
                               const char* fromDir,
                               const char* config);
  void AddInstallNamePatchRule(std::ostream& os, const char* config,
                               const std::string& toFullPath);
  void AddStripRule(std::ostream& os, cmTarget::TargetType type,
                    const std::string& toFullPath);
  void AddRanlibRule(std::ostream& os, cmTarget::TargetType type,
                     const std::string& toFullPath);

  cmTarget* Target;
  bool ImportLibrary;
  std::string FilePermissions;
  std::vector<std::string> Configurations;
  std::string Component;
  bool Optional;
};

#endif
