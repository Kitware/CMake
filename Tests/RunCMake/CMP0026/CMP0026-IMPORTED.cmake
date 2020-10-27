
enable_language(CXX)

cmake_policy(SET CMP0111 OLD)
add_library(someimportedlib SHARED IMPORTED)

get_target_property(_loc someimportedlib LOCATION)
