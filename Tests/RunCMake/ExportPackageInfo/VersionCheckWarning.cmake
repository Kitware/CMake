add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)

# Try exporting with an unrecognized schema.
export(
  PACKAGE_INFO foo
  EXPORT foo
  VERSION "irrelevant"
  VERSION_SCHEMA "unrecognized"
)
