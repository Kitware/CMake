/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmCTestTypes.h"

#include <string>

namespace cmCTestTypes {

bool SetTruncationMode(TruncationMode& mode, cm::string_view str)
{
  if (str == "tail") {
    mode = cmCTestTypes::TruncationMode::Tail;
  } else if (str == "middle") {
    mode = cmCTestTypes::TruncationMode::Middle;
  } else if (str == "head") {
    mode = cmCTestTypes::TruncationMode::Head;
  } else {
    return false;
  }
  return true;
}

} // namespace cmCTestTypes
