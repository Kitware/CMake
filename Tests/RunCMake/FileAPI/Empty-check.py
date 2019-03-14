from check_index import *

def check_reply(r):
    assert is_dict(r)
    assert sorted(r.keys()) == []

def check_objects(o):
    assert is_list(o)
    assert len(o) == 0

assert is_dict(index)
assert sorted(index.keys()) == ["cmake", "objects", "reply"]
check_cmake(index["cmake"])
check_reply(index["reply"])
check_objects(index["objects"])
