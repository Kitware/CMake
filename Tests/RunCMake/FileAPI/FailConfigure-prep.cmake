file(REMOVE_RECURSE ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query)

# Shared Stateless
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/codemodel-v2" "")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/configureLog-v1" "")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/cache-v2" "")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/cmakeFiles-v1" "")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/toolchains-v1" "")

# Client Stateless
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-foo/codemodel-v2" "")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-foo/configureLog-v1" "")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-foo/cache-v2" "")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-foo/cmakeFiles-v1" "")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-foo/toolchains-v1" "")

# Client Stateful
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-bar/query.json" [[
{ "requests": [
    { "kind": "codemodel", "version" : 2 },
    { "kind": "configureLog", "version" : 1 },
    { "kind": "cache", "version" : 2 },
    { "kind": "cmakeFiles", "version" : 1 },
    { "kind": "toolchains", "version" : 1 }
  ] }
]])
