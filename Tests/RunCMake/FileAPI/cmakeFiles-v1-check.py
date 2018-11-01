from check_index import *

def check_objects(o):
    assert is_list(o)
    assert len(o) == 1
    check_index_object(o[0], "cmakeFiles", 1, 0, check_object_cmakeFiles)

def check_object_cmakeFiles(o):
    assert sorted(o.keys()) == ["inputs", "kind", "paths", "version"]
    # The "kind" and "version" members are handled by check_index_object.
    # FIXME: Check "paths" and "inputs" members.

assert is_dict(index)
assert sorted(index.keys()) == ["cmake", "objects", "reply"]
check_objects(index["objects"])
