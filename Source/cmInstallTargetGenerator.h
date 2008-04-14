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

  /** Select the policy for installing shared library linkable name
      symlinks.  */
  enum NamelinkModeType
  {
    NamelinkModeNone,
    NamelinkModeOnly,
    NamelinkModeSkip
  };
  void SetNamelinkMode(NamelinkModeType mode) { this->NamelinkMode = mode; }
  NamelinkModeType GetNamelinkMode() const { return this->NamelinkMode; }

  std::string GetInstallFilename(const char* config) const;

  enum NameType
  {
    NameNormal,
    NameImplib,
    NameSO,
    NameReal
  };

  static std::string GetInstallFilename(cmTarget*target, const char* config,
                                        NameType nameType = NameNormal);

  cmTarget* GetTarget() const { return this->Target; }
  bool IsImportLibrary() const { return this->ImportLibrary; }

protected:
  typedef cmInstallGeneratorIndent Indent;
  virtual void GenerateScript(std::ostream& os);
  void GenerateScriptForConfig(std::ostream& os,
                               const char* fromDir,
                               const char* config,
                               Indent const& indent);
  void GenerateScriptForConfigDir(std::ostream& os,
                                  const char* fromDirConfig,
                                  const char* config,
                                  Indent const& indent);
  void AddInstallNamePatchRule(std::ostream& os, Indent const& indent,
                               const char* config,
                               const std::string& toDestDirPath);
  void AddChrpathPatchRule(std::ostream& os, Indent const& indent,
                           const char* config,
                           std::string const& toDestDirPath);
  void AddRPathCheckRule(std::ostream& os, Indent const& indent,
                         const char* config,
                         std::string const& toDestDirPath);
  
  void AddStripRule(std::ostream& os, Indent const& indent,
                    cmTarget::TargetType type,
                    const std::string& toDestDirPath);
  void AddRanlibRule(std::ostream& os, Indent const& indent,
                     cmTarget::TargetType type,
                     const std::string& toDestDirPath);

  cmTarget* Target;
  bool ImportLibrary;
  std::string FilePermissions;
  bool Optional;
  NamelinkModeType NamelinkMode;
};

#endif
