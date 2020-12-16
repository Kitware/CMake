enable_language(C)

add_executable(echo echo.c)

add_custom_command(
  OUTPUT echo_raw_$<CONFIG>.txt
  COMMAND echo $<CONFIG> echo_raw_$<CONFIG>.txt
  WORKING_DIRECTORY $<CONFIG>
  )

add_custom_command(
  OUTPUT echo_genex_$<IF:$<CONFIG:Debug>,Debug,$<IF:$<CONFIG:Release>,Release,$<IF:$<CONFIG:MinSizeRel>,MinSizeRel,RelWithDebInfo>>>.txt
  COMMAND $<TARGET_FILE:echo> $<COMMAND_CONFIG:$<CONFIG>> echo_genex_$<OUTPUT_CONFIG:$<CONFIG>>.txt
  WORKING_DIRECTORY $<OUTPUT_CONFIG:$<CONFIG>>
  )

add_custom_command(
  OUTPUT echo_genex_out_$<CONFIG>.txt
  COMMAND $<OUTPUT_CONFIG:$<TARGET_FILE:echo>> $<CONFIG> echo_genex_out_$<CONFIG>.txt
  WORKING_DIRECTORY $<COMMAND_CONFIG:$<CONFIG>>
  )

add_custom_command(
  OUTPUT depend_$<CONFIG>.txt
  COMMAND ${CMAKE_COMMAND} -E echo depend_$<CONFIG>.txt
  )

add_custom_command(
  OUTPUT echo_depend_$<CONFIG>.txt
  COMMAND ${CMAKE_COMMAND} -E echo echo_depend_$<CONFIG>.txt
  DEPENDS depend_$<CONFIG>.txt
  )

add_custom_command(
  OUTPUT echo_depend_out_$<CONFIG>.txt
  COMMAND ${CMAKE_COMMAND} -E echo echo_depend_out_$<CONFIG>.txt
  DEPENDS $<OUTPUT_CONFIG:depend_$<CONFIG>.txt>
  )

add_custom_command(
  OUTPUT echo_depend_cmd_$<CONFIG>.txt
  COMMAND ${CMAKE_COMMAND} -E echo echo_depend_cmd_$<CONFIG>.txt
  DEPENDS $<COMMAND_CONFIG:depend_$<CONFIG>.txt>
  )

add_custom_command(
  OUTPUT depend_echo_raw_$<CONFIG>.txt
  COMMAND ${CMAKE_COMMAND} -E echo depend_echo_raw_$<CONFIG>.txt
  DEPENDS echo
  )

add_custom_command(
  OUTPUT depend_echo_genex_$<CONFIG>.txt
  COMMAND ${CMAKE_COMMAND} -E echo depend_echo_genex_$<CONFIG>.txt
  DEPENDS $<TARGET_FILE:echo>
  )

add_custom_command(
  OUTPUT depend_echo_genex_out_$<CONFIG>.txt
  COMMAND ${CMAKE_COMMAND} -E echo depend_echo_genex_out_$<CONFIG>.txt
  DEPENDS $<OUTPUT_CONFIG:$<TARGET_FILE:echo>>
  )

add_custom_command(
  OUTPUT depend_echo_genex_cmd_$<CONFIG>.txt
  COMMAND ${CMAKE_COMMAND} -E echo depend_echo_genex_cmd_$<CONFIG>.txt
  DEPENDS $<COMMAND_CONFIG:$<TARGET_FILE:echo>>
  )

# An OUTPUT that is not per-config prevents cross-config generation.
add_custom_command(
  OUTPUT echo_no_cross_output.txt echo_no_cross_output_$<CONFIG>.txt
  COMMAND echo $<CONFIG> echo_no_cross_output_$<CONFIG>.txt
  WORKING_DIRECTORY $<CONFIG>
  )
add_custom_command(
  OUTPUT echo_no_cross_output_$<IF:$<CONFIG:Debug>,a,b>.txt echo_no_cross_output_if_$<CONFIG>.txt
  COMMAND echo $<CONFIG> echo_no_cross_output_if_$<CONFIG>.txt
  WORKING_DIRECTORY $<CONFIG>
  )

# BYPRODUCTS that are not per-config prevent cross-config generation.
add_custom_command(
  OUTPUT echo_no_cross_byproduct_$<CONFIG>.txt
  BYPRODUCTS echo_no_cross_byproduct.txt
  COMMAND echo $<CONFIG> echo_no_cross_byproduct_$<CONFIG>.txt
  WORKING_DIRECTORY $<CONFIG>
  )
add_custom_command(
  OUTPUT echo_no_cross_byproduct_if_$<CONFIG>.txt
  BYPRODUCTS echo_no_cross_byproduct_$<IF:$<CONFIG:Debug>,a,b>.txt
  COMMAND echo $<CONFIG> echo_no_cross_byproduct_if_$<CONFIG>.txt
  WORKING_DIRECTORY $<CONFIG>
  )

foreach(case
    echo_raw
    echo_genex
    echo_genex_out
    echo_depend
    echo_depend_out
    echo_depend_cmd
    depend
    depend_echo_raw
    depend_echo_genex
    depend_echo_genex_out
    depend_echo_genex_cmd
    echo_no_cross_output
    echo_no_cross_output_if
    echo_no_cross_byproduct
    echo_no_cross_byproduct_if
    )
  set_property(SOURCE
    ${CMAKE_CURRENT_BINARY_DIR}/${case}_Debug.txt
    ${CMAKE_CURRENT_BINARY_DIR}/${case}_Release.txt
    ${CMAKE_CURRENT_BINARY_DIR}/${case}_MinSizeRel.txt
    ${CMAKE_CURRENT_BINARY_DIR}/${case}_RelWithDebInfo.txt
    PROPERTY SYMBOLIC 1)
  add_custom_target(${case} DEPENDS ${case}_$<CONFIG>.txt)
endforeach()

add_custom_target(echo_target_raw
  BYPRODUCTS echo_target_raw_$<CONFIG>.txt
  COMMENT echo_target_raw
  COMMAND echo $<CONFIG> echo_target_raw_$<CONFIG>.txt
  WORKING_DIRECTORY $<CONFIG>
  )

add_custom_target(echo_target_genex
  BYPRODUCTS echo_target_genex_$<CONFIG>.txt
  COMMENT echo_target_genex
  COMMAND $<TARGET_FILE:echo> $<COMMAND_CONFIG:$<CONFIG>> echo_target_genex_$<OUTPUT_CONFIG:$<CONFIG>>.txt
  WORKING_DIRECTORY $<OUTPUT_CONFIG:$<CONFIG>>
  )

add_custom_target(echo_target_genex_out
  BYPRODUCTS echo_target_genex_out_$<CONFIG>.txt
  COMMENT echo_target_genex_out
  COMMAND $<OUTPUT_CONFIG:$<TARGET_FILE:echo>> $<CONFIG> echo_target_genex_out_$<CONFIG>.txt
  WORKING_DIRECTORY $<COMMAND_CONFIG:$<CONFIG>>
  )

add_custom_target(echo_target_depend
  COMMAND ${CMAKE_COMMAND} -E echo echo_target_depend_$<CONFIG>.txt
  DEPENDS depend_$<CONFIG>.txt
  COMMENT echo_target_depend
  )

add_custom_target(echo_target_depend_out
  COMMAND ${CMAKE_COMMAND} -E echo echo_target_depend_out_$<CONFIG>.txt
  DEPENDS $<OUTPUT_CONFIG:depend_$<CONFIG>.txt>
  COMMENT echo_target_depend_out
  )

add_custom_target(echo_target_depend_cmd
  COMMAND ${CMAKE_COMMAND} -E echo echo_target_depend_cmd_$<CONFIG>.txt
  DEPENDS $<COMMAND_CONFIG:depend_$<CONFIG>.txt>
  COMMENT echo_target_depend_cmd
  )

# BYPRODUCTS that are not per-config block cross-configs.
add_custom_target(target_no_cross_byproduct
  BYPRODUCTS target_no_cross_byproduct.txt
  COMMENT target_no_cross_byproduct
  COMMAND echo $<CONFIG> target_no_cross_byproduct.txt
  WORKING_DIRECTORY $<CONFIG>
  )
