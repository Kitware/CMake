/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
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
  std::string const& globalGenerator, std::string const& extraGenerator)
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
  std::string const& /*bindir*/, std::string const& /*projectName*/,
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
  std::string const& base)
{
  this->SupportedGlobalGenerators.push_back(base);
}
