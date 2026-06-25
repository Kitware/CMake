enable_language(C)

# Setting CMAKE_SYSROOT to "/" must not strip the leading "/" from RPATH entries.
set(CMAKE_SYSROOT "/")

function(CheckRpath target rpath)
  add_custom_command(
    TARGET ${target}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -Dfile=$<TARGET_FILE:${target}> -Drpath=${rpath}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/RelativeCheck.cmake"
    VERBATIM
  )
endfunction()

add_library(sysroot-slash-lib SHARED A.c)
add_executable(sysroot-slash-exe main.c)
target_link_libraries(sysroot-slash-exe sysroot-slash-lib)
CheckRpath(sysroot-slash-exe "${CMAKE_CURRENT_BINARY_DIR}${cfg_dir}")
