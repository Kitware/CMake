# - ADD_FILE_DEPENDENCIES(source_file depend_files...)
# Adds the given files as dependencies to source_file
#

MACRO(ADD_FILE_DEPENDENCIES _file)

   GET_SOURCE_FILE_PROPERTY(_deps ${_file} OBJECT_DEPENDS)
   IF (_deps)
      SET(_deps ${_deps} ${ARGN})
   ELSE (_deps)
      SET(_deps ${ARGN})
   ENDIF (_deps)

   SET_SOURCE_FILES_PROPERTIES(${_file} PROPERTIES OBJECT_DEPENDS "${_deps}")

ENDMACRO(ADD_FILE_DEPENDENCIES)
