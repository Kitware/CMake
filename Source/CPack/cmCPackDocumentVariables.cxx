#include "cmCPackDocumentVariables.h"
#include "cmake.h"

void cmCPackDocumentVariables::DefineVariables(cmake* cm)
{
  // Subsection: variables defined/used by cpack,
  // which are common to all CPack generators

  cm->DefineProperty
      ("CPACK_PACKAGING_INSTALL_PREFIX", cmProperty::VARIABLE,
       "The prefix used in the built package.",
       "Each CPack generator has a default value (like /usr)."
       " This default value may"
       " be overwritten from the CMakeLists.txt or the cpack command line"
       " by setting an alternative value.\n"
       "e.g. "
       " set(CPACK_PACKAGING_INSTALL_PREFIX \"/opt\")\n"
       "This is not the same purpose as CMAKE_INSTALL_PREFIX which"
       " is used when installing from the build tree without building"
       " a package."
       "", false,
       "Variables common to all CPack generators");

  // Subsection: variables defined/used by cpack,
  // which are specific to one CPack generator
//  cm->DefineProperty
//      ("CPACK_RPM_PACKAGE_NAME", cmProperty::VARIABLE,
//       "RPM specific package name.",
//       "If not specified, defaults to CPACK_PACKAGE_NAME."
//       "", false,
//       "Variables specific to a CPack generator");
}
