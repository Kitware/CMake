from check_index import *
import os

def check_reply(r):
    assert is_dict(r)
    assert sorted(r.keys()) == [
        "client-bar",
        "client-foo",
        "codemodel-v2",
    ]
    check_reply_client_bar(r["client-bar"])
    check_reply_client_foo(r["client-foo"])
    check_error(r["codemodel-v2"], "no buildsystem generated")

def check_reply_client_bar(r):
    assert is_dict(r)
    assert sorted(r.keys()) == ["query.json"]
    query = r["query.json"]
    assert sorted(query.keys()) == ["requests", "responses"]
    requests = query["requests"]
    assert is_list(requests)
    assert len(requests) == 1
    responses = query["responses"]
    assert is_list(responses)
    assert len(responses) == 1
    check_error(responses[0], "no buildsystem generated")

def check_reply_client_foo(r):
    assert is_dict(r)
    assert sorted(r.keys()) == [
        "codemodel-v2",
    ]
    check_error(r["codemodel-v2"], "no buildsystem generated")

def check_objects(o):
    assert is_list(o)
    assert len(o) == 0

assert is_dict(index)
assert sorted(index.keys()) == ["cmake", "objects", "reply"]
check_cmake(index["cmake"])
check_reply(index["reply"])
check_objects(index["objects"])
