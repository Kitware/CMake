from check_index import *
import os

def check_reply(r):
    assert is_dict(r)
    assert sorted(r.keys()) == [
        "cache-v2",
        "client-bar",
        "client-foo",
        "cmakeFiles-v1",
        "codemodel-v2",
        "configureLog-v1",
        "toolchains-v1",
    ]
    check_error(r["cache-v2"], "no buildsystem generated")
    check_error(r["cmakeFiles-v1"], "no buildsystem generated")
    check_reply_client_bar(r["client-bar"])
    check_reply_client_foo(r["client-foo"])
    check_error(r["codemodel-v2"], "no buildsystem generated")
    check_index_object(r["configureLog-v1"], "configureLog", 1, 0, None)
    check_error(r["toolchains-v1"], "no buildsystem generated")

def check_reply_client_bar(r):
    assert is_dict(r)
    assert sorted(r.keys()) == ["query.json"]
    query = r["query.json"]
    assert sorted(query.keys()) == ["requests", "responses"]
    requests = query["requests"]
    assert is_list(requests)
    assert len(requests) == 5
    responses = query["responses"]
    assert is_list(responses)
    assert len(responses) == 5
    check_error(responses[0], "no buildsystem generated")
    check_index_object(responses[1], "configureLog", 1, 0, None)
    check_error(responses[2], "no buildsystem generated")
    check_error(responses[3], "no buildsystem generated")
    check_error(responses[4], "no buildsystem generated")

def check_reply_client_foo(r):
    assert is_dict(r)
    assert sorted(r.keys()) == [
        "cache-v2",
        "cmakeFiles-v1",
        "codemodel-v2",
        "configureLog-v1",
        "toolchains-v1",
    ]
    check_error(r["cache-v2"], "no buildsystem generated")
    check_error(r["cmakeFiles-v1"], "no buildsystem generated")
    check_error(r["codemodel-v2"], "no buildsystem generated")
    check_index_object(r["configureLog-v1"], "configureLog", 1, 0, None)
    check_error(r["toolchains-v1"], "no buildsystem generated")

def check_objects(o):
    assert is_list(o)
    assert len(o) == 1
    check_index_object(o[0], "configureLog", 1, 0, check_object_configureLog)

def check_object_configureLog(o):
    assert sorted(o.keys()) == ["eventKindNames", "kind", "path", "version"]
    # The "kind" and "version" members are handled by check_index_object.
    path = o["path"]
    assert matches(path, "^.*/CMakeFiles/CMakeConfigureLog\\.yaml$")
    assert os.path.exists(path)
    eventKindNames = o["eventKindNames"]
    assert is_list(eventKindNames)
    assert sorted(eventKindNames) == ["find-v1", "find_package-v1", "message-v1", "try_compile-v1", "try_run-v1"]

assert is_dict(index)
assert sorted(index.keys()) == ["cmake", "objects", "reply"]
check_cmake(index["cmake"])
check_reply(index["reply"])
check_objects(index["objects"])
