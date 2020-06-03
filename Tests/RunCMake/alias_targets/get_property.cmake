
enable_language(CXX)

function (check_property alias property value)
  get_property (data TARGET ${alias} PROPERTY ${property})
  if (NOT "${value}" STREQUAL "${data}")
    message (SEND_ERROR "get_property(): Target property '${property}' from ALIAS '${alias}' has wrong value: '${data}' instead of '${value}'.")
  endif()
  get_target_property (data ${alias} ${property})
  if (NOT "${value}" STREQUAL "${data}")
    message (SEND_ERROR "get_target_property(): Target property '${property}' from ALIAS '${alias}' has wrong value: '${data}' instead of '${value}'.")
  endif()
endfunction()


add_library(lib empty.cpp)
set_property (TARGET lib PROPERTY LIB_PROPERTY "LIB")

add_library(alias::lib ALIAS lib)

check_property (alias::lib ALIASED_TARGET "lib")
check_property (alias::lib IMPORTED "FALSE")
check_property (alias::lib ALIAS_GLOBAL "TRUE")
check_property (alias::lib LIB_PROPERTY "LIB")


add_library(import-global SHARED IMPORTED GLOBAL)
set_property (TARGET import-global PROPERTY IMPORT_GLOBAL_PROPERTY "IMPORT_GLOBAL")

add_library(alias::import-global ALIAS import-global)

check_property (alias::import-global ALIASED_TARGET "import-global")
check_property (alias::import-global IMPORTED "TRUE")
check_property (alias::import-global ALIAS_GLOBAL "TRUE")
check_property (alias::import-global IMPORT_GLOBAL_PROPERTY "IMPORT_GLOBAL")


add_library(import-local SHARED IMPORTED)
set_property (TARGET import-local PROPERTY IMPORT_LOCAL_PROPERTY "IMPORT_LOCAL")

add_library(alias::import-local ALIAS import-local)

check_property (alias::import-local ALIASED_TARGET "import-local")
check_property (alias::import-local IMPORTED "TRUE")
check_property (alias::import-local ALIAS_GLOBAL "FALSE")
check_property (alias::import-local IMPORT_LOCAL_PROPERTY "IMPORT_LOCAL")


## upgrade imported target from local to global, alias stay local
add_library(import-lib SHARED IMPORTED)
add_library(alias::import-lib ALIAS import-lib)
check_property (alias::import-lib IMPORTED_GLOBAL "FALSE")
check_property (alias::import-lib ALIAS_GLOBAL "FALSE")
set_property (TARGET import-lib PROPERTY IMPORTED_GLOBAL "TRUE")
check_property (alias::import-lib IMPORTED_GLOBAL "TRUE")
check_property (alias::import-lib ALIAS_GLOBAL "FALSE")


add_subdirectory (get_property-subdir)
