macro(ensure_props_set projectFile)
  if(NOT EXISTS "${projectFile}")
    set(RunCMake_TEST_FAILED "Project file ${projectFile} does not exist.")
    return()
  endif()

  set(SettingFound FALSE)

  file(STRINGS "${projectFile}" lines)
  foreach(line IN LISTS lines)
    if(line MATCHES "<SourceProperty1.*Debug.*>SourceProperty1Value</SourceProperty1>")
      message("SourceProperty1 setting found")
      set(SettingFound TRUE)
    endif()
  endforeach()

  if (NOT SettingFound)
    set(RunCMake_TEST_FAILED "SourceProperty1 setting was not found")
    return()
  endif()
endmacro()

ensure_props_set("${RunCMake_TEST_BINARY_DIR}/foo.vcxproj")
