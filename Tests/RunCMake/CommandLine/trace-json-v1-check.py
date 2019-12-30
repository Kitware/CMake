#!/usr/bin/env python3

import json
import os
import sys

if sys.version_info[0] >= 3:
    unicode = str

trace_file = None
expand = False

for i in sys.argv[1:]:
    if trace_file is None and not i.startswith('-'):
        trace_file = i
        continue

    if i in ['-e', '--expand']:
        expand = True

assert trace_file is not None
assert os.path.exists(trace_file)

if expand:
    msg_args = ['STATUS', 'fff', 'fff;sss;  SPACES !!!  ', ' 42  space in string!', '  SPACES !!!  ']
else:
    msg_args = ['STATUS', 'fff', '${ASDF}', ' ${FOO} ${BAR}', '  SPACES !!!  ']

required_traces = [
    {
        'args': ['STATUS', 'JSON-V1 str', 'spaces'],
        'cmd': 'message',
    },
    {
        'args': ['ASDF', 'fff', 'sss', '  SPACES !!!  '],
        'cmd': 'set',
    },
    {
        'args': ['FOO', '42'],
        'cmd': 'set',
    },
    {
        'args': ['BAR', ' space in string!'],
        'cmd': 'set',
    },
    {
        'args': msg_args,
        'cmd': 'message',
    },
]

with open(trace_file, 'r') as fp:
    # Check for version (must be the first document)
    vers = json.loads(fp.readline())
    assert sorted(vers.keys()) == ['version']
    assert sorted(vers['version'].keys()) == ['major', 'minor']
    assert vers['version']['major'] == 1
    assert vers['version']['minor'] == 0

    for i in fp.readlines():
        line = json.loads(i)
        assert sorted(line.keys()) == ['args', 'cmd', 'file', 'line']
        assert isinstance(line['args'], list)
        assert isinstance(line['cmd'], unicode)
        assert isinstance(line['file'], unicode)
        assert isinstance(line['line'], int)

        for i in required_traces:
            if i['cmd'] == line['cmd'] and i['args'] == line['args']:
                i['found'] = True

assert all([x.get('found', False) == True for x in required_traces])
