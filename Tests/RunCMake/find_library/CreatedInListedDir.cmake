list(APPEND CMAKE_FIND_LIBRARY_PREFIXES lib)
list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES .a)
set(dir "${CMAKE_CURRENT_BINARY_DIR}/lib")
file(MAKE_DIRECTORY "${dir}")

# Create a library right after a search lists its directory, so the
# directory changes twice within the same second.
find_library(MISSING_LIBRARY NAMES missing PATHS "${dir}" NO_DEFAULT_PATH)
file(WRITE "${dir}/libcreated.a" "created")
find_library(CREATED_LIBRARY NAMES created PATHS "${dir}" NO_DEFAULT_PATH)
if(NOT CREATED_LIBRARY)
  message(SEND_ERROR "libcreated.a not found after creating it")
endif()
