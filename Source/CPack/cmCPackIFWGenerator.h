/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackIFWGenerator_h
#define cmCPackIFWGenerator_h


#include "cmCPackGenerator.h"
#include <set>

/** \class cmCPackIFWGenerator
 * \brief A generator for Qt Installer Framework tools
 *
 * http://qt-project.org/doc/qtinstallerframework/index.html
 */
class cmCPackIFWGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackIFWGenerator, cmCPackGenerator);

  /**
   * Construct generator
   */
  cmCPackIFWGenerator();
  virtual ~cmCPackIFWGenerator();

protected:
  virtual int InitializeInternal();
  virtual int PackageFiles();
  virtual const char* GetPackagingInstallPrefix();

  virtual const char* GetOutputExtension();

  std::string IfwGetGroupId(cmCPackComponentGroup *group);
  std::string IfwGetComponentId(cmCPackComponent *component);

   std::string IfwGetGroupName(cmCPackComponentGroup *group);

  std::string IfwGetComponentName(cmCPackComponent *component);
  std::string IfwGetComponentName(const std::string &componentName);

  virtual std::string GetComponentInstallDirNamePrefix(
      const std::string& componentName);

  virtual std::string GetComponentInstallDirNameSuffix(
      const std::string& componentName);

  bool GetListOfSubdirectories(const char* dir,
    std::vector<std::string>& dirs);

  enum cmCPackGenerator::CPackSetDestdirSupport SupportsSetDestdir() const;
  virtual bool SupportsAbsoluteDestination() const;
  virtual bool SupportsComponentInstallation() const;

private:
  int IfwCreateConfigFile();
  int IfwCreatePackageFile();
  std::string IfwCreateCurrentDate();
  bool IfwParseLicenses(std::vector<std::string> &licenses,
                        const std::string &variable,
                        const std::string &metaDir);

  std::string ifwRepoGen;
  std::string ifwBinCreator;

  std::string ifwDownloadSite;

  bool ifwOnlineOnly;
  bool ifwResolveDuplicateNames;
  std::vector<std::string> ifwPkgsDirsVector;
};

#endif
