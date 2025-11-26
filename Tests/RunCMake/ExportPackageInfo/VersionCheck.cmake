add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)

# Try exporting a 'properly' simple version.
export(EXPORT foo PACKAGE_INFO foo1 VERSION 1.2.3)

# Try exporting a version with many components.
export(EXPORT foo PACKAGE_INFO foo2 VERSION 1.21.23.33.37.42.9.0.12)

# Try exporting a version with a label.
export(EXPORT foo PACKAGE_INFO foo3 VERSION "1.2.3+git1234abcd")

# Try exporting a version with a different label.
export(EXPORT foo PACKAGE_INFO foo4 VERSION "1.2.3-0.example")

# Try exporting with the schema explicitly specified.
export(
  EXPORT foo
  PACKAGE_INFO foo5
  VERSION "1.2.3-0.example"
  VERSION_SCHEMA "simple"
)

# Try exporting with a custom-schema version.
export(
  EXPORT foo
  PACKAGE_INFO foo6
  VERSION "foo!test"
  VERSION_SCHEMA "custom"
)

# Try exporting with a recognized but not-checked schema.
export(
  EXPORT foo
  PACKAGE_INFO foo7
  VERSION "invalid"
  VERSION_SCHEMA "pep440"
)
