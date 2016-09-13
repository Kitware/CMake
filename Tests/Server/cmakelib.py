import sys, subprocess, json

termwidth = 150

print_communication = True

def ordered(obj):
  if isinstance(obj, dict):
    return sorted((k, ordered(v)) for k, v in obj.items())
  if isinstance(obj, list):
    return sorted(ordered(x) for x in obj)
  else:
    return obj

def col_print(title, array):
  print
  print
  print(title)

  indentwidth = 4
  indent = " " * indentwidth

  if not array:
    print(indent + "<None>")
    return

  padwidth = 2

  maxitemwidth = len(max(array, key=len))

  numCols = max(1, int((termwidth - indentwidth + padwidth) / (maxitemwidth + padwidth)))

  numRows = len(array) // numCols + 1

  pad = " " * padwidth

  for index in range(numRows):
    print(indent + pad.join(item.ljust(maxitemwidth) for item in array[index::numRows]))

def waitForRawMessage(cmakeCommand):
  stdoutdata = ""
  payload = ""
  while not cmakeCommand.poll():
    stdoutdataLine = cmakeCommand.stdout.readline()
    if stdoutdataLine:
      stdoutdata += stdoutdataLine.decode('utf-8')
    else:
      break
    begin = stdoutdata.find("[== CMake Server ==[\n")
    end = stdoutdata.find("]== CMake Server ==]")

    if (begin != -1 and end != -1):
      begin += len("[== CMake Server ==[\n")
      payload = stdoutdata[begin:end]
      if print_communication:
        print("\nSERVER>", json.loads(payload), "\n")
      return json.loads(payload)

def writeRawData(cmakeCommand, content):
  writeRawData.counter += 1
  payload = """
[== CMake Server ==[
%s
]== CMake Server ==]
""" % content

  rn = ( writeRawData.counter % 2 ) == 0

  if rn:
    payload = payload.replace('\n', '\r\n')

  if print_communication:
    print("\nCLIENT>", content, "(Use \\r\\n:", rn, ")\n")
  cmakeCommand.stdin.write(payload.encode('utf-8'))
  cmakeCommand.stdin.flush()
writeRawData.counter = 0

def writePayload(cmakeCommand, obj):
  writeRawData(cmakeCommand, json.dumps(obj))

def initProc(cmakeCommand):
  cmakeCommand = subprocess.Popen([cmakeCommand, "-E", "server"],
                                  stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE)

  packet = waitForRawMessage(cmakeCommand)
  if packet == None:
    print("Not in server mode")
    sys.exit(1)

  if packet['type'] != 'hello':
    print("No hello message")
    sys.exit(1)

  return cmakeCommand

def waitForMessage(cmakeCommand, expected):
  data = ordered(expected)
  packet = ordered(waitForRawMessage(cmakeCommand))

  if packet != data:
    sys.exit(-1)
  return packet

def waitForReply(cmakeCommand, originalType, cookie):
  packet = waitForRawMessage(cmakeCommand)
  if packet['cookie'] != cookie or packet['type'] != 'reply' or packet['inReplyTo'] != originalType:
    sys.exit(1)

def waitForError(cmakeCommand, originalType, cookie, message):
  packet = waitForRawMessage(cmakeCommand)
  if packet['cookie'] != cookie or packet['type'] != 'error' or packet['inReplyTo'] != originalType or packet['errorMessage'] != message:
    sys.exit(1)

def waitForProgress(cmakeCommand, originalType, cookie, current, message):
  packet = waitForRawMessage(cmakeCommand)
  if packet['cookie'] != cookie or packet['type'] != 'progress' or packet['inReplyTo'] != originalType or packet['progressCurrent'] != current or packet['progressMessage'] != message:
    sys.exit(1)

def handshake(cmakeCommand, major, minor):
  version = { 'major': major }
  if minor >= 0:
    version['minor'] = minor

  writePayload(cmakeCommand, { 'type': 'handshake', 'protocolVersion': version, 'cookie': 'TEST_HANDSHAKE' })
  waitForReply(cmakeCommand, 'handshake', 'TEST_HANDSHAKE')
