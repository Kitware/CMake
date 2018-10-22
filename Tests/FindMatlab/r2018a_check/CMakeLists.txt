
cmake_minimum_required (VERSION 2.8.12)
enable_testing()
project(r2018a_checks)

set(MATLAB_FIND_DEBUG TRUE)

# this test doesn't do much if MATLAB version < R2018a

if(IS_MCR)
    set(RUN_UNIT_TESTS FALSE)
else()
    set(RUN_UNIT_TESTS TRUE)
    set(components MAIN_PROGRAM)
endif()

if(NOT "${MCR_ROOT}" STREQUAL "")
    set(Matlab_ROOT_DIR "${MCR_ROOT}")
    if(NOT EXISTS "${MCR_ROOT}")
        message(FATAL_ERROR "MCR does not exist ${MCR_ROOT}")
    endif()
endif()

find_package(Matlab REQUIRED COMPONENTS ${components})

set(IS_R2018a 1)
if(${Matlab_VERSION_STRING} VERSION_LESS "9.4")
    # This is an older version of MATLAB, tests will fail
    set(IS_R2018a 0)
endif()

matlab_add_mex(
    # target name
    NAME cmake_matlab_test_wrapper2a
    # output name
    OUTPUT_NAME cmake_matlab_mex2a
    SRC ${CMAKE_CURRENT_SOURCE_DIR}/../matlab_wrapper2.cpp
    R2017b
    )

matlab_add_mex(
    # target name
    NAME cmake_matlab_test_wrapper2b
    # output name
    OUTPUT_NAME cmake_matlab_mex2b
    SRC ${CMAKE_CURRENT_SOURCE_DIR}/../matlab_wrapper2.cpp
    R2018a
    )

if(IS_R2018a)
    matlab_add_mex(
        # target name
        NAME cmake_matlab_test_wrapper3
        # output name
        OUTPUT_NAME cmake_matlab_mex3
        SRC ${CMAKE_CURRENT_SOURCE_DIR}/../matlab_wrapper3.cpp
        )
    set_target_properties(
        cmake_matlab_test_wrapper3
        PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
        )
endif()

if(RUN_UNIT_TESTS)
    # Check that the R2017b and R2018a APIs work.
    matlab_add_unit_test(
        NAME ${PROJECT_NAME}_matlabtest-1
        TIMEOUT 300
        UNITTEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../cmake_matlab_unit_tests4.m
        ADDITIONAL_PATH $<TARGET_FILE_DIR:cmake_matlab_test_wrapper2a>
        )

    # Check that the C++ API works (test run only on R2018a and newer)
    if(IS_R2018a)
        matlab_add_unit_test(
            NAME ${PROJECT_NAME}_matlabtest-3
            TIMEOUT 300
            UNITTEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../cmake_matlab_unit_tests5.m
            ADDITIONAL_PATH $<TARGET_FILE_DIR:cmake_matlab_test_wrapper3>
            )
    endif()
endif()
