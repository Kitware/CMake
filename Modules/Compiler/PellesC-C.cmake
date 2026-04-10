# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

include (Compiler/PellesC)
__compiler_pellesc(C)

set(CMAKE_C_OUTPUT_EXTENSION ".obj")
set(CMAKE_C_VERBOSE_FLAG "-V2")
