# this one must silently fail
include(I_am_not_here OPTIONAL)

# this one must be found and the result must be put into _includedFile
include(CMake RESULT_VARIABLE _includedFile)

set(fileOne "${_includedFile}")
set(fileTwo "${CMAKE_ROOT}/Modules/CMake.cmake")
if(WIN32)
  string(TOLOWER "${fileOne}" fileOne)
  string(TOLOWER "${fileTwo}" fileTwo)
endif(WIN32)

if(NOT "${fileOne}"   STREQUAL "${fileTwo}")
   message(FATAL_ERROR "Wrong CMake.cmake was included: \"${fileOne}\" expected \"${fileTwo}\"")
endif(NOT "${fileOne}"   STREQUAL "${fileTwo}")

# this one must return NOTFOUND in _includedFile
include(I_do_not_exist OPTIONAL RESULT_VARIABLE _includedFile)

if(_includedFile)
   message(FATAL_ERROR "File \"I_do_not_exist\" was included, although it shouldn't exist,\nIncluded file is \"${_includedFile}\"")
endif(_includedFile)

# and this one must succeed too
include(CMake OPTIONAL RESULT_VARIABLE _includedFile)
set(fileOne "${_includedFile}")
set(fileTwo "${CMAKE_ROOT}/Modules/CMake.cmake")
if(WIN32)
  string(TOLOWER "${fileOne}" fileOne)
  string(TOLOWER "${fileTwo}" fileTwo)
endif(WIN32)

if(NOT "${fileOne}"   STREQUAL "${fileTwo}")
   message(FATAL_ERROR "Wrong CMake.cmake was included: \"${fileOne}\" expected \"${fileTwo}\"")
endif(NOT "${fileOne}"   STREQUAL "${fileTwo}")

