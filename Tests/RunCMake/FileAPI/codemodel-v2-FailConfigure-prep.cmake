file(REMOVE_RECURSE ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query)
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/codemodel-v2" "")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-foo/codemodel-v2" "")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-bar/query.json" [[
{ "requests": [ { "kind": "codemodel", "version" : 2 } ] }
]])
