cmake_minimum_required(VERSION 3.12)
project(GlobalAutogenTarget)
include("../AutogenCoreTest.cmake")

# This tests
# CMAKE_GLOBAL_AUTOGEN_TARGET,
# CMAKE_GLOBAL_AUTORCC_TARGET,
# CMAKE_GLOBAL_AUTOGEN_TARGET_NAME and
# CMAKE_GLOBAL_AUTORCC_TARGET_NAME
# for the latter two with different values in different subdirectories.

# Directories
set(GAT_SDIR "${CMAKE_CURRENT_SOURCE_DIR}/GAT")
set(GAT_BDIR "${CMAKE_CURRENT_BINARY_DIR}/GAT")
# Files
set(MCA "sda/sda_autogen/mocs_compilation.cpp")
set(MCB "sdb/sdb_autogen/mocs_compilation.cpp")
set(MCC "sdc/sdc_autogen/mocs_compilation.cpp")
set(MCG "gat_autogen/mocs_compilation.cpp")

set(DRA "sda/sda_autogen/*qrc_data.cpp")
set(DRB "sdb/sdb_autogen/*qrc_data.cpp")
set(DRC "sdc/sdc_autogen/*qrc_data.cpp")
set(DRG "gat_autogen/*qrc_data.cpp")

# -- Utility macros
macro(GAT_FIND_FILES VAR NAME)
    file(GLOB_RECURSE ${VAR} ${GAT_BDIR}/*${NAME})
endmacro()

macro(GAT_FIND_FILE NAME)
    GAT_FIND_FILES(LST ${NAME})
    if(LST)
        message("Good find ${LST}")
    else()
        message(SEND_ERROR "Expected to find ${GAT_BDIR}/${NAME}")
    endif()
    unset(LST)
endmacro()

macro(GAT_FIND_FILE_NOT NAME)
    GAT_FIND_FILES(LST ${NAME})
    if(LST)
        message(SEND_ERROR "Not expected to find ${GAT_BDIR}/${NAME}")
    else()
        message("Good not find ${GAT_BDIR}/${NAME}")
    endif()
    unset(LST)
endmacro()

macro(GAT_BUILD_TARGET NAME)
    message("___ Building GAT ${NAME} target ___")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" --build "${GAT_BDIR}" --target ${NAME}
        WORKING_DIRECTORY "${GAT_BDIR}"
        RESULT_VARIABLE result)
    if (result)
      message(SEND_ERROR "Building of GAT ${NAME} target failed")
    endif()
endmacro()


# -- Remove and recreate build directory
file(REMOVE_RECURSE ${GAT_BDIR})
file(MAKE_DIRECTORY ${GAT_BDIR})


# -- Configure project
message("___ Configuring GAT project ___")
execute_process(
    COMMAND "${CMAKE_COMMAND}" "${GAT_SDIR}"
        -G "${CMAKE_GENERATOR}"
        -A "${CMAKE_GENERATOR_PLATFORM}"
        -T "${CMAKE_GENERATOR_TOOLSET}"
        "-DQT_TEST_VERSION=${QT_TEST_VERSION}"
        "-DCMAKE_AUTOGEN_VERBOSE=${CMAKE_AUTOGEN_VERBOSE}"
        "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
    WORKING_DIRECTORY "${GAT_BDIR}"
    OUTPUT_VARIABLE output
    RESULT_VARIABLE result)
if (result)
  message(SEND_ERROR "Configuring of GAT project failed")
else()
  message("Configuring of GAT project succeeded")
  message("${output}")
endif()


# -- Build autogen subtargets
GAT_BUILD_TARGET("autogen")
GAT_FIND_FILE("${MCA}")
GAT_FIND_FILE_NOT("${MCB}")
GAT_FIND_FILE_NOT("${MCC}")
GAT_FIND_FILE("${MCG}")

GAT_BUILD_TARGET("global_autogen_sdb")
GAT_FIND_FILE("${MCA}")
GAT_FIND_FILE("${MCB}")
GAT_FIND_FILE_NOT("${MCC}")
GAT_FIND_FILE("${MCG}")

GAT_BUILD_TARGET("all_autogen")
GAT_FIND_FILE("${MCA}")
GAT_FIND_FILE("${MCB}")
GAT_FIND_FILE("${MCC}")
GAT_FIND_FILE("${MCG}")


# -- Build autorcc subtargets
GAT_BUILD_TARGET("autorcc")
GAT_FIND_FILE("${DRA}")
GAT_FIND_FILE_NOT("${DRB}")
GAT_FIND_FILE_NOT("${DRC}")
GAT_FIND_FILE("${DRG}")

GAT_BUILD_TARGET("global_autorcc_sdb")
GAT_FIND_FILE("${DRA}")
GAT_FIND_FILE("${DRB}")
GAT_FIND_FILE_NOT("${DRC}")
GAT_FIND_FILE("${DRG}")

GAT_BUILD_TARGET("all_autorcc")
GAT_FIND_FILE("${DRA}")
GAT_FIND_FILE("${DRB}")
GAT_FIND_FILE("${DRC}")
GAT_FIND_FILE("${DRG}")
