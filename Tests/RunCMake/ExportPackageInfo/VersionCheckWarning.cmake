set(
  CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_INFO
  "7fa7d13b-8308-4dc7-af39-9e450456d68f"
)

add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)

# Try exporting with an unrecognized schema.
export(
  PACKAGE_INFO foo
  EXPORT foo
  VERSION "irrelevant"
  VERSION_SCHEMA "unrecognized"
)
