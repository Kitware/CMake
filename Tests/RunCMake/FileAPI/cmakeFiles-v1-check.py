from check_index import *

def check_objects(o):
    assert is_list(o)
    assert len(o) == 1
    check_index_object(o[0], "cmakeFiles", 1, 0, check_object_cmakeFiles)

def check_input(actual, expected):
    assert is_dict(actual)
    expected_keys = ["path"]

    if expected["isGenerated"] is not None:
        expected_keys.append("isGenerated")
        assert is_bool(actual["isGenerated"], expected["isGenerated"])

    if expected["isExternal"] is not None:
        expected_keys.append("isExternal")
        assert is_bool(actual["isExternal"], expected["isExternal"])

    if expected["isCMake"] is not None:
        expected_keys.append("isCMake")
        assert is_bool(actual["isCMake"], expected["isCMake"])

    assert sorted(actual.keys()) == sorted(expected_keys)

def check_object_cmakeFiles(o):
    assert sorted(o.keys()) == ["inputs", "kind", "paths", "version"]
    # The "kind" and "version" members are handled by check_index_object.
    assert is_dict(o["paths"])
    assert sorted(o["paths"].keys()) == ["build", "source"]
    assert matches(o["paths"]["build"], "^.*/Tests/RunCMake/FileAPI/cmakeFiles-v1-build$")
    assert matches(o["paths"]["source"], "^.*/Tests/RunCMake/FileAPI$")

    expected = [
        {
            "path": "^CMakeLists\\.txt$",
            "isGenerated": None,
            "isExternal": None,
            "isCMake": None,
        },
        {
            "path": "^cmakeFiles-v1\\.cmake$",
            "isGenerated": None,
            "isExternal": None,
            "isCMake": None,
        },
        {
            "path": "^dir/CMakeLists\\.txt$",
            "isGenerated": None,
            "isExternal": None,
            "isCMake": None,
        },
        {
            "path": "^dir/dir/CMakeLists\\.txt$",
            "isGenerated": None,
            "isExternal": None,
            "isCMake": None,
        },
        {
            "path": "^dir/dirtest\\.cmake$",
            "isGenerated": None,
            "isExternal": None,
            "isCMake": None,
        },
        {
            "path": "^.*/Tests/RunCMake/FileAPIDummyFile\\.cmake$",
            "isGenerated": None,
            "isExternal": True,
            "isCMake": None,
        },
        {
            "path": "^.*/Tests/RunCMake/FileAPI/cmakeFiles-v1-build/generated\\.cmake",
            "isGenerated": True,
            "isExternal": None,
            "isCMake": None,
        },
        {
            "path": "^.*/Modules/CMakeParseArguments\\.cmake$",
            "isGenerated": None,
            "isExternal": True,
            "isCMake": True,
        },
    ]

    inSource = os.path.dirname(o["paths"]["build"]) == o["paths"]["source"]
    if inSource:
        for e in expected:
            e["path"] = e["path"].replace("^.*/Tests/RunCMake/FileAPI/", "^", 1)

    check_list_match(lambda a, e: matches(a["path"], e["path"]), o["inputs"], expected, check=check_input, allow_extra=True)

assert is_dict(index)
assert sorted(index.keys()) == ["cmake", "objects", "reply"]
check_objects(index["objects"])
