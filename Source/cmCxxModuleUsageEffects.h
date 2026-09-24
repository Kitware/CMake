/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include "cmListFileCache.h"

class cmGeneratorTarget;

class cmCxxModuleUsageEffects
{
public:
  cmCxxModuleUsageEffects(cmGeneratorTarget const* gt,
                          std::string const& config);
  std::string const& GetHash() const;
  std::string const& GetMsvcRuntimeLibrary() const;
  std::vector<BT<std::string>> const& GetPreprocessorCompileOptions() const;

private:
  std::string Hash;
  std::string MsvcRuntimeLibrary;
  std::vector<BT<std::string>> PreprocessorCompileOptions;
};
