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
        'frame': 3 if expand else 2
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
        assert sorted(line.keys()) == ['args', 'cmd', 'file', 'frame', 'line', 'time']
        assert isinstance(line['args'], list)
        assert isinstance(line['cmd'], unicode)
        assert isinstance(line['file'], unicode)
        assert isinstance(line['frame'], int)
        assert isinstance(line['line'], int)
        assert isinstance(line['time'], float)

        for j in required_traces:
            # Compare the subset of required keys with line
            if {k: line[k] for k in j} == j:
                required_traces.remove(j)

assert not required_traces
