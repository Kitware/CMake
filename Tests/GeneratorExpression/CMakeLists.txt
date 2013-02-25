cmake_minimum_required (VERSION 2.8.8)
project(GeneratorExpression CXX)

add_custom_target(check-part1 ALL
  COMMAND ${CMAKE_COMMAND}
    -Dtest_0=$<0:nothing>
    -Dtest_0_with_comma=$<0:-Wl,--no-undefined>
    -Dtest_1=$<1:content>
    -Dtest_1_with_comma=$<1:-Wl,--no-undefined>
    -Dconfig=$<CONFIGURATION>
    -Dtest_and_0=$<AND:0>
    -Dtest_and_0_0=$<AND:0,0>
    -Dtest_and_0_1=$<AND:0,1>
    -Dtest_and_1=$<AND:1>
    -Dtest_and_1_0=$<AND:1,0>
    -Dtest_and_1_1=$<AND:1,1>
    # Ordinarily, the 'invalidcontent' would cause an error, but
    # the '0' makes the AND abort early.
    -Dtest_and_0_invalidcontent=$<AND:0,invalidcontent>
    -Dtest_config_0=$<CONFIG:$<CONFIGURATION>x>
    -Dtest_config_1=$<CONFIG:$<CONFIGURATION>>
    -Dtest_config_debug=$<CONFIG:Debug>$<CONFIG:DEBUG>$<CONFIG:DeBuG>
    -Dtest_config_release=$<CONFIG:Release>$<CONFIG:RELEASE>$<CONFIG:ReLeAsE>
    -Dtest_config_relwithdebinfo=$<CONFIG:RelWithDebInfo>$<CONFIG:RELWITHDEBINFO>$<CONFIG:relwithdebinfo>
    -Dtest_config_minsizerel=$<CONFIG:MinSizeRel>$<CONFIG:MINSIZEREL>$<CONFIG:minsizerel>
    -Dtest_not_0=$<NOT:0>
    -Dtest_not_1=$<NOT:1>
    -Dtest_or_0=$<OR:0>
    -Dtest_or_0_0=$<OR:0,0>
    -Dtest_or_0_1=$<OR:0,1>
    -Dtest_or_1=$<OR:1>
    -Dtest_or_1_0=$<OR:1,0>
    -Dtest_or_1_1=$<OR:1,1>
    -Dtest_or_1_invalidcontent=$<OR:1,invalidcontent>
    -Dtest_bool_notfound=$<BOOL:NOTFOUND>
    -Dtest_bool_foo_notfound=$<BOOL:Foo-NOTFOUND>
    -Dtest_bool_true=$<BOOL:True>
    -Dtest_bool_false=$<BOOL:False>
    -Dtest_bool_on=$<BOOL:On>
    -Dtest_bool_off=$<BOOL:Off>
    -Dtest_bool_no=$<BOOL:No>
    -Dtest_bool_n=$<BOOL:N>
    -Dtest_bool_empty=$<BOOL:>
    -Dtest_strequal_yes_yes=$<STREQUAL:Yes,Yes>
    -Dtest_strequal_yes_yes_cs=$<STREQUAL:Yes,yes>
    -Dtest_strequal_yes_no=$<STREQUAL:Yes,No>
    -Dtest_strequal_no_yes=$<STREQUAL:No,Yes>
    -Dtest_strequal_angle_r=$<STREQUAL:$<ANGLE-R>,$<ANGLE-R>>
    -Dtest_strequal_comma=$<STREQUAL:$<COMMA>,$<COMMA>>
    -Dtest_strequal_angle_r_comma=$<STREQUAL:$<ANGLE-R>,$<COMMA>>
    -Dtest_strequal_both_empty=$<STREQUAL:,>
    -Dtest_strequal_one_empty=$<STREQUAL:something,>
    -Dtest_angle_r=$<ANGLE-R>
    -Dtest_comma=$<COMMA>
    -Dtest_colons_1=$<1::>
    -Dtest_colons_2=$<1:::>
    -Dtest_colons_3=$<1:Qt5::Core>
    -Dtest_colons_4=$<1:C:\\CMake>
    -Dtest_colons_5=$<1:C:/CMake>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/check-part1.cmake
  COMMAND ${CMAKE_COMMAND} -E echo "check done (part 1 of 2)"
  VERBATIM
  )

add_library(empty1 empty.cpp)
target_include_directories(empty1 PUBLIC /empty1/public)
target_include_directories(empty1 PRIVATE /empty1/private)

add_library(empty2 empty.cpp)
target_include_directories(empty2 PUBLIC /empty2/public)

add_library(empty3 empty.cpp)
target_include_directories(empty3 PUBLIC /empty3/public)
target_include_directories(empty3 PRIVATE /empty3/private)

add_library(empty4 empty.cpp)
target_include_directories(empty4 PUBLIC /empty4/public)

target_link_libraries(empty1 LINK_PUBLIC empty2)
target_link_libraries(empty2 LINK_PUBLIC empty3 empty4)
target_link_libraries(empty3 LINK_PUBLIC empty2 empty4)

add_library(empty5 empty.cpp)
target_include_directories(empty5 PRIVATE /empty5/private1 /empty5/private2)

add_custom_target(check-part2 ALL
  COMMAND ${CMAKE_COMMAND}
    -Dtest_incomplete_1=$<
    -Dtest_incomplete_2=$<something
    -Dtest_incomplete_3=$<something:
    -Dtest_incomplete_4=$<something:,
    -Dtest_incomplete_5=$something:,>
    -Dtest_incomplete_6=<something:,>
    -Dtest_incomplete_7=$<something::
    -Dtest_incomplete_8=$<something:,
    -Dtest_incomplete_9=$<something:,,
    -Dtest_incomplete_10=$<something:,:
    -Dtest_incomplete_11=$<something,,
    -Dtest_incomplete_12=$<,,
    -Dtest_incomplete_13=$<some$<1:special>thing
    -Dtest_incomplete_14=$<$<ANGLE-R>
    -Dtest_incomplete_15=$<some$<thing
    -Dtest_incomplete_16=$<BOOL:something
    -Dtest_incomplete_17=$<1:some$thing>
    -Dtest_incomplete_18=$<1:some,thing
    -Dtest_incomplete_19=$<1:some,thing$<ANGLE-R>
    -Dtest_incomplete_20=$<CONFIGURATION$<ANGLE-R>
    -Dtest_incomplete_21=$<BOOL:something$<ANGLE-R>
    -Dtest_build_interface=$<BUILD_INTERFACE:build>
    -Dtest_install_interface=$<INSTALL_INTERFACE:install>
    -Dtest_target_name_1=$<TARGET_NAME:tgt,ok>
    -Dtest_target_name_2=$<TARGET_NAME:tgt:ok>
    -Dtest_target_includes1=$<TARGET_PROPERTY:empty1,INTERFACE_INCLUDE_DIRECTORIES>
    -Dtest_target_includes2=$<TARGET_PROPERTY:empty2,INTERFACE_INCLUDE_DIRECTORIES>
    -Dtest_target_includes3=$<TARGET_PROPERTY:empty3,INTERFACE_INCLUDE_DIRECTORIES>
    -Dtest_target_includes4=$<TARGET_PROPERTY:empty1,INCLUDE_DIRECTORIES>
    -Dtest_target_includes5=$<TARGET_PROPERTY:empty2,INCLUDE_DIRECTORIES>
    -Dtest_target_includes6=$<TARGET_PROPERTY:empty3,INCLUDE_DIRECTORIES>
    -Dtest_target_includes7=$<TARGET_PROPERTY:empty1,INTERFACE_INCLUDE_DIRECTORIES>
    -Dtest_target_includes8=$<TARGET_PROPERTY:empty5,INCLUDE_DIRECTORIES>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/check-part2.cmake
  COMMAND ${CMAKE_COMMAND} -E echo "check done (part 2 of 2)"
  VERBATIM
  )
