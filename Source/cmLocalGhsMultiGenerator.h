/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Geoffrey Viola <geoffrey.viola@asirobots.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLocalGhsMultiGenerator_h
#define cmLocalGhsMultiGenerator_h

#include "cmLocalGenerator.h"

class cmGeneratedFileStream;

/** \class cmLocalGhsMultiGenerator
 * \brief Write Green Hills MULTI project files.
 *
 * cmLocalGhsMultiGenerator produces a set of .gpj
 * file for each target in its mirrored directory.
 */
class cmLocalGhsMultiGenerator : public cmLocalGenerator
{
public:
  cmLocalGhsMultiGenerator(cmLocalGenerator* parent);

  virtual ~cmLocalGhsMultiGenerator();

  /// @returns the relative path between the HomeOutputDirectory and this
  /// local generators StartOutputDirectory.
  std::string GetHomeRelativeOutputPath() const
  {
    return this->HomeRelativeOutputPath;
  }

  /**
   * Generate the makefile for this directory.
   */
  virtual void Generate();

  /// Overloaded methods. @see cmLocalGenerator::Configure()
  virtual void Configure();
  const char *GetBuildFileName() { return this->BuildFileName.c_str(); }

protected:
  virtual bool CustomCommandUseLocal() const { return true; }

private:
  std::string BuildFileName;
  std::string HomeRelativeOutputPath;
};

#endif
