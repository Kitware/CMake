/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmDebuggerStackFrame.h"

#include <utility>

#include "cmListFileCache.h"

namespace cmDebugger {

std::atomic<int64_t> cmDebuggerStackFrame::NextId(1);

cmDebuggerStackFrame::cmDebuggerStackFrame(cmMakefile* mf,
                                           std::string sourcePath,
                                           cmListFileFunction const& lff)
  : Id(NextId.fetch_add(1))
  , FileName(std::move(sourcePath))
  , Function(lff)
  , Makefile(mf)
{
}

int64_t cmDebuggerStackFrame::GetLine() const noexcept
{
  return this->Function.Line();
}

} // namespace cmDebugger
