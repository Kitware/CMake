/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2010 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmCustomCommandGenerator_h
#define cmCustomCommandGenerator_h

#include "cmStandardIncludes.h"

class cmCustomCommand;
class cmMakefile;
class cmLocalGenerator;
class cmGeneratorExpression;

class cmCustomCommandGenerator
{
  cmCustomCommand const& CC;
  const char* Config;
  cmMakefile* Makefile;
  cmLocalGenerator* LG;
  bool OldStyle;
  bool MakeVars;
  cmGeneratorExpression* GE;
public:
  cmCustomCommandGenerator(cmCustomCommand const& cc, const char* config,
                           cmMakefile* mf);
  ~cmCustomCommandGenerator();
  unsigned int GetNumberOfCommands() const;
  std::string GetCommand(unsigned int c) const;
  void AppendArguments(unsigned int c, std::string& cmd) const;
};

#endif
