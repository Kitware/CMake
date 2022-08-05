
set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# when REGISTRY_VIEW is not specified, should not be defined in module
set (EXPECTED_REGISTRY_VIEW "UNDEFINED")
find_package(RegistryView)

# query package to check if variable is propagated correctly
set(EXPECTED_REGISTRY_VIEW "TARGET")
find_package(RegistryView REGISTRY_VIEW TARGET)

set(EXPECTED_REGISTRY_VIEW "64_32")
find_package(RegistryView REGISTRY_VIEW 64_32)

set(EXPECTED_REGISTRY_VIEW "32")
find_package(RegistryView REGISTRY_VIEW 32)
