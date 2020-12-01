
file (GLOB_RECURSE files LIST_DIRECTORIES TRUE RELATIVE "${SUPPORT_FILES_DIRECTORY}" "${SUPPORT_FILES_DIRECTORY}/*")

list(SORT files)

if (NOT files STREQUAL "NSExample.cs;NSExamplePINVOKE.cs;ns;ns/my_class_in_namespace.cs")
  message (FATAL_ERROR "Support files not correctly collected.")
endif()
