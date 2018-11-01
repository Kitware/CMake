from check_index import *

def check_objects(o):
    assert is_list(o)
    assert len(o) == 1
    check_index_object(o[0], "cache", 2, 0, check_object_cache)

def check_object_cache(o):
    assert sorted(o.keys()) == ["entries", "kind", "version"]
    # The "kind" and "version" members are handled by check_index_object.
    # FIXME: Check "entries" member

assert is_dict(index)
assert sorted(index.keys()) == ["cmake", "objects", "reply"]
check_objects(index["objects"])
