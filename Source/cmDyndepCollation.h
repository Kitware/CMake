/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <functional>
#include <string>

class cmGeneratorTarget;
class cmSourceFile;

namespace Json {
class Value;
}

struct cmDyndepGeneratorCallbacks
{
  std::function<std::string(cmSourceFile const* sf, std::string const& config)>
    ObjectFilePath;
};

struct cmDyndepCollation
{
  static void AddCollationInformation(Json::Value& tdi,
                                      cmGeneratorTarget const* gt,
                                      std::string const& config,
                                      cmDyndepGeneratorCallbacks const& cb);
};
