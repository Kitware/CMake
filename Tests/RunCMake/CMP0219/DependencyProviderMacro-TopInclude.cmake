macro(cmp0219_provider method package_name)
  set(cmp0219_provider_method "${method}")
  set(cmp0219_provider_package "${package_name}")
  set(cmp0219_provider_argn "${ARGN}")
  set(${package_name}_FOUND TRUE)
endmacro()

cmake_language(
  SET_DEPENDENCY_PROVIDER cmp0219_provider
  SUPPORTED_METHODS FIND_PACKAGE
)
