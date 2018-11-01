import sys
import json
import re

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

def check_re(x, regex):
    assert re.search(regex, x)

with open(sys.argv[1]) as f:
    ctest_json = json.load(f)
