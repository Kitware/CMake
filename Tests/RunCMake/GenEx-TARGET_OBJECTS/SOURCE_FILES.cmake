enable_language(C)

include(common.cmake)

add_library(c_objs OBJECT EXCLUDE_FROM_ALL empty1.c empty2.c empty3.c)

check_target_objects(single_absolute
  "$<TARGET_OBJECTS:c_objs,SOURCE_FILES:${CMAKE_CURRENT_SOURCE_DIR}/empty2.c>"
  [[/Tests/RunCMake/GenEx-TARGET_OBJECTS/SOURCE_FILES-build((/CMakeFiles)?/c_objs\.dir(/(\.|[a-zA-Z]+))?|/build/c_objs\.build/.*)/empty2(\.c)?\.(o|obj)$]]
  )
check_target_objects(multi
  "$<TARGET_OBJECTS:c_objs,SOURCE_FILES:${CMAKE_CURRENT_SOURCE_DIR}/empty1.c;${CMAKE_CURRENT_SOURCE_DIR}/empty2.c>"
  [[/Tests/RunCMake/GenEx-TARGET_OBJECTS/SOURCE_FILES-build((/CMakeFiles)?/c_objs\.dir(/(\.|[a-zA-Z]+))?|/build/c_objs\.build/.*)/empty1(\.c)?\.(o|obj)$]]
  [[/Tests/RunCMake/GenEx-TARGET_OBJECTS/SOURCE_FILES-build((/CMakeFiles)?/c_objs\.dir(/(\.|[a-zA-Z]+))?|/build/c_objs\.build/.*)/empty2(\.c)?\.(o|obj)$]]
  )
check_target_objects(multi_reordered
  "$<TARGET_OBJECTS:c_objs,SOURCE_FILES:${CMAKE_CURRENT_SOURCE_DIR}/empty3.c;${CMAKE_CURRENT_SOURCE_DIR}/empty2.c>"
  [[/Tests/RunCMake/GenEx-TARGET_OBJECTS/SOURCE_FILES-build((/CMakeFiles)?/c_objs\.dir(/(\.|[a-zA-Z]+))?|/build/c_objs\.build/.*)/empty2(\.c)?\.(o|obj)$]]
  [[/Tests/RunCMake/GenEx-TARGET_OBJECTS/SOURCE_FILES-build((/CMakeFiles)?/c_objs\.dir(/(\.|[a-zA-Z]+))?|/build/c_objs\.build/.*)/empty3(\.c)?\.(o|obj)$]]
  )
check_target_objects(multi_duplicate
  "$<TARGET_OBJECTS:c_objs,SOURCE_FILES:${CMAKE_CURRENT_SOURCE_DIR}/empty1.c;${CMAKE_CURRENT_SOURCE_DIR}/empty3.c;${CMAKE_CURRENT_SOURCE_DIR}/empty1.c>"
  [[/Tests/RunCMake/GenEx-TARGET_OBJECTS/SOURCE_FILES-build((/CMakeFiles)?/c_objs\.dir(/(\.|[a-zA-Z]+))?|/build/c_objs\.build/.*)/empty1(\.c)?\.(o|obj)$]]
  [[/Tests/RunCMake/GenEx-TARGET_OBJECTS/SOURCE_FILES-build((/CMakeFiles)?/c_objs\.dir(/(\.|[a-zA-Z]+))?|/build/c_objs\.build/.*)/empty3(\.c)?\.(o|obj)$]]
  )

add_subdirectory(SOURCE_FILES_subdir)

if(CMake_TEST_CUDA STREQUAL "NVIDIA")
  enable_language(CUDA)

  include(CheckIPOSupported)
  check_ipo_supported(RESULT ipo_supported LANGUAGES CUDA)

  if(ipo_supported)
    add_library(fatbin_objs OBJECT EXCLUDE_FROM_ALL empty1.cu empty2.cu)
    set_target_properties(fatbin_objs PROPERTIES
      CUDA_SEPARABLE_COMPILATION ON
      CUDA_FATBIN_COMPILATION ON
      POSITION_INDEPENDENT_CODE ON
      INTERPROCEDURAL_OPTIMIZATION ON
      )

    check_target_objects(fatbin_1
      "$<TARGET_OBJECTS:fatbin_objs,SOURCE_FILES:${CMAKE_CURRENT_SOURCE_DIR}/empty1.cu>"
      [[/Tests/RunCMake/GenEx-TARGET_OBJECTS/SOURCE_FILES-build((/CMakeFiles)?/fatbin_objs\.dir(/(\.|[a-zA-Z]+))?|/build/fatbin_objs\.build/.*)/empty1(\.cu\.o)?\.fatbin$]]
      )
    check_target_objects(fatbin_2
      "$<TARGET_OBJECTS:fatbin_objs,SOURCE_FILES:${CMAKE_CURRENT_SOURCE_DIR}/empty2.cu>"
      [[/Tests/RunCMake/GenEx-TARGET_OBJECTS/SOURCE_FILES-build((/CMakeFiles)?/fatbin_objs\.dir(/(\.|[a-zA-Z]+))?|/build/fatbin_objs\.build/.*)/empty2(\.cu\.o)?\.fatbin$]]
      )
  endif()
endif()
