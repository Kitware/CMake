set(CMAKE_CONFIGURATION_TYPES Debug)

# Test regular tree grouping.
set(SRC_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/Dir/foo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Dir/DirNested/foo_nested.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_FILES})


# Test files that are not present at configuration time.
set(GENERATED_SRC_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/Generated/generated.cpp
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Generated)

if(WIN32)
  add_custom_command(OUTPUT ${GENERATED_SRC_FILES}
    COMMAND echo. 2>${CMAKE_CURRENT_BINARY_DIR}\\Generated\\generated.cpp
  )
else()
  add_custom_command(OUTPUT ${GENERATED_SRC_FILES}
    COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/Generated/generated.cpp
  )
endif()

source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} FILES ${GENERATED_SRC_FILES})


# Test prefixed tree grouping.
set(PREFIXED_SRC_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/Prefixed/bar.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Prefixed/PrefixedNested/bar_nested.cpp
)

add_custom_target(SourceGroupTree
  SOURCES
    ${SRC_FILES}
    ${GENERATED_SRC_FILES}
    ${PREFIXED_SRC_FILES}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Prefixed PREFIX SourcesPrefix FILES ${PREFIXED_SRC_FILES})
