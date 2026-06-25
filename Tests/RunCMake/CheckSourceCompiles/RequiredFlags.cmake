enable_language (C)
include(CheckSourceCompiles)

# If CMAKE_REQUIRED_FLAGS is a list, all items but the first are passed as
# arguments to 'cmake'. For backwards compatibility, test that this works.
#
# It would be nice if we could also test that a literal semicolon makes it
# through. Unfortunately, that needs different levels of escaping depending on
# the generator, because the build rule may or may not be subject to shell
# evaluation.
set(CMAKE_REQUIRED_FLAGS
  "-DFUNC=main"
  "-DCMAKE_PROJECT_INCLUDE=${CMAKE_CURRENT_LIST_DIR}/Extra.cmake")
check_source_compiles(C "int FUNC() {return 0;}" SHOULD_BUILD)
if(NOT SHOULD_BUILD)
  message(SEND_ERROR "Test fail for valid C source.")
endif()
