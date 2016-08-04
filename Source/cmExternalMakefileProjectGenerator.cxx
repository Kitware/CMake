/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmExternalMakefileProjectGenerator.h"

#include <assert.h>

void cmExternalMakefileProjectGenerator::EnableLanguage(
  std::vector<std::string> const&, cmMakefile*, bool)
{
}

std::string cmExternalMakefileProjectGenerator::CreateFullGeneratorName(
  const std::string& globalGenerator, const std::string& extraGenerator)
{
  std::string fullName;
  if (!globalGenerator.empty()) {
    if (!extraGenerator.empty()) {
      fullName = extraGenerator;
      fullName += " - ";
    }
    fullName += globalGenerator;
  }
  return fullName;
}

cmExternalMakefileProjectGeneratorFactory::
  cmExternalMakefileProjectGeneratorFactory(const std::string& n,
                                            const std::string& doc)
  : Name(n)
  , Documentation(doc)
{
}

cmExternalMakefileProjectGeneratorFactory::
  ~cmExternalMakefileProjectGeneratorFactory()
{
}

std::string cmExternalMakefileProjectGeneratorFactory::GetName() const
{
  return this->Name;
}

std::string cmExternalMakefileProjectGeneratorFactory::GetDocumentation() const
{
  return this->Documentation;
}

std::vector<std::string>
cmExternalMakefileProjectGeneratorFactory::GetSupportedGlobalGenerators() const
{
  return this->SupportedGlobalGenerators;
}

void cmExternalMakefileProjectGeneratorFactory::AddSupportedGlobalGenerator(
  const std::string& base)
{
  this->SupportedGlobalGenerators.push_back(base);
}
