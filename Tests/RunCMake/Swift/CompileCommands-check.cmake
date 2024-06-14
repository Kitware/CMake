if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/compile_commands.json")
  set(RunCMake_TEST_FAILED "compile_commands.json not generated")
  return()
endif()

# The compile command for both files should contain all Swift source files in
# the module
set(expected_compile_commands
[==[^\[
{
  "directory": ".*(/Tests/RunCMake/Swift/CompileCommands-build|\\\\Tests\\\\RunCMake\\\\Swift\\\\CompileCommands-build)",
  "command": ".*swiftc .* (\\")?.*(/Tests/RunCMake/Swift/E.swift|\\\\Tests\\\\RunCMake\\\\Swift\\\\E.swift)(\\")? (\\")?.*(/Tests/RunCMake/Swift/L.swift|\\\\Tests\\\\RunCMake\\\\Swift\\\\L.swift)(\\")?",
  "file": ".*(/Tests/RunCMake/Swift/E.swift|\\\\Tests\\\\RunCMake\\\\Swift\\\\E.swift)",
  "output": "CMakeFiles/CompileCommandLib.dir/E.swift.o|CMakeFiles\\\\CompileCommandLib.dir\\\\E.swift.obj"
},
{
  "directory": ".*(/Tests/RunCMake/Swift/CompileCommands-build|\\\\Tests\\\\RunCMake\\\\Swift\\\\CompileCommands-build)",
  "command": ".*swiftc .* (\\")?.*(/Tests/RunCMake/Swift/E.swift|\\\\Tests\\\\RunCMake\\\\Swift\\\\E.swift)(\\")? (\\")?.*(/Tests/RunCMake/Swift/L.swift|\\\\Tests\\\\RunCMake\\\\Swift\\\\L.swift)(\\")?",
  "file": ".*/Tests/RunCMake/Swift/L.swift",
  "output": "CMakeFiles/CompileCommandLib.dir/L.swift.o|CMakeFiles\\\\CompileCommandLib.dir\\\\L.swift.obj"
}
]$]==]
)

file(READ "${RunCMake_TEST_BINARY_DIR}/compile_commands.json" compile_commands)
if(NOT compile_commands MATCHES "${expected_compile_commands}")
  string(REPLACE "\n" "\n  " expected_compile_commands_formatted "${expected_compile_commands}")
  string(REPLACE "\n" "\n  " compile_commands_formatted "${compile_commands}")
  string(APPEND RunCMake_TEST_FAILED "Expected compile_commands.json to match:\n  ${expected_compile_commands_formatted}\nActual compile_commands.json:\n  ${compile_commands_formatted}\n")
endif()
