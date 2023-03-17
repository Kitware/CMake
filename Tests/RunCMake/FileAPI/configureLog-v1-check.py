from check_index import *
import os

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
    assert sorted(eventKindNames) == ["message-v1", "try_compile-v1", "try_run-v1"]

assert is_dict(index)
assert sorted(index.keys()) == ["cmake", "objects", "reply"]
check_objects(index["objects"])
