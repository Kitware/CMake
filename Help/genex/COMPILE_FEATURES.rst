COMPILE_FEATURES
----------------

.. |genex-family| replace::
   :ref:`genex-family-toolchain-and-language-expressions`

.. include:: ../manual/include/GENEX_FAMILY.rst

Syntax
^^^^^^

.. code-block:: cmake

  $<COMPILE_FEATURES:features>

.. versionadded:: 3.1

where ``features`` is a comma-separated list. Evaluates to ``1`` if all of the
``features`` are available for the 'head' target, and ``0`` otherwise. If this
expression is used while evaluating the link implementation of a target and if
any dependency transitively increases the required :prop_tgt:`C_STANDARD` or
:prop_tgt:`CXX_STANDARD` for the 'head' target, an error is reported.  See the
:manual:`cmake-compile-features(7)` manual for information on compile features
and a list of supported compilers.
