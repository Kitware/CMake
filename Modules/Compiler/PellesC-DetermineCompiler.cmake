# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

set(_compiler_id_pp_test "defined(__POCC__)")

set(_compiler_id_version_compute "
# define @PREFIX@COMPILER_VERSION_MAJOR @MACRO_DEC@(__POCC__/100)
# define @PREFIX@COMPILER_VERSION_MINOR @MACRO_DEC@(__POCC__%100)")
