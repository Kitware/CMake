# Configure-time: prints a status-line note and continues.
cmake_language(
  PRINT_PROPERTIES
  TARGETS does_not_exist
  NAMED MY_PROP
)

# Deferred: prints a status-line note at generate time and continues.
cmake_language(
  PRINT_PROPERTIES
  TARGETS does_not_exist
  DEFERRED
  NAMED MY_PROP
)
