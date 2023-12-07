/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <iostream>

#define CM_DBG(expr) cm::dbg_impl(__FILE__, __LINE__, #expr, expr)

namespace cm {

namespace {

template <typename T>
T dbg_impl(const char* fname, int line, const char* expr, T value)
{
  std::cerr << fname << ':' << line << ": " << expr << " = " << value
            << std::endl;
  return value;
}

} // namespace

} // namespace cm
