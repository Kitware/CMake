/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#if defined(NDEBUG)
#  include <cm/utility>
#  define CM_UNREACHABLE                                                      \
    do {                                                                      \
      cm::unreachable();                                                      \
    } while (false)
#else
#  include <cassert>
#  include <cstdlib>
#  include <iostream>
#  define CM_UNREACHABLE                                                      \
    do {                                                                      \
      std::cerr << "unreachable code path at " << __FILE__ << ':' << __LINE__ \
                << " in " << __func__ << '\n';                                \
      assert(false && "unreachable code path");                               \
      std::abort();                                                           \
    } while (false)
#endif
