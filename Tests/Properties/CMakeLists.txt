# a simple CXX only test case
cmake_minimum_required (VERSION 2.6)
project (Properties)

# these first three tests really test both properties and the management of
# cmSourceFile objects by CMake.

# test properties on a build tree file that is relative (yuck)
configure_file(properties.h.in "${Properties_BINARY_DIR}/properties.h")
set_source_files_properties(properties.h PROPERTIES TEST1 1)
get_source_file_property(RESULT1 properties.h TEST1)

# test properties on a headerfile in the source tree
# accessed without an extension (also yuck)
set_source_files_properties(properties2 PROPERTIES TEST2 1)
get_source_file_property(RESULT2 properties2 TEST2)

# test properties on a relative source that is not generated
set_source_files_properties(SubDir/properties3.cxx PROPERTIES TEST3 1)
get_source_file_property(RESULT3 SubDir/properties3.cxx TEST3)

include_directories("${Properties_SOURCE_DIR}" "${Properties_BINARY_DIR}")


# test generic property interfaces
get_property(GLOBALRESULT GLOBAL PROPERTY GLOBALTEST DEFINED)
if (GLOBALRESULT)
    message(SEND_ERROR "Error: global prop defined when it should not be, "
            "result is GLOBALRESULT=${GLOBALRESULT}")
endif ()

define_property(GLOBAL PROPERTY GLOBALTEST
  BRIEF_DOCS "A test property"
  FULL_DOCS "A long description of this test property"
  )

get_property(GLOBALRESULT GLOBAL PROPERTY GLOBALTEST DEFINED)
if (NOT GLOBALRESULT)
    message(SEND_ERROR "Error: global prop not defined "
            "result is GLOBALRESULT=${GLOBALRESULT}")
endif ()

set_property(GLOBAL PROPERTY GLOBALTEST 1)
set_property(DIRECTORY PROPERTY DIRECTORYTEST 1)
set_property(SOURCE SubDir/properties3.cxx PROPERTY SOURCETEST 1)
get_property(GLOBALRESULT GLOBAL PROPERTY GLOBALTEST)
get_property(DIRECTORYRESULT DIRECTORY PROPERTY DIRECTORYTEST)
get_property(SOURCERESULT
  SOURCE SubDir/properties3.cxx
  PROPERTY SOURCETEST
  )

if (RESULT1 AND RESULT2 AND RESULT3 AND GLOBALRESULT AND
    DIRECTORYRESULT AND SOURCERESULT)
  add_executable (Properties SubDir/properties3.cxx properties)
else ()
  message(SEND_ERROR
    "Error: test results are RESULT1=${RESULT1} RESULT2=${RESULT2} "
    "RESULT3=${RESULT3} GLOBALRESULT=${GLOBALRESULT} "
    "DIRECTORYRESULT=${DIRECTORYRESULT} "
    "SOURCERESULT=${SOURCERESULT}")
endif ()

# test the target property
set_property(TARGET Properties PROPERTY TARGETTEST 1)
get_property(TARGETRESULT TARGET Properties PROPERTY TARGETTEST)
if (NOT TARGETRESULT)
    message(SEND_ERROR
      "Error: target result is TARGETRESULT=${TARGETRESULT}")
endif ()

# test APPEND and APPEND_STRING set_property()
set_property(TARGET Properties PROPERTY FOO foo)
set_property(TARGET Properties PROPERTY BAR bar)
set_property(TARGET Properties APPEND PROPERTY FOO 123)
set_property(TARGET Properties APPEND_STRING PROPERTY BAR 456)

get_property(APPEND_RESULT TARGET Properties PROPERTY FOO)
if (NOT "${APPEND_RESULT}" STREQUAL "foo;123")
    message(SEND_ERROR
      "Error: target result is APPEND_RESULT=${APPEND_RESULT}")
endif ()

get_property(APPEND_STRING_RESULT TARGET Properties PROPERTY BAR)
if (NOT "${APPEND_STRING_RESULT}" STREQUAL "bar456")
    message(SEND_ERROR
      "Error: target result is APPEND_STRING_RESULT=${APPEND_STRING_RESULT}")
endif ()

# test get_property SET
get_property(TARGETRESULT TARGET Properties PROPERTY TARGETTEST SET)
if (NOT TARGETRESULT)
    message(SEND_ERROR
      "Error: target prop not set, result is TARGETRESULT=${TARGETRESULT}")
endif ()

# test unsetting a property
set_property(TARGET Properties PROPERTY TARGETTEST)
get_property(TARGETRESULT TARGET Properties PROPERTY TARGETTEST SET)
if (TARGETRESULT)
    message(SEND_ERROR "Error: target prop not unset, "
            "result is TARGETRESULT=${TARGETRESULT}")
endif ()



# test the target SOURCES property
get_property(Properties_SOURCES TARGET Properties PROPERTY SOURCES)
set_source_files_properties(${Properties_SOURCES} PROPERTIES TEST4 1)
get_source_file_property(RESULT4 properties.h TEST4)
if(NOT RESULT4)
  message(SEND_ERROR "Error: target result is"
    " RESULT4=${RESULT4}"
    " Properties_SOURCES=[${Properties_SOURCES}]")
endif()

# test CACHE properties
macro(check_cache_props)
  foreach(prop VALUE TYPE HELPSTRING ADVANCED STRINGS)
    get_property(result CACHE SOME_ENTRY PROPERTY ${prop})
    if(NOT "x${result}" STREQUAL "x${expect_${prop}}")
      message(SEND_ERROR "CACHE property ${prop} is [${result}], not [${expect_${prop}}]")
    endif()
  endforeach()
endmacro()
set(expect_VALUE "ON")
set(expect_TYPE "BOOL")
set(expect_HELPSTRING "sample cache entry")
set(expect_ADVANCED 0)
set(expect_STRINGS "")
set(SOME_ENTRY "${expect_VALUE}" CACHE ${expect_TYPE} "${expect_HELPSTRING}" FORCE)
mark_as_advanced(CLEAR SOME_ENTRY)
set_property(CACHE SOME_ENTRY PROPERTY STRINGS "")
check_cache_props()
set(expect_VALUE "Some string")
set(expect_TYPE "STRING")
set(expect_HELPSTRING "sample cache entry help")
set(expect_ADVANCED 1)
set(expect_STRINGS "Some string;Some other string;Some third string")
set_property(CACHE SOME_ENTRY PROPERTY TYPE "${expect_TYPE}")
set_property(CACHE SOME_ENTRY PROPERTY HELPSTRING "${expect_HELPSTRING}")
set_property(CACHE SOME_ENTRY PROPERTY VALUE "${expect_VALUE}")
set_property(CACHE SOME_ENTRY PROPERTY ADVANCED "${expect_ADVANCED}")
set_property(CACHE SOME_ENTRY PROPERTY STRINGS "${expect_STRINGS}")
check_cache_props()

function(generate_file_for_set_property_test i target_name)
    set(src_path "${CMAKE_CURRENT_BINARY_DIR}/src${i}.cpp")
    file(CONFIGURE OUTPUT "${src_path}" CONTENT
        "#ifndef def${i}\n\
        #error Expected def${i}\n\
        #endif\n\
        #ifdef _WIN32\n\
        __declspec(dllexport)\n\
        #endif\n\
        void dummy_symbol${i}() {}\n"
        NEWLINE_STYLE UNIX)
    target_sources(${target_name} PRIVATE "${src_path}")
endfunction()

add_library(maindirtest SHARED)

# Generate file to be used with both DIRECTORY and TARGET_DIRECTORY options in
# set_source_files_properties and set_property().
generate_file_for_set_property_test(32 maindirtest)
generate_file_for_set_property_test(33 maindirtest)

# Set/get properties by binary directory path.
add_subdirectory(SubDir SubDirA)
get_property(dir_prop_top DIRECTORY PROPERTY dir_prop_top)
if(NOT dir_prop_top STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/SubDirA")
  message(SEND_ERROR "dir_prop_top unexpected value after SubDirA:\n ${dir_prop_top}")
endif()
add_subdirectory(SubDir SubDirB)
get_property(dir_prop_top DIRECTORY PROPERTY dir_prop_top)
if(NOT dir_prop_top STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/SubDirB")
  message(SEND_ERROR "dir_prop_top unexpected value after SubDirB:\n ${dir_prop_top}")
endif()
get_property(dir_prop_subA DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/SubDirA PROPERTY dir_prop_sub)
if(NOT dir_prop_subA STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/SubDirA")
  message(SEND_ERROR "SubDirA property dir_prop_sub incorrect:\n ${dir_prop_subA}")
endif()
get_property(dir_prop_subB DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/SubDirB PROPERTY dir_prop_sub)
if(NOT dir_prop_subB STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/SubDirB")
  message(SEND_ERROR "SubDirB property dir_prop_sub incorrect:\n ${dir_prop_subB}")
endif()
get_directory_property(dir_prop_subA DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/SubDirA dir_prop_sub)
if(NOT dir_prop_subA STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/SubDirA")
  message(SEND_ERROR "SubDirA property dir_prop_sub incorrect:\n ${dir_prop_subA}")
endif()
get_directory_property(dir_prop_subB DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/SubDirB dir_prop_sub)
if(NOT dir_prop_subB STREQUAL "${CMAKE_CURRENT_BINARY_DIR}/SubDirB")
  message(SEND_ERROR "SubDirB property dir_prop_sub incorrect:\n ${dir_prop_subB}")
endif()

add_subdirectory(SubDir2)

set(src_prefix "${CMAKE_CURRENT_BINARY_DIR}/SubDir2/")

# Set property + target directory
set_property(SOURCE "${src_prefix}/src1.cpp"
             TARGET_DIRECTORY set_prop_lib_1
             PROPERTY COMPILE_DEFINITIONS def1)

# Append property + target directory
set_property(SOURCE "${src_prefix}/src2.cpp"
             TARGET_DIRECTORY set_prop_lib_1
             APPEND PROPERTY COMPILE_DEFINITIONS def2)

# Set property + relative directory path
set_property(SOURCE "${src_prefix}/src3.cpp"
             DIRECTORY SubDir2
             PROPERTY COMPILE_DEFINITIONS def3)

# Set property + absolute directory path
set_property(SOURCE "${src_prefix}/src4.cpp"
             DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/SubDir2"
             PROPERTY COMPILE_DEFINITIONS def4)

# Append property + relative directory path
set_property(SOURCE "${src_prefix}/src5.cpp"
             DIRECTORY SubDir2
             APPEND PROPERTY COMPILE_DEFINITIONS def5)

# Append property + absolute directory path
set_property(SOURCE "${src_prefix}/src6.cpp"
             DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/SubDir2"
             APPEND PROPERTY COMPILE_DEFINITIONS def6)


# Target directory
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/SubDir2/src10.cpp"
                             TARGET_DIRECTORY set_prop_lib_1
                             PROPERTIES COMPILE_DEFINITIONS def10)

# Relative directory path
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/SubDir2/src11.cpp"
                            DIRECTORY SubDir2
                            PROPERTIES COMPILE_DEFINITIONS def11)

# Absolute directory path
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/SubDir2/src12.cpp"
                            DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/SubDir2"
                            PROPERTIES COMPILE_DEFINITIONS def12)


# Multiple files + absolute directory path
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/SubDir2/src20.cpp"
                            "${CMAKE_CURRENT_BINARY_DIR}/SubDir2/src21.cpp"
                            DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/SubDir2"
                            PROPERTIES COMPILE_DEFINITIONS "def20;def21")

# Multiple files + multiple target directories
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/SubDir2/src22.cpp"
                            "${CMAKE_CURRENT_BINARY_DIR}/SubDir2/src23.cpp"
                            TARGET_DIRECTORY set_prop_lib_2 set_prop_lib_3
                            PROPERTIES COMPILE_DEFINITIONS "def22;def23")


# Multiple files in multiple relative directories
generate_file_for_set_property_test(30 maindirtest)
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/src30.cpp"
                            "${CMAKE_CURRENT_BINARY_DIR}/SubDir2/src31.cpp"
                            DIRECTORY . SubDir2
                            PROPERTIES COMPILE_DEFINITIONS "def30;def31")

# Check that specifying files without any properties doesn't crash.
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/src30.cpp"
                            "${CMAKE_CURRENT_BINARY_DIR}/SubDir2/src31.cpp")


# Check that specifying both DIRECTORY and TARGET_DIRECTORY works.
set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/src32.cpp"
                            DIRECTORY .
                            TARGET_DIRECTORY set_prop_lib_3
                            PROPERTIES COMPILE_DEFINITIONS "def32")

set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/src33.cpp"
             DIRECTORY SubDir2
             TARGET_DIRECTORY maindirtest
             PROPERTY COMPILE_DEFINITIONS "def33")


function(check_get_property_value expected)
    if(NOT actual STREQUAL expected)
        message(SEND_ERROR "Error: get_property returned unexpected value\n"
                "actual: ${actual}\n"
                "expected: ${expected}")
    endif()
endfunction()

# Check that source file directory scopes are deduplicated.
set_property(SOURCE "${CMAKE_CURRENT_BINARY_DIR}/src32.cpp"
    DIRECTORY SubDir2 SubDir2 SubDir2
    TARGET_DIRECTORY set_prop_lib_3 set_prop_lib_3 set_prop_lib_3
    APPEND
    PROPERTY NON_DUPLICATED_CUSTOM_PROP 1
)

get_property(actual
             SOURCE "${CMAKE_CURRENT_BINARY_DIR}/src32.cpp"
             DIRECTORY SubDir2
             PROPERTY NON_DUPLICATED_CUSTOM_PROP)
check_get_property_value("1")

get_source_file_property(actual "${CMAKE_CURRENT_BINARY_DIR}/src32.cpp"
                         TARGET_DIRECTORY set_prop_lib_3
                         NON_DUPLICATED_CUSTOM_PROP)
check_get_property_value("1")

# Get property + target directory
get_property(actual
             SOURCE "${src_prefix}/src1.cpp"
             TARGET_DIRECTORY set_prop_lib_1
             PROPERTY COMPILE_DEFINITIONS)
check_get_property_value("def1")

# Get property + relative directory path
get_property(actual
             SOURCE "${src_prefix}/src3.cpp"
             DIRECTORY SubDir2
             PROPERTY COMPILE_DEFINITIONS)
check_get_property_value("def3")

# Get property + absolute directory path
get_property(actual
             SOURCE "${src_prefix}/src4.cpp"
             DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/SubDir2"
             PROPERTY COMPILE_DEFINITIONS)
check_get_property_value("def4")


# Get property + target directory
unset(actual)
get_source_file_property(actual
                         "${CMAKE_CURRENT_BINARY_DIR}/SubDir2/src10.cpp"
                         TARGET_DIRECTORY set_prop_lib_1
                         COMPILE_DEFINITIONS)
check_get_property_value("def10")

# Get property + relative directory path
get_source_file_property(actual
                         "${CMAKE_CURRENT_BINARY_DIR}/SubDir2/src11.cpp"
                         DIRECTORY SubDir2
                         COMPILE_DEFINITIONS)
check_get_property_value("def11")

# Get property + absolute directory path
get_source_file_property(actual
                         "${CMAKE_CURRENT_BINARY_DIR}/SubDir2/src12.cpp"
                         DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/SubDir2"
                         COMPILE_DEFINITIONS)
check_get_property_value("def12")
