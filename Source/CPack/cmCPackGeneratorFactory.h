/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <string>

class cmCPackGenerator;
class cmCPackLog;

/** \class cmCPackGeneratorFactory
 * \brief A container for CPack generators
 *
 */
class cmCPackGeneratorFactory
{
public:
  cmCPackGeneratorFactory();

  //! Get the generator
  std::unique_ptr<cmCPackGenerator> NewGenerator(const std::string& name);

  using CreateGeneratorCall = cmCPackGenerator*();

  void RegisterGenerator(const std::string& name,
                         const char* generatorDescription,
                         CreateGeneratorCall* createGenerator);

  void SetLogger(cmCPackLog* logger) { this->Logger = logger; }

  using DescriptionsMap = std::map<std::string, std::string>;
  const DescriptionsMap& GetGeneratorsList() const
  {
    return this->GeneratorDescriptions;
  }

private:
  using t_GeneratorCreatorsMap = std::map<std::string, CreateGeneratorCall*>;
  t_GeneratorCreatorsMap GeneratorCreators;
  DescriptionsMap GeneratorDescriptions;
  cmCPackLog* Logger{};
};
