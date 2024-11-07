# Check whether the 'BuildInParallel' setting is set as expected for a specified project file.
# Note: if the setting is not present in the project file then it is assumed to be implicitly 'false'.
function(check_build_in_parallel_setting projectFile expectedEnabled)
  set(SettingEnabledRegex "<BuildInParallel.*>true</BuildInParallel>")
  set(SettingDisabledRegex "<BuildInParallel.*>false</BuildInParallel>")

  if(NOT EXISTS "${projectFile}")
    set(RunCMake_TEST_FAILED "Project file '${projectFile}' does not exist." PARENT_SCOPE)
    return()
  endif()

  set(settingEnabled FALSE)
  set(settingExplicitlyDisabled FALSE)

  file(STRINGS "${projectFile}" lines)

  foreach(line IN LISTS lines)
    if(line MATCHES "${SettingEnabledRegex}")
      set(settingEnabled TRUE)
    elseif(line MATCHES "${SettingDisabledRegex}")
      set(settingExplicitlyDisabled TRUE)
    endif()
  endforeach()

  if(expectedEnabled)
    if(NOT settingEnabled)
      set(RunCMake_TEST_FAILED "Expected 'BuildInParallel' to be enabled for projectFile '${projectFile}' but it was not!" PARENT_SCOPE)
    endif()
    if(settingExplicitlyDisabled)
      set(RunCMake_TEST_FAILED "Expected 'BuildInParallel' to be enabled for projectFile '${projectFile}' but instead found it explicitly disabled!" PARENT_SCOPE)
    endif()
  else()
    if(settingEnabled)
      set(RunCMake_TEST_FAILED "Expected 'BuildInParallel' to be disabled for projectFile '${projectFile}' but it was not!")
    endif()
  endif()
endfunction()

check_build_in_parallel_setting("${RunCMake_TEST_BINARY_DIR}/foo1.vcxproj" TRUE)
check_build_in_parallel_setting("${RunCMake_TEST_BINARY_DIR}/bar1.vcxproj" FALSE)
check_build_in_parallel_setting("${RunCMake_TEST_BINARY_DIR}/foo2.vcxproj" FALSE)
check_build_in_parallel_setting("${RunCMake_TEST_BINARY_DIR}/bar2.vcxproj" FALSE)
