# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

set(_compiler_id_pp_test "defined(__ADAPTIVECPP__) || defined(__ACPP__) || defined(__OPENSYCL__) || defined(__HIPSYCL__)")

set(_compiler_id_version_compute "
# if __has_include(<AdaptiveCpp/common/config.hpp>)
#  include <AdaptiveCpp/common/config.hpp>
# elif __has_include(<hipSYCL/common/config.hpp>)
#  include <hipSYCL/common/config.hpp>
# endif
# if defined(ACPP_VERSION_MAJOR)
#  define @PREFIX@COMPILER_VERSION_MAJOR @MACRO_DEC@(ACPP_VERSION_MAJOR)
#  define @PREFIX@COMPILER_VERSION_MINOR @MACRO_DEC@(ACPP_VERSION_MINOR)
#  define @PREFIX@COMPILER_VERSION_PATCH @MACRO_DEC@(ACPP_VERSION_PATCH)
# elif defined(HIPSYCL_VERSION_MAJOR)
#  define @PREFIX@COMPILER_VERSION_MAJOR @MACRO_DEC@(HIPSYCL_VERSION_MAJOR)
#  define @PREFIX@COMPILER_VERSION_MINOR @MACRO_DEC@(HIPSYCL_VERSION_MINOR)
#  define @PREFIX@COMPILER_VERSION_PATCH @MACRO_DEC@(HIPSYCL_VERSION_PATCH)
# endif
# if defined(_MSC_VER)
#  define @PREFIX@SIMULATE_VERSION_MAJOR @MACRO_DEC@(_MSC_VER / 100)
#  define @PREFIX@SIMULATE_VERSION_MINOR @MACRO_DEC@(_MSC_VER % 100)
# endif")

set(_compiler_id_simulate "
# if defined(_MSC_VER)
#  define @PREFIX@SIMULATE_ID \"MSVC\"
# endif")
