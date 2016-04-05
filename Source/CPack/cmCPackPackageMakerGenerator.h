/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackPackageMakerGenerator_h
#define cmCPackPackageMakerGenerator_h

#include "cmCPackPKGGenerator.h"

class cmCPackComponent;

/** \class cmCPackPackageMakerGenerator
 * \brief A generator for PackageMaker files
 *
 * http://developer.apple.com/documentation/Darwin
 * /Reference/ManPages/man1/packagemaker.1.html
 */
class cmCPackPackageMakerGenerator : public cmCPackPKGGenerator
{
public:
  cmCPackTypeMacro(cmCPackPackageMakerGenerator, cmCPackPKGGenerator);

  /**
   * Construct generator
   */
  cmCPackPackageMakerGenerator();
  virtual ~cmCPackPackageMakerGenerator();
  bool SupportsComponentInstallation() const;

protected:
  virtual int InitializeInternal();
  int PackageFiles();
  virtual const char* GetOutputExtension() { return ".dmg"; }

  // Run PackageMaker with the given command line, which will (if
  // successful) produce the given package file. Returns true if
  // PackageMaker succeeds, false otherwise.
  bool RunPackageMaker(const char* command, const char* packageFile);

  // Generate a package in the file packageFile for the given
  // component.  All of the files within this component are stored in
  // the directory packageDir. Returns true if successful, false
  // otherwise.
  bool GenerateComponentPackage(const char* packageFile,
                                const char* packageDir,
                                const cmCPackComponent& component);

  double PackageMakerVersion;
  unsigned int PackageCompatibilityVersion;
};

#endif
