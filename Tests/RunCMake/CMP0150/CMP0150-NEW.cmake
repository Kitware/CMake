set(policyCommand "cmake_policy(SET CMP0150 NEW)")

# Need to keep paths and file names short to avoid hitting limits on Windows.
# Directory names "a" through to "g" are used here according to the following:
#   a   = Top project
#   b/c = Middle project
#   d   = Bottom project
#   e/f = Cloned Top project
#   g   = Build directory for cloned Top project
#
# Dependency names map as follows:
#   X = middle dependency
#   Y = bottom dependency

set(projName top)
set(depName X)
set(epRelativeGitRepo ../b/c)
set(fcRelativeGitRepo ../b/c)
configure_file(CMakeLists.txt.in a/CMakeLists.txt @ONLY)
initGitRepo("${CMAKE_CURRENT_BINARY_DIR}/a")

set(projName middle)
set(depName Y)
set(epRelativeGitRepo ../../d)
set(fcRelativeGitRepo ../../d)
configure_file(CMakeLists.txt.in b/c/CMakeLists.txt @ONLY)
initGitRepo("${CMAKE_CURRENT_BINARY_DIR}/b/c")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/d")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/d/CMakeLists.txt" [[
cmake_minimum_required(VERSION 3.26)
project(bottom LANGUAGES NONE)
message(STATUS "Configured bottom project")
]])
initGitRepo("${CMAKE_CURRENT_BINARY_DIR}/d")

set(clonedTopDir "${CMAKE_CURRENT_BINARY_DIR}/e/f")
file(MAKE_DIRECTORY "${clonedTopDir}")
execGitCommand(${CMAKE_CURRENT_BINARY_DIR} clone --quiet "file://${CMAKE_CURRENT_BINARY_DIR}/a" "${clonedTopDir}")
add_subdirectory("${clonedTopDir}" "${CMAKE_CURRENT_BINARY_DIR}/g")

# Ensure build order is predictable
add_custom_target(non-ep-top ALL COMMAND ${CMAKE_COMMAND} -E echo "Non-ep top project")
add_dependencies(non-ep-top ep-X)
add_dependencies(ep-X ep-Y)
