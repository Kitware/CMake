PROJECT(CMake)

SUBDIRS(Source Modules Templates)

# Include the standard Dart testing module
INCLUDE (${CMAKE_ROOT}/Modules/Dart.cmake)


INCLUDE_REGULAR_EXPRESSION("^cm.*")


