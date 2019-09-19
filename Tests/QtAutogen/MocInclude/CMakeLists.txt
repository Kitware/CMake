cmake_minimum_required(VERSION 3.15)
project(MocInclude)
get_filename_component(CS_REAL ${CMAKE_CURRENT_SOURCE_DIR} REALPATH)
include("${CS_REAL}/../AutogenCoreTest.cmake")

# Test moc include patterns

set(COM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Common")

macro(addCopyCommand from to)
    add_custom_command(
      OUTPUT ${to}
      COMMAND ${CMAKE_COMMAND} -E copy ${from} ${to}
      DEPENDS ${from})
endmacro()

# Create an executable
function(makeExecutable TARGET_NAME)
    # Utility variables
    set(CB_DIR "${CMAKE_CURRENT_BINARY_DIR}")

    # Copy directory
    file(REMOVE_RECURSE "${CB_DIR}/InIncludes")
    file(COPY "${COM_DIR}/InIncludes.in" DESTINATION "${CB_DIR}")
    file(RENAME "${CB_DIR}/InIncludes.in" "${CB_DIR}/InIncludes")

    # Generate .moc file from the header externally and
    # enabled SKIP_AUTOMOC on the source file
    qtx_wrap_cpp(ExternDotMOC ${COM_DIR}/ExternDot.hpp OPTIONS "-p" "./")
    addCopyCommand(${ExternDotMOC}
                   ${CB_DIR}/ExternDot.moc)
    set_property(
      SOURCE ${COM_DIR}/ExternDot.cpp
      PROPERTY SKIP_AUTOMOC ON)

    # Generate .moc file from the GENERATED header externally
    # and enabled SKIP_AUTOMOC on the source file
    addCopyCommand(${COM_DIR}/ExternDotGenerated.hpp.in
                   ${CB_DIR}/ExternDotGenerated.hpp)
    addCopyCommand(${COM_DIR}/ExternDotGenerated.cpp.in
                   ${CB_DIR}/ExternDotGenerated.cpp)
    qtx_wrap_cpp(ExternDotGeneratedMOC
                 ${CB_DIR}/ExternDotGenerated.hpp
                 OPTIONS "-p" "./")
    addCopyCommand(${ExternDotGeneratedMOC}
                   ${CB_DIR}/ExternDotGenerated.moc)
    set_property(
      SOURCE ${CB_DIR}/ExternDotGenerated.cpp
      PROPERTY SKIP_AUTOMOC ON)

    # Generate header moc file externally with a custom name
    # and enabled SKIP_AUTOMOC on the header
    qtx_wrap_cpp(MixedCustomMOC
                 ${COM_DIR}/MixedCustom.hpp
                 OPTIONS "-p" "./")
    addCopyCommand(${MixedCustomMOC}
                   ${CB_DIR}/MixedCustom_extMoc.cpp)
    set_property(
      SOURCE ${COM_DIR}/MixedCustom.hpp
      PROPERTY SKIP_AUTOMOC ON)
    # Custom target to depend on
    add_custom_target("${TARGET_NAME}_MixedCustom"
      DEPENDS ${CB_DIR}/MixedCustom_extMoc.cpp
      BYPRODUCTS ${CB_DIR}/moc_MixedCustom.cpp
      COMMAND ${CMAKE_COMMAND} -E copy
        ${COM_DIR}/moc_MixedCustom.cpp.in
        ${CB_DIR}/moc_MixedCustom.cpp)

    add_executable(${TARGET_NAME}
      # Test own "*.moc" and "moc_*.cpp" includes
      ${COM_DIR}/None.cpp
      ${COM_DIR}/OwnDot.cpp
      ${COM_DIR}/OwnUnderscore.cpp
      ${COM_DIR}/OwnDotUnderscore.cpp

      # Test "moc_*.cpp" includes of other files
      ${COM_DIR}/OtherUnderscore.cpp
      ${COM_DIR}/OtherUnderscoreExtra.cpp
      ${COM_DIR}/OtherUnderscoreSub.cpp
      ${COM_DIR}/OtherUnderscoreSubDir/SubExtra.cpp

      # Test relative ../../ path for moc includes
      ${COM_DIR}/DualSub/Second/Second.cpp
      ${COM_DIR}/DualSubMocked.cpp

      # Test externally generated moc files
      ${COM_DIR}/ExternDot.cpp
      ${CB_DIR}/ExternDot.moc

      # Test externally generated moc files for GENERATED source
      ${CB_DIR}/ExternDotGenerated.cpp
      ${CB_DIR}/ExternDotGenerated.moc

      # Test externally generated moc files and SKIP_AUTOMOC enabled header
      ${COM_DIR}/MixedSkipped.cpp
      ${COM_DIR}/MixedCustom.hpp
      ${COM_DIR}/MixedCustom.cpp

      # Test sources in a subdirectory
      ${CB_DIR}/InIncludes/SubOwnDot.cpp
      ${COM_DIR}/InIncludesMoc.cpp
    )
    add_dependencies(${TARGET_NAME} "${TARGET_NAME}_MixedCustom")
    target_include_directories(${TARGET_NAME} PRIVATE "${COM_DIR}")
    target_include_directories(${TARGET_NAME} PRIVATE "${CB_DIR}")
    target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}")
    target_link_libraries(${TARGET_NAME} ${QT_LIBRARIES})
    set_target_properties(${TARGET_NAME} PROPERTIES AUTOMOC ON)
endfunction()

add_subdirectory(Strict)
add_subdirectory(Relaxed)
