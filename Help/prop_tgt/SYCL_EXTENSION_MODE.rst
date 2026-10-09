SYCL_EXTENSION_MODE
-------------------

.. versionadded:: 4.5

.. note::
   Experimental. Gated by ``CMAKE_EXPERIMENTAL_SYCL``.

Controls whether C++ language flags and compile-feature requirements are
combined with their SYCL counterparts for SYCL compilation and linking.

Supported values are:

``APPEND``
  Use :variable:`CMAKE_CXX_FLAGS <CMAKE_<LANG>_FLAGS>` and its
  configuration-specific value before
  :variable:`CMAKE_SYCL_FLAGS <CMAKE_<LANG>_FLAGS>` and its
  configuration-specific value.  C++ and SYCL compile-feature and
  language-standard requirements are combined.  This is the default when the
  property is unset.

``REPLACE``
  Use only SYCL language flags and SYCL compile-feature and language-standard
  requirements.

This property does not change generator-expression language contexts.  For a
SYCL compilation, ``$<COMPILE_LANGUAGE:SYCL>`` is true and
``$<COMPILE_LANGUAGE:CXX>`` is false.  For a target linked with the SYCL
driver, ``$<LINK_LANGUAGE:SYCL>`` is true and ``$<LINK_LANGUAGE:CXX>`` is
false.  Properties and usage requirements are not evaluated a second time in a
C++ context.
