from check_index import *
import os

class ExpectedVar(object):
    def __init__(self, name):
        self.name = name

class ExpectedList(object):
    def __init__(self, name):
        self.name = name

EXPECTED_TOOLCHAIN = {
    "language": "CXX",
    "compiler": {
        "path": ExpectedVar("CMAKE_CXX_COMPILER"),
        "id": ExpectedVar("CMAKE_CXX_COMPILER_ID"),
        "version": ExpectedVar("CMAKE_CXX_COMPILER_VERSION"),
        "target": ExpectedVar("CMAKE_CXX_COMPILER_TARGET"),
        "implicit": {
            "includeDirectories": \
                ExpectedList("CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES"),
            "linkDirectories": \
                ExpectedList("CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES"),
            "linkFrameworkDirectories": \
                ExpectedList(
                    "CMAKE_CXX_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES"),
            "linkLibraries": \
                ExpectedList("CMAKE_CXX_IMPLICIT_LINK_LIBRARIES"),
        }
    },
    "sourceFileExtensions": \
        ExpectedList("CMAKE_CXX_SOURCE_FILE_EXTENSIONS"),
}

def check_objects(o):
    assert is_list(o)
    assert len(o) == 1
    check_index_object(o[0], "toolchains", 1, 0, check_object_toolchains)

def check_object_toolchains(o):
    assert sorted(o.keys()) == ["kind", "toolchains", "version"]
    # The "kind" and "version" members are handled by check_index_object.
    toolchains = o["toolchains"]
    assert is_list(toolchains)

    # Other platform-specific toolchains may exist (like RC on Windows).
    has_cxx_toolchain = False
    for toolchain in toolchains:
        assert is_dict(toolchain)
        assert "language" in toolchain
        if toolchain["language"] == "CXX":
            check_object_toolchain(toolchain, EXPECTED_TOOLCHAIN)
            has_cxx_toolchain = True

    assert has_cxx_toolchain

def check_object_toolchain(o, expected):
    expected_keys = [
        key for (key, value) in expected.items()
        if is_string(value) or is_dict(value)
            or (type(value) in (ExpectedVar, ExpectedList)
                and variables[value.name]["defined"])]
    assert sorted(o.keys()) == sorted(expected_keys)

    for key in expected_keys:
        value = expected[key]
        if is_string(value):
            assert o[key] == value
        elif is_dict(value):
            check_object_toolchain(o[key], value)
        elif type(value) == ExpectedVar:
            assert o[key] == variables[value.name]["value"]
        elif type(value) == ExpectedList:
            expected_items = filter(
                None, variables[value.name]["value"].split(";"))
            check_list_match(lambda a, b: a == b, o[key], expected_items)
        else:
            assert False

with open(os.path.join(sys.argv[3], "toolchain_variables.json")) as f:
    variables = json.load(f)

assert is_dict(variables)
assert is_dict(index)
assert sorted(index.keys()) == ["cmake", "objects", "reply"]
check_objects(index["objects"])
