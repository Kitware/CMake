/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

class cmLocalGenerator;

namespace cm {
namespace GenEx {

struct Context final
{
  Context(cmLocalGenerator const* lg, std::string config,
          std::string language = std::string());

  cmLocalGenerator const* LG;
  std::string Config;
  std::string Language;
};

}
}
