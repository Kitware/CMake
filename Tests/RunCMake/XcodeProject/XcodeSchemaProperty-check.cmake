function(check_property property matcher)
  set(schema "${RunCMake_TEST_BINARY_DIR}/XcodeSchemaProperty.xcodeproj/xcshareddata/xcschemes/${property}.xcscheme")
  file(READ ${schema} schema-content-${property})
  string(REGEX MATCHALL "${matcher}" matched-${property} ${schema-content-${property}})
  if(NOT matched-${property})
    string(APPEND RunCMake_TEST_FAILED
      "Xcode schema property ${property}: Could not find\n"
      "  ${matcher}\n"
      "in schema\n"
      "  ${schema}\n"
    )
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
  endif()
endfunction()

function(check_property_count property matcher expected_count)
  set(schema "${RunCMake_TEST_BINARY_DIR}/XcodeSchemaProperty.xcodeproj/xcshareddata/xcschemes/${property}.xcscheme")
  file(STRINGS ${schema} actual-${property}
       REGEX "${matcher}")
  if(NOT actual-${property})
    string(APPEND RunCMake_TEST_FAILED
      "Xcode schema property ${property}: Could not find\n"
      "  ${matcher}\n"
      "in schema\n"
      "  ${schema}\n"
    )
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
    return()
  endif()
  list(LENGTH actual-${property} match_count)
  if(NOT ${expected_count} EQUAL ${match_count})
    string(APPEND RunCMake_TEST_FAILED
      "Xcode schema property ${property}: Expected ${expected_count} matches of\n"
      "  ${matcher}\n"
      "in schema\n"
      "  ${schema}\n"
      "but found ${match_count}.\n\n"
    )
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
  endif()
endfunction()

function(check_no_property property matcher)
  set(schema "${RunCMake_TEST_BINARY_DIR}/XcodeSchemaProperty.xcodeproj/xcshareddata/xcschemes/${property}.xcscheme")
  file(STRINGS ${schema} actual-${property}
       REGEX "${matcher}" LIMIT_COUNT 1)
  if(actual-${property})
    string(APPEND RunCMake_TEST_FAILED
      "Xcode schema property ${property}: Found\n"
      "  ${matcher}\n"
      "which is not expected in schema\n"
      "  ${schema}\n"
    )
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
  endif()
endfunction()

function(expect_schema target)
  set(schema "${RunCMake_TEST_BINARY_DIR}/XcodeSchemaProperty.xcodeproj/xcshareddata/xcschemes/${target}.xcscheme")
  if(NOT EXISTS ${schema})
    string(APPEND RunCMake_TEST_FAILED
      "Missing schema for target ${target}\n"
    )
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
  endif()
endfunction()

function(expect_no_schema target)
  set(schema "${RunCMake_TEST_BINARY_DIR}/XcodeSchemaProperty.xcodeproj/xcshareddata/xcschemes/${target}.xcscheme")
  if(EXISTS ${schema})
    string(APPEND RunCMake_TEST_FAILED
      "Found unexpected schema\n"
      "  ${schema}\n"
    )
    set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
  endif()
endfunction()

check_property("ADDRESS_SANITIZER" "enableAddressSanitizer")
check_property("ADDRESS_SANITIZER_USE_AFTER_RETURN" "enableASanStackUseAfterReturn")
check_property("THREAD_SANITIZER" "enableThreadSanitizer")
check_property("THREAD_SANITIZER_STOP" "stopOnEveryThreadSanitizerIssue")
check_property("UNDEFINED_BEHAVIOUR_SANITIZER" "enableUBSanitizer")
check_property("UNDEFINED_BEHAVIOUR_SANITIZER_STOP" "stopOnEveryUBSanitizerIssue")
check_property("DISABLE_MAIN_THREAD_CHECKER" "disableMainThreadChecker")
check_property("MAIN_THREAD_CHECKER_STOP" "stopOnEveryMainThreadCheckerIssue")
check_property("DISABLE_XCODE_SCHEME_ENABLE_GPU_API_VALIDATION" "enableGPUValidationMode")
check_property("ENABLE_XCODE_SCHEME_ENABLE_GPU_SHADER_VALIDATION" "enableGPUShaderValidationMode")

check_property("MALLOC_SCRIBBLE" "MallocScribble")
check_property("MALLOC_GUARD_EDGES" "MallocGuardEdges")
check_property("GUARD_MALLOC" "DYLD_INSERT_LIBRARIES")
check_property("ZOMBIE_OBJECTS" "NSZombieEnabled")
check_property("MALLOC_STACK" "MallocStackLogging")
check_property("DYNAMIC_LINKER_API_USAGE" "DYLD_PRINT_APIS")
check_property("DYNAMIC_LIBRARY_LOADS" "DYLD_PRINT_LIBRARIES")
check_property("ENABLE_GPU_FRAME_CAPTURE_MODE_1" "enableGPUFrameCaptureMode=\"1\"")
check_property("ENABLE_GPU_FRAME_CAPTURE_MODE_3" "enableGPUFrameCaptureMode=\"3\"")
check_property("ENABLE_GPU_FRAME_CAPTURE_MODE_DISABLED" "enableGPUFrameCaptureMode=\"3\"")
check_property("ENABLE_GPU_FRAME_CAPTURE_MODE_METAL" "enableGPUFrameCaptureMode=\"1\"")
check_property("ENABLE_GPU_FRAME_CAPTURE_MODE_DISABLED_MIXED_CASE" "enableGPUFrameCaptureMode=\"3\"")
check_property("ENABLE_GPU_FRAME_CAPTURE_MODE_METAL_MIXED_CASE" "enableGPUFrameCaptureMode=\"1\"")
check_property("LAUNCH_MODE_AUTO" "launchStyle=\"0\"")
check_property("LAUNCH_MODE_WAIT" "launchStyle=\"1\"")
check_property("LAUNCH_CONFIGURATION_EMPTY" "<LaunchAction.*buildConfiguration=\"Debug\".*</LaunchAction>")
check_property("LAUNCH_CONFIGURATION_DEBUG" "<LaunchAction.*buildConfiguration=\"Debug\".*</LaunchAction>")
check_property("LAUNCH_CONFIGURATION_RELEASE" "<LaunchAction.*buildConfiguration=\"Release\".*</LaunchAction>")
check_property("TEST_CONFIGURATION_EMPTY" "<TestAction.*buildConfiguration=\"Debug\".*</TestAction>")
check_property("TEST_CONFIGURATION_DEBUG" "<TestAction.*buildConfiguration=\"Debug\".*</TestAction>")
check_property("TEST_CONFIGURATION_RELEASE" "<TestAction.*buildConfiguration=\"Release\".*</TestAction>")
check_no_property("LLDB_INIT_FILE_EMPTY" "customLLDBInitFile")
check_property_count("LLDB_INIT_FILE_EVAL" "customLLDBInitFile=\"${RunCMake_TEST_BINARY_DIR}/.lldbinit\"" 2)
check_property_count("LLDB_INIT_FILE_FULL" "customLLDBInitFile=\"/full/path/to/.lldbinit\"" 2)

check_property("EXECUTABLE" "myExecutable")
check_property("ARGUMENTS" [=["--foo"]=])
check_property("ARGUMENTS" [=["--bar=baz"]=])
check_property("ENVIRONMENT" [=[key="FOO"]=])
check_property("ENVIRONMENT" [=[value="foo"]=])
check_property("ENVIRONMENT" [=[key="BAR"]=])
check_property("ENVIRONMENT" [=[value="bar"]=])
check_property("WORKING_DIRECTORY" [=["/working/dir"]=])

expect_no_schema("NoSchema")

expect_schema("CustomTarget")
expect_schema("ALL_BUILD")
