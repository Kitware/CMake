add_executable(importedGlobalTarget IMPORTED GLOBAL)

cmake_policy(SET CMP0070 NEW)
file(GENERATE OUTPUT TARGET_NAME_IF_EXISTS-generated-imported-global.txt CONTENT "$<TARGET_NAME_IF_EXISTS:importedGlobalTarget>")
