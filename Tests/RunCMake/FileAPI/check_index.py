import sys
import os
import json
import re

if sys.version_info[0] >= 3:
    unicode = str

def is_bool(x):
    return isinstance(x, bool)

def is_dict(x):
    return isinstance(x, dict)

def is_list(x):
    return isinstance(x, list)

def is_int(x):
    return isinstance(x, int) or isinstance(x, long)

def is_string(x):
    return isinstance(x, str) or isinstance(x, unicode)

def check_cmake(cmake):
    assert is_dict(cmake)
    assert sorted(cmake.keys()) == ["paths", "version"]
    check_cmake_version(cmake["version"])
    check_cmake_paths(cmake["paths"])

def check_cmake_version(v):
    assert is_dict(v)
    assert sorted(v.keys()) == ["isDirty", "major", "minor", "patch", "string", "suffix"]
    assert is_string(v["string"])
    assert is_int(v["major"])
    assert is_int(v["minor"])
    assert is_int(v["patch"])
    assert is_string(v["suffix"])
    assert is_bool(v["isDirty"])

def check_cmake_paths(v):
    assert is_dict(v)
    assert sorted(v.keys()) == ["cmake", "cpack", "ctest", "root"]
    assert is_string(v["cmake"])
    assert is_string(v["cpack"])
    assert is_string(v["ctest"])
    assert is_string(v["root"])

def check_index_object(indexEntry, kind, major, minor, check):
    assert is_dict(indexEntry)
    assert sorted(indexEntry.keys()) == ["jsonFile", "kind", "version"]
    assert is_string(indexEntry["kind"])
    assert indexEntry["kind"] == kind
    assert is_dict(indexEntry["version"])
    assert sorted(indexEntry["version"].keys()) == ["major", "minor"]
    assert indexEntry["version"]["major"] == major
    assert indexEntry["version"]["minor"] == minor
    assert is_string(indexEntry["jsonFile"])
    filepath = os.path.join(reply_dir, indexEntry["jsonFile"])
    with open(filepath) as f:
        object = json.load(f)
    assert is_dict(object)
    assert "kind" in object
    assert is_string(object["kind"])
    assert object["kind"] == kind
    assert "version" in object
    assert is_dict(object["version"])
    assert sorted(object["version"].keys()) == ["major", "minor"]
    assert object["version"]["major"] == major
    assert object["version"]["minor"] == minor
    if check:
        check(object)

def check_index__test(indexEntry, major, minor):
    def check(object):
        assert sorted(object.keys()) == ["kind", "version"]
    check_index_object(indexEntry, "__test", major, minor, check)

def check_error(value, error):
    assert is_dict(value)
    assert sorted(value.keys()) == ["error"]
    assert is_string(value["error"])
    assert value["error"] == error

def check_error_re(value, error):
    assert is_dict(value)
    assert sorted(value.keys()) == ["error"]
    assert is_string(value["error"])
    assert re.search(error, value["error"])

reply_index = sys.argv[1]
reply_dir = os.path.dirname(reply_index)

with open(reply_index) as f:
    index = json.load(f)
