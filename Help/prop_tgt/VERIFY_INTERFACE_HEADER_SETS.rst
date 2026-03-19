VERIFY_INTERFACE_HEADER_SETS
----------------------------

.. versionadded:: 3.24

.. versionchanged:: 4.4
  ``OBJC`` and ``OBJCXX`` languages are now supported in addition to ``C``
  and ``CXX``.

Used to verify that all headers in a target's ``PUBLIC`` and ``INTERFACE``
header sets can be included on their own.

.. versionchanged:: 4.4
  Previously, the verification target was only created when the target had
  at least one matching header. Now it is always created when the property
  is enabled.

When this property is set to true, and the target is an object library, static
library, shared library, interface library, or executable (subject to policy
:policy:`CMP0209`), an object library target named
``<target_name>_verify_interface_header_sets`` is created. This verification
target has one source file per header in the target's ``PUBLIC`` and
``INTERFACE`` header sets. Each source file only includes its associated
header file. If the target has no matching header sets, a utility target is
created instead so that the target name always exists for build system
dependencies.
The verification target links against the original target to get all of its
usage requirements.

.. |xxx| replace:: interface
.. |THIS_PROPERTY| replace:: ``VERIFY_INTERFACE_HEADER_SETS``
.. |COMPLEMENTARY_PROPERTY| replace:: :prop_tgt:`VERIFY_PRIVATE_HEADER_SETS`
.. |THIS_ALL_TARGET| replace:: ``all_verify_interface_header_sets``
.. |COMPLEMENTARY_ALL_TARGET| replace:: ``all_verify_private_header_sets``
.. |INIT_VARIABLE| replace:: :variable:`CMAKE_VERIFY_INTERFACE_HEADER_SETS`
.. |SETS_TO_VERIFY_PROPERTY| replace:: :prop_tgt:`INTERFACE_HEADER_SETS_TO_VERIFY`
.. include:: include/VERIFY_XXX_HEADER_SETS.rst
