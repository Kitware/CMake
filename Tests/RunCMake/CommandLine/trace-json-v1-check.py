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
        'line': 1,
        'line_end': 5
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
        'frame': 3 if expand else 2,
        'global_frame': 3 if expand else 2
    },
    {
        'args': ['STATUS', 'nested global_frame'],
        'cmd': 'message',
        'frame': 3,
        'global_frame': 6 if expand else 5
    },
    {
        'cmd': 'else',
        'global_frame': 4 if expand else 3,
        'line': 3
    }
]

def assert_fields_look_good(line):
    expected_fields = {'args', 'cmd', 'file', 'frame', 'global_frame','line', 'time'}
    if "line_end" in line:
        assert isinstance(line['line_end'], int)
        assert line['line'] != line['line_end']
        expected_fields.add("line_end")

    assert set(line.keys()) == expected_fields

    assert isinstance(line['args'], list)
    assert isinstance(line['cmd'], unicode)
    assert isinstance(line['file'], unicode)
    assert isinstance(line['frame'], int)
    assert isinstance(line['global_frame'], int)
    assert isinstance(line['line'], int)
    assert isinstance(line['time'], float)


with open(trace_file, 'r') as fp:
    # Check for version (must be the first document)
    vers = json.loads(fp.readline())
    assert sorted(vers.keys()) == ['version']
    assert sorted(vers['version'].keys()) == ['major', 'minor']
    assert vers['version']['major'] == 1
    assert vers['version']['minor'] == 2

    for i in fp.readlines():
        line = json.loads(i)
        assert_fields_look_good(line)
        for j in required_traces:
            # Compare the subset of required keys with line
            subset = {
                k: line[k]
                for k in j
                if k in line
            }
            if subset == j:
                required_traces.remove(j)

assert not required_traces, (
    "The following traces were expected to be part of the "
    "output but weren't", required_traces
)
