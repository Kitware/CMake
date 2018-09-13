from check_index import *

def check_reply(r):
    assert is_dict(r)
    assert sorted(r.keys()) == ["__test-v1", "__test-v3", "client-foo", "query.json"]
    check_index__test(r["__test-v1"], 1, 3)
    check_error(r["__test-v3"], "unknown query file")
    check_reply_client_foo(r["client-foo"])
    check_error(r["query.json"], "unknown query file")

def check_reply_client_foo(r):
    assert is_dict(r)
    assert sorted(r.keys()) == ["__test-v2", "unknown"]
    check_index__test(r["__test-v2"], 2, 0)
    check_error(r["unknown"], "unknown query file")

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
