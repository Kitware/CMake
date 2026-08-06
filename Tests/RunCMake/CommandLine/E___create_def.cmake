string(ASCII 66 67 192 222 llvm_bitcode_magic)
string(REPEAT "x" 64 llvm_bitcode_padding)
file(WRITE "${CMAKE_BINARY_DIR}/bitcode.obj"
  "${llvm_bitcode_magic}${llvm_bitcode_padding}")
file(WRITE "${CMAKE_BINARY_DIR}/nm-not-found.objs" [[
bitcode.obj
]])

file(WRITE "${CMAKE_BINARY_DIR}/single.obj" "")
file(WRITE "${CMAKE_BINARY_DIR}/single.objs" [[
single.obj
]])

file(WRITE "${CMAKE_BINARY_DIR}/symbol-types.obj" "")
file(WRITE "${CMAKE_BINARY_DIR}/symbol-types.objs" [[
symbol-types.obj
]])

file(WRITE "${CMAKE_BINARY_DIR}/first.obj" "")
file(WRITE "${CMAKE_BINARY_DIR}/no-symbols.obj" "")
file(WRITE "${CMAKE_BINARY_DIR}/second.obj" "")
file(WRITE "${CMAKE_BINARY_DIR}/multiple.objs" [[
first.obj
no-symbols.obj
second.obj
]])

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/directory with spaces")
file(WRITE "${CMAKE_BINARY_DIR}/directory with spaces/spaced.obj" "")
file(WRITE "${CMAKE_BINARY_DIR}/spaces.objs" [[
directory with spaces/spaced.obj
]])

file(WRITE "${CMAKE_BINARY_DIR}/manual.def" [[
EXPORTS
  manual
]])
file(WRITE "${CMAKE_BINARY_DIR}/mixed.obj" "")
file(WRITE "${CMAKE_BINARY_DIR}/mixed.objs" [[
manual.def
mixed.obj
]])
