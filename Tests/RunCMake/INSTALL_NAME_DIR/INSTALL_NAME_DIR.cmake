function(add_install_name_dir_libraries install_name_dir)
  add_library(build_dir SHARED test.c)
  add_library(install_dir SHARED test.c)
  if(NOT install_name_dir STREQUAL "NONE")
    set_target_properties(build_dir install_dir PROPERTIES
      INSTALL_NAME_DIR "${install_name_dir}"
      )
  endif()
  set_target_properties(install_dir PROPERTIES
    BUILD_WITH_INSTALL_NAME_DIR TRUE
    )
  install(TARGETS build_dir install_dir EXPORT InstallNameDirTest DESTINATION lib)
  install(EXPORT InstallNameDirTest DESTINATION lib/cmake/InstallNameDirTest FILE InstallNameDirTest-targets.cmake)
  file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/targets.txt" CONTENT "$<TARGET_FILE:build_dir>\n$<TARGET_FILE:install_dir>\n" CONDITION $<CONFIG:Debug>)
endfunction()
