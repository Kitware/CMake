EMBT_RUNTIME_LIBRARY
--------------------

.. versionadded:: 4.6

Select the runtime library used by the Embarcadero (``bcc64x``) toolchain.

The allowed values are:

.. include:: include/EMBT_RUNTIME_LIBRARY-VALUES.rst

Use :manual:`generator expressions <cmake-generator-expressions(7)>` to
support per-configuration specification.

For example, the code:

.. code-block:: cmake

  add_library(mylib SHARED mylib.cxx)
  set_property(TARGET mylib PROPERTY EMBT_RUNTIME_LIBRARY "Dynamic")

selects for the target ``mylib`` the dynamically-linked runtime library.

This property is initialized by the :variable:`CMAKE_EMBT_RUNTIME_LIBRARY`
variable if it is set when a target is created.  It has no effect on
toolchains other than ``EMBTClang``.
