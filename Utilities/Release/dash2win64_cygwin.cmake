set(CMAKE_RELEASE_DIRECTORY "c:/cygwin/home/dashboard/CMakeReleaseCygwin")
set(PROCESSORS 9)
set(MAKE_PROGRAM "make")
set(MAKE "${MAKE_PROGRAM} -j8")
set(HOST dash2win64)
set(CPACK_BINARY_GENERATORS "CygwinBinary")
set(CPACK_SOURCE_GENERATORS "CygwinSource")
set(MAKE_PROGRAM "make")
set(INITIAL_CACHE "CMAKE_BUILD_TYPE:STRING=Release
CMAKE_Fortran_COMPILER_FULLPATH:FILEPATH=FALSE
DART_TESTING_TIMEOUT:STRING=7200
")
set(CXX g++)
set(CC  gcc)
set(SCRIPT_NAME dash2win64cygwin)
set(GIT_EXTRA "git config core.autocrlf true")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
