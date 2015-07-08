/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2015 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCommonTargetGenerator.h"

#include "cmGeneratorTarget.h"
#include "cmGlobalCommonGenerator.h"
#include "cmLocalCommonGenerator.h"
#include "cmTarget.h"

cmCommonTargetGenerator::cmCommonTargetGenerator(cmGeneratorTarget* gt)
  : GeneratorTarget(gt)
  , Target(gt->Target)
  , Makefile(gt->Makefile)
  , LocalGenerator(static_cast<cmLocalCommonGenerator*>(gt->LocalGenerator))
  , GlobalGenerator(static_cast<cmGlobalCommonGenerator*>(
                      gt->LocalGenerator->GetGlobalGenerator()))
  , ConfigName(LocalGenerator->GetConfigName())
{
}

cmCommonTargetGenerator::~cmCommonTargetGenerator()
{
}

std::string const& cmCommonTargetGenerator::GetConfigName() const
{
  return this->ConfigName;
}
