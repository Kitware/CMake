enable_language(C)

file(REMOVE "${CMAKE_BINARY_DIR}/target_files_custom.cmake")

function(get_write_file_command var filename)
  set(${var} ${CMAKE_COMMAND} -DOUTPUT_FILE=${filename} -DGENEX_CONFIG=$<CONFIG> -DINTDIR_CONFIG=${CMAKE_CFG_INTDIR} -P ${CMAKE_SOURCE_DIR}/WriteFile.cmake PARENT_SCOPE)
endfunction()

function(create_targets prefix)
  get_write_file_command(cmd ${prefix}Command.txt)
  add_custom_command(OUTPUT ${prefix}Command.txt COMMAND ${cmd})
  add_custom_target(${prefix}Command DEPENDS ${prefix}Command.txt)

  get_write_file_command(cmd ${prefix}Target.txt)
  add_custom_target(${prefix}Target COMMAND ${cmd} BYPRODUCTS ${prefix}Target.txt)

  get_write_file_command(cmd ${prefix}PostBuild.txt)
  add_executable(${prefix}PostBuild ${CMAKE_SOURCE_DIR}/main.c)
  add_custom_command(TARGET ${prefix}PostBuild COMMAND ${cmd} BYPRODUCTS ${prefix}PostBuild.txt)

  get_write_file_command(cmd ${prefix}TargetPostBuild.txt)
  add_custom_target(${prefix}TargetPostBuild)
  add_custom_command(TARGET ${prefix}TargetPostBuild COMMAND ${cmd} BYPRODUCTS ${prefix}TargetPostBuild.txt)

  file(APPEND "${CMAKE_BINARY_DIR}/target_files_custom.cmake"
"set(TARGET_DEPENDS_${prefix}Command [==[${CMAKE_CURRENT_BINARY_DIR}/${prefix}Command.txt]==])
set(TARGET_BYPRODUCTS_${prefix}Target [==[${CMAKE_CURRENT_BINARY_DIR}/${prefix}Target.txt]==])
set(TARGET_BYPRODUCTS_${prefix}PostBuild [==[${CMAKE_CURRENT_BINARY_DIR}/${prefix}PostBuild.txt]==])
set(TARGET_BYPRODUCTS_${prefix}TargetPostBuild [==[${CMAKE_CURRENT_BINARY_DIR}/${prefix}TargetPostBuild.txt]==])
")
endfunction()

add_subdirectory(CustomCommandsAndTargetsSubdir)

create_targets(Top)

include(${CMAKE_CURRENT_LIST_DIR}/Common.cmake)
generate_output_files(TopPostBuild SubdirPostBuild)
file(APPEND "${CMAKE_BINARY_DIR}/target_files.cmake" "include(\${CMAKE_CURRENT_LIST_DIR}/target_files_custom.cmake)\n")
