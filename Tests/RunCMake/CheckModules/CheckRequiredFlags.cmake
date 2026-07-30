enable_language(C)
enable_language(CXX)

# Warning options after the first entry are ignored for compatibility.
set(CMAKE_REQUIRED_FLAGS
  "-DCHECK_REQUIRED_FLAGS -Drequired_flag_function=memcpy -Drequired_flag_library=memcpy -Drequired_flag_type=int"
  "-Wseen-by-cmake")
set(CMAKE_REQUIRED_INCLUDES "${CMAKE_CURRENT_LIST_DIR}")

include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckPrototypeDefinition)
include(CheckTypeSize)

check_function_exists(required_flag_function CHECK_FUNCTION_EXISTS_RESULT)
check_include_file(CheckRequiredFlags.h CHECK_INCLUDE_FILE_RESULT)
check_include_file_cxx(CheckRequiredFlags.h CHECK_INCLUDE_FILE_CXX_RESULT)
check_include_files("stddef.h;CheckRequiredFlags.h" CHECK_INCLUDE_FILES_RESULT)
check_library_exists("" required_flag_library "" CHECK_LIBRARY_EXISTS_RESULT)
check_prototype_definition(
  required_flag_prototype
  "int required_flag_prototype(int value)"
  "value"
  "CheckRequiredFlags.h"
  CHECK_PROTOTYPE_DEFINITION_RESULT
  )
check_type_size(required_flag_type CHECK_TYPE_SIZE_RESULT)

foreach(result IN ITEMS
    CHECK_FUNCTION_EXISTS_RESULT
    CHECK_INCLUDE_FILE_RESULT
    CHECK_INCLUDE_FILE_CXX_RESULT
    CHECK_INCLUDE_FILES_RESULT
    CHECK_LIBRARY_EXISTS_RESULT
    CHECK_PROTOTYPE_DEFINITION_RESULT
    CHECK_TYPE_SIZE_RESULT
    )
  if(NOT ${result})
    message(SEND_ERROR "${result} did not honor CMAKE_REQUIRED_FLAGS")
  endif()
endforeach()
