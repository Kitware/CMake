include(RunCMake)

run_cmake(CREATE_LINK)
run_cmake(CREATE_LINK-COPY_ON_ERROR-file)
run_cmake(CREATE_LINK-noarg)
run_cmake(CREATE_LINK-noexist)

if(NOT WIN32
    AND NOT MSYS # FIXME: This works on CYGWIN but not on MSYS
    )
  run_cmake(CREATE_LINK-SYMBOLIC)
  run_cmake(CREATE_LINK-SYMBOLIC-noexist)
endif()

file(MAKE_DIRECTORY ${RunCMake_BINARY_DIR}/CMP0205-Inspect/Dest)

file(REMOVE_RECURSE ${RunCMake_BINARY_DIR}/CMP0205-Inspect-SymLink)
file(CREATE_LINK
  ${RunCMake_BINARY_DIR}/CMP0205-Inspect/Dest ${RunCMake_BINARY_DIR}/CMP0205-Inspect-SymLink
  SYMBOLIC
  RESULT SymLink_RESULT
)
if(SymLink_RESULT STREQUAL "0")
  message(STATUS "CMP0205-SymLink-* skipped: directory symbolic link creation works")
  file(REMOVE ${RunCMake_BINARY_DIR}/CMP0205-Inspect-SymLink)
else()
  run_cmake_script(CMP0205-SymLink-WARN)
  run_cmake_script(CMP0205-SymLink-OLD)
  run_cmake_script(CMP0205-SymLink-NEW)
endif()

file(REMOVE_RECURSE ${RunCMake_BINARY_DIR}/CMP0205-Inspect-HardLink)
file(CREATE_LINK
  ${RunCMake_BINARY_DIR}/CMP0205-Inspect/Dest ${RunCMake_BINARY_DIR}/CMP0205-Inspect-HardLink
  RESULT HardLink_RESULT
)
if(HardLink_RESULT STREQUAL "0")
  message(STATUS "CMP0205-HardLink-* skipped: directory hard link creation works")
  file(REMOVE_RECURSE ${RunCMake_BINARY_DIR}/CMP0205-Inspect-HardLink)
else()
  run_cmake_script(CMP0205-HardLink-WARN)
  run_cmake_script(CMP0205-HardLink-OLD)
  run_cmake_script(CMP0205-HardLink-NEW)
endif()
