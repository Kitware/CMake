add_executable(MakeTable)

target_sources(MakeTable
  PRIVATE
    MakeTable.cxx
)

add_custom_command(
  OUTPUT SqrtTable.h
  COMMAND MakeTable SqrtTable.h
  DEPENDS MakeTable
  VERBATIM
)

add_custom_target(RunMakeTable DEPENDS SqrtTable.h)

add_library(SqrtTable INTERFACE)

target_sources(SqrtTable
  INTERFACE
    FILE_SET HEADERS
    BASE_DIRS
      ${CMAKE_CURRENT_BINARY_DIR}
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/SqrtTable.h
)

add_dependencies(SqrtTable RunMakeTable)
