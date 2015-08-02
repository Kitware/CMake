/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2015 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmLocalCommonGenerator.h"

#include "cmMakefile.h"

cmLocalCommonGenerator::cmLocalCommonGenerator(cmGlobalGenerator* gg,
                                               cmMakefile* mf):
  cmLocalGenerator(gg, mf)
{
}

cmLocalCommonGenerator::~cmLocalCommonGenerator()
{
}

void cmLocalCommonGenerator::SetConfigName()
{
  // Store the configuration name that will be generated.
  if(const char* config = this->Makefile->GetDefinition("CMAKE_BUILD_TYPE"))
    {
    // Use the build type given by the user.
    this->ConfigName = config;
    }
  else
    {
    // No configuration type given.
    this->ConfigName = "";
    }
}
