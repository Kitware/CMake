cmake_minimum_required (VERSION 2.6)
project(TarTest)

# this is macro that we will be running
macro(EXEC_TAR_COMMAND DIR ARGS)
  exec_program("${CMAKE_COMMAND}" "${DIR}" ARGS "-E tar ${ARGS}" RETURN_VALUE RET)
  if(${RET})
    message(FATAL_ERROR "CMake tar command failed with arguments \"${ARGS}\"")
  endif()
endmacro()

# Create a directory structure
set(CHECK_FILES)
macro(COPY F1 F2)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${F1}" "${CMAKE_CURRENT_BINARY_DIR}/tar_dir/${F2}" COPYONLY)
  set(CHECK_FILES ${CHECK_FILES} "${F2}")
endmacro()
COPY("CMakeLists.txt" "f1.txt")
COPY("CMakeLists.txt" "d1/f1.txt")
COPY("CMakeLists.txt" "d 2/f1.txt")
COPY("CMakeLists.txt" "d + 3/f1.txt")
COPY("CMakeLists.txt" "d_4/f1.txt")
COPY("CMakeLists.txt" "d-4/f1.txt")
COPY("CMakeLists.txt" "My Special Directory/f1.txt")

if(UNIX)
  exec_program("ln" ARGS "-sf f1.txt \"${CMAKE_CURRENT_BINARY_DIR}/tar_dir/d1/f2.txt\"")
  set(CHECK_FILES ${CHECK_FILES} "d1/f2.txt")
endif()

# cleanup first in case there are files left from previous runs
# if the umask is odd on the machine it might create files that
# are not automatically over written.  These tests are run
# each time the configure step is run.
file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/test_tar.tar")
file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/test_tgz.tgz")
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/test_output_tar")
file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/test_output_tgz")

make_directory("${CMAKE_CURRENT_BINARY_DIR}/test_output_tar")
make_directory("${CMAKE_CURRENT_BINARY_DIR}/test_output_tgz")


# Run tests
EXEC_TAR_COMMAND("${CMAKE_CURRENT_BINARY_DIR}" "cvf \"${CMAKE_CURRENT_BINARY_DIR}/test_tar.tar\" tar_dir")
EXEC_TAR_COMMAND("${CMAKE_CURRENT_BINARY_DIR}" "cvfz \"${CMAKE_CURRENT_BINARY_DIR}/test_tgz.tgz\" tar_dir")

EXEC_TAR_COMMAND("${CMAKE_CURRENT_BINARY_DIR}/test_output_tar" "xvf \"${CMAKE_CURRENT_BINARY_DIR}/test_tar.tar\"")
EXEC_TAR_COMMAND("${CMAKE_CURRENT_BINARY_DIR}/test_output_tgz" "xvfz \"${CMAKE_CURRENT_BINARY_DIR}/test_tgz.tgz\"")

macro(CHECK_DIR_STRUCTURE DIR)
  foreach(file ${CHECK_FILES})
    set(sfile "${DIR}/${file}")
    set(rfile "${CMAKE_CURRENT_BINARY_DIR}/tar_dir/${file}")
    if(NOT EXISTS "${sfile}")
      message(SEND_ERROR "Cannot find file ${sfile}")
    else()
      exec_program("${CMAKE_COMMAND}" ARGS "-E compare_files \"${sfile}\" \"${rfile}\"" RETURN_VALUE ret)
      if(${ret})
        message(SEND_ERROR "Files \"${sfile}\" \"${rfile}\" are different")
      endif()
    endif()
  endforeach()
endmacro()

CHECK_DIR_STRUCTURE("${CMAKE_CURRENT_BINARY_DIR}/test_output_tar/tar_dir")

add_executable(TarTest TestTarExec.cxx)

