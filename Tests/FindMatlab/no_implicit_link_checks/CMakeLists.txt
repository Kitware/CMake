
cmake_minimum_required (VERSION 2.8.12)
enable_testing()
project(no_implicit_links_checks)

set(MATLAB_FIND_DEBUG TRUE)

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


matlab_add_mex(
    # target name
    NAME cmake_matlab_test_wrapper1
    # output name
    OUTPUT_NAME cmake_matlab_mex1
    SRC ${CMAKE_CURRENT_SOURCE_DIR}/../matlab_wrapper1.cpp
    DOCUMENTATION ${CMAKE_CURRENT_SOURCE_DIR}/../help_text1.m.txt
    NO_IMPLICIT_LINK_TO_MATLAB_LIBRARIES
    )

# Inspect the LINK_LIBRARIES properties to check if mex or mx are present
get_target_property(MATLAB_TEST_WRAPPER1_LINK_LIBRARIES
                    cmake_matlab_test_wrapper1 LINK_LIBRARIES)

string(FIND "${MATLAB_TEST_WRAPPER1_LINK_LIBRARIES}" "mx" SEARCH_RESULT)
if(NOT "${SEARCH_RESULT}" EQUAL "-1")
    message(FATAL_ERROR "Matlab::mx linked even if NO_IMPLICIT_LINK_TO_MATLAB_LIBRARIES was passed")
endif()

string(FIND "${MATLAB_TEST_WRAPPER1_LINK_LIBRARIES}" "mex" SEARCH_RESULT)
if(NOT "${SEARCH_RESULT}" EQUAL "-1")
    message(FATAL_ERROR "Matlab::mex linked even if NO_IMPLICIT_LINK_TO_MATLAB_LIBRARIES was passed")
endif()

string(FIND "${MATLAB_TEST_WRAPPER1_LINK_LIBRARIES}" "MatlabEngine" SEARCH_RESULT)
if(NOT "${SEARCH_RESULT}" EQUAL "-1")
    message(FATAL_ERROR "Matlab::MatlabEngine linked even if NO_IMPLICIT_LINK_TO_MATLAB_LIBRARIES was passed")
endif()

string(FIND "${MATLAB_TEST_WRAPPER1_LINK_LIBRARIES}" "MatlabDataArray" SEARCH_RESULT)
if(NOT "${SEARCH_RESULT}" EQUAL "-1")
    message(FATAL_ERROR "Matlab::MatlabDataArray linked even if NO_IMPLICIT_LINK_TO_MATLAB_LIBRARIES was passed")
endif()

# Link separately with Matlab::mx and Matlab::mex to ensure that compilation
# and run of the test is successful
target_link_libraries(cmake_matlab_test_wrapper1 PRIVATE Matlab::mx Matlab::mex)

if(RUN_UNIT_TESTS)
    matlab_add_unit_test(
        NAME ${PROJECT_NAME}_matlabtest-1
        TIMEOUT 300
        UNITTEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../cmake_matlab_unit_tests1.m
        ADDITIONAL_PATH $<TARGET_FILE_DIR:cmake_matlab_test_wrapper1>
        )
endif()
