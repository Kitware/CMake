from check_index import *

def check_objects(o):
    assert is_list(o)
    assert len(o) == 1
    check_index_object(o[0], "cache", 2, 0, check_object_cache)

def check_cache_entry(actual, expected):
    assert is_dict(actual)
    assert sorted(actual.keys()) == ["name", "properties", "type", "value"]

    assert is_string(actual["type"], expected["type"])
    assert is_string(actual["value"], expected["value"])

    def check_property(actual, expected):
        assert is_dict(actual)
        assert sorted(actual.keys()) == ["name", "value"]
        assert is_string(actual["value"], expected["value"])

    check_list_match(lambda a, e: is_string(a["name"], e["name"]), actual["properties"], expected["properties"], check=check_property)

def check_object_cache(o):
    assert sorted(o.keys()) == ["entries", "kind", "version"]
    # The "kind" and "version" members are handled by check_index_object.
    check_list_match(lambda a, e: is_string(a["name"], e["name"]), o["entries"], [
        {
            "name": "CM_OPTION_BOOL",
            "type": "BOOL",
            "value": "OFF",
            "properties": [
                {
                    "name": "HELPSTRING",
                    "value": "Testing option()",
                },
            ],
        },
        {
            "name": "CM_SET_BOOL",
            "type": "BOOL",
            "value": "ON",
            "properties": [
                {
                    "name": "HELPSTRING",
                    "value": "Testing set(CACHE BOOL)",
                },
                {
                    "name": "ADVANCED",
                    "value": "1",
                },
            ],
        },
        {
            "name": "CM_SET_FILEPATH",
            "type": "FILEPATH",
            "value": "dir1/dir2/empty.txt",
            "properties": [
                {
                    "name": "HELPSTRING",
                    "value": "Testing set(CACHE FILEPATH)",
                },
            ],
        },
        {
            "name": "CM_SET_PATH",
            "type": "PATH",
            "value": "dir1/dir2",
            "properties": [
                {
                    "name": "HELPSTRING",
                    "value": "Testing set(CACHE PATH)",
                },
                {
                    "name": "ADVANCED",
                    "value": "ON",
                },
            ],
        },
        {
            "name": "CM_SET_STRING",
            "type": "STRING",
            "value": "test",
            "properties": [
                {
                    "name": "HELPSTRING",
                    "value": "Testing set(CACHE STRING)",
                },
            ],
        },
        {
            "name": "CM_SET_STRINGS",
            "type": "STRING",
            "value": "1",
            "properties": [
                {
                    "name": "HELPSTRING",
                    "value": "Testing set(CACHE STRING) with STRINGS",
                },
                {
                    "name": "STRINGS",
                    "value": "1;2;3;4",
                },
            ],
        },
        {
            "name": "CM_SET_INTERNAL",
            "type": "INTERNAL",
            "value": "int2",
            "properties": [
                {
                    "name": "HELPSTRING",
                    "value": "Testing set(CACHE INTERNAL)",
                },
            ],
        },
        {
            "name": "CM_SET_TYPE",
            "type": "STRING",
            "value": "1",
            "properties": [
                {
                    "name": "HELPSTRING",
                    "value": "Testing set(CACHE INTERNAL) with set_property(TYPE)",
                },
                {
                    "name": "ADVANCED",
                    "value": "0",
                },
            ],
        },
    ], check=check_cache_entry, allow_extra=True)

assert is_dict(index)
assert sorted(index.keys()) == ["cmake", "objects", "reply"]
check_objects(index["objects"])
