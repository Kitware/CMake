
set(_compiler_id_pp_test "defined(__clang__) && __has_include(<hip/hip_version.h>)")

set(_compiler_id_version_compute "
# if defined(__clang__) && __has_include(<hip/hip_version.h>)
#  include <hip/hip_version.h>
#  define @PREFIX@COMPILER_VERSION_MAJOR @MACRO_DEC@(HIP_VERSION_MAJOR)
#  define @PREFIX@COMPILER_VERSION_MINOR @MACRO_DEC@(HIP_VERSION_MINOR)
#  define @PREFIX@COMPILER_VERSION_PATCH @MACRO_DEC@(HIP_VERSION_PATCH)
# endif")

set(_compiler_id_simulate "
# if defined(_MSC_VER)
#  define @PREFIX@SIMULATE_ID \"MSVC\"
# elif defined(__clang__)
#  define @PREFIX@SIMULATE_ID \"Clang\"
# elif defined(__GNUC__)
#  define @PREFIX@SIMULATE_ID \"GNU\"
# endif")
