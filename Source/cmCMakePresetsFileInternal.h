/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCMakePresetsFile.h"

#define CHECK_OK(expr)                                                        \
  {                                                                           \
    auto _result = expr;                                                      \
    if (_result != ReadFileResult::READ_OK)                                   \
      return _result;                                                         \
  }

namespace cmCMakePresetsFileInternal {
enum class ExpandMacroResult
{
  Ok,
  Ignore,
  Error,
};

using MacroExpander = std::function<ExpandMacroResult(
  const std::string&, const std::string&, std::string&, int version)>;
}
