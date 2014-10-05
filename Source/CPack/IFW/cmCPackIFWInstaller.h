/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackIFWInstaller_h
#define cmCPackIFWInstaller_h

#include <cmStandardIncludes.h>

class cmCPackIFWPackage;
class cmCPackIFWGenerator;

/** \class cmCPackIFWInstaller
 * \brief A binary installer to be created CPack IFW generator
 */
class cmCPackIFWInstaller
{
public: // Types

  typedef std::map<std::string, cmCPackIFWPackage*> PackagesMap;

  struct RepositoryStruct
  {
    std::string Url;
    std::string Enabled;
    std::string Username;
    std::string Password;
    std::string DisplayName;
  };

public: // Constructor

  /**
   * Construct installer
   */
  cmCPackIFWInstaller();

public: // Configuration

  /// Name of the product being installed
  std::string Name;

  /// Version number of the product being installed
  std::string Version;

  /// Name of the installer as displayed on the title bar
  std::string Title;

  /// Publisher of the software (as shown in the Windows Control Panel)
  std::string Publisher;

  /// URL to a page that contains product information on your web site
  std::string ProductUrl;

  /// Filename for a custom installer icon
  std::string InstallerApplicationIcon;

  /// Filename for a custom window icon
  std::string InstallerWindowIcon;

  /// Filename for a logo
  std::string Logo;

  /// Default target directory for installation
  std::string TargetDir;

  /// Default target directory for installation with administrator rights
  std::string AdminTargetDir;

public: // Internal implementation

  const char* GetOption(const std::string& op) const;
  bool IsOn(const std::string& op) const;

  void ConfigureFromOptions();

  void GenerateInstallerFile();

  void GeneratePackageFiles();

  cmCPackIFWGenerator* Generator;
  PackagesMap Packages;
  std::vector<RepositoryStruct> Repositories;
  std::string Directory;
};

#endif // cmCPackIFWInstaller_h
