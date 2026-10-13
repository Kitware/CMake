include(GeneratedCommon.cmake)

# install(FILES) and install(PROGRAMS) imply build dependencies only for files
# that the build generates.  A file produced by a custom command attached to
# a single target is prepared by running only that command.
add_generated_file(generated_file generated.txt)
add_generated_file(generated_program generated.sh)
add_library(generated_files STATIC EXCLUDE_FROM_ALL
  "${generated_target_source}" "${generated_file}" "${generated_program}")
install(FILES "${generated_file}" lib.c
  DESTINATION share/install-prep
  COMPONENT Files
)
install(PROGRAMS "${generated_program}"
  DESTINATION bin
  COMPONENT Files
)

add_library(unit_tests STATIC neverbuild.c)
