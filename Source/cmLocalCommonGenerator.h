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

#include "cmLocalGenerator.h"

class cmCommonTargetGenerator;

/** \class cmLocalCommonGenerator
 * \brief Common infrastructure for Makefile and Ninja local generators.
 */
class cmLocalCommonGenerator: public cmLocalGenerator
{
public:
  cmLocalCommonGenerator(cmGlobalGenerator* gg, cmMakefile* mf);
  ~cmLocalCommonGenerator();

  std::string const& GetConfigName() { return this->ConfigName; }

protected:
  void SetConfigName();
  std::string ConfigName;

  friend class cmCommonTargetGenerator;
};

#endif
