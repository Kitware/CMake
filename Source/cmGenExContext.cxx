/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGenExContext.h"

#include <utility>

namespace cm {
namespace GenEx {

Context::Context(cmLocalGenerator const* lg, std::string config,
                 std::string language)
  : LG(lg)
  , Config(std::move(config))
  , Language(std::move(language))
{
}

}
}
