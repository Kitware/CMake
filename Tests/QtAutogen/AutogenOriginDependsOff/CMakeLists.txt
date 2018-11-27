cmake_minimum_required(VERSION 3.11)
project(AutogenOriginDependsOff)
include("../AutogenCoreTest.cmake")

set(CSD ${CMAKE_CURRENT_SOURCE_DIR})
set(CBD ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CSD})
include_directories(${CBD})

# A GENERATED file ensures there will be an _autogen target in VS
add_custom_command (
    OUTPUT "${CBD}/config.hpp"
    COMMAND ${CMAKE_COMMAND} -E copy "${CSD}/config.hpp.in" "${CBD}/config.hpp"
    )


# Library "a_mc" provides a header that holds a string with the content of
# mocs_compilation.cpp from a_qt.  It therefore must depend on a_qt_autogen.
add_custom_target ( a_mc
    COMMAND ${CMAKE_COMMAND} -E sleep 2
    COMMAND ${CMAKE_COMMAND}
        "-DMCF=${CBD}/a_qt_autogen/mocs_compilation.cpp"
        "-DCF_IN=${CSD}/a_mc.hpp.in"
        "-DCF_OUT=${CBD}/a_mc.hpp"
        -P ${CSD}/configure_content.cmake
    )
add_dependencies ( a_mc a_qt_autogen )

# Library "a_qt"
# - depends on a GENERATED file
# - AUTOMOC enabled
# - depends on a target (a_mc) that depends on a_qt_qutogen
add_library ( a_qt a_qt.cpp "${CBD}/config.hpp" )
add_dependencies ( a_qt a_mc )
target_link_libraries ( a_qt ${QT_QTCORE_TARGET})
set_target_properties ( a_qt PROPERTIES AUTOMOC TRUE)
# Disable AUTOGEN_ORIGIN_DEPENDS to avoid loop dependencies
set_target_properties ( a_qt PROPERTIES AUTOGEN_ORIGIN_DEPENDS OFF)


# Library "b_mc" provides a header that holds a string function that returns
# the content of mocs_compilation.cpp from b_qt.
# It therefore must depend on b_qt_autogen.
add_custom_command (
    OUTPUT ${CBD}/b_mc.cpp
    DEPENDS b_qt_autogen
    COMMAND ${CMAKE_COMMAND} -E sleep 2
    COMMAND ${CMAKE_COMMAND}
        "-DMCF=${CBD}/b_qt_autogen/mocs_compilation.cpp"
        "-DCF_IN=${CSD}/b_mc.cpp.in"
        "-DCF_OUT=${CBD}/b_mc.cpp"
        -P ${CSD}/configure_content.cmake
    )
add_library ( b_mc ${CSD}/b_mc.hpp ${CBD}/b_mc.cpp )

# Library "b_qt"
# - depends on a GENERATED file
# - AUTOMOC enabled
# - depends on a library (b_mc) that depends on b_qt_qutogen
add_library ( b_qt b_qt.cpp "${CBD}/config.hpp" )
target_link_libraries ( b_qt b_mc )
target_link_libraries ( b_qt ${QT_QTCORE_TARGET})
set_target_properties ( b_qt PROPERTIES AUTOMOC TRUE)
# Disable AUTOGEN_ORIGIN_DEPENDS to avoid loop dependencies
set_target_properties ( b_qt PROPERTIES AUTOGEN_ORIGIN_DEPENDS OFF)


# The main target depends on both libraries which depend on the _autogen
# target of the main target.
add_executable ( autogenOriginDependsOff main.cpp )
target_link_libraries ( autogenOriginDependsOff a_qt b_qt )
