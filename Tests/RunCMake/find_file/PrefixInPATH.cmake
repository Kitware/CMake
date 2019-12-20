set(ENV_PATH "$ENV{PATH}")

set(CMAKE_FIND_DEBUG_MODE 1)
set(ENV{PATH} "${CMAKE_CURRENT_SOURCE_DIR}/bin")
find_file(PrefixInPATH_INCLUDE_DIR NAMES PrefixInPATH.h)
set(CMAKE_FIND_DEBUG_MODE 0)

foreach(path "/does_not_exist" "" "/bin" "/sbin")
  unset(PrefixInPATH_INCLUDE_DIR CACHE)
  set(ENV{PATH} "${CMAKE_CURRENT_SOURCE_DIR}${path}")
  find_file(PrefixInPATH_INCLUDE_DIR NAMES PrefixInPATH.h)
  message(STATUS "PrefixInPATH_INCLUDE_DIR='${PrefixInPATH_INCLUDE_DIR}'")
endforeach()
set(ENV{PATH} "${ENV_PATH}")
