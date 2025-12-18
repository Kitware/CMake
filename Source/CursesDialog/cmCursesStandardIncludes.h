/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#if defined(__hpux)
#  define _BOOL_DEFINED
#  include <sys/time.h>
#endif

// Tell curses headers on Solaris to not define macros
// that conflict with C++ standard library interfaces.
#if defined(__sun__)
#  define NOMACROS
#  define NCURSES_NOMACROS
#endif

#include <form.h>
