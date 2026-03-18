VERIFY_PRIVATE_HEADER_SETS
--------------------------

.. versionadded:: 4.3

.. versionchanged:: 4.4
  ``OBJC`` and ``OBJCXX`` languages are now supported in addition to ``C``
  and ``CXX``.

Used to verify that all headers in a target's ``PUBLIC`` and ``PRIVATE``
header sets can be included on their own.

.. versionchanged:: 4.4
  Previously, the verification target was only created when the target had
  at least one matching header. Now it is always created when the property
  is enabled.

When this property is set to true, and the target is an object library, static
library, shared library, module library, interface library, or executable, an
object library target named ``<target_name>_verify_private_header_sets`` is
created. This verification target has one source file per header in the
target's ``PUBLIC`` and ``PRIVATE`` header sets. Each source file only
includes its associated header file. If the target has no matching header
sets, a utility target is created instead so that the target name always
exists for build system dependencies.

Properties affecting compilation are copied from the original target to the
verification target so that the headers will be interpreted the same way by
the compiler as when compiling the original target's sources.  There are some
caveats with this approach.  It cannot replicate the same conditions if any
of those properties or properties inherited through build requirements from
transitive dependencies contain
:ref:`target-dependent generator expressions <Target-Dependent Expressions>`
that do not specify the target for the expansion.  Such expressions can expand
to different contents depending on the target they are being used on.

.. |xxx| replace:: private
.. |THIS_PROPERTY| replace:: ``VERIFY_PRIVATE_HEADER_SETS``
.. |COMPLEMENTARY_PROPERTY| replace:: :prop_tgt:`VERIFY_INTERFACE_HEADER_SETS`
.. |THIS_ALL_TARGET| replace:: ``all_verify_private_header_sets``
.. |COMPLEMENTARY_ALL_TARGET| replace:: ``all_verify_interface_header_sets``
.. |INIT_VARIABLE| replace:: :variable:`CMAKE_VERIFY_PRIVATE_HEADER_SETS`
.. |SETS_TO_VERIFY_PROPERTY| replace:: :prop_tgt:`PRIVATE_HEADER_SETS_TO_VERIFY`
.. include:: include/VERIFY_XXX_HEADER_SETS.rst
