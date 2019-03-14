from check_index import *

def check_reply(q):
    assert is_dict(q)
    assert sorted(q.keys()) == [
        "client-client-member",
        "client-empty-array",
        "client-empty-object",
        "client-json-bad-root",
        "client-json-empty",
        "client-json-extra",
        "client-not-file",
        "client-request-array-negative-major-version",
        "client-request-array-negative-minor-version",
        "client-request-array-negative-version",
        "client-request-array-no-major-version",
        "client-request-array-no-supported-version",
        "client-request-array-no-supported-version-among",
        "client-request-array-version-1",
        "client-request-array-version-1-1",
        "client-request-array-version-2",
        "client-request-negative-major-version",
        "client-request-negative-minor-version",
        "client-request-negative-version",
        "client-request-no-major-version",
        "client-request-no-version",
        "client-request-version-1",
        "client-request-version-1-1",
        "client-request-version-2",
        "client-requests-bad",
        "client-requests-empty",
        "client-requests-not-kinded",
        "client-requests-not-objects",
        "client-requests-unknown",
    ]
    expected = [
        (check_query_client_member, "client-client-member"),
        (check_query_empty_array, "client-empty-array"),
        (check_query_empty_object, "client-empty-object"),
        (check_query_json_bad_root, "client-json-bad-root"),
        (check_query_json_empty, "client-json-empty"),
        (check_query_json_extra, "client-json-extra"),
        (check_query_not_file, "client-not-file"),
        (check_query_requests_bad, "client-requests-bad"),
        (check_query_requests_empty, "client-requests-empty"),
        (check_query_requests_not_kinded, "client-requests-not-kinded"),
        (check_query_requests_not_objects, "client-requests-not-objects"),
        (check_query_requests_unknown, "client-requests-unknown"),
    ]
    for (f, k) in expected:
        assert is_dict(q[k])
        assert sorted(q[k].keys()) == ["query.json"]
        f(q[k]["query.json"])
    expected = [
        (check_query_response_array_negative_major_version,     "client-request-array-negative-major-version"),
        (check_query_response_array_negative_minor_version,     "client-request-array-negative-minor-version"),
        (check_query_response_array_negative_version,           "client-request-array-negative-version"),
        (check_query_response_array_no_major_version,           "client-request-array-no-major-version"),
        (check_query_response_array_no_supported_version,       "client-request-array-no-supported-version"),
        (check_query_response_array_no_supported_version_among, "client-request-array-no-supported-version-among"),
        (check_query_response_array_version_1,                  "client-request-array-version-1"),
        (check_query_response_array_version_1_1,                "client-request-array-version-1-1"),
        (check_query_response_array_version_2,                  "client-request-array-version-2"),
        (check_query_response_negative_major_version,           "client-request-negative-major-version"),
        (check_query_response_negative_minor_version,           "client-request-negative-minor-version"),
        (check_query_response_negative_version,                 "client-request-negative-version"),
        (check_query_response_no_major_version,                 "client-request-no-major-version"),
        (check_query_response_no_version,                       "client-request-no-version"),
        (check_query_response_version_1,                        "client-request-version-1"),
        (check_query_response_version_1_1,                      "client-request-version-1-1"),
        (check_query_response_version_2,                        "client-request-version-2"),
    ]
    for (f, k) in expected:
        assert is_dict(q[k])
        assert sorted(q[k].keys()) == ["query.json"]
        assert is_dict(q[k]["query.json"])
        assert sorted(q[k]["query.json"].keys()) == ["requests", "responses"]
        r = q[k]["query.json"]["requests"]
        assert is_list(r)
        assert len(r) == 1
        assert is_dict(r[0])
        assert r[0]["kind"] == "__test"
        r = q[k]["query.json"]["responses"]
        assert is_list(r)
        assert len(r) == 1
        assert is_dict(r[0])
        f(r[0])

def check_query_client_member(q):
    assert is_dict(q)
    assert sorted(q.keys()) == ["client", "responses"]
    assert is_dict(q["client"])
    assert sorted(q["client"].keys()) == []
    check_error(q["responses"], "'requests' member missing")

def check_query_empty_array(q):
    check_error(q, "query root is not an object")

def check_query_empty_object(q):
    assert is_dict(q)
    assert sorted(q.keys()) == ["responses"]
    check_error(q["responses"], "'requests' member missing")

def check_query_json_bad_root(q):
    check_error_re(q, "A valid JSON document must be either an array or an object value")

def check_query_json_empty(q):
    check_error_re(q, "value, object or array expected")

def check_query_json_extra(q):
    check_error_re(q, "Extra non-whitespace after JSON value")

def check_query_not_file(q):
    check_error_re(q, "failed to read from file")

def check_query_requests_bad(q):
    assert is_dict(q)
    assert sorted(q.keys()) == ["requests", "responses"]
    r = q["requests"]
    assert is_dict(r)
    assert sorted(r.keys()) == []
    check_error(q["responses"], "'requests' member is not an array")

def check_query_requests_empty(q):
    assert is_dict(q)
    assert sorted(q.keys()) == ["requests", "responses"]
    r = q["requests"]
    assert is_list(r)
    assert len(r) == 0
    r = q["responses"]
    assert is_list(r)
    assert len(r) == 0

def check_query_requests_not_kinded(q):
    assert is_dict(q)
    assert sorted(q.keys()) == ["requests", "responses"]
    r = q["requests"]
    assert is_list(r)
    assert len(r) == 4
    assert is_dict(r[0])
    assert sorted(r[0].keys()) == []
    assert is_dict(r[1])
    assert sorted(r[1].keys()) == ["kind"]
    assert is_dict(r[1]["kind"])
    assert is_dict(r[2])
    assert sorted(r[2].keys()) == ["kind"]
    assert is_list(r[2]["kind"])
    assert is_dict(r[3])
    assert sorted(r[3].keys()) == ["kind"]
    assert is_int(r[3]["kind"])
    r = q["responses"]
    assert is_list(r)
    assert len(r) == 4
    check_error(r[0], "'kind' member missing")
    check_error(r[1], "'kind' member is not a string")
    check_error(r[2], "'kind' member is not a string")
    check_error(r[3], "'kind' member is not a string")

def check_query_requests_not_objects(q):
    assert is_dict(q)
    assert sorted(q.keys()) == ["requests", "responses"]
    r = q["requests"]
    assert is_list(r)
    assert len(r) == 3
    assert is_int(r[0])
    assert is_string(r[1])
    assert is_list(r[2])
    r = q["responses"]
    assert is_list(r)
    assert len(r) == 3
    check_error(r[0], "request is not an object")
    check_error(r[1], "request is not an object")
    check_error(r[2], "request is not an object")

def check_query_requests_unknown(q):
    assert is_dict(q)
    assert sorted(q.keys()) == ["requests", "responses"]
    r = q["requests"]
    assert is_list(r)
    assert len(r) == 3
    assert is_dict(r[0])
    assert sorted(r[0].keys()) == ["kind"]
    assert r[0]["kind"] == "unknownC"
    assert is_dict(r[1])
    assert sorted(r[1].keys()) == ["kind"]
    assert r[1]["kind"] == "unknownB"
    assert is_dict(r[2])
    assert sorted(r[2].keys()) == ["kind"]
    assert r[2]["kind"] == "unknownA"
    r = q["responses"]
    assert is_list(r)
    assert len(r) == 3
    check_error(r[0], "unknown request kind 'unknownC'")
    check_error(r[1], "unknown request kind 'unknownB'")
    check_error(r[2], "unknown request kind 'unknownA'")

def check_query_response_array_negative_major_version(r):
    check_error(r, "'version' object 'major' member is not a non-negative integer")

def check_query_response_array_negative_minor_version(r):
    check_error(r, "'version' object 'minor' member is not a non-negative integer")

def check_query_response_array_negative_version(r):
    check_error(r, "'version' array entry is not a non-negative integer or object")

def check_query_response_array_no_major_version(r):
    check_error(r, "'version' object 'major' member missing")

def check_query_response_array_no_supported_version(r):
    check_error(r, "no supported version specified")

def check_query_response_array_no_supported_version_among(r):
    check_error(r, "no supported version specified among: 4.0 3.0")

def check_query_response_array_version_1(r):
    check_index__test(r, 1, 3)

def check_query_response_array_version_1_1(r):
    check_index__test(r, 1, 3) # always uses latest minor version

def check_query_response_array_version_2(r):
    check_index__test(r, 2, 0)

def check_query_response_negative_major_version(r):
    check_error(r, "'version' object 'major' member is not a non-negative integer")

def check_query_response_negative_minor_version(r):
    check_error(r, "'version' object 'minor' member is not a non-negative integer")

def check_query_response_negative_version(r):
    check_error(r, "'version' member is not a non-negative integer, object, or array")

def check_query_response_no_major_version(r):
    check_error(r, "'version' object 'major' member missing")

def check_query_response_no_version(r):
    check_error(r, "'version' member missing")

def check_query_response_version_1(r):
    check_index__test(r, 1, 3)

def check_query_response_version_1_1(r):
    check_index__test(r, 1, 3) # always uses latest minor version

def check_query_response_version_2(r):
    check_index__test(r, 2, 0)

def check_objects(o):
    assert is_list(o)
    assert len(o) == 2
    check_index__test(o[0], 1, 3)
    check_index__test(o[1], 2, 0)

assert is_dict(index)
assert sorted(index.keys()) == ["cmake", "objects", "reply"]
check_cmake(index["cmake"])
check_reply(index["reply"])
check_objects(index["objects"])
