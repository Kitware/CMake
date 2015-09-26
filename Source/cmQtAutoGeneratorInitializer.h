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
class cmTarget;
class cmLocalGenerator;

class cmQtAutoGeneratorInitializer
{
public:
  static void InitializeAutogenSources(cmTarget* target);
  static void InitializeAutogenTarget(cmLocalGenerator* lg, cmTarget* target);
  static void SetupAutoGenerateTarget(cmTarget const* target);

  static std::string GetAutogenTargetName(cmTarget const* target);
  static std::string GetAutogenTargetDir(cmTarget const* target);

private:
  static void SetupSourceFiles(cmTarget const* target,
                        std::vector<std::string>& skipMoc,
                        std::vector<std::string>& mocSources,
                        std::vector<std::string>& mocHeaders,
                        std::vector<std::string>& skipUic);

  static void SetupAutoMocTarget(cmTarget const* target,
                          const std::string &autogenTargetName,
                          const std::vector<std::string>& skipMoc,
                          const std::vector<std::string>& mocHeaders,
                          std::map<std::string, std::string> &configIncludes,
                          std::map<std::string, std::string> &configDefines);
  static void SetupAutoUicTarget(cmTarget const* target,
                        const std::vector<std::string>& skipUic,
                        std::map<std::string, std::string> &configUicOptions);
  static void SetupAutoRccTarget(cmTarget const* target);

  static void MergeRccOptions(std::vector<std::string> &opts,
                       const std::vector<std::string> &fileOpts, bool isQt5);

  static std::string GetRccExecutable(cmTarget const* target);

  static std::string ListQt5RccInputs(cmSourceFile* sf, cmTarget const* target,
                               std::vector<std::string>& depends);

  static std::string ListQt4RccInputs(cmSourceFile* sf,
                               std::vector<std::string>& depends);
};

#endif
