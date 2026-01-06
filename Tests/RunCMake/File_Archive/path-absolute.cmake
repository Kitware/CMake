# Test that path traversal attacks are blocked during file(ARCHIVE_EXTRACT)

set(EXTRACT_DIR "${CMAKE_CURRENT_BINARY_DIR}/extract_dir_abs")
# Use an absolute path within the build tree (but outside EXTRACT_DIR)
set(MALICIOUS_FILE "${CMAKE_CURRENT_BINARY_DIR}/SHOULD_NOT_EXIST_ABS.txt")

# Clean up
file(REMOVE_RECURSE "${EXTRACT_DIR}")
file(REMOVE "${MALICIOUS_FILE}")
file(MAKE_DIRECTORY "${EXTRACT_DIR}")

# Create a malicious tar archive using Python
# The archive contains a file with an absolute path
set(MALICIOUS_TAR "${CMAKE_CURRENT_BINARY_DIR}/malicious_abs.tar")
file(REMOVE "${MALICIOUS_TAR}")

execute_process(
  COMMAND "${Python_EXECUTABLE}" -c [==[
import sys
import tarfile
import io

# Create a tar archive in memory
tar_data = io.BytesIO()
with tarfile.open(fileobj=tar_data, mode='w') as tar:
    # Add a file with absolute path
    data = b'malicious content'
    info = tarfile.TarInfo(name=sys.argv[2])
    info.size = len(data)
    tar.addfile(info, io.BytesIO(data))

# Write to file
with open(sys.argv[1], 'wb') as f:
    f.write(tar_data.getvalue())
]==] "${MALICIOUS_TAR}" "${MALICIOUS_FILE}"
  RESULT_VARIABLE result
)

if(NOT result EQUAL 0)
  message(FATAL_ERROR "Failed to create malicious tar archive")
endif()

# Try to extract the malicious archive using file(ARCHIVE_EXTRACT)
file(ARCHIVE_EXTRACT
  INPUT "${MALICIOUS_TAR}"
  DESTINATION "${EXTRACT_DIR}"
)

# The file should not exist outside the extraction directory
if(EXISTS "${MALICIOUS_FILE}")
  message(FATAL_ERROR "PATH TRAVERSAL VULNERABILITY: File was created outside extraction directory!")
endif()
