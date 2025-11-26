set(
  CMAKE_EXPERIMENTAL_EXPORT_PACKAGE_INFO
  "b80be207-778e-46ba-8080-b23bba22639e"
)

add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)

# Try exporting with an unrecognized schema.
install(
  PACKAGE_INFO foo
  EXPORT foo
  VERSION "irrelevant"
  VERSION_SCHEMA "unrecognized"
)
