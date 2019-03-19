# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

add_library(lib1 STATIC func2.c func3.c func4.c)
target_link_libraries(lib1 lib2)

add_library(lib2 STATIC func5.c func6.c func7.c)
