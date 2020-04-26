# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# This module is shared by multiple languages; use include blocker.
include_guard()

string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " system linux")
string(APPEND CMAKE_MODULE_LINKER_FLAGS_INIT " system linux")
string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " system linux")

set(CMAKE_BUILD_TYPE_INIT Debug)

# single/multi-threaded                 /-bm
# default is setup for single-threaded libraries
string(APPEND CMAKE_C_FLAGS_INIT " -bt=linux")
string(APPEND CMAKE_CXX_FLAGS_INIT " -bt=linux -xs")
