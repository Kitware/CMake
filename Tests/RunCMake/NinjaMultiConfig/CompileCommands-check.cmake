set(expected_compile_commands
[==[^\[
{
  "directory": "[^
]*(/Tests/RunCMake/NinjaMultiConfig/CompileCommands-build|\\\\Tests\\\\RunCMake\\\\NinjaMultiConfig\\\\CompileCommands-build)",
  "command": "[^
]*Debug[^
]*",
  "file": "[^
]*(/Tests/RunCMake/NinjaMultiConfig/main\.c|\\\\Tests\\\\RunCMake\\\\NinjaMultiConfig\\\\main\.c)",
  "output": "(CMakeFiles/exe\.dir/Debug/main\.c\.o|CMakeFiles\\\\exe\.dir\\\\Debug\\\\main\.c\.obj)"
},
{
  "directory": "[^
]*(/Tests/RunCMake/NinjaMultiConfig/CompileCommands-build|\\\\Tests\\\\RunCMake\\\\NinjaMultiConfig\\\\CompileCommands-build)",
  "command": "[^
]*Release[^
]*",
  "file": "[^
]*(/Tests/RunCMake/NinjaMultiConfig/main\.c|\\\\Tests\\\\RunCMake\\\\NinjaMultiConfig\\\\main\.c)",
  "output": "(CMakeFiles/exe\.dir/Release/main\.c\.o|CMakeFiles\\\\exe\.dir\\\\Release\\\\main\.c\.obj)"
}
]$]==])

file(READ "${RunCMake_TEST_BINARY_DIR}/compile_commands.json" actual_compile_commands)
if(NOT actual_compile_commands MATCHES "${expected_compile_commands}")
  string(REPLACE "\n" "\n  " expected_compile_commands_formatted "${expected_compile_commands}")
  string(REPLACE "\n" "\n  " actual_compile_commands_formatted "${actual_compile_commands}")
  string(APPEND RunCMake_TEST_FAILED "Expected compile_commands.json to match:\n  ${expected_compile_commands_formatted}\nActual compile_commands.json:\n  ${actual_compile_commands_formatted}\n")
endif()
