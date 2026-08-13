cmake_policy(SET CMP0157 NEW)
cmake_policy(SET CMP0195 NEW)
cmake_policy(SET CMP0215 NEW)

if(NOT CMAKE_GENERATOR MATCHES "Ninja")
  message(SEND_ERROR "this test must use a Ninja generator, found ${CMAKE_GENERATOR}")
endif()

enable_language(Swift)

# The compile and emit-module edges share the same -output-file-map.
# Within the target, the object compile edge must order-only depend on the
# .swiftmodule so the two edges do not run concurrently and corrupt the
# shared module-level swift-dependencies entry.
add_library(L STATIC L.swift)
