# Figure out which build tool the test files in a project are using
macro(get_build_tools_from_project_file projectFile)
  set(_s "[ \t\r\n]")   # Whitespace character class

  set(ItemGroupBeginRegex "<${_s}*ItemGroup${_s}*>")
  set(ItemGroupEndRegex "</${_s}*ItemGroup${_s}*>")
  set(GroupItemRegex ".*<${_s}*([A-Za-z0-9_]+)${_s}+Include${_s}*=${_s}*\"([^\"]*)\".*")

  if(NOT EXISTS "${projectFile}")
    set(RunCMake_TEST_FAILED "Project file ${projectFile} does not exist.")
    return()
  endif()

  file(STRINGS "${projectFile}" lines)

  foreach(line IN LISTS lines)
    if(line MATCHES "${ItemGroupBeginRegex}")
      set(InItemGroup TRUE)
    elseif(line MATCHES "${ItemGroupEndRegex}")
      set(InItemGroup FALSE)
    elseif(line MATCHES "${GroupItemRegex}")
      if(InItemGroup)
        string(REGEX REPLACE "${GroupItemRegex}" "\\1" itemTool "${line}")
        string(REGEX REPLACE "${GroupItemRegex}" "\\2" itemPath "${line}")

        if(itemPath MATCHES ".*foo\\.cpp")
          set(fooCppTool "${itemTool}")
        elseif(itemPath MATCHES ".*foo\\.txt")
          set(fooTxtTool "${itemTool}")
        elseif(itemPath MATCHES ".*bar\\.cpp")
          set(barCppTool "${itemTool}")
        elseif(itemPath MATCHES ".*bar\\.txt")
          set(barTxtTool "${itemTool}")
        endif()
      endif()
    endif()
  endforeach()
endmacro()

# Verify a build tool is as expected
macro(verify_build_tool fileName expectedBuildTool actualBuildTool)
  if("${actualBuildTool}" STREQUAL "${expectedBuildTool}")
    message(STATUS "File '${fileName}' in project file '${projectFile}' has expected build tool '${expectedBuildTool}'")
  else()
    set(RunCMake_TEST_FAILED "File '${fileName}' in project file '${projectFile}' has unexpected build tool '${actualBuildTool}'! Expected: '${expectedBuildTool}'" PARENT_SCOPE)
    return()
  endif()
endmacro()

# Test using VS_TOOL_OVERRIDE
block()
  set(projectFile "${RunCMake_TEST_BINARY_DIR}/foo.vcxproj")
  get_build_tools_from_project_file("${projectFile}")
  verify_build_tool("foo.cpp" "CustomFooCppTool" "${fooCppTool}")
  verify_build_tool("foo.txt" "CustomFooTxtTool" "${fooTxtTool}")
endblock()

# Test default behavior without using VS_TOOL_OVERRIDE
block()
  set(projectFile "${RunCMake_TEST_BINARY_DIR}/bar.vcxproj")
  get_build_tools_from_project_file("${projectFile}")
  verify_build_tool("bar.cpp" "ClCompile" "${barCppTool}")
  verify_build_tool("bar.txt" "None" "${barTxtTool}")
endblock()
