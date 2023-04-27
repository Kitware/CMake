# There's no point testing more than one level for OLD, since the behavior only
# depends on the current build, not anything about the parent git repo, etc.
set(projName top)
set(depName bottom)
set(epRelativeGitRepo ../../../Bottom)
set(fcRelativeGitRepo ../Bottom)
configure_file(CMakeLists.txt.in Top/CMakeLists.txt @ONLY)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Bottom")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/Bottom/CMakeLists.txt" [[
cmake_minimum_required(VERSION 3.26)
project(bottom LANGUAGES NONE)
message(STATUS "Configured bottom project")
]])
initGitRepo("${CMAKE_CURRENT_BINARY_DIR}/Bottom")

add_subdirectory("${CMAKE_CURRENT_BINARY_DIR}/Top" "${CMAKE_CURRENT_BINARY_DIR}/Top-build")

# Ensure build order is predictable
add_custom_target(non-ep-top ALL COMMAND ${CMAKE_COMMAND} -E echo "Non-ep top project")
add_dependencies(non-ep-top ep-bottom)
