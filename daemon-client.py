#!/usr/bin/env python2

import sys, subprocess, json, re

is_debug = False

termwidth = 150

print_communication = True

def col_print(title, array):
    print
    print
    print title

    indentwidth = 4
    indent = ' ' * indentwidth

    if not array:
        print indent + '<None>'
        return

    padwidth = 2

    maxitemwidth = len(max(array, key=len))

    numCols = max(1, int((termwidth - indentwidth + padwidth) / (maxitemwidth + padwidth)))

    numRows = len(array) // numCols + 1

    pad = ' ' * padwidth

    for index in range(numRows):
        print indent + pad.join(item.ljust(maxitemwidth) for item in array[index::numRows])


def print_error(packet):
    col_print('error', [packet['error']])


def print_build_system(build_info):
    col_print('version', [build_info['version']])
    col_print('generator', [build_info['generator']])

    col_print('configs', build_info['configs'])
    col_print('globalTargets', build_info['globalTargets'])

    # for target in build_info['targets']:
    #     col_print('project', [target['projectName']]);
    #     col_print('name', [target['name']]);
    #     col_print('type', [target['type']]);
        # col_print('backtrace', target['backtrace']);


def print_target_info(target_name, target_info):
    col_print('target_name',[target_name]);
    if 'build_location' in target_info.keys():
        col_print('build_location', [target_info['build_location']])

    col_print('compile_definitions', target_info['compile_definitions'])
    col_print('compile_options', target_info['compile_options'])
    col_print('compile_features', target_info['compile_features'])
    col_print('include_directories', target_info['include_directories'])

    col_print('header_sources', target_info['header_sources'])
    col_print('object_sources', target_info['object_sources'])

    col_print('generated_header_sources', target_info['generated_header_sources'])
    col_print('generated_object_sources', target_info['generated_object_sources'])


def print_file_info(file_name, file_info):
    col_print('target_name',[file_info['targetName']]);
    col_print('file_path',[file_info['filePath']]);

    col_print('c_compiler',[file_info['c_compiler']]);
    col_print('c_compiler_version',[file_info['c_compiler_version']]);

    col_print('cxx_compiler',[file_info['cxx_compiler']]);
    col_print('cxx_compiler_version',[file_info['cxx_compiler_version']]);

    col_print('compile_flags',[file_info['compile_flags']]);
    col_print('compile_definitions',file_info['compile_definitions']);

    col_print('include_directories',file_info['include_directories']);


def print_file_parse(file_path, parse):
    for token in parse['tokens']:
        print_parse_token(token);


def print_parse_token(token):
    col_print(token['type'],map(str,[token['line'], token['column'], token['length']]));


def print_cmake_cache(cache):
    for k,v in cache.iteritems():
        col_print(k,[v]);


def printUsage():
  print '''
  Usage: daemon-client.py /path/to/build

  The cmake binary is expected to be in the PATH.

  '''


def waitForMessage():
    stdoutdata = ''
    payload = ''
    while not cmakeProcess.poll():
        # this is the blocking statement, and there is no easy way to place a timeout on it.
        stdoutdataLine = cmakeProcess.stdout.readline()
        if is_debug and stdoutdataLine.strip(' \t\r\n') != '':
            sys.stdout.write('D> ')
            sys.stdout.write(stdoutdataLine)

        if stdoutdataLine:
            stdoutdata += stdoutdataLine
        else:
            break
        begin = stdoutdata.find('[== CMake MetaMagic ==[\n')
        end = stdoutdata.find(']== CMake MetaMagic ==]')

        if (begin != -1 and end != -1):
            begin += len('[== CMake MetaMagic ==[\n')
            payload = stdoutdata[begin:end]
            if print_communication:
                print '\nDAEMON>', json.loads(payload), '\n'
            return json.loads(payload)


def writePayload(content):
    payload = '''[== CMake MetaMagic ==[
%s
]== CMake MetaMagic ==]
''' % json.dumps(content)
    if print_communication:
        print '\nCLIENT>', content, '\n'
    cmakeProcess.stdin.write(payload)
    cmakeProcess.stdin.flush()


def serve_files (target_name, target_info):
    files = []
    files.extend(target_info['object_sources'])

    while True:
        print '''Serving files on target

        Source Dir: %(srcDir)s
        Binary Dir: %(binDir)s
        Project: %(projName)s
        Target: %(targetName)s
        ''' % {'srcDir': srcDir, 'binDir': binDir, 'projName': projectName, 'targetName': target_name}

        col_print('Source files', files)

        print
        print
        file_path = raw_input('Select a file of [%s] (\'q\' to exit to target selection): ' % target_name).strip(' \t\r\n');

        if file_path == 'q':
            return;

        if not file_path in files:
            for file in files:
                if re.search(file_path,file,re.IGNORECASE):
                    oldName = file_path
                    file_path = file
                    print
                    print
                    print 'Corrected \'' + oldName + '\' to \'' + file_path + '\''
                    raw_input('press any key to continue ...')
                    break

        if not file_path in files:
            print 'File not found: ' + file_path
            raw_input('press any key to continue ...')
            continue

        writePayload({'type': 'file_info', 'target_name': target_name, 'file_path': file_path, 'config':''})

        packet = waitForMessage()

        if 'error' in packet.keys():
            print_error(packet);
        else:
            print_file_info(file_path, packet['file_info']);
        print

        input = raw_input('file info dump complete, type \'parse\' to parse file, or anything else to continue: ').strip(' \t\r\n');
        print

        if input == 'parse':
            print 'parsing file ' + file_path

            writePayload({'type': 'parse', 'file_path': file_path })
            packet = waitForMessage()

            if 'error' in packet.keys():
                print_error(packet);
            else:
                print_file_parse(file_path, packet['parsed']);
            print


def serve_targets (packet):
    tgts = []
    for tgt in packet['buildsystem']['targets']:
        if tgt['type'] != 'UTILITY':
            tgts.append(tgt['name'])

    tgts.sort()

    while True:
        print '''Serving targets

        Source Dir: %(srcDir)s
        Binary Dir: %(binDir)s
        Project: %(projName)s
        ''' % {'srcDir': srcDir, 'binDir': binDir, 'projName': projectName}

        col_print('Targets', tgts)

        print
        print
        tgtName = raw_input('Select a target (\'q\' to exit): ').strip(' \t\r\n');

        if tgtName == 'q':
            sys.exit();

        if not tgtName in tgts:
            for tgt in tgts:
                if re.search(tgtName,tgt,re.IGNORECASE):
                    oldName = tgtName
                    tgtName = tgt
                    print
                    print
                    print 'Corrected \'' + oldName + '\' to \'' + tgtName + '\''
                    raw_input('press any key to continue ...')
                    break

        if not tgtName in tgts:
            print 'Target not found: ' + tgtName
            raw_input('press any key to continue ...')
            continue

        writePayload({'type': 'target_info', 'target_name': tgtName, 'config':''})

        packet = waitForMessage()

        if 'error' in packet.keys():
            print_error(packet);
        else:
            print_target_info(tgtName, packet['target_info']);
        print

        input = raw_input('target info dump complete. type \'fi\' for file service, or anything else to continue: ').strip(' \t\r\n');
        print
        if input == 'fi':
            print 'serving fileinfo on ' + tgtName
            serve_files(tgtName,packet['target_info'])


if len(sys.argv) < 2:
  printUsage()
  sys.exit(1)

cmakeProcess = subprocess.Popen(['cmake', '-E', 'daemon', sys.argv[1]],
                                stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE)

if is_debug:
    raw_input('cmake daemon process [pid=%s] created, press any key to continue ... ' % cmakeProcess.pid)

packet = waitForMessage()

if packet['progress'] != 'process-started':
    print 'Process error'
    sys.exit(1)

writePayload({'type': 'handshake'})

packet = waitForMessage()

if packet['progress'] != 'initialized':
    print 'Process error'
    sys.exit(1)

packet = waitForMessage()

if packet['progress'] != 'configured':
    print 'Process error'
    sys.exit(1)

packet = waitForMessage()

if packet['progress'] != 'computed':
    print 'Process error'
    sys.exit(1)

packet = waitForMessage()

if packet['progress'] != 'idle':
    print 'Process error'
    sys.exit(1)

srcDir = packet['source_dir']
binDir = packet['binary_dir']
projectName = packet['project_name']

writePayload({'type': 'cmake_variables'})

packet = waitForMessage()

print_cmake_cache(packet['cmake_variables']);

writePayload({'type': 'buildsystem'})

packet = waitForMessage()

print_build_system(packet['buildsystem']);
print
print

serve_targets(packet)
