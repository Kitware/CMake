set(CMAKE_CONFIGURATION_TYPES Debug)

set(SRC_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/Dir/foo.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Dir/DirNested/foo_nested.cpp
)

set(PREFIXED_SRC_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/Prefixed/bar.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Prefixed/PrefixedNested/bar_nested.cpp
)

add_custom_target(SourceGroupTree SOURCES ${SRC_FILES} ${PREFIXED_SRC_FILES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Prefixed PREFIX SourcesPrefix FILES ${PREFIXED_SRC_FILES})
