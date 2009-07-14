# - MACRO_ADD_FILE_DEPENDENCIES(<_file> depend_files...)
# Using the macro MACRO_ADD_FILE_DEPENDENCIES() is discouraged. There are usually
# better ways to specifiy the correct dependencies.
#
# MACRO_ADD_FILE_DEPENDENCIES(<_file> depend_files...) is just a convenience 
# wrapper around the OBJECT_DEPENDS source file property. You can just
# use SET_PROPERTY(SOURCE <file> APPEND PROPERTY OBJECT_DEPENDS depend_files) instead.


MACRO (MACRO_ADD_FILE_DEPENDENCIES _file)

   GET_SOURCE_FILE_PROPERTY(_deps ${_file} OBJECT_DEPENDS)
   IF (_deps)
      SET(_deps ${_deps} ${ARGN})
   ELSE (_deps)
      SET(_deps ${ARGN})
   ENDIF (_deps)

   SET_SOURCE_FILES_PROPERTIES(${_file} PROPERTIES OBJECT_DEPENDS "${_deps}")

ENDMACRO (MACRO_ADD_FILE_DEPENDENCIES)
