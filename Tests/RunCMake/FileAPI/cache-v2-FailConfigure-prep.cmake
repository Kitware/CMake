file(REMOVE_RECURSE ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query)
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/cache-v2" "")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-foo/cache-v2" "")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-bar/query.json" [[
{ "requests": [ { "kind": "cache", "version" : 2 } ] }
]])
