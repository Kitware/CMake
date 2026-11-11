.. cmake-manual-description: CMake Generator Expressions

cmake-generator-expressions(7)
******************************

.. only:: html

   .. contents::

Introduction
============

Generator expressions are evaluated during build system generation to produce
information specific to each build configuration.  They have the form
``$<...>``.  For example:

.. code-block:: cmake

  target_include_directories(tgt PRIVATE /opt/include/$<CXX_COMPILER_ID>)

This would expand to ``/opt/include/GNU``, ``/opt/include/Clang``, etc.
depending on the C++ compiler used.

Generator expressions are allowed in the context of many target properties,
such as :prop_tgt:`LINK_LIBRARIES`, :prop_tgt:`INCLUDE_DIRECTORIES`,
:prop_tgt:`COMPILE_DEFINITIONS` and others.  They may also be used when using
commands to populate those properties, such as :command:`target_link_libraries`,
:command:`target_include_directories`, :command:`target_compile_definitions`
and others.  They enable conditional linking, conditional definitions used when
compiling, conditional include directories, and more.  The conditions may be
based on the build configuration, target properties, platform information,
or any other queryable information.

Generator expressions can be nested:

.. code-block:: cmake

  target_compile_definitions(tgt PRIVATE
    $<$<VERSION_LESS:$<CXX_COMPILER_VERSION>,4.2.0>:OLD_COMPILER>
  )

The above would expand to ``OLD_COMPILER`` if the
:variable:`CMAKE_CXX_COMPILER_VERSION <CMAKE_<LANG>_COMPILER_VERSION>` is less
than 4.2.0.

.. note::

  Generator expressions are supported in the values of many properties,
  but not all.  Check a property's documentation for explicit mention
  of generator expressions before using them.

.. note::

  Property names specified as part of a generator expression must respect
  the following regular expression: ``^[A-Za-z0-9_:]+$``.

  .. versionchanged:: 4.5
    The character ``:`` is now allowed.

Whitespace And Quoting
======================

Generator expressions are typically parsed after command arguments.
If a generator expression contains spaces, new lines, semicolons or
other characters that may be interpreted as command argument separators,
the whole expression should be surrounded by quotes when passed to a
command.  Failure to do so may result in the expression being split and
it may no longer be recognized as a generator expression.

When using :command:`add_custom_command` or :command:`add_custom_target`,
use the ``VERBATIM`` and ``COMMAND_EXPAND_LISTS`` options to obtain robust
argument splitting and quoting.

.. code-block:: cmake

  # WRONG: Embedded space will be treated as an argument separator.
  # This ends up not being seen as a generator expression at all.
  add_custom_target(run_some_tool
    COMMAND some_tool -I$<JOIN:$<TARGET_PROPERTY:tgt,INCLUDE_DIRECTORIES>, -I>
    VERBATIM
  )

.. code-block:: cmake

  # Better, but still not robust. Quotes prevent the space from splitting the
  # expression. However, the tool will receive the expanded value as a single
  # argument.
  add_custom_target(run_some_tool
    COMMAND some_tool "-I$<JOIN:$<TARGET_PROPERTY:tgt,INCLUDE_DIRECTORIES>, -I>"
    VERBATIM
  )

.. code-block:: cmake

  # Nearly correct. Using a semicolon to separate arguments and adding the
  # COMMAND_EXPAND_LISTS option means that paths with spaces will be handled
  # correctly. Quoting the whole expression ensures it is seen as a generator
  # expression. But if the target property is empty, we will get a bare -I
  # with nothing after it.
  add_custom_target(run_some_tool
    COMMAND some_tool "-I$<JOIN:$<TARGET_PROPERTY:tgt,INCLUDE_DIRECTORIES>,;-I>"
    COMMAND_EXPAND_LISTS
    VERBATIM
  )

Using variables to build up a more complex generator expression is also a
good way to reduce errors and improve readability.  The above example can be
improved further like so:

.. code-block:: cmake

  # The $<BOOL:...> check prevents adding anything if the property is empty,
  # assuming the property value cannot be one of CMake's false constants.
  set(prop "$<TARGET_PROPERTY:tgt,INCLUDE_DIRECTORIES>")
  add_custom_target(run_some_tool
    COMMAND some_tool "$<$<BOOL:${prop}>:-I$<JOIN:${prop},;-I>>"
    COMMAND_EXPAND_LISTS
    VERBATIM
  )

Finally, the above example can be expressed in a more simple and robust way
using an alternate generator expression:

.. code-block:: cmake

  add_custom_target(run_some_tool
    COMMAND some_tool "$<LIST:TRANSFORM,$<TARGET_PROPERTY:tgt,INCLUDE_DIRECTORIES>,PREPEND,-I>"
    COMMAND_EXPAND_LISTS
    VERBATIM
  )

For tools that expect ``-I``'s value to be a separate argument, use the
semicolon trick learned earlier:

.. code-block:: cmake

  add_custom_target(run_some_tool
    COMMAND some_tool "$<LIST:TRANSFORM,$<TARGET_PROPERTY:tgt,INCLUDE_DIRECTORIES>,PREPEND,-I;>"
    COMMAND_EXPAND_LISTS
    VERBATIM
  )

A common mistake is to try to split a generator expression across multiple
lines with indenting:

.. code-block:: cmake

  # WRONG: New lines and spaces all treated as argument separators, so the
  # generator expression is split and not recognized correctly.
  target_compile_definitions(tgt PRIVATE
    $<$<AND:
        $<CXX_COMPILER_ID:GNU>,
        $<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,5>
      >:HAVE_5_OR_LATER>
  )

Again, use helper variables with well-chosen names to build up a readable
expression instead:

.. code-block:: cmake

  set(is_gnu "$<CXX_COMPILER_ID:GNU>")
  set(v5_or_later "$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,5>")
  set(meet_requirements "$<AND:${is_gnu},${v5_or_later}>")
  target_compile_definitions(tgt PRIVATE
    "$<${meet_requirements}:HAVE_5_OR_LATER>"
  )

Debugging
=========

Since generator expressions are evaluated during generation of the buildsystem,
and not during processing of ``CMakeLists.txt`` files, it is not possible to
inspect their result with the :command:`message()` command.  One possible way
to generate debug messages is to add a custom target:

.. code-block:: cmake

  add_custom_target(genexdebug COMMAND ${CMAKE_COMMAND} -E echo "$<...>")

After running :program:`cmake`, you can then build the ``genexdebug`` target to print
the result of the ``$<...>`` expression (i.e. run the command
:option:`cmake --build ... --target genexdebug <cmake--build --target>`).

Another way is to write debug messages to a file with :command:`file(GENERATE)`:

.. code-block:: cmake

  file(GENERATE OUTPUT filename CONTENT "$<...>")

Generator Expression Reference
==============================

.. note::

  This reference deviates from most of the CMake documentation in that it
  omits angular brackets ``<...>`` around placeholders like ``condition``,
  ``string``, ``target``, etc.  This is to prevent an opportunity for those
  placeholders to be misinterpreted as generator expressions.

.. _`Conditional Generator Expressions`:

Conditional Expressions
-----------------------

A fundamental category of generator expressions relates to conditional logic.
Two forms of conditional generator expressions are supported:


.. toctree::
   :maxdepth: 1

   /genex/CONDITION
   /genex/IF
   /genex/BOOL

Typically, the ``condition`` is itself a generator expression.  For instance,
the following expression expands to ``DEBUG_MODE`` when the ``Debug``
configuration is used, and the empty string for all other configurations:

.. code-block:: cmake

  $<$<CONFIG:Debug>:DEBUG_MODE>

Boolean-like ``condition`` values other than ``1`` or ``0`` can be handled
by wrapping them with the ``$<BOOL:...>`` generator expression:

The ``$<BOOL:...>`` generator expression is often used when a ``condition``
is provided by a CMake variable:

.. code-block:: cmake

  $<$<BOOL:${HAVE_SOME_FEATURE}>:-DENABLE_SOME_FEATURE>


.. _`Boolean Generator Expressions`:

Logical Operators
-----------------

The common boolean logic operators are supported:


.. toctree::
   :maxdepth: 1

   /genex/AND
   /genex/OR
   /genex/NOT

.. versionadded:: 3.28

  Logical operators short-circuit such that generator expressions in the
  arguments list will not be evaluated once a return value can be determined.

.. _`Comparison Expressions`:

Primary Comparison Expressions
------------------------------

CMake supports a variety of generator expressions that compare things.
This section covers the primary and most widely used comparison types.
Other more specific comparison types are documented in their own separate
sections further below.

Numeric Comparisons
^^^^^^^^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/EQUAL

Version Comparisons
^^^^^^^^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/VERSION_LESS
   /genex/VERSION_GREATER
   /genex/VERSION_EQUAL
   /genex/VERSION_LESS_EQUAL
   /genex/VERSION_GREATER_EQUAL

String Expressions
------------------

Most of the expressions in this section are closely associated with the
:command:`string` command, providing the same capabilities, but in
the form of a generator expression.

In each of the following string-related generator expressions, the ``string``
must not contain any commas if that generator expression expects something to
be provided after the ``string``.  For example, the expression
``$<STRING:FIND,string,value>`` requires a ``value`` after the ``string``.
Since a comma is used to separate the ``string`` and the ``value``, the
``string`` cannot itself contain a comma.  This restriction does not apply to
the :command:`string` command, it is specific to the string-handling generator
expressions only. The :genex:`$<COMMA>` generator expression can be used to
specify a comma as part of the arguments of the string-related generator
expressions.

.. _`String Comparisons Generator Expressions`:

String Comparisons
^^^^^^^^^^^^^^^^^^

The comparisons are case-sensitive.  For a case-insensitive comparison,
combine with a :ref:`string transforming generator expression
<String Transforming Generator Expressions>`.  For example, the following
evaluates to ``1`` if ``${foo}`` is any of ``BAR``, ``Bar``, ``bar``, etc.

  .. code-block:: cmake

    $<STREQUAL:$<STRING:TOUPPER,${foo}>,BAR>


.. toctree::
   :maxdepth: 1

   /genex/STREQUAL
   /genex/STRLESS
   /genex/STRGREATER
   /genex/STRLESS_EQUAL
   /genex/STRGREATER_EQUAL

.. _`String Queries Generator Expressions`:

String Queries
^^^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/STRING-LENGTH
   /genex/STRING-SUBSTRING
   /genex/STRING-FIND
   /genex/STRING-MATCH

.. _`String Generating Generator Expressions`:

String Generations
^^^^^^^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/STRING-JOIN
   /genex/STRING-ASCII
   /genex/STRING-TIMESTAMP
   /genex/STRING-RANDOM
   /genex/STRING-UUID

.. _`String Transforming Generator Expressions`:

String Transformations
^^^^^^^^^^^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/STRING-REPLACE
   /genex/STRING-APPEND
   /genex/STRING-PREPEND
   /genex/STRING-TOLOWER
   /genex/STRING-TOUPPER
   /genex/STRING-STRIP
   /genex/STRING-QUOTE
   /genex/STRING-HEX
   /genex/STRING-HASH
   /genex/STRING-MAKE_C_IDENTIFIER
   /genex/LOWER_CASE
   /genex/UPPER_CASE
   /genex/MAKE_C_IDENTIFIER

List Expressions
----------------

Most of the expressions in this section are closely associated with the
:command:`list` command, providing the same capabilities, but in
the form of a generator expression.

In each of the following list-related generator expressions, the ``list``
must not contain any commas if that generator expression expects something to
be provided after the ``list``.  For example, the expression
``$<LIST:FIND,list,value>`` requires a ``value`` after the ``list``.
Since a comma is used to separate the ``list`` and the ``value``, the ``list``
cannot itself contain a comma.  This restriction does not apply to the
:command:`list` command, it is specific to the list-handling generator
expressions only.

.. _GenEx List Comparisons:

List Comparisons
^^^^^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/IN_LIST

.. _GenEx List Queries:

List Queries
^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/LIST-LENGTH
   /genex/LIST-GET
   /genex/LIST-SUBLIST
   /genex/LIST-FIND

.. _GenEx List Transformations:

List Transformations
^^^^^^^^^^^^^^^^^^^^

.. _GenEx LIST-JOIN:


.. toctree::
   :maxdepth: 1

   /genex/LIST-JOIN
   /genex/LIST-APPEND
   /genex/LIST-PREPEND
   /genex/LIST-INSERT
   /genex/LIST-POP_BACK
   /genex/LIST-POP_FRONT
   /genex/LIST-REMOVE_ITEM
   /genex/LIST-REMOVE_AT
   /genex/LIST-REMOVE_DUPLICATES
   /genex/LIST-FILTER
   /genex/LIST-TRANSFORM
   /genex/JOIN
   /genex/REMOVE_DUPLICATES
   /genex/FILTER

.. _GenEx LIST-REMOVE_DUPLICATES:

.. _GenEx LIST-FILTER:

.. _GenEx List Ordering:

List Ordering
^^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/LIST-REVERSE
   /genex/LIST-SORT

.. _GenEx Bound Operands:

Bound Operands
^^^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/_0
   /genex/_1

Path Expressions
----------------

Most of the expressions in this section are closely associated with the
:command:`cmake_path` command, providing the same capabilities, but in
the form of a generator expression.

For all generator expressions in this section, paths are expected to be in
cmake-style format. The :cref:`$\<PATH:CMAKE_PATH\> <GenEx PATH-CMAKE_PATH>`
generator expression can be used to convert a native path to a cmake-style
one.

.. _GenEx Path Comparisons:

Path Comparisons
^^^^^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/PATH_EQUAL

.. _GenEx Path Queries:

Path Queries
^^^^^^^^^^^^

These expressions provide the generation-time capabilities equivalent to the
:ref:`Query <Path Query>` options of the :command:`cmake_path` command.
All paths are expected to be in cmake-style format.


.. toctree::
   :maxdepth: 1

   /genex/PATH-HAS_
   /genex/PATH-IS_ABSOLUTE
   /genex/PATH-IS_RELATIVE
   /genex/PATH-IS_PREFIX

.. _GenEx Path Decomposition:

Path Decomposition
^^^^^^^^^^^^^^^^^^

These expressions provide the generation-time capabilities equivalent to the
:ref:`Decomposition <Path Decomposition>` options of the :command:`cmake_path`
command.  All paths are expected to be in cmake-style format.


.. toctree::
   :maxdepth: 1

   /genex/PATH-GET_

.. _GenEx Path Transformations:

Path Transformations
^^^^^^^^^^^^^^^^^^^^

These expressions provide the generation-time capabilities equivalent to the
:ref:`Modification <Path Modification>` and :ref:`Generation <Path Generation>`
options of the :command:`cmake_path` command.  All paths are expected to be
in cmake-style format.

.. versionchanged:: 3.27
  All operations now accept a list of paths as argument. When a list of paths
  is specified, the operation will be applied to each path.


.. _GenEx PATH-CMAKE_PATH:


.. toctree::
   :maxdepth: 1

   /genex/PATH-CMAKE_PATH
   /genex/PATH-NATIVE_PATH
   /genex/PATH-APPEND
   /genex/PATH-REMOVE_FILENAME
   /genex/PATH-REPLACE_FILENAME
   /genex/PATH-REMOVE_EXTENSION
   /genex/PATH-REPLACE_EXTENSION
   /genex/PATH-NORMAL_PATH
   /genex/PATH-RELATIVE_PATH
   /genex/PATH-ABSOLUTE_PATH

Shell Paths
^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/SHELL_PATH

Configuration Expressions
-------------------------


.. toctree::
   :maxdepth: 1

   /genex/CONFIG
   /genex/OUTPUT_CONFIG
   /genex/COMMAND_CONFIG

Global Expressions
------------------

Global Properties
^^^^^^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/GLOBAL_PROPERTY

Directory-dependent Expressions
-------------------------------

Directory Properties
^^^^^^^^^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/DIRECTORY_PROPERTY

Rule-Dependent Expressions
--------------------------

Rule Properties
^^^^^^^^^^^^^^^

These expressions look up the values of rule properties.


.. toctree::
   :maxdepth: 1

   /genex/RULE_PROPERTY

Toolchain And Language Expressions
----------------------------------

Platform
^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/PLATFORM_ID

Compiler Version
^^^^^^^^^^^^^^^^

See also the :variable:`CMAKE_<LANG>_COMPILER_VERSION` variable, which is
closely related to the expressions in this sub-section.


.. toctree::
   :maxdepth: 1

   /genex/C_COMPILER_VERSION
   /genex/CXX_COMPILER_VERSION
   /genex/CUDA_COMPILER_VERSION
   /genex/OBJC_COMPILER_VERSION
   /genex/OBJCXX_COMPILER_VERSION
   /genex/FORTRAN_COMPILER_VERSION
   /genex/HIP_COMPILER_VERSION
   /genex/ISPC_COMPILER_VERSION

Compiler Language, ID, and Frontend-Variant
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

See also the :variable:`CMAKE_<LANG>_COMPILER_ID` and
:variable:`CMAKE_<LANG>_COMPILER_FRONTEND_VARIANT` variables, which are closely
related to most of the expressions in this sub-section.


.. toctree::
   :maxdepth: 1

   /genex/C_COMPILER_ID
   /genex/CXX_COMPILER_ID
   /genex/CUDA_COMPILER_ID
   /genex/OBJC_COMPILER_ID
   /genex/OBJCXX_COMPILER_ID
   /genex/FORTRAN_COMPILER_ID
   /genex/HIP_COMPILER_ID
   /genex/ISPC_COMPILER_ID
   /genex/C_COMPILER_FRONTEND_VARIANT
   /genex/CXX_COMPILER_FRONTEND_VARIANT
   /genex/CUDA_COMPILER_FRONTEND_VARIANT
   /genex/OBJC_COMPILER_FRONTEND_VARIANT
   /genex/OBJCXX_COMPILER_FRONTEND_VARIANT
   /genex/FORTRAN_COMPILER_FRONTEND_VARIANT
   /genex/HIP_COMPILER_FRONTEND_VARIANT
   /genex/ISPC_COMPILER_FRONTEND_VARIANT
   /genex/COMPILE_LANGUAGE
   /genex/COMPILE_LANG_AND_ID

Compile Features
^^^^^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/COMPILE_FEATURES

Compile Context
^^^^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/COMPILE_ONLY

Link Language and ID
^^^^^^^^^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/LINK_LANGUAGE
   /genex/LINK_LANG_AND_ID

.. _`Boolean LINK_LANGUAGE Generator Expression`:

Link Features
^^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/LINK_LIBRARY
   /genex/LINK_GROUP

Link Context
^^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/LINK_ONLY
   /genex/DEVICE_LINK
   /genex/HOST_LINK

Linker ID and Frontend-Variant
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

See also the :variable:`CMAKE_<LANG>_COMPILER_LINKER_ID` and
:variable:`CMAKE_<LANG>_COMPILER_LINKER_FRONTEND_VARIANT` variables, which are
closely related to most of the expressions in this sub-section.


.. toctree::
   :maxdepth: 1

   /genex/C_COMPILER_LINKER_ID
   /genex/CXX_COMPILER_LINKER_ID
   /genex/CUDA_COMPILER_LINKER_ID
   /genex/OBJC_COMPILER_LINKER_ID
   /genex/OBJCXX_COMPILER_LINKER_ID
   /genex/FORTRAN_COMPILER_LINKER_ID
   /genex/HIP_COMPILER_LINKER_ID
   /genex/SYCL_COMPILER_LINKER_ID
   /genex/C_COMPILER_LINKER_FRONTEND_VARIANT
   /genex/CXX_COMPILER_LINKER_FRONTEND_VARIANT
   /genex/CUDA_COMPILER_LINKER_FRONTEND_VARIANT
   /genex/OBJC_COMPILER_LINKER_FRONTEND_VARIANT
   /genex/OBJCXX_COMPILER_LINKER_FRONTEND_VARIANT
   /genex/FORTRAN_COMPILER_LINKER_FRONTEND_VARIANT
   /genex/HIP_COMPILER_LINKER_FRONTEND_VARIANT
   /genex/SYCL_COMPILER_LINKER_FRONTEND_VARIANT

.. _`Source-Dependent Expressions`:

Source-Dependent Expressions
----------------------------

The source file, as specified in the following expressions, can be nonexistent
on the file system (i.e. generated file) but must be known from CMake. A source
file becomes known from CMake if it is part of some target (library or
executable) or when a source file property is defined. Moreover, this
information is specific to the directory where the declaration occurred.

For example, these generator expressions enable to offer a uniform behavior,
for the :command:`add_custom_command` and :command:`add_custom_target`
commands, regarding the source properties:

.. code-block:: cmake

  function(custom_add_library target)
    unset(sources)
    foreach(source IN LISTS ARGN)
      add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${source}.bin
        COMMAND my-compiler -o ${CMAKE_CURRENT_BINARY_DIR}/${source}.bin
                            "$<$<SOURCE_EXISTS:${source}>:$<SOURCE_PROPERTY:${source},COMPILE_OPTIONS>>"
                            ${source})
      list(APPEND sources ${CMAKE_CURRENT_BINARY_DIR}/${source}.bin)
    endforeach()
    add_custom_target(${target}
                      DEPENDS ${sources})
  endfunction()

  custom_add_library(my-lib file1.x file2.x file3.x)
  set_property(SOURCE file1.x PROPERTY COMPILE_OPTIONS -X)
  set_property(SOURCE file2.x PROPERTY COMPILE_OPTIONS -Y)

Source Meta-Data
^^^^^^^^^^^^^^^^

These expressions look up information about a source file.


.. toctree::
   :maxdepth: 1

   /genex/SOURCE_EXISTS

Source Properties
^^^^^^^^^^^^^^^^^

These expressions look up the values of
:ref:`source file properties <Source File Properties>`.


.. toctree::
   :maxdepth: 1

   /genex/SOURCE_PROPERTY

.. _`FileSet-Dependent Expressions`:

FileSet-Dependent Expressions
-----------------------------

FileSet Meta-Data
^^^^^^^^^^^^^^^^^

These expressions look up information about a file set.


.. toctree::
   :maxdepth: 1

   /genex/FILE_SET_EXISTS

FileSet Properties
^^^^^^^^^^^^^^^^^^

These expressions look up the values of file set properties.


.. toctree::
   :maxdepth: 1

   /genex/FILE_SET_PROPERTY

.. _`Target-Dependent Expressions`:

Target-Dependent Expressions
----------------------------

Target Meta-Data
^^^^^^^^^^^^^^^^

These expressions look up information about a target.


.. toctree::
   :maxdepth: 1

   /genex/TARGET_EXISTS
   /genex/TARGET_NAME_IF_EXISTS
   /genex/TARGET_NAME
   /genex/TARGET_POLICY

Target Properties
^^^^^^^^^^^^^^^^^

These expressions look up the values of
:ref:`target properties <Target Properties>`.


.. toctree::
   :maxdepth: 1

   /genex/TARGET_PROPERTY

The expressions have special evaluation rules for some properties:

:ref:`Target Build Specification Properties <Target Build Specification>`
  These evaluate as a :ref:`semicolon-separated list <CMake Language Lists>`
  representing the union of the value on the target itself with the values
  of the corresponding :ref:`Target Usage Requirements` on targets named by
  the target's :prop_tgt:`LINK_LIBRARIES`:

  * For :ref:`Target Compile Properties`, evaluation of corresponding usage
    requirements is transitive over the closure of the linked targets'
    :prop_tgt:`INTERFACE_LINK_LIBRARIES` *excluding* entries guarded by the
    :genex:`LINK_ONLY` generator expression.

  * For :ref:`Target Link Properties`, evaluation of corresponding usage
    requirements is transitive over the closure of the linked targets'
    :prop_tgt:`INTERFACE_LINK_LIBRARIES` *including* entries guarded by the
    :genex:`LINK_ONLY` generator expression.  See policy :policy:`CMP0166`.

  .. versionchanged:: 4.1

    Evaluation of :prop_tgt:`LINK_LIBRARIES` itself is now transitive.
    See policy :policy:`CMP0189`.

:ref:`Target Usage Requirement Properties <Target Usage Requirements>`
  These evaluate as a :ref:`semicolon-separated list <CMake Language Lists>`
  representing the union of the value on the target itself with the values
  of the same properties on targets named by the target's
  :prop_tgt:`INTERFACE_LINK_LIBRARIES`:

  * For :ref:`Transitive Compile Properties`, evaluation is transitive over
    the closure of the target's :prop_tgt:`INTERFACE_LINK_LIBRARIES`
    *excluding* entries guarded by the :genex:`LINK_ONLY` generator expression.

  * For :ref:`Transitive Link Properties`, evaluation is transitive over
    the closure of the target's :prop_tgt:`INTERFACE_LINK_LIBRARIES`
    *including* entries guarded by the :genex:`LINK_ONLY` generator expression.
    See policy :policy:`CMP0166`.

  .. versionchanged:: 4.1

    Evaluation of :prop_tgt:`INTERFACE_LINK_LIBRARIES` itself is now
    transitive.  See policy :policy:`CMP0189`.

:ref:`Custom Transitive Properties`
  .. versionadded:: 3.30

  These are processed during evaluation as follows:

  * Evaluation of :genex:`$<TARGET_PROPERTY:tgt,PROP>` for some property
    ``PROP``, named without an ``INTERFACE_`` prefix,
    checks the :prop_tgt:`TRANSITIVE_COMPILE_PROPERTIES`
    and :prop_tgt:`TRANSITIVE_LINK_PROPERTIES` properties on target ``tgt``,
    on targets named by its :prop_tgt:`LINK_LIBRARIES`, and on the
    transitive closure of targets named by the linked targets'
    :prop_tgt:`INTERFACE_LINK_LIBRARIES`.

    If ``PROP`` is listed by one of those properties, then it evaluates as
    a :ref:`semicolon-separated list <CMake Language Lists>` representing
    the union of the value on the target itself with the values of the
    corresponding ``INTERFACE_PROP`` on targets named by the target's
    :prop_tgt:`LINK_LIBRARIES`:

    * If ``PROP`` is named by :prop_tgt:`TRANSITIVE_COMPILE_PROPERTIES`,
      evaluation of the corresponding ``INTERFACE_PROP`` is transitive over
      the closure of the linked targets' :prop_tgt:`INTERFACE_LINK_LIBRARIES`,
      excluding entries guarded by the :genex:`LINK_ONLY` generator expression.

    * If ``PROP`` is named by :prop_tgt:`TRANSITIVE_LINK_PROPERTIES`,
      evaluation of the corresponding ``INTERFACE_PROP`` is transitive over
      the closure of the linked targets' :prop_tgt:`INTERFACE_LINK_LIBRARIES`,
      including entries guarded by the :genex:`LINK_ONLY` generator expression.

  * Evaluation of :genex:`$<TARGET_PROPERTY:tgt,INTERFACE_PROP>` for some
    property ``INTERFACE_PROP``, named with an ``INTERFACE_`` prefix,
    checks the :prop_tgt:`TRANSITIVE_COMPILE_PROPERTIES`
    and :prop_tgt:`TRANSITIVE_LINK_PROPERTIES` properties on target ``tgt``,
    and on the transitive closure of targets named by its
    :prop_tgt:`INTERFACE_LINK_LIBRARIES`.

    If the corresponding ``PROP`` is listed by one of those properties,
    then ``INTERFACE_PROP`` evaluates as a
    :ref:`semicolon-separated list <CMake Language Lists>` representing the
    union of the value on the target itself with the value of the same
    property on targets named by the target's
    :prop_tgt:`INTERFACE_LINK_LIBRARIES`:

    * If ``PROP`` is named by :prop_tgt:`TRANSITIVE_COMPILE_PROPERTIES`,
      evaluation of the corresponding ``INTERFACE_PROP`` is transitive over
      the closure of the target's :prop_tgt:`INTERFACE_LINK_LIBRARIES`,
      excluding entries guarded by the :genex:`LINK_ONLY` generator expression.

    * If ``PROP`` is named by :prop_tgt:`TRANSITIVE_LINK_PROPERTIES`,
      evaluation of the corresponding ``INTERFACE_PROP`` is transitive over
      the closure of the target's :prop_tgt:`INTERFACE_LINK_LIBRARIES`,
      including entries guarded by the :genex:`LINK_ONLY` generator expression.

  If a ``PROP`` is named by both :prop_tgt:`TRANSITIVE_COMPILE_PROPERTIES`
  and :prop_tgt:`TRANSITIVE_LINK_PROPERTIES`, the latter takes precedence.

:ref:`Compatible Interface Properties`
  These evaluate as a single value combined from the target itself,
  from targets named by the target's :prop_tgt:`LINK_LIBRARIES`, and
  from the transitive closure of the linked targets'
  :prop_tgt:`INTERFACE_LINK_LIBRARIES`.  Values of a compatible
  interface property from multiple targets combine based on the type
  of compatibility required by the ``COMPATIBLE_INTERFACE_*`` property
  defining it.


Target Artifacts
^^^^^^^^^^^^^^^^

These expressions look up information about artifacts associated with
a given target ``tgt``.  Unless otherwise stated, this can be any
runtime artifact, namely:

* An executable target created by :command:`add_executable`.
* A shared library target (``.so``, ``.dll`` but not their ``.lib`` import
  library) created by :command:`add_library`.
* A static library target created by :command:`add_library`.

In the following, the phrase "the ``tgt`` filename" means the name of the
``tgt`` binary file. This has to be distinguished from the phrase
"the target name", which is just the string ``tgt``.


.. toctree::
   :maxdepth: 1

   /genex/TARGET_FILE
   /genex/TARGET_FILE_BASE_NAME
   /genex/TARGET_FILE_PREFIX
   /genex/TARGET_FILE_SUFFIX
   /genex/TARGET_FILE_NAME
   /genex/TARGET_FILE_DIR
   /genex/TARGET_IMPORT_FILE
   /genex/TARGET_IMPORT_FILE_BASE_NAME
   /genex/TARGET_IMPORT_FILE_PREFIX
   /genex/TARGET_IMPORT_FILE_SUFFIX
   /genex/TARGET_IMPORT_FILE_NAME
   /genex/TARGET_IMPORT_FILE_DIR
   /genex/TARGET_LINKER_FILE
   /genex/TARGET_LINKER_FILE_BASE_NAME
   /genex/TARGET_LINKER_FILE_PREFIX
   /genex/TARGET_LINKER_FILE_SUFFIX
   /genex/TARGET_LINKER_FILE_NAME
   /genex/TARGET_LINKER_FILE_DIR
   /genex/TARGET_LINKER_LIBRARY_FILE
   /genex/TARGET_LINKER_LIBRARY_FILE_BASE_NAME
   /genex/TARGET_LINKER_LIBRARY_FILE_PREFIX
   /genex/TARGET_LINKER_LIBRARY_FILE_SUFFIX
   /genex/TARGET_LINKER_LIBRARY_FILE_NAME
   /genex/TARGET_LINKER_LIBRARY_FILE_DIR
   /genex/TARGET_LINKER_IMPORT_FILE
   /genex/TARGET_LINKER_IMPORT_FILE_BASE_NAME
   /genex/TARGET_LINKER_IMPORT_FILE_PREFIX
   /genex/TARGET_LINKER_IMPORT_FILE_SUFFIX
   /genex/TARGET_LINKER_IMPORT_FILE_NAME
   /genex/TARGET_LINKER_IMPORT_FILE_DIR
   /genex/TARGET_SONAME_FILE
   /genex/TARGET_SONAME_FILE_NAME
   /genex/TARGET_SONAME_FILE_DIR
   /genex/TARGET_SONAME_IMPORT_FILE
   /genex/TARGET_SONAME_IMPORT_FILE_NAME
   /genex/TARGET_SONAME_IMPORT_FILE_DIR
   /genex/TARGET_PDB_FILE
   /genex/TARGET_PDB_FILE_BASE_NAME
   /genex/TARGET_PDB_FILE_NAME
   /genex/TARGET_PDB_FILE_DIR
   /genex/TARGET_BUNDLE_DIR
   /genex/TARGET_BUNDLE_DIR_NAME
   /genex/TARGET_BUNDLE_CONTENT_DIR
   /genex/TARGET_OBJECTS
   /genex/TARGET_RUNTIME_DLLS
   /genex/TARGET_RUNTIME_DLL_DIRS
   /genex/TARGET_INTERMEDIATE_DIR

Export And Install Expressions
------------------------------


.. toctree::
   :maxdepth: 1

   /genex/INSTALL_INTERFACE
   /genex/BUILD_INTERFACE
   /genex/BUILD_LOCAL_INTERFACE
   /genex/INSTALL_PREFIX

Multi-level Expression Evaluation
---------------------------------


.. toctree::
   :maxdepth: 1

   /genex/GENEX_EVAL
   /genex/TARGET_GENEX_EVAL

Support Expressions
-------------------

Conversions
^^^^^^^^^^^


.. toctree::
   :maxdepth: 1

   /genex/ARGS

Escaped Characters
^^^^^^^^^^^^^^^^^^

These expressions evaluate to specific string literals. Use them in place of
the actual string literal where you need to prevent them from having their
special meaning.


.. toctree::
   :maxdepth: 1

   /genex/ANGLE-R
   /genex/COMMA
   /genex/SEMICOLON
   /genex/QUOTE

Deprecated Expressions
----------------------


.. toctree::
   :maxdepth: 1

   /genex/CONFIGURATION
