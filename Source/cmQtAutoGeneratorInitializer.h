/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2011 Kitware, Inc.
  Copyright 2011 Alexander Neundorf (neundorf@kde.org)

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmQtAutoGeneratorInitializer_h
#define cmQtAutoGeneratorInitializer_h

#include "cmStandardIncludes.h"

#include <string>
#include <vector>
#include <map>

class cmSourceFile;
class cmGeneratorTarget;
class cmLocalGenerator;

class cmQtAutoGeneratorInitializer
{
public:
  static void InitializeAutogenSources(cmGeneratorTarget* target);
  static void InitializeAutogenTarget(cmLocalGenerator* lg,
                                      cmGeneratorTarget* target);
  static void SetupAutoGenerateTarget(cmGeneratorTarget const* target);
};

#endif
