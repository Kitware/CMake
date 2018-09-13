file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-client-member/query.json" [[{ "client": {} }]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-empty-array/query.json" "[]")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-empty-object/query.json" "{}")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-json-bad-root/query.json" [["invalid root"]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-json-empty/query.json" "")
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-json-extra/query.json" "{}x")
file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-not-file/query.json")

file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-requests-bad/query.json" [[{ "requests": {} }]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-requests-empty/query.json" [[{ "requests": [] }]])

file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-requests-not-objects/query.json" [[
{ "requests": [ 0, "", [] ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-requests-not-kinded/query.json" [[
{ "requests": [ {}, { "kind": {} }, { "kind": [] }, { "kind": 0 } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-requests-unknown/query.json" [[
{ "requests": [ { "kind": "unknownC" }, { "kind": "unknownB" }, { "kind": "unknownA" } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-no-version/query.json" [[
{ "requests": [ { "kind": "__test" } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-negative-version/query.json" [[
{ "requests": [ { "kind": "__test", "version" : -1 } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-no-major-version/query.json" [[
{ "requests": [ { "kind": "__test", "version" : {} } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-negative-major-version/query.json" [[
{ "requests": [ { "kind": "__test", "version" : { "major": -1 } } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-negative-minor-version/query.json" [[
{ "requests": [ { "kind": "__test", "version" : { "major": 0, "minor": -1 } } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-array-negative-version/query.json" [[
{ "requests": [ { "kind": "__test", "version" : [ 1, -1 ] } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-array-no-major-version/query.json" [[
{ "requests": [ { "kind": "__test", "version" : [ 1, {} ] } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-array-negative-major-version/query.json" [[
{ "requests": [ { "kind": "__test", "version" : [ 1, { "major": -1 } ] } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-array-negative-minor-version/query.json" [[
{ "requests": [ { "kind": "__test", "version" : [ 1, { "major": 0, "minor": -1 } ] } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-array-no-supported-version/query.json" [[
{ "requests": [ { "kind": "__test", "version" : [] } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-array-no-supported-version-among/query.json" [[
{ "requests": [ { "kind": "__test", "version" : [4, 3] } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-version-1/query.json" [[
{ "requests": [ { "kind": "__test", "version" : 1 } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-version-1-1/query.json" [[
{ "requests": [ { "kind": "__test", "version" : { "major": 1, "minor": 1 } } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-version-2/query.json" [[
{ "requests": [ { "kind": "__test", "version" : { "major": 2 } } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-array-version-1/query.json" [[
{ "requests": [ { "kind": "__test", "version" : [3, 1] } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-array-version-1-1/query.json" [[
{ "requests": [ { "kind": "__test", "version" : [3, { "major": 1, "minor": 1 }, 2 ] } ] }
]])
file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/query/client-request-array-version-2/query.json" [[
{ "requests": [ { "kind": "__test", "version" : [3, { "major": 2 } ]  } ] }
]])

file(WRITE "${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/reply/object-to-be-deleted.json" "")
