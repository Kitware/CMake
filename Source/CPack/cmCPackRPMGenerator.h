/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackRPMGenerator_h
#define cmCPackRPMGenerator_h


#include "cmCPackGenerator.h"

/** \class cmCPackRPMGenerator
 * \brief A generator for RPM packages
 * The idea of the CPack RPM generator is to use
 * as minimal C++ code as possible.
 * Ideally the C++ part of the CPack RPM generator
 * will only 'execute' (aka ->ReadListFile) several
 * CMake macros files.
 */
class cmCPackRPMGenerator : public cmCPackGenerator
{
public:
  cmCPackTypeMacro(cmCPackRPMGenerator, cmCPackGenerator);

  /**
   * Construct generator
   */
  cmCPackRPMGenerator();
  virtual ~cmCPackRPMGenerator();

protected:
  virtual int InitializeInternal();
  virtual int PackageFiles();
  /**
   * This method factors out the work done in component packaging case.
   */
  int PackageOnePack(std::string initialToplevel, std::string packageName);
  /**
   * The method used to package files when component
   * install is used. This will create one
   * archive for each component group.
   */
  int PackageComponents(bool ignoreGroup);
  /**
   * Special case of component install where all
   * components will be put in a single installer.
   */
  int PackageComponentsAllInOne();
  virtual const char* GetOutputExtension() { return ".rpm"; }
  virtual bool SupportsComponentInstallation() const;
  virtual std::string GetComponentInstallDirNameSuffix(
      const std::string& componentName);

};

#endif
