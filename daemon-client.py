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


def printUsage():
  print """
  Usage: daemon-client.py /path/to/build

  The cmake binary is expected to be in the PATH.

  """

if len(sys.argv) < 2:
  printUsage()
  sys.exit(1)

cmakeProcess = subprocess.Popen(["cmake", "-E", "daemon", sys.argv[1]],
                                stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE)

def waitForMessage():
  stdoutdata = ""
  payload = ""
  while not cmakeProcess.poll():
    stdoutdataLine = cmakeProcess.stdout.readline()
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

def writePayload(content):
  payload = """
[== CMake MetaMagic ==[
%s
]== CMake MetaMagic ==]
""" % json.dumps(content)
  if print_communication:
    print "\nCLIENT>", content, "\n"
  cmakeProcess.stdin.write(payload)
  cmakeProcess.stdin.flush()


packet = waitForMessage()

if packet["progress"] != "process-started":
  print "Process error"
  sys.exit(1)

writePayload({"type": "handshake"})

packet = waitForMessage()

if packet["progress"] != "initialized":
  print "Process error"
  sys.exit(1)

packet = waitForMessage()

if packet["progress"] != "configured":
  print "Process error"
  sys.exit(1)

packet = waitForMessage()

if packet["progress"] != "computed":
  print "Process error"
  sys.exit(1)

packet = waitForMessage()

if packet["progress"] != "idle":
  print "Process error"
  sys.exit(1)

srcDir = packet["source_dir"]
binDir = packet["binary_dir"]
projectName = packet["project_name"]

writePayload({"type": "buildsystem"})

packet = waitForMessage()

tgts = []
for tgt in packet["buildsystem"]["targets"]:
  if tgt["type"] != "UTILITY":
    tgts.append(tgt["name"])

tgts.sort()


while True:

  print """Serving

  Source Dir: %(srcDir)s
  Binary Dir: %(binDir)s
  Project: %(projName)s
  """ % {"srcDir": srcDir, "binDir": binDir, "projName": projectName}

  col_print("Targets", tgts)

  print
  print
  tgtName = raw_input("Select a target: ")

  if not tgtName in tgts:
    for tgt in tgts:
      if tgtName in tgt:
        oldName = tgtName
        tgtName = tgt
        print
        print
        print "Corrected \"" + oldName + "\" to \"" + tgtName + "\""
        break
  if not tgtName in tgts:
    print
    print "Target not found: " + tgtName
    print
    continue

  writePayload({"type": "target_info",
                "target_name": tgtName,
                "config":""})

  packet = waitForMessage()

  tgtInfo = packet["target_info"]

  col_print("Sources", tgtInfo["object_sources"])
  col_print("Generated Sources", tgtInfo["generated_object_sources"])
  col_print("Include Directories", tgtInfo["include_directories"])
  col_print("Compile Definitions", tgtInfo["compile_definitions"])
  col_print("Compile Options", tgtInfo["compile_options"])
  print

  raw_input("Press Enter to continue")
  print
