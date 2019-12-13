#!/usr/bin/env python3

import argparse
import json
import os
import sys

if sys.version_info[0] >= 3:
    unicode = str

parser = argparse.ArgumentParser(description='Checks the trace output')
parser.add_argument('-e', '--expand', action='store_true')
parser.add_argument('trace', type=str, help='the trace file to check')

args = parser.parse_args()

assert os.path.exists(args.trace)

if args.expand:
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

with open(args.trace, 'r') as fp:
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
