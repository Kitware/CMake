#!/usr/bin/env python

import sys, os

if len(sys.argv) != 2:
  sys.exit(-1)
name = sys.argv[1] + "/CMake.qhp"

f = open(name)

if not f:
  sys.exit(-1)

lines = f.read().splitlines()

if not lines:
  sys.exit(-1)

newlines = []

for line in lines:
  if "<keyword name=\"command\"" in line:
    if not "id=\"" in line:
      prefix = "<keyword name=\"command\" "
      part1, part2 = line.split(prefix)
      head, tail = part2.split("#command:")
      cmdname, rest = tail.split("\"")
      line = part1 + prefix + "id=\"command/" + cmdname + "\" " + part2
  newlines.append(line + "\n")

f = open(name, "w")
f.writelines(newlines)
