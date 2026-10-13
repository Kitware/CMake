enable_language(C)

# Installing the artifact linked by a target as a file prepares it by building
# the target.
add_library(file_artifact STATIC EXCLUDE_FROM_ALL lib.c)
install(FILES "$<TARGET_FILE:file_artifact>"
  DESTINATION lib
  COMPONENT Artifacts
)

add_library(unit_tests STATIC neverbuild.c)
