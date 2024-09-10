cmake_policy(SET CMP0075 NEW)

enable_language(C)
enable_language(CXX)

function(test_check_result isSucceed functionName)
  if(isSucceed AND NOT IS_NEED_SUCCESS)
    message(SEND_ERROR "${functionName}: check succeeded instead of failure")
  elseif((NOT isSucceed) AND IS_NEED_SUCCESS)
    message(SEND_ERROR "${functionName}: check failed instead of success")
  endif()
endfunction()

# Common variables
set(validCSourceCode "int main() {return 0;}")

###
# Checking checkers
###

# It uses common internal function `CMAKE_CHECK_SOURCE_COMPILES()`
# include(CheckCCompilerFlag)

# Also checks common internal function `CMAKE_CHECK_SOURCE_COMPILES()`
include(CheckCSourceCompiles)
check_c_source_compiles("${validCSourceCode}" CHECK_C_SOURCE_COMPILES_SUCCEED)
test_check_result("${CHECK_C_SOURCE_COMPILES_SUCCEED}" check_c_source_compiles)

# Also checks common internal function `CMAKE_CHECK_SOURCE_RUNS()`
include(CheckCSourceRuns)
check_c_source_runs("${validCSourceCode}" CHECK_C_SOURCE_RUNS_SUCCEED)
test_check_result("${CHECK_C_SOURCE_RUNS_SUCCEED}" check_c_source_runs)

# Shares code with similar C checkers
# include(CheckCXXCompilerFlag)
# include(CheckCXXSourceCompiles)
# include(CheckCXXSourceRuns)
# include(CheckCXXSymbolExists)

# Shares code with similar C checkers
# include(CheckCompilerFlag)
# include(CheckSourceCompiles)
# include(CheckSourceRuns)

# Shares code with similar C checkers
# include(CheckFortranCompilerFlag)
# include(CheckFortranFunctionExists) # No way to test it
# include(CheckFortranSourceCompiles)
# include(CheckFortranSourceRuns)

include(CheckFunctionExists)
check_function_exists (memcpy CHECK_FUNCTION_EXISTS_SUCCEED)
test_check_result("${CHECK_FUNCTION_EXISTS_SUCCEED}" check_function_exists)

include(CheckIncludeFile)
check_include_file("stddef.h" CHECK_INCLUDE_FILE_SUCCEED)
test_check_result("${CHECK_INCLUDE_FILE_SUCCEED}" check_include_file)

include(CheckIncludeFileCXX)
check_include_file_cxx("stddef.h" CHECK_INCLUDE_FILE_CXX_SUCCEED)
test_check_result("${CHECK_INCLUDE_FILE_CXX_SUCCEED}" check_include_file_cxx)

include(CheckIncludeFiles)
check_include_files("stddef.h;stdlib.h" CHECK_INCLUDE_FILES_SUCCEED)
test_check_result("${CHECK_INCLUDE_FILES_SUCCEED}" check_include_files)

include(CheckLibraryExists)
block(PROPAGATE HAVE_LIBM)
  unset(CMAKE_REQUIRED_LIBRARIES)
  unset(CMAKE_REQUIRED_LINK_DIRECTORIES)
  check_library_exists(m ceil "" HAVE_LIBM)
endblock()

if(HAVE_LIBM)
  check_library_exists(m ceil "" CHECK_LIBRARY_EXISTS_SUCCEED)
  test_check_result("${CHECK_LIBRARY_EXISTS_SUCCEED}" check_library_exists)
endif()

# Shares code with similar C checkers
# include(CheckOBJCCompilerFlag)
# include(CheckOBJCSourceCompiles)
# include(CheckOBJCSourceRuns)

# Shares code with similar C checkers
# include(CheckOBJCXXCompilerFlag)
# include(CheckOBJCXXSourceCompiles)
# include(CheckOBJCXXSourceRuns)

include(CheckPrototypeDefinition)
block(PROPAGATE CHECK_PROTOTYPE_DEFINITION_WORKS)
  unset(CMAKE_REQUIRED_LIBRARIES)
  unset(CMAKE_REQUIRED_LINK_DIRECTORIES)
  check_prototype_definition(memmove
    "void *memmove(void *dest, const void *src, size_t n)"
    "NULL"
    "string.h"
    CHECK_PROTOTYPE_DEFINITION_SUCCEED)
endblock()

if (CHECK_PROTOTYPE_DEFINITION_WORKS)
  check_prototype_definition(memmove
    "void *memmove(void *dest, const void *src, size_t n)"
    "NULL"
    "string.h"
    CHECK_PROTOTYPE_DEFINITION_SUCCEED)
  test_check_result("${CHECK_PROTOTYPE_DEFINITION_SUCCEED}" check_prototype_definition)
endif()

# It uses common internal function `CMAKE_CHECK_SOURCE_COMPILES()`
# include(CheckStructHasMember)

include(CheckSymbolExists)
check_symbol_exists(errno "errno.h" CHECK_SYMBOL_EXISTS_SUCCEED)
test_check_result("${CHECK_SYMBOL_EXISTS_SUCCEED}" check_symbol_exists)

include(CheckTypeSize)
check_type_size(int SIZEOF_INT)
test_check_result("${HAVE_SIZEOF_INT}" check_type_size)

include(CheckVariableExists)
check_variable_exists(myTestVar CHECK_VARIABLE_EXISTS_SUCCEED)
test_check_result("${CHECK_VARIABLE_EXISTS_SUCCEED}" check_variable_exists)
