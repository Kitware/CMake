from check_index import *

def check_objects(o):
    assert is_list(o)
    assert len(o) == 1
    check_index_object(o[0], "codemodel", 2, 0, check_object_codemodel)

def check_object_codemodel(o):
    assert sorted(o.keys()) == ["configurations", "kind", "paths", "version"]
    # The "kind" and "version" members are handled by check_index_object.
    # FIXME: Check "configurations"  and "paths" members

assert is_dict(index)
assert sorted(index.keys()) == ["cmake", "objects", "reply"]
check_objects(index["objects"])
