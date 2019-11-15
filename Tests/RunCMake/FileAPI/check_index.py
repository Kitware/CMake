import sys
import os
import json
import re

if sys.version_info[0] >= 3:
    unicode = str

def is_bool(x, val=None):
    return isinstance(x, bool) and (val is None or x == val)

def is_dict(x):
    return isinstance(x, dict)

def is_list(x):
    return isinstance(x, list)

def is_int(x, val=None):
    return (isinstance(x, int) or isinstance(x, long)) and (val is None or x == val)

def is_string(x, val=None):
    return (isinstance(x, str) or isinstance(x, unicode)) and (val is None or x == val)

def matches(s, pattern):
    return is_string(s) and bool(re.search(pattern, s))

def check_list_match(match, actual, expected, check=None, check_exception=None, missing_exception=None, extra_exception=None, allow_extra=False):
    """
    Handle the common pattern of making sure every actual item "matches" some
    item in the expected list, and that neither list has extra items after
    matching is completed.

    @param match: Callback to check if an actual item matches an expected
    item. Return True if the item matches, return False if the item doesn't
    match.
    @param actual: List of actual items to search.
    @param expected: List of expected items to match.
    @param check: Optional function to check that the actual item is valid by
    comparing it to the expected item.
    @param check_exception: Optional function that returns an argument to
    append to any exception thrown by the check function.
    @param missing_exception: Optional function that returns an argument to
    append to the exception thrown when an item is not found.
    @param extra_exception: Optional function that returns an argument to
    append to the exception thrown when an extra item is found.
    @param allow_extra: Optional parameter allowing there to be extra actual
    items after all the expected items have been found.
    """
    assert is_list(actual)
    _actual = actual[:]
    for expected_item in expected:
        found = False
        for i, actual_item in enumerate(_actual):
            if match(actual_item, expected_item):
                if check:
                    try:
                        check(actual_item, expected_item)
                    except BaseException as e:
                        if check_exception:
                            e.args += (check_exception(actual_item, expected_item),)
                        raise
                found = True
                del _actual[i]
                break
        if missing_exception:
            assert found, missing_exception(expected_item)
        else:
            assert found
    if not allow_extra:
        if extra_exception:
            assert len(_actual) == 0, [extra_exception(a) for a in _actual]
        else:
            assert len(_actual) == 0

def filter_list(f, l):
    if l is not None:
        l = list(filter(f, l))
    if l == []:
        l = None
    return l

def check_cmake(cmake):
    assert is_dict(cmake)
    assert sorted(cmake.keys()) == ["generator", "paths", "version"]
    check_cmake_version(cmake["version"])
    check_cmake_paths(cmake["paths"])
    check_cmake_generator(cmake["generator"])

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

def check_cmake_generator(g):
    assert is_dict(g)
    name = g.get("name", None)
    assert is_string(name)
    if name.startswith("Visual Studio"):
        assert sorted(g.keys()) == ["multiConfig", "name", "platform"]
        assert is_string(g["platform"])
    else:
        assert sorted(g.keys()) == ["multiConfig", "name"]
    assert is_bool(g["multiConfig"], matches(name, "^(Visual Studio |Xcode$|Ninja Multi-Config$)"))

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
