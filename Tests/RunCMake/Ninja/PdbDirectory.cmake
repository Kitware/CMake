cmake_policy(SET CMP0141 NEW)
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT ProgramDatabase)
enable_language(CXX)

foreach(target IN ITEMS slash backslash namedSlash namedBackslash default)
  add_library(${target} OBJECT PdbDirectory.cxx)
endforeach()
# With no COMPILE_PDB_NAME, /Fd names a directory with a trailing separator.
# Keep the property values literal to exercise directory creation alone.
set_property(TARGET slash PROPERTY COMPILE_PDB_OUTPUT_DIRECTORY "pdb space/slash/nested")
set_property(TARGET backslash PROPERTY COMPILE_PDB_OUTPUT_DIRECTORY [[pdb space\backslash\nested]])
set_property(TARGET namedSlash PROPERTY COMPILE_PDB_OUTPUT_DIRECTORY "pdb space/namedSlash/nested")
set_property(TARGET namedBackslash PROPERTY COMPILE_PDB_OUTPUT_DIRECTORY [[pdb space\namedBackslash\nested]])
set_property(TARGET namedSlash namedBackslash PROPERTY COMPILE_PDB_NAME "named-control")
