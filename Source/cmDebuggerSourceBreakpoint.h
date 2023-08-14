/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstdint>

namespace cmDebugger {

class cmDebuggerSourceBreakpoint
{
  int64_t Id;
  int64_t Line;
  bool IsValid = true;

public:
  cmDebuggerSourceBreakpoint(int64_t id, int64_t line);
  int64_t GetId() const noexcept { return this->Id; }
  int64_t GetLine() const noexcept { return this->Line; }
  void ChangeLine(int64_t line) noexcept { this->Line = line; }
  bool GetIsValid() const noexcept { return this->IsValid; }
  void Invalid() noexcept { this->IsValid = false; }
};

} // namespace cmDebugger
