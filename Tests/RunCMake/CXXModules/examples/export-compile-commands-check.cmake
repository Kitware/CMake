if (NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/compile_commands.json")
  list(APPEND RunCMake_TEST_FAILED
    "No compile commands database detected.")
endif ()

file(READ "${RunCMake_TEST_BINARY_DIR}/compile_commands.json" compile_commands)

string(JSON length
  LENGTH "${compile_commands}")
math(EXPR length "${length} - 1")
foreach (item RANGE "${length}")
  string(JSON entry
    GET "${compile_commands}"
    "${item}")
  string(JSON command
    GET "${entry}"
    "command")
  if (NOT command MATCHES "(@|-fmodule-mapper=).*\\.modmap")
    string(JSON output
      GET "${entry}"
      "output")
    list(APPEND RunCMake_TEST_FAILED
      "Missing `.modmap` argument for '${output}'")
  endif ()
endforeach ()
