
message(STATUS "Checking whether Doxygen wrote "
               "output to expected directories...")

foreach(DIR IN LISTS OUTPUT_DIRS)
    file(GLOB DIR_CONTENTS "${DIR}/*")

    list(LENGTH DIR_CONTENTS DIR_SIZE)

    if(DIR_SIZE EQUAL 0)
        message(FATAL_ERROR "Target directory \"${DIR}\" is empty. "
                            "Doxygen failed to generate output.")
    else()
        message(STATUS "Target directory \"${DIR}\" is not empty. "
                       "Doxygen generated output.")
    endif()
endforeach()

message(STATUS "Building 'clean' target...")

execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${BUILD_DIR} --target clean
    RESULT_VARIABLE CLEAN_RESULT
    ERROR_QUIET
)

if(NOT CLEAN_RESULT EQUAL 0)
    message(FATAL_ERROR "The 'clean' target failed to build")
endif()

message(STATUS "Successfully built 'clean' target")

message(STATUS "Checking whether the 'clean' target "
               "successfully cleaned the Doxygen output...")

foreach(DIR IN LISTS OUTPUT_DIRS)
    file(GLOB DIR_CONTENTS "${DIR}/*")

    list(LENGTH DIR_CONTENTS DIR_SIZE)

    if(DIR_SIZE EQUAL 0)
        message(STATUS "Target directory \"${DIR}\" is empty. "
                       "The 'clean' target succeeded.")
    else()
        message(FATAL_ERROR "Target directory \"${DIR}\" is not empty. "
                            "The 'clean' target failed.")
    endif()
endforeach()
