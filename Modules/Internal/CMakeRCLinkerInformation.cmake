# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.


# This file sets the basic flags for the linker used by the C compiler in CMake.
# It also loads the available platform file for the system-linker
# if it exists.
# It also loads a system - linker - processor (or target hardware)
# specific file, which is mainly useful for crosscompiling and embedded systems.

include(Internal/CMakeCommonLinkerInformation)

set(_INCLUDED_FILE 0)

# Foe now, no linker associated with RC language

set(CMAKE_RC_LINKER_INFORMATION_LOADED 1)
