if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/compile_commands.json")
  set(RunCMake_TEST_FAILED "compile_commands.json not generated")
  return()
endif()

set(ESCAPED_BINARY_DIR [==[[^
]*/Tests/RunCMake/Swift/CompileCommands-build]==])
set(E_SOURCE_PATH [==[(\\")?[^
]*(/Tests/RunCMake/Swift/E.swift|\\\\Tests\\\\RunCMake\\\\Swift\\\\E.swift)(\\")?]==])
set(L_SOURCE_PATH [==[(\\")?[^
]*(/Tests/RunCMake/Swift/L.swift|\\\\Tests\\\\RunCMake\\\\Swift\\\\L.swift)(\\")?]==])

# The compile command for both files should contain all Swift source files in
# the module
set(expected_compile_commands
[==[^\[
{
  "directory": "${BINARY_DIR}",
  "command": "[^
]*swiftc[^
]* ${E_SOURCE_PATH} ${L_SOURCE_PATH}",
  "file": "[^
]*/Tests/RunCMake/Swift/E.swift",
  "output": "[^
]*/CMakeFiles/CompileCommandLib.dir/(Debug/)?E.swift.(o|obj)"
},
{
  "directory": "${BINARY_DIR}",
  "command": "[^
]*swiftc[^
]* ${E_SOURCE_PATH} ${L_SOURCE_PATH}",
  "file": "[^
]*/Tests/RunCMake/Swift/L.swift",
  "output": "[^
]*/CMakeFiles/CompileCommandLib.dir/(Debug/)?L.swift.(o|obj)"
}]==]
)

string(REPLACE [=[${BINARY_DIR}]=] "${ESCAPED_BINARY_DIR}" expected_compile_commands "${expected_compile_commands}")
string(REPLACE [=[${E_SOURCE_PATH}]=] "${E_SOURCE_PATH}" expected_compile_commands "${expected_compile_commands}")
string(REPLACE [=[${L_SOURCE_PATH}]=] "${L_SOURCE_PATH}" expected_compile_commands "${expected_compile_commands}")

file(READ "${RunCMake_TEST_BINARY_DIR}/compile_commands.json" compile_commands)
if(NOT compile_commands MATCHES "${expected_compile_commands}")
  string(REPLACE "\n" "\n  " expected_compile_commands_formatted "${expected_compile_commands}")
  string(REPLACE "\n" "\n  " compile_commands_formatted "${compile_commands}")
  string(APPEND RunCMake_TEST_FAILED "Expected compile_commands.json to match:\n  ${expected_compile_commands_formatted}\nActual compile_commands.json:\n  ${compile_commands_formatted}\n")
endif()
