# Test that path traversal attacks are blocked during extraction

set(EXTRACT_DIR "${CMAKE_CURRENT_BINARY_DIR}/extract_dir")
set(PARENT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(MALICIOUS_FILE "${PARENT_DIR}/SHOULD_NOT_EXIST.txt")

# Clean up
file(REMOVE_RECURSE "${EXTRACT_DIR}")
file(REMOVE "${MALICIOUS_FILE}")
file(MAKE_DIRECTORY "${EXTRACT_DIR}")

# Create a malicious tar archive using Python
# The archive contains a file with path "../SHOULD_NOT_EXIST.txt"
set(MALICIOUS_TAR "${CMAKE_CURRENT_BINARY_DIR}/malicious.tar")
file(REMOVE "${MALICIOUS_TAR}")

execute_process(
  COMMAND "${Python_EXECUTABLE}" -c [==[
import sys
import tarfile
import io

# Create a tar archive in memory
tar_data = io.BytesIO()
with tarfile.open(fileobj=tar_data, mode='w') as tar:
    # Add a file with path traversal
    data = b'malicious content'
    info = tarfile.TarInfo(name='../SHOULD_NOT_EXIST.txt')
    info.size = len(data)
    tar.addfile(info, io.BytesIO(data))

# Write to file
with open(sys.argv[1], 'wb') as f:
    f.write(tar_data.getvalue())
]==] "${MALICIOUS_TAR}"
  RESULT_VARIABLE result
)

if(NOT result EQUAL 0)
  message(FATAL_ERROR "Failed to create malicious tar archive")
endif()

# Try to extract the malicious archive
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E tar xf "${MALICIOUS_TAR}"
  WORKING_DIRECTORY "${EXTRACT_DIR}"
  RESULT_VARIABLE extract_result
)

# The extraction should fail or the file should not exist outside extract dir
if(EXISTS "${MALICIOUS_FILE}")
  message(FATAL_ERROR "PATH TRAVERSAL VULNERABILITY: File was created outside extraction directory!")
endif()

if(extract_result EQUAL 0)
  message(FATAL_ERROR "Extraction of malicious path did not fail!")
endif()
