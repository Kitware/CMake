.. cmake-manual-description: CMake Generator Expressions

cmake-generator-expressions(7)
******************************

.. only:: html

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

They enable conditional linking, conditional definitions used when compiling,
conditional include directories, and more.  The conditions may be based on
the build configuration, target properties, platform information or any other
queryable information.

Generator expressions have the form ``$<...>``.  To avoid confusion, this page
deviates from most of the CMake documentation in that it omits angular brackets
``<...>`` around placeholders like ``condition``, ``string``, ``target``,
among others.

Generator expressions can be nested, as shown in most of the examples below.

.. _`Boolean Generator Expressions`:

Boolean Generator Expressions
=============================

Boolean expressions evaluate to either ``0`` or ``1``.
They are typically used to construct the condition in a :ref:`conditional
generator expression<Conditional Generator Expressions>`.

Available boolean expressions are:

Logical Operators
-----------------

``$<BOOL:string>``
  Converts ``string`` to ``0`` or ``1`` according to the rules of the
  :command:`if()` command.  Evaluates to ``0`` if any of the following is true:

  * ``string`` is empty,
  * ``string`` is a case-insensitive equal of
    ``0``, ``FALSE``, ``OFF``, ``N``, ``NO``, ``IGNORE``, or ``NOTFOUND``, or
  * ``string`` ends in the suffix ``-NOTFOUND`` (case-sensitive).

  Otherwise evaluates to ``1``.

``$<AND:conditions>``
  where ``conditions`` is a comma-separated list of boolean expressions.
  Evaluates to ``1`` if all conditions are ``1``.
  Otherwise evaluates to ``0``.

``$<OR:conditions>``
  where ``conditions`` is a comma-separated list of boolean expressions.
  Evaluates to ``1`` if at least one of the conditions is ``1``.
  Otherwise evaluates to ``0``.

``$<NOT:condition>``
  ``0`` if ``condition`` is ``1``, else ``1``.

String Comparisons
------------------

``$<STREQUAL:string1,string2>``
  ``1`` if ``string1`` and ``string2`` are equal, else ``0``.
  The comparison is case-sensitive.  For a case-insensitive comparison,
  combine with a :ref:`string transforming generator expression
  <String Transforming Generator Expressions>`,

  .. code-block:: cmake

    $<STREQUAL:$<UPPER_CASE:${foo}>,"BAR"> # "1" if ${foo} is any of "BAR", "Bar", "bar", ...

``$<EQUAL:value1,value2>``
  ``1`` if ``value1`` and ``value2`` are numerically equal, else ``0``.
``$<IN_LIST:string,list>``
  ``1`` if ``string`` is member of the comma-separated ``list``, else ``0``.
  Uses case-sensitive comparisons.
``$<VERSION_LESS:v1,v2>``
  ``1`` if ``v1`` is a version less than ``v2``, else ``0``.
``$<VERSION_GREATER:v1,v2>``
  ``1`` if ``v1`` is a version greater than ``v2``, else ``0``.
``$<VERSION_EQUAL:v1,v2>``
  ``1`` if ``v1`` is the same version as ``v2``, else ``0``.
``$<VERSION_LESS_EQUAL:v1,v2>``
  ``1`` if ``v1`` is a version less than or equal to ``v2``, else ``0``.
``$<VERSION_GREATER_EQUAL:v1,v2>``
  ``1`` if ``v1`` is a version greater than or equal to ``v2``, else ``0``.


Variable Queries
----------------

``$<TARGET_EXISTS:target>``
  ``1`` if ``target`` exists, else ``0``.
``$<CONFIG:cfg>``
  ``1`` if config is ``cfg``, else ``0``. This is a case-insensitive comparison.
  The mapping in :prop_tgt:`MAP_IMPORTED_CONFIG_<CONFIG>` is also considered by
  this expression when it is evaluated on a property on an :prop_tgt:`IMPORTED`
  target.
``$<PLATFORM_ID:platform_id>``
  ``1`` if the CMake-id of the platform matches ``platform_id``
  otherwise ``0``.
  See also the :variable:`CMAKE_SYSTEM_NAME` variable.
``$<C_COMPILER_ID:compiler_id>``
  ``1`` if the CMake-id of the C compiler matches ``compiler_id``,
  otherwise ``0``.
  See also the :variable:`CMAKE_<LANG>_COMPILER_ID` variable.
``$<CXX_COMPILER_ID:compiler_id>``
  ``1`` if the CMake-id of the CXX compiler matches ``compiler_id``,
  otherwise ``0``.
  See also the :variable:`CMAKE_<LANG>_COMPILER_ID` variable.
``$<Fortran_COMPILER_ID:compiler_id>``
  ``1`` if the CMake-id of the Fortran compiler matches ``compiler_id``,
  otherwise ``0``.
  See also the :variable:`CMAKE_<LANG>_COMPILER_ID` variable.
``$<C_COMPILER_VERSION:version>``
  ``1`` if the version of the C compiler matches ``version``, otherwise ``0``.
  See also the :variable:`CMAKE_<LANG>_COMPILER_VERSION` variable.
``$<CXX_COMPILER_VERSION:version>``
  ``1`` if the version of the CXX compiler matches ``version``, otherwise ``0``.
  See also the :variable:`CMAKE_<LANG>_COMPILER_VERSION` variable.
``$<Fortran_COMPILER_VERSION:version>``
  ``1`` if the version of the Fortran compiler matches ``version``, otherwise ``0``.
  See also the :variable:`CMAKE_<LANG>_COMPILER_VERSION` variable.
``$<TARGET_POLICY:policy>``
  ``1`` if the ``policy`` was NEW when the 'head' target was created,
  else ``0``.  If the ``policy`` was not set, the warning message for the policy
  will be emitted. This generator expression only works for a subset of
  policies.
``$<COMPILE_FEATURES:features>``
  where ``features`` is a comma-spearated list.
  Evaluates to ``1`` if all of the ``features`` are available for the 'head'
  target, and ``0`` otherwise. If this expression is used while evaluating
  the link implementation of a target and if any dependency transitively
  increases the required :prop_tgt:`C_STANDARD` or :prop_tgt:`CXX_STANDARD`
  for the 'head' target, an error is reported.  See the
  :manual:`cmake-compile-features(7)` manual for information on
  compile features and a list of supported compilers.

.. _`Boolean COMPILE_LANGUAGE Generator Expression`:

``$<COMPILE_LANGUAGE:language>``
  ``1`` when the language used for compilation unit matches ``language``,
  otherwise ``0``.  This expression may be used to specify compile options,
  compile definitions, and include directories for source files of a
  particular language in a target. For example:

  .. code-block:: cmake

    add_executable(myapp main.cpp foo.c bar.cpp zot.cu)
    target_compile_options(myapp
      PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    )
    target_compile_definitions(myapp
      PRIVATE $<$<COMPILE_LANGUAGE:CXX>:COMPILING_CXX>
              $<$<COMPILE_LANGUAGE:CUDA>:COMPILING_CUDA>
    )
    target_include_directories(myapp
      PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/opt/foo/cxx_headers>
    )

  This specifies the use of the ``-fno-exceptions`` compile option,
  ``COMPILING_CXX`` compile definition, and ``cxx_headers`` include
  directory for C++ only (compiler id checks elided).  It also specifies
  a ``COMPILING_CUDA`` compile definition for CUDA.

  Note that with :ref:`Visual Studio Generators` and :generator:`Xcode` there
  is no way to represent target-wide compile definitions or include directories
  separately for ``C`` and ``CXX`` languages.
  Also, with :ref:`Visual Studio Generators` there is no way to represent
  target-wide flags separately for ``C`` and ``CXX`` languages.  Under these
  generators, expressions for both C and C++ sources will be evaluated
  using ``CXX`` if there are any C++ sources and otherwise using ``C``.
  A workaround is to create separate libraries for each source file language
  instead:

  .. code-block:: cmake

    add_library(myapp_c foo.c)
    add_library(myapp_cxx bar.cpp)
    target_compile_options(myapp_cxx PUBLIC -fno-exceptions)
    add_executable(myapp main.cpp)
    target_link_libraries(myapp myapp_c myapp_cxx)

String-Valued Generator Expressions
===================================

These expressions expand to some string.
For example,

.. code-block:: cmake

  include_directories(/usr/include/$<CXX_COMPILER_ID>/)

expands to ``/usr/include/GNU/`` or ``/usr/include/Clang/`` etc, depending on
the compiler identifier.

String-valued expressions may also be combined with other expressions.
Here an example for a string-valued expression within a boolean expressions
within a conditional expression:

.. code-block:: cmake

  $<$<VERSION_LESS:$<CXX_COMPILER_VERSION>,4.2.0>:OLD_COMPILER>

expands to ``OLD_COMPILER`` if the
:variable:`CMAKE_CXX_COMPILER_VERSION <CMAKE_<LANG>_COMPILER_VERSION>` is less
than 4.2.0.

And here two nested string-valued expressions:

.. code-block:: cmake

  -I$<JOIN:$<TARGET_PROPERTY:INCLUDE_DIRECTORIES>, -I>

generates a string of the entries in the :prop_tgt:`INCLUDE_DIRECTORIES` target
property with each entry preceded by ``-I``.

Expanding on the previous example, if one first wants to check if the
``INCLUDE_DIRECTORIES`` property is non-empty, then it is advisable to
introduce a helper variable to keep the code readable:

.. code-block:: cmake

  set(prop "$<TARGET_PROPERTY:INCLUDE_DIRECTORIES>") # helper variable
  $<$<BOOL:${prop}>:-I$<JOIN:${prop}, -I>>

The following string-valued generator expressions are available:

Escaped Characters
------------------

String literals to escape the special meaning a character would otherwise have:

``$<ANGLE-R>``
  A literal ``>``. Used for example to compare strings that contain a ``>``.
``$<COMMA>``
  A literal ``,``. Used for example to compare strings which contain a ``,``.
``$<SEMICOLON>``
  A literal ``;``. Used to prevent list expansion on an argument with ``;``.

.. _`Conditional Generator Expressions`:

Conditional Expressions
-----------------------

Conditional generator expressions depend on a boolean condition
that must be ``0`` or ``1``.

``$<condition:true_string>``
  Evaluates to ``true_string`` if ``condition`` is ``1``.
  Otherwise evaluates to the empty string.

``$<IF:condition,true_string,false_string>``
  Evaluates to ``true_string`` if ``condition`` is ``1``.
  Otherwise evaluates to ``false_string``.

Typically, the ``condition`` is a :ref:`boolean generator expression
<Boolean Generator Expressions>`.  For instance,

.. code-block:: cmake

  $<$<CONFIG:Debug>:DEBUG_MODE>

expands to ``DEBUG_MODE`` when the ``Debug`` configuration is used, and
otherwise expands to the empty string.

.. _`String Transforming Generator Expressions`:

String Transformations
----------------------

``$<JOIN:list,string>``
  Joins the list with the content of ``string``.
``$<LOWER_CASE:string>``
  Content of ``string`` converted to lower case.
``$<UPPER_CASE:string>``
  Content of ``string`` converted to upper case.

``$<GENEX_EVAL:expr>``
  Content of ``expr`` evaluated as a generator expression in the current
  context. This enables consumption of generator expressions whose
  evaluation results itself in generator expressions.
``$<TARGET_GENEX_EVAL:tgt,expr>``
  Content of ``expr`` evaluated as a generator expression in the context of
  ``tgt`` target. This enables consumption of custom target properties that
  themselves contain generator expressions.

  Having the capability to evaluate generator expressions is very useful when
  you want to manage custom properties supporting generator expressions.
  For example:

  .. code-block:: cmake

    add_library(foo ...)

    set_property(TARGET foo PROPERTY
      CUSTOM_KEYS $<$<CONFIG:DEBUG>:FOO_EXTRA_THINGS>
    )

    add_custom_target(printFooKeys
      COMMAND ${CMAKE_COMMAND} -E echo $<TARGET_PROPERTY:foo,CUSTOM_KEYS>
    )

  This naive implementation of the ``printFooKeys`` custom command is wrong
  because ``CUSTOM_KEYS`` target property is not evaluated and the content
  is passed as is (i.e. ``$<$<CONFIG:DEBUG>:FOO_EXTRA_THINGS>``).

  To have the expected result (i.e. ``FOO_EXTRA_THINGS`` if config is
  ``Debug``), it is required to evaluate the output of
  ``$<TARGET_PROPERTY:foo,CUSTOM_KEYS>``:

  .. code-block:: cmake

    add_custom_target(printFooKeys
      COMMAND ${CMAKE_COMMAND} -E
        echo $<TARGET_GENEX_EVAL:foo,$<TARGET_PROPERTY:foo,CUSTOM_KEYS>>
    )

Variable Queries
----------------

``$<CONFIG>``
  Configuration name.
``$<CONFIGURATION>``
  Configuration name. Deprecated since CMake 3.0. Use ``CONFIG`` instead.
``$<PLATFORM_ID>``
  The CMake-id of the platform.
  See also the :variable:`CMAKE_SYSTEM_NAME` variable.
``$<C_COMPILER_ID>``
  The CMake-id of the C compiler used.
  See also the :variable:`CMAKE_<LANG>_COMPILER_ID` variable.
``$<CXX_COMPILER_ID>``
  The CMake-id of the CXX compiler used.
  See also the :variable:`CMAKE_<LANG>_COMPILER_ID` variable.
``$<Fortran_COMPILER_ID>``
  The CMake-id of the Fortran compiler used.
  See also the :variable:`CMAKE_<LANG>_COMPILER_ID` variable.
``$<C_COMPILER_VERSION>``
  The version of the C compiler used.
  See also the :variable:`CMAKE_<LANG>_COMPILER_VERSION` variable.
``$<CXX_COMPILER_VERSION>``
  The version of the CXX compiler used.
  See also the :variable:`CMAKE_<LANG>_COMPILER_VERSION` variable.
``$<Fortran_COMPILER_VERSION>``
  The version of the Fortran compiler used.
  See also the :variable:`CMAKE_<LANG>_COMPILER_VERSION` variable.
``$<COMPILE_LANGUAGE>``
  The compile language of source files when evaluating compile options.
  See :ref:`the related boolean expression
  <Boolean COMPILE_LANGUAGE Generator Expression>`
  ``$<COMPILE_LANGUAGE:language>``
  for notes about the portability of this generator expression.

Target-Dependent Queries
------------------------

``$<TARGET_NAME_IF_EXISTS:tgt>``
  Expands to the ``tgt`` if the given target exists, an empty string
  otherwise.
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
``$<TARGET_PDB_FILE:tgt>``
  Full path to the linker generated program database file (.pdb)
  where ``tgt`` is the name of a target.

  See also the :prop_tgt:`PDB_NAME` and :prop_tgt:`PDB_OUTPUT_DIRECTORY`
  target properties and their configuration specific variants
  :prop_tgt:`PDB_NAME_<CONFIG>` and :prop_tgt:`PDB_OUTPUT_DIRECTORY_<CONFIG>`.
``$<TARGET_PDB_FILE_NAME:tgt>``
  Name of the linker generated program database file (.pdb).
``$<TARGET_PDB_FILE_DIR:tgt>``
  Directory of the linker generated program database file (.pdb).
``$<TARGET_BUNDLE_DIR:tgt>``
  Full path to the bundle directory (``my.app``, ``my.framework``, or
  ``my.bundle``) where ``tgt`` is the name of a target.
``$<TARGET_BUNDLE_CONTENT_DIR:tgt>``
  Full path to the bundle content directory where ``tgt`` is the name of a
  target. For the macOS SDK it leads to ``my.app/Contents``, ``my.framework``,
  or ``my.bundle/Contents``. For all other SDKs (e.g. iOS) it leads to
  ``my.app``, ``my.framework``, or ``my.bundle`` due to the flat bundle
  structure.
``$<TARGET_PROPERTY:tgt,prop>``
  Value of the property ``prop`` on the target ``tgt``.

  Note that ``tgt`` is not added as a dependency of the target this
  expression is evaluated on.
``$<TARGET_PROPERTY:prop>``
  Value of the property ``prop`` on the target on which the generator
  expression is evaluated. Note that for generator expressions in
  :ref:`Target Usage Requirements` this is the value of the property
  on the consuming target rather than the target specifying the
  requirement.
``$<INSTALL_PREFIX>``
  Content of the install prefix when the target is exported via
  :command:`install(EXPORT)` and empty otherwise.

Output-Related Expressions
--------------------------

``$<TARGET_NAME:...>``
  Marks ``...`` as being the name of a target.  This is required if exporting
  targets to multiple dependent export sets.  The ``...`` must be a literal
  name of a target- it may not contain generator expressions.
``$<LINK_ONLY:...>``
  Content of ``...`` except when evaluated in a link interface while
  propagating :ref:`Target Usage Requirements`, in which case it is the
  empty string.
  Intended for use only in an :prop_tgt:`INTERFACE_LINK_LIBRARIES` target
  property, perhaps via the :command:`target_link_libraries` command,
  to specify private link dependencies without other usage requirements.
``$<INSTALL_INTERFACE:...>``
  Content of ``...`` when the property is exported using :command:`install(EXPORT)`,
  and empty otherwise.
``$<BUILD_INTERFACE:...>``
  Content of ``...`` when the property is exported using :command:`export`, or
  when the target is used by another target in the same buildsystem. Expands to
  the empty string otherwise.
``$<MAKE_C_IDENTIFIER:...>``
  Content of ``...`` converted to a C identifier.  The conversion follows the
  same behavior as :command:`string(MAKE_C_IDENTIFIER)`.
``$<TARGET_OBJECTS:objLib>``
  List of objects resulting from build of ``objLib``. ``objLib`` must be an
  object of type ``OBJECT_LIBRARY``.
``$<SHELL_PATH:...>``
  Content of ``...`` converted to shell path style. For example, slashes are
  converted to backslashes in Windows shells and drive letters are converted
  to posix paths in MSYS shells. The ``...`` must be an absolute path.

Debugging
=========

Since generator expressions are evaluated during generation of the buildsystem,
and not during processing of ``CMakeLists.txt`` files, it is not possible to
inspect their result with the :command:`message()` command.

One possible way to generate debug messages is to add a custom target,

.. code-block:: cmake

  add_custom_target(genexdebug COMMAND ${CMAKE_COMMAND} -E echo "$<...>")

The shell command ``make genexdebug`` (invoked after execution of ``cmake``)
would then print the result of ``$<...>``.

Another way is to write debug messages to a file:

.. code-block:: cmake

  file(GENERATE OUTPUT filename CONTENT "$<...>")
