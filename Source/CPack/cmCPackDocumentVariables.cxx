#include "cmCPackDocumentVariables.h"
#include "cmake.h"

void cmCPackDocumentVariables::DefineVariables(cmake* cm)
{
  // Subsection: variables defined/used by cpack,
  // which are common to all CPack generators
  cm->DefineProperty
    ("CPACK_PACKAGE_NAME", cmProperty::VARIABLE,
     "The name of the package (or application).",
     "If not specified, defaults to the project name."
     "", false,
     "Variables common to all CPack generators");

  cm->DefineProperty
      ("CPACK_PACKAGE_VENDOR", cmProperty::VARIABLE,
       "The name of the package vendor.",
       "If not specified, defaults to \"Humanity\"."
       "", false,
       "Variables common to all CPack generators");

  // Subsection: variables defined/used by cpack,
  // which are specific to one CPack generator
  cm->DefineProperty
      ("CPACK_RPM_PACKAGE_NAME", cmProperty::VARIABLE,
       "RPM specific package name.",
       "If not specified, defaults to CPACK_PACKAGE_NAME."
       "", false,
       "Variables specific to a CPack generator");
}
