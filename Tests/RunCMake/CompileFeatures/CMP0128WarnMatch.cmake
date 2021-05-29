enable_language(@lang@)
cmake_policy(SET CMP0128 OLD)
set(CMAKE_POLICY_WARNING_CMP0128 ON)

set(CMAKE_@lang@_EXTENSIONS @extensions_default@)
set(CMAKE_@lang@_STANDARD @standard_default@)
add_library(foo "@RunCMake_SOURCE_DIR@/empty.@ext@")
