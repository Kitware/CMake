message(
    STATUS
    "JSON-V1 str"
    "spaces"
    )
set(ASDF fff sss "  SPACES !!!  ")
set(FOO 42)
set(BAR " space in string!")
message(STATUS fff ${ASDF} " ${FOO} ${BAR}" "  SPACES !!!  ")
add_subdirectory(trace-json-v1-nested)
function_that_uses_else()
