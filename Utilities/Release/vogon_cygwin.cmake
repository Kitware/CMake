set(CMAKE_RELEASE_DIRECTORY "c:/hoffman/CMakeReleaseCygwin")
set(PROCESSORS 2)
set(HOST vogon)
set(CPACK_BINARY_GENERATORS "CygwinBinary")
set(CPACK_SOURCE_GENERATORS "CygwinSource")
set(MAKE_PROGRAM "make")
set(INITIAL_CACHE "CMAKE_BUILD_TYPE:STRING=Release
Subversion_SVNADMIN_EXECUTABLE:STRING=FALSE
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
