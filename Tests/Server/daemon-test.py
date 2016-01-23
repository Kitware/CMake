#!/usr/bin/env python2

import sys, cmakelib, json

def ordered(obj):
    if isinstance(obj, dict):
        return sorted((k, ordered(v)) for k, v in obj.items())
    if isinstance(obj, list):
        return sorted(ordered(x) for x in obj)
    else:
        return obj

cmakeCommand = sys.argv[1]

proc = cmakelib.initProc(cmakeCommand, sys.argv[2])

req = json.loads(sys.argv[3])
res = ordered(json.loads(sys.argv[4]))

cmakelib.writePayload(proc, req)

packet = ordered(cmakelib.waitForMessage(proc))

if packet != res:
  print "NOT EQUAL\n", packet, "\n\n", res
  sys.exit(-1)

sys.exit(0)
