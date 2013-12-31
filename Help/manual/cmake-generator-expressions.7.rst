.. cmake-manual-description: CMake Generator Expressions

cmake-generator-expressions(7)
******************************

.. only:: html or latex

   .. contents::

Introduction
============

Generator expressions are evaluated during build system generation to produce
information specific to each build configuration.

Generator expressions are allowed in the context of many target properties,
such as :prop_tgt:`LINK_LIBRARIES`, :prop_tgt:`INCLUDE_DIRECTORIES`,
:prop_tgt:`COMPILE_DEFINITIONS` and others.  They may also be used when using
commands to populate those properties, such as :command:`target_link_libraries`,
:command:`target_include_directories`, :command:`target_compile_definitions`
and others.

This means that they enable conditional linking, conditional
definitions used when compiling, and conditional include directories and
more.  The conditions may be based on the build configuration, target
properties, platform information or any other queryable information.

Logical Expressions
===================

Logical expressions are used to create conditional output.  The basic
expressions are the ``0`` and ``1`` expressions.  Because other logical
expressions evaluate to either ``0`` or ``1``, they can be composed to
create conditional output::

  $<$<CONFIG:Debug>:DEBUG_MODE>

expands to ``DEBUG_MODE`` when the ``Debug`` configuration is used, and
otherwise expands to nothing.

``$<0:...>``
  Empty string (ignores ``...``)
``$<1:...>``
  Content of ``...``
``$<BOOL:...>``
  ``1`` if the ``...`` is true, else ``0``
``$<AND:?[,?]...>``
  ``1`` if all ``?`` are ``1``, else ``0``

  The ``?`` must always be either ``0`` or ``1`` in boolean expressions.

``$<OR:?[,?]...>``
  ``0`` if all ``?`` are ``0``, else ``1``
``$<NOT:?>``
  ``0`` if ``?`` is ``1``, else ``1``
``$<STREQUAL:a,b>``
  ``1`` if ``a`` is STREQUAL ``b``, else ``0``
``$<EQUAL:a,b>``
  ``1`` if ``a`` is EQUAL ``b`` in a numeric comparison, else ``0``
``$<CONFIG:cfg>``
  ``1`` if config is ``cfg``, else ``0``. This is a case-insensitive comparison.
  The mapping in :prop_tgt:`MAP_IMPORTED_CONFIG_<CONFIG>` is also considered by
  this expression when it is evaluated on a property on an :prop_tgt:`IMPORTED`
  target.
``$<PLATFORM_ID:comp>``
  ``1`` if the CMake-id of the platform matches ``comp``, otherwise ``0``.
``$<C_COMPILER_ID:comp>``
  ``1`` if the CMake-id of the C compiler matches ``comp``, otherwise ``0``.
``$<CXX_COMPILER_ID:comp>``
  ``1`` if the CMake-id of the CXX compiler matches ``comp``, otherwise ``0``.
``$<VERSION_GREATER:v1,v2>``
  ``1`` if ``v1`` is a version greater than ``v2``, else ``0``.
``$<VERSION_LESS:v1,v2>``
  ``1`` if ``v1`` is a version less than ``v2``, else ``0``.
``$<VERSION_EQUAL:v1,v2>``
  ``1`` if ``v1`` is the same version as ``v2``, else ``0``.
``$<C_COMPILER_VERSION:ver>``
  ``1`` if the version of the C compiler matches ``ver``, otherwise ``0``.
``$<CXX_COMPILER_VERSION:ver>``
  ``1`` if the version of the CXX compiler matches ``ver``, otherwise ``0``.
``$<TARGET_POLICY:pol>``
  ``1`` if the policy ``pol`` was NEW when the 'head' target was created,
  else ``0``.  If the policy was not set, the warning message for the policy
  will be emitted. This generator expression only works for a subset of
  policies.

Informational Expressions
=========================

These expressions expand to some information. The information may be used
directly, eg::

  include_directories(/usr/include/$<CXX_COMPILER_ID>/)

expands to ``/usr/include/GNU/`` or ``/usr/include/Clang/`` etc, depending on
the Id of the compiler.

These expressions may also may be combined with logical expressions::

  $<$<VERSION_LESS:$<CXX_COMPILER_VERSION>,4.2.0>:OLD_COMPILER>

expands to ``OLD_COMPILER`` if the
:variable:`CMAKE_CXX_COMPILER_VERSION <CMAKE_<LANG>_COMPILER_VERSION>` is less
than 4.2.0.

``$<CONFIGURATION>``
  Configuration name. Deprecated. Use ``CONFIG`` instead.
``$<CONFIG>``
  Configuration name
``$<PLATFORM_ID>``
  The CMake-id of the platform
``$<C_COMPILER_ID>``
  The CMake-id of the C compiler used.
``$<CXX_COMPILER_ID>``
  The CMake-id of the CXX compiler used.
``$<C_COMPILER_VERSION>``
  The version of the C compiler used.
``$<CXX_COMPILER_VERSION>``
  The version of the CXX compiler used.
``$<TARGET_FILE:tgt>``
  Full path to main file (.exe, .so.1.2, .a) where ``tgt`` is the name of a target.
``$<TARGET_FILE_NAME:tgt>``
  Name of main file (.exe, .so.1.2, .a).
``$<TARGET_FILE_DIR:tgt>``
  Directory of main file (.exe, .so.1.2, .a).
``$<TARGET_LINKER_FILE:tgt>``
  File used to link (.a, .lib, .so) where ``tgt`` is the name of a target.
``$<TARGET_LINKER_FILE_NAME:tgt>``
  Name of file used to link (.a, .lib, .so).
``$<TARGET_LINKER_FILE_DIR:tgt>``
  Directory of file used to link (.a, .lib, .so).
``$<TARGET_SONAME_FILE:tgt>``
  File with soname (.so.3) where ``tgt`` is the name of a target.
``$<TARGET_SONAME_FILE_NAME:tgt>``
  Name of file with soname (.so.3).
``$<TARGET_SONAME_FILE_DIR:tgt>``
  Directory of with soname (.so.3).
``$<TARGET_PROPERTY:tgt,prop>``
  Value of the property ``prop`` on the target ``tgt``.

  Note that ``tgt`` is not added as a dependency of the target this
  expression is evaluated on.
``$<TARGET_PROPERTY:prop>``
  Value of the property ``prop`` on the target on which the generator
  expression is evaluated.
``$<INSTALL_PREFIX>``
  Content of the install prefix when the target is exported via
  :command:`install(EXPORT)` and empty otherwise.

Output Expressions
==================

These expressions generate output, in some cases depending on an input. These
expressions may be combined with other expressions for information or logical
comparison::

  -I$<JOIN:$<TARGET_PROPERTY:INCLUDE_DIRECTORIES>, -I>

generates a string of the entries in the :prop_tgt:`INCLUDE_DIRECTORIES` target
property with each entry preceeded by ``-I``. Note that a more-complete use
in this situation would require first checking if the INCLUDE_DIRECTORIES
property is non-empty::

  $<$<BOOL:$<TARGET_PROPERTY:INCLUDE_DIRECTORIES>>:-I$<JOIN:$<TARGET_PROPERTY:INCLUDE_DIRECTORIES>, -I>>

``$<JOIN:list,...>``
  Joins the list with the content of ``...``
``$<ANGLE-R>``
  A literal ``>``. Used to compare strings which contain a ``>`` for example.
``$<COMMA>``
  A literal ``,``. Used to compare strings which contain a ``,`` for example.
``$<SEMICOLON>``
  A literal ``;``. Used to prevent list expansion on an argument with ``;``.
``$<TARGET_NAME:...>``
  Marks ``...`` as being the name of a target.  This is required if exporting
  targets to multiple dependent export sets.  The ``...`` must be a literal
  name of a target- it may not contain generator expressions.
``$<INSTALL_INTERFACE:...>``
  Content of ``...`` when the property is exported using :command:`install(EXPORT)`,
  and empty otherwise.
``$<BUILD_INTERFACE:...>``
  Content of ``...`` when the property is exported using :command:`export`, or
  when the target is used by another target in the same buildsystem. Expands to
  the empty string otherwise.
``$<LOWER_CASE:...>``
  Content of ``...`` converted to lower case.
``$<UPPER_CASE:...>``
  Content of ``...`` converted to upper case.
``$<MAKE_C_IDENTIFIER:...>``
  Content of ``...`` converted to a C identifier.
