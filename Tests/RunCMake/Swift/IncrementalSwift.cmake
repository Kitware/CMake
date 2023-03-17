enable_language(Swift)

# Write initial files to build directory
# The files are generated into the build directory to avoid dirtying the source
# directory. This is done because the source files are changed during the test
# to ensure correct incremental build behavior.
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/a.swift
  "let number: Int = 32\n"
  "public func callA() -> Int { return number }\n")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/b.swift
  "import A\n"
  "public func callB() -> Int { return callA() + 1 }\n")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/exec.swift
  "import B\n"
  "print(callB())\n")

add_library(A STATIC ${CMAKE_CURRENT_BINARY_DIR}/a.swift)
add_library(B STATIC ${CMAKE_CURRENT_BINARY_DIR}/b.swift)
target_link_libraries(B PRIVATE A)

add_executable(exec ${CMAKE_CURRENT_BINARY_DIR}/exec.swift)
target_link_libraries(exec PRIVATE B)
