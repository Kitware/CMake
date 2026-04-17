set_directory_properties(PROPERTIES MY_PROP "top_val" LABELS "top_label")
add_subdirectory(PrintPropertiesDirectories-sub)

cmake_language(
  PRINT_PROPERTIES
  DIRECTORIES . PrintPropertiesDirectories-sub
  NAMED
    MY_PROP
    LABELS
    NOT_SET
)
