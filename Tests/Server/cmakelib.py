#!/usr/bin/env python2

import sys, subprocess, json

termwidth = 150

print_communication = False

def col_print(title, array):
  print
  print
  print title

  indentwidth = 4
  indent = " " * indentwidth

  if not array:
    print indent + "<None>"
    return

  padwidth = 2

  maxitemwidth = len(max(array, key=len))

  numCols = max(1, int((termwidth - indentwidth + padwidth) / (maxitemwidth + padwidth)))

  numRows = len(array) // numCols + 1

  pad = " " * padwidth

  for index in range(numRows):
    print indent + pad.join(item.ljust(maxitemwidth) for item in array[index::numRows])

def waitForMessage(process):
  stdoutdata = ""
  payload = ""
  while not process.poll():
    stdoutdataLine = process.stdout.readline()
    if stdoutdataLine:
      stdoutdata += stdoutdataLine
    else:
      break
    begin = stdoutdata.find("[== CMake MetaMagic ==[\n")
    end = stdoutdata.find("]== CMake MetaMagic ==]")

    if (begin != -1 and end != -1):
      begin += len("[== CMake MetaMagic ==[\n")
      payload = stdoutdata[begin:end]
      if print_communication:
        print "\nDAEMON>", json.loads(payload), "\n"
      return json.loads(payload)

def writePayload(process, content):
  payload = """
[== CMake MetaMagic ==[
%s
]== CMake MetaMagic ==]
""" % json.dumps(content)
  if print_communication:
    print "\nCLIENT>", content, "\n"
  process.stdin.write(payload)

def initProc(cmakeCommand, build_dir):

  cmakeProcess = subprocess.Popen([cmakeCommand, "-E", "daemon", build_dir],
                                  stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE)

  packet = waitForMessage(cmakeProcess)

  if packet["progress"] != "process-started":
    print "Process error"
    sys.exit(1)

  writePayload(cmakeProcess, {"type": "handshake"})

  packet = waitForMessage(cmakeProcess)

  if packet["progress"] != "initialized":
    print "Process error"
    sys.exit(1)

  packet = waitForMessage(cmakeProcess)

  if packet["progress"] != "configured":
    print "Process error"
    sys.exit(1)

  packet = waitForMessage(cmakeProcess)

  if packet["progress"] != "computed":
    print "Process error"
    sys.exit(1)

  packet = waitForMessage(cmakeProcess)

  if packet["progress"] != "idle":
    print "Process error"
    sys.exit(1)

  return cmakeProcess
