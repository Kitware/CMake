cmake_minimum_required (VERSION 2.6)
PROJECT(TarTest)

# this is macro that we will be running
MACRO(EXEC_TAR_COMMAND DIR ARGS)
  EXEC_PROGRAM("${CMAKE_COMMAND}" "${DIR}" ARGS "-E tar ${ARGS}" RETURN_VALUE RET)
  IF(${RET})
    MESSAGE(FATAL_ERROR "CMake tar command failed with arguments \"${ARGS}\"")
  ENDIF(${RET})
ENDMACRO(EXEC_TAR_COMMAND)

# Create a directory structure
SET(CHECK_FILES)
MACRO(COPY F1 F2)
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/${F1}" "${CMAKE_CURRENT_BINARY_DIR}/tar_dir/${F2}" COPYONLY)
  SET(CHECK_FILES ${CHECK_FILES} "${F2}")
ENDMACRO(COPY)
COPY("CMakeLists.txt" "f1.txt")
COPY("CMakeLists.txt" "d1/f1.txt")
COPY("CMakeLists.txt" "d 2/f1.txt")
COPY("CMakeLists.txt" "d + 3/f1.txt")
COPY("CMakeLists.txt" "d_4/f1.txt")
COPY("CMakeLists.txt" "d-4/f1.txt")
COPY("CMakeLists.txt" "My Special Directory/f1.txt")

IF(UNIX)
  EXEC_PROGRAM("ln" ARGS "-sf f1.txt \"${CMAKE_CURRENT_BINARY_DIR}/tar_dir/d1/f2.txt\"")
  SET(CHECK_FILES ${CHECK_FILES} "d1/f2.txt")
ENDIF(UNIX)

MAKE_DIRECTORY("${CMAKE_CURRENT_BINARY_DIR}/test_output_tar")
MAKE_DIRECTORY("${CMAKE_CURRENT_BINARY_DIR}/test_output_tgz")

# Run tests
EXEC_TAR_COMMAND("${CMAKE_CURRENT_BINARY_DIR}" "cvf \"${CMAKE_CURRENT_BINARY_DIR}/test_tar.tar\" tar_dir")
EXEC_TAR_COMMAND("${CMAKE_CURRENT_BINARY_DIR}" "cvfz \"${CMAKE_CURRENT_BINARY_DIR}/test_tgz.tgz\" tar_dir")

EXEC_TAR_COMMAND("${CMAKE_CURRENT_BINARY_DIR}/test_output_tar" "xvf \"${CMAKE_CURRENT_BINARY_DIR}/test_tar.tar\"")
EXEC_TAR_COMMAND("${CMAKE_CURRENT_BINARY_DIR}/test_output_tgz" "xvfz \"${CMAKE_CURRENT_BINARY_DIR}/test_tgz.tgz\"")

MACRO(CHECK_DIR_STRUCTURE DIR)
  FOREACH(file ${CHECK_FILES})
    SET(sfile "${DIR}/${file}")
    SET(rfile "${CMAKE_CURRENT_BINARY_DIR}/tar_dir/${file}")
    IF(NOT EXISTS "${sfile}")
      MESSAGE(SEND_ERROR "Cannot find file ${sfile}")
    ELSE(NOT EXISTS "${sfile}")
      EXEC_PROGRAM("${CMAKE_COMMAND}" ARGS "-E compare_files \"${sfile}\" \"${rfile}\"" RETURN_VALUE ret)
      IF(${ret})
        MESSAGE(SEND_ERROR "Files \"${sfile}\" \"${rfile}\" are different")
      ENDIF(${ret})
    ENDIF(NOT EXISTS "${sfile}")
  ENDFOREACH(file)
ENDMACRO(CHECK_DIR_STRUCTURE)

CHECK_DIR_STRUCTURE("${CMAKE_CURRENT_BINARY_DIR}/test_output_tar/tar_dir")

ADD_EXECUTABLE(TarTest TestTarExec.cxx)

