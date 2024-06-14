# Create files named 1 to 9
foreach(i RANGE 1 9)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${i}.txt" "This is file ${i}")
endforeach()

set(COMPONENT_NAMES c1 c2 c3 c4 c5)
foreach(j RANGE 1 5)
    # Select 4 file and install to the component
    math(EXPR COMPONENT_IDX "${j} - 1")
    list(GET COMPONENT_NAMES "${COMPONENT_IDX}" SELECTED_COMPONENT)
    math(EXPR END_FILE "${j} + 4")
    foreach(k RANGE ${j} ${END_FILE})
        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${k}.txt" DESTINATION "files" COMPONENT ${SELECTED_COMPONENT})
    endforeach()
endforeach()

if(RunCMake_SUBTEST_SUFFIX STREQUAL "conflict_file")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/conflict/1.txt" "This should create a conflict.")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/conflict/1.txt" DESTINATION "files" COMPONENT c2)
endif ()

# You cannot create symlink in Windows test environment. Instead mock the symlink.
if(NOT CMAKE_HOST_WIN32)
    file(CREATE_LINK "${CMAKE_CURRENT_BINARY_DIR}/2.txt" "${CMAKE_CURRENT_BINARY_DIR}/symlink2" SYMBOLIC)
else()
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/symlink2" "This is file 2")
endif()
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/symlink2" DESTINATION "files" COMPONENT c1)

if(RunCMake_SUBTEST_SUFFIX STREQUAL "conflict_symlink" AND NOT CMAKE_HOST_WIN32)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/conflict)
    file(CREATE_LINK "${CMAKE_CURRENT_BINARY_DIR}/1.txt" "${CMAKE_CURRENT_BINARY_DIR}/conflict/symlink2" SYMBOLIC)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/conflict/symlink2" DESTINATION "files" COMPONENT c2)
elseif(RunCMake_SUBTEST_SUFFIX STREQUAL "conflict_symlink" AND CMAKE_HOST_WIN32)
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/conflict/symlink2" "This should create a conflict.")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/conflict/symlink2" DESTINATION "files" COMPONENT c2)
else()
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/symlink2" DESTINATION "files" COMPONENT c2)
endif ()


if(PACKAGING_TYPE STREQUAL "COMPONENT")
    set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON)
    set(CPACK_COMPONENTS_ALL "c1;c2;c3;c4")
elseif(PACKAGING_TYPE STREQUAL "GROUP")
    set(CPACK_COMPONENTS_ONE_PACKAGE_PER_GROUP ON)
    set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
    include(CPackComponent)

    cpack_add_component_group(g1 DISPLAY_NAME "Group 1")
    cpack_add_component_group(g2 DISPLAY_NAME "Group 2")
    cpack_add_component(c1
            DISPLAY_NAME "Group 1"
            DESCRIPTION "Component for Group 1"
            GROUP g1
    )
    cpack_add_component(c2
            DISPLAY_NAME "Group 1"
            DESCRIPTION "Component for Group 1"
            GROUP g1
    )
    cpack_add_component(c3
            DISPLAY_NAME "Group 2"
            DESCRIPTION "Component for Group 2"
            GROUP g2
    )
    cpack_add_component(c4
            DISPLAY_NAME "Group 2"
            DESCRIPTION "Component for Group 2"
            GROUP g2
    )

    set(CPACK_${GENERATOR_TYPE}_PACKAGE_GROUP g1 g2)
endif ()
