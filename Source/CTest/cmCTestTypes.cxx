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

cm::optional<ResourceErrorAction> GetResourceErrorAction(cm::string_view str)
{
  if (str == "FAIL") {
    return cmCTestTypes::ResourceErrorAction::Fail;
  }
  if (str == "SKIP") {
    return cmCTestTypes::ResourceErrorAction::Skip;
  }
  return {};
}

std::string ResourceErrorActionToString(ResourceErrorAction action)
{
  switch (action) {
    case cmCTestTypes::ResourceErrorAction::Fail:
      return "FAIL";
    case cmCTestTypes::ResourceErrorAction::Skip:
      return "SKIP";
  }
  return {};
}

} // namespace cmCTestTypes
