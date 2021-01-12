file(REMOVE_RECURSE ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query)
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-foo/query.json" [[
{ "requests": [ { "kind": "toolchains", "version" : 1 } ] }
]])
