// -*-c++-*-
// vim: set ft=cpp:

/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#pragma once

#define CMAKE_CXX_STD_98 199711L
#define CMAKE_CXX_STD_11 201103L
#define CMAKE_CXX_STD_14 201402L
#define CMAKE_CXX_STD_17 201703L
#define CMAKE_CXX_STD_20 202002L
#define CMAKE_CXX_STD_23 202302L

#if defined(_MSVC_LANG) && (_MSVC_LANG > __cplusplus)
#  define CMAKE_CXX_STD _MSVC_LANG
#else
#  define CMAKE_CXX_STD __cplusplus
#endif

#ifndef CONSTEXPR_CXX14
#  if CMAKE_CXX_STD >= CMAKE_CXX_STD_14
#    define CONSTEXPR_CXX14 constexpr
#  else
#    define CONSTEXPR_CXX14
#  endif
#endif

#ifndef CONSTEXPR_CXX17
#  if CMAKE_CXX_STD >= CMAKE_CXX_STD_17
#    define CONSTEXPR_CXX17 constexpr
#  else
#    define CONSTEXPR_CXX17
#  endif
#endif

#ifndef CONSTEXPR_CXX20
#  if CMAKE_CXX_STD >= CMAKE_CXX_STD_20
#    define CONSTEXPR_CXX20 constexpr
#  else
#    define CONSTEXPR_CXX20
#  endif
#endif

#ifndef CONSTEXPR_CXX23
#  if CMAKE_CXX_STD >= CMAKE_CXX_STD_23
#    define CONSTEXPR_CXX23 constexpr
#  else
#    define CONSTEXPR_CXX23
#  endif
#endif
