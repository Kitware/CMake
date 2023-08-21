/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmImportedCxxModuleInfo.h"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "cmCryptoHash.h"
#include "cmList.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

bool ImportedCxxModuleLookup::Initialized() const
{
  return this->DoneInit;
}

void ImportedCxxModuleLookup::Initialize(std::string const& importedModules)
{
  for (auto const& entry : cmList{ importedModules }) {
    auto nameSep = entry.find('=');
    if (nameSep == std::string::npos) {
      // Invalid entry; ignore.
      continue;
    }

    auto name = entry.substr(0, nameSep);

    auto sourceSep = entry.find(',', nameSep);
    std::string source;
    if (sourceSep == std::string::npos) {
      source = entry.substr(nameSep + 1);
    } else {
      source = entry.substr(nameSep + 1, sourceSep - nameSep - 1);
    }

    std::vector<std::string> bmis;
    if (sourceSep != std::string::npos) {
      auto bmiPaths = entry.substr(sourceSep + 1);
      bmis = cmSystemTools::SplitString(bmiPaths, ',');
    }

    this->ImportedInfo.emplace(source,
                               ImportedCxxModuleInfo{ name, std::move(bmis) });
  }

  this->DoneInit = true;
}

std::string ImportedCxxModuleLookup::BmiNameForSource(std::string const& path)
{
  auto genit = this->GeneratorInfo.find(path);
  if (genit != this->GeneratorInfo.end()) {
    return genit->second.BmiName;
  }

  auto importit = this->ImportedInfo.find(path);
  std::string bmiName;
  auto hasher = cmCryptoHash::New("SHA3_512");
  constexpr size_t HASH_TRUNCATION = 12;
  if (importit != this->ImportedInfo.end()) {
    auto safename = hasher->HashString(importit->second.Name);
    bmiName = cmStrCat(safename.substr(0, HASH_TRUNCATION), ".bmi");
  } else {
    auto dirhash = hasher->HashString(path);
    bmiName = cmStrCat(dirhash.substr(0, HASH_TRUNCATION), ".bmi");
  }

  this->GeneratorInfo.emplace(
    path, ImportedCxxModuleGeneratorInfo{ &importit->second, bmiName });
  return bmiName;
}
