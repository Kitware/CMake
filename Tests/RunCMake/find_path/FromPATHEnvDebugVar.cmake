set(ENV_PATH "$ENV{PATH}")

set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)

set(ENV{PATH} "${CMAKE_CURRENT_SOURCE_DIR}/include")
find_path(PATH_IN_ENV_PATH NAMES PrefixInPATH.h)

set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH ON)
find_path(PATH_IN_ENV_PATH NAMES PrefixInPATH.h)


foreach(path "/does_not_exist" "/include" "")
  unset(PATH_IN_ENV_PATH_A CACHE)
  set(ENV{PATH} "${CMAKE_CURRENT_SOURCE_DIR}${path}")
  find_path(PATH_IN_ENV_PATH_A NAMES PrefixInPATH.h)
  message(STATUS "PATH_IN_ENV_PATH='${PATH_IN_ENV_PATH_A}'")
endforeach()

set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH OFF)
foreach(path "/does_not_exist" "/include" "")
  unset(PATH_IN_ENV_PATH_A CACHE)
  set(ENV{PATH} "${CMAKE_CURRENT_SOURCE_DIR}${path}")
  find_path(PATH_IN_ENV_PATH_A NAMES PrefixInPATH.h)
  message(STATUS "PATH_IN_ENV_PATH='${PATH_IN_ENV_PATH_A}'")
endforeach()

set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH ON)
foreach(path "/does_not_exist" "/include" "")
  unset(PATH_IN_ENV_PATH_A CACHE)
  set(ENV{PATH} "${CMAKE_CURRENT_SOURCE_DIR}${path}")
  find_path(PATH_IN_ENV_PAT_H_A NAMES PrefixInPATH.h NO_SYSTEM_ENVIRONMENT_PATH)
  message(STATUS "PATH_IN_ENV_PATH='${PATH_IN_ENV_PATH_A}'")
endforeach()

set(ENV{PATH} "${ENV_PATH}")
