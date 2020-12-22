# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details

include(Platform/Linux-Fujitsu)
__linux_compiler_fujitsu(CXX)

# Special sauce to propagate the -std=xxx flag when linking
set(CMAKE_CXX_LINK_WITH_STANDARD_COMPILE_OPTION ON)
