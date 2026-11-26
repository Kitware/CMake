set(fw_dir "${CMAKE_CURRENT_BINARY_DIR}/Frameworks")
file(WRITE "${fw_dir}/Bar.framework/Headers/Val.h" "")
file(WRITE "${fw_dir}/Foo.framework/Headers/Val.h" "")
set(CMAKE_FIND_FRAMEWORK ONLY)

function(check_ko result path)
  set(${result} FALSE PARENT_SCOPE)
endfunction()

function(check_bar result path)
  if(NOT path MATCHES "/Bar\\.framework/")
    set(${result} FALSE PARENT_SCOPE)
  endif()
endfunction()

find_path(DIR NAMES Val.h PATHS "${fw_dir}" NO_DEFAULT_PATH VALIDATOR check_ko)
message(STATUS "DIR='${DIR}'")
unset(DIR CACHE)

find_path(DIR NAMES Val.h PATHS "${fw_dir}" NO_DEFAULT_PATH VALIDATOR check_bar)
message(STATUS "DIR='${DIR}'")
unset(DIR CACHE)

find_file(FILE NAMES Val.h PATHS "${fw_dir}" NO_DEFAULT_PATH VALIDATOR check_bar)
message(STATUS "FILE='${FILE}'")
unset(FILE CACHE)
