/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExternalMakefileProjectGenerator.h"

#include <utility>

#include "cmStringAlgorithms.h"

class cmMakefile;

void cmExternalMakefileProjectGenerator::EnableLanguage(
  std::vector<std::string> const& /*unused*/, cmMakefile* /*unused*/,
  bool /*unused*/)
{
}

std::string cmExternalMakefileProjectGenerator::CreateFullGeneratorName(
  const std::string& globalGenerator, const std::string& extraGenerator)
{
  if (globalGenerator.empty()) {
    return {};
  }
  if (extraGenerator.empty()) {
    return globalGenerator;
  }
  return cmStrCat(extraGenerator, " - ", globalGenerator);
}

bool cmExternalMakefileProjectGenerator::Open(
  const std::string& /*bindir*/, const std::string& /*projectName*/,
  bool /*dryRun*/)
{
  return false;
}

cmExternalMakefileProjectGeneratorFactory::
  cmExternalMakefileProjectGeneratorFactory(std::string n, std::string doc)
  : Name(std::move(n))
  , Documentation(std::move(doc))
{
}

cmExternalMakefileProjectGeneratorFactory::
  ~cmExternalMakefileProjectGeneratorFactory() = default;

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
