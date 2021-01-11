add_executable(importedTarget IMPORTED)

cmake_policy(SET CMP0070 NEW)
file(GENERATE OUTPUT TARGET_NAME_IF_EXISTS-generated-imported.txt CONTENT "$<TARGET_NAME_IF_EXISTS:importedTarget>")
