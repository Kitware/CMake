COMPATIBLE_INTERFACE_BOOL
-------------------------

Properties which must be compatible with their link interface

The COMPATIBLE_INTERFACE_BOOL property may contain a list of
propertiesfor this target which must be consistent when evaluated as a
boolean in the INTERFACE of all linked dependees.  For example, if a
property "FOO" appears in the list, then for each dependee, the
"INTERFACE_FOO" property content in all of its dependencies must be
consistent with each other, and with the "FOO" property in the
depender.  Consistency in this sense has the meaning that if the
property is set, then it must have the same boolean value as all
others, and if the property is not set, then it is ignored.  Note that
for each dependee, the set of properties from this property must not
intersect with the set of properties from the
COMPATIBLE_INTERFACE_STRING property.
