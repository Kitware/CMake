/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2015 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLocalCommonGenerator_h
#define cmLocalCommonGenerator_h

#include <cmConfigure.h>

#include "cmLocalGenerator.h"
#include "cmOutputConverter.h"

#include <string>

class cmGeneratorTarget;
class cmGlobalGenerator;
class cmMakefile;

/** \class cmLocalCommonGenerator
 * \brief Common infrastructure for Makefile and Ninja local generators.
 */
class cmLocalCommonGenerator : public cmLocalGenerator
{
public:
  cmLocalCommonGenerator(cmGlobalGenerator* gg, cmMakefile* mf,
                         std::string const& wd);
  ~cmLocalCommonGenerator() CM_OVERRIDE;

  std::string const& GetConfigName() { return this->ConfigName; }

  std::string GetWorkingDirectory() const { return this->WorkingDirectory; }

  std::string GetTargetFortranFlags(cmGeneratorTarget const* target,
                                    std::string const& config) CM_OVERRIDE;

protected:
  std::string WorkingDirectory;

  void SetConfigName();
  std::string ConfigName;

  friend class cmCommonTargetGenerator;
};

#endif
