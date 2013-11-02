/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackProductBuildGenerator_h
#define cmCPackProductBuildGenerator_h


#include "cmCPackPKGGenerator.h"

class cmCPackComponent;

/** \class cmCPackProductBuildGenerator
 * \brief A generator for ProductBuild files
 *
 */
class cmCPackProductBuildGenerator : public cmCPackPKGGenerator
{
public:
  cmCPackTypeMacro(cmCPackProductBuildGenerator, cmCPackPKGGenerator);

  /**
   * Construct generator
   */
  cmCPackProductBuildGenerator();
  virtual ~cmCPackProductBuildGenerator();

protected:
  virtual int InitializeInternal();
  int PackageFiles();
  virtual const char* GetOutputExtension() { return ".pkg"; }

  // Run ProductBuild with the given command line, which will (if
  // successful) produce the given package file. Returns true if
  // ProductBuild succeeds, false otherwise.
  bool RunProductBuild(const std::string& command);

  // Generate a package in the file packageFile for the given
  // component.  All of the files within this component are stored in
  // the directory packageDir. Returns true if successful, false
  // otherwise.
  bool GenerateComponentPackage(const std::string& packageFileDir,
                                const std::string& packageFileName,
                                const std::string& packageDir,
                                const cmCPackComponent* component);

  const char* GetComponentScript(const char* script,
                                 const char* script_component);

};

#endif
