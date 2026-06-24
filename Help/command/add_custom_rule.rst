add_custom_rule
---------------

.. versionadded:: 4.5

Add a custom template rule to the generated build system.

Synopsis
^^^^^^^^

.. parsed-literal::
  `Generating Files`_
    add_custom_rule(<name> `OUTPUT`_ <output1> [<output2> ...]
                    COMMAND <command1> [<args1>...]
                    [...])

  `Derived Rule`_
    add_custom_rule(<name> `FROM_RULE`_ <rule>
                    [...])

Generating Files
^^^^^^^^^^^^^^^^

.. signature::
  add_custom_rule(<name> OUTPUT <output1> [<output2> ...]
                         COMMAND <command1> [<args1>...]
                         [...])
  :target:
    OUTPUT

  Add a custom template rule to produce an output:

  .. code-block:: cmake

    add_custom_rule(<name> OUTPUT <output1> [<output2> ...]
                           COMMAND <command1> [<args1>...]
                           [COMMAND <command2> [<args2>...]] ...
                           [DEPENDS <depends>...]
                           [BYPRODUCTS <files>...]
                           [DEPFILE <depfile>]
                           [CONFIGURATOR [FOR_FILE_SET <configurator>]
                                         [FOR_SOURCE <configurator>]]
                           [GLOBAL])

  This defines a template rule ``<name>`` to generate specified ``OUTPUT``
  file(s). Rule names defined in all uppercase are reserved for CMake's own
  built-in rules.

  The association of source files with the template rule is done by creating
  :ref:`file sets <File Sets>` of type ``<name>``. For each file of the file
  set, a :command:`custom command <add_custom_command>` will be created in the
  same directory as the target owning the file set and the output files of this
  custom command will be declared as part of a file set attached to
  the same target. The type of this file set, as well as its name, are
  controlled by the :prop_rule:`OUTPUT_FILE_SET` rule property. This output
  file set will have the same scope (``PRIVATE``, ``PUBLIC``, or ``INTERFACE``)
  as the input file set.

  To parameterize the template, some patterns are defined which can be used as
  part of the ``add_custom_rule`` arguments as well as the :ref:`rule's
  properties <Rule Properties>`. These patterns will be
  instantiated for each source file. The supported patterns are:

  .. note::

    The instantiation of the patterns are done in the context of the directory
    where the file set was created.

    These patterns cannot be changed by the functions specified by the
    ``CONFIGURATOR`` option.

  ``<RULE>``
    Name of the rule used as template.

  ``<TARGET>``
    Name of the target to which the file set of sources is attached.

  ``<FILE_SET>``
    Name of the file set used for the rule instantiation.

  ``<SOURCE_DIR>``
    The value of the :variable:`CMAKE_SOURCE_DIR` variable.

  ``<BINARY_DIR>``
    The value of the :variable:`CMAKE_BINARY_DIR` variable.

  ``<CURRENT_SOURCE_DIR>``
    The path to the source directory of the file set creation.

  ``<CURRENT_BINARY_DIR>``
    The path to the binary directory of the file set creation.

  ``<SOURCE>``
    The full path of the current source file being processed.

  ``<INPUT_DIR>``
    The directory of the current source file being processed.

  ``<FILE_NAME>``
    The file name of the current source file being processed.

  ``<BASE_NAME>``
    The stem name (i.e. without directory and extension) of the source file
    being processed.

  ``<INCLUDE_DIRECTORIES>``
    Content, in this order, of the :prop_fs:`INCLUDE_DIRECTORIES` file set
    property, :prop_sf:`INCLUDE_DIRECTORIES` source property, and
    :prop_rule:`INCLUDE_DIRECTORIES` rule property.

    Because CMake is not aware of the tool involved by the rule, there is
    no specific processing regarding this pattern. This is the user's
    responsibility to format, using  :manual:`generator expressions
    <cmake-generator-expressions(7)>`, the content of pattern to be compatible
    with the tool.

    For example, if the tool requires the flag ``-inc:`` to identify an include
    directory, the following can be specified as part of the ``COMMAND``
    option:

    .. code-block:: cmake

      $<LIST:TRANSFORM,<INCLUDE_DIRECTORIES>,PREPEND,-inc:>

  ``<COMPILE_DEFINITIONS>``
    Content, in this order, of the :prop_rule:`COMPILE_DEFINITIONS` rule
    property, :prop_sf:`COMPILE_DEFINITIONS` source property, and
    :prop_fs:`COMPILE_DEFINITIONS` file set property.

    Because CMake is not aware of the tool involved by the rule, there is
    no specific processing regarding this pattern. This is the user's
    responsibility to format, using  :manual:`generator expressions
    <cmake-generator-expressions(7)>`, the content of pattern to be compatible
    with the tool.

    For example, if the tool requires the flag ``-def:`` to identify a compile
    definition, the following can be specified as part of the ``COMMAND``
    option:

    .. code-block:: cmake

      $<LIST:TRANSFORM,<COMPILE_DEFINITIONS>,PREPEND,-def:>

  ``<COMPILE_OPTIONS>``
    Content, in this order, of the :prop_rule:`COMPILE_OPTIONS` rule property,
    :prop_sf:`COMPILE_OPTIONS` source property, and:prop_fs:`COMPILE_OPTIONS`
    file set property.

  The options, which have the same semantics as those of the
  :command:`add_custom_command` command, are:

  ``OUTPUT``
    Specify the output files the command is expected to produce.
    Each output file will be marked with the :prop_sf:`GENERATED`
    source file property automatically. At least one ``OUTPUT`` must be given.

  ``COMMAND``
    Specify the command-line(s) to execute at build time.
    At least one ``COMMAND`` must be given.

  ``DEPENDS``
    Specify files on which the command depends.

  ``BYPRODUCTS``
    Specify the files the command is expected to produce but whose
    modification time may or may not be newer than the dependencies.

  ``DEPFILE``
    Specify a depfile which holds dependencies for the custom command. It is
    usually emitted by the custom command itself.

  ``CONFIGURATOR``
    Specify one or two CMake functions which will be called at the generation
    step, in the context of the file set directory, before the effective
    instantiation and custom commands definition.

    .. note::

       The rule properties are all read-only during the execution of the
       configurators. Moreover, it is strongly discouraged to change the
       target properties.

    ``FOR_FILE_SET``
      The specified function will be called once per file set.
      The expected signature is the following:

      .. signature::
        configurator(rule target fileset outputFileset patterns)

      The arguments provide the names of the effective artifacts involved in
      the current rule instantiation.

      The ``patterns`` argument holds the name of the variable which can be
      used to enrich the list of patterns. The expected value is a
      :ref:`semicolon-separated list <CMake Language Lists>` of  items having
      the syntax ``PATTERN=VALUE`` or ``PATTERN=``. More precisely, each item
      must match the regular expression ``(^[A-Z][A-Z0-9_]+)=(.*)$``. Items
      which does not this regular expression will be ignored.

    ``FOR_SOURCE``
      The specified function will be called for each file of the file set.
      The expected signature is the following:

      .. signature::
        configurator(rule target fileset outputFileset source patterns)

      The arguments provide the names of the effective artifacts involved in
      the current rule instantiation.

      The ``patterns`` argument holds the name of the variable which can be
      used to enrich the list of patterns. The expected value is a
      :ref:`semicolon-separated list <CMake Language Lists>` of  items having
      the syntax ``PATTERN=VALUE`` or ``PATTERN=``. More precisely, each item
      must match the regular expression ``(^[A-Z][A-Z0-9_]+)=(.*)$``. Items
      which does not this regular expression will be ignored.

      .. note::

        The source configurator is evaluated after the file set one. So, the
        changes done by it will overwrite any changes done by the file set
        configurator.

    .. note::

      Any patterns specified through The :prop_fs:`RULE_PATTERNS` file set
      and :prop_sf:`<RULE>_PATTERNS` source file properties will take
      precedence over, respectively, the file set and the source configurators.

  ``GLOBAL``
    Make the rule name globally visible. Without this keyword, the rule will
    only be visible in the directory where it was created as well as the
    sub-directories.

Example
=======

Define a rule to compile swig files:

.. code-block:: cmake

  function(fileset_configurator rule target fileset patterns)
    # define <OUTFILE_DIR> pattern
    set(${patterns} "OUTFILE_DIR=<CURRENT_BINARY_DIR>" PARENT_SCOPE)
  endfunction()

  function(source_configurator rule target fileset source patterns)
    # define flag to handle C++
    get_property(cxx SOURCE "${source}" TARGET_DIRECTORY "${target}" PROPERTY CPLUSPLUS)
    if (cxx)
      set_property(SOURCE "${source}" TARGET_DIRECTORY "${target}"
                   APPEND PROPERTY COMPILE_OPTIONS -c++)
    endif()
  endfunction()

  set(OUTFILE_EXT "$<IF:$<BOOL:$<SOURCE_PROPERTY:<SOURCE>,TARGET_DIRECTORY:<TARGET>,CPLUSPLUS>>,.cxx,.c>")
  set(SWIG_LANGUAGE "-$<STRING:TOLOWER,$<FILE_SET_PROPERTY:<FILE_SET>,TARGET:<TARGET>,LANGUAGE>>")

  add_custom_rule(swig
    OUTPUT "<OUTFILE_DIR>/<BASE_NAME>${OUTFILE_EXT}"
    COMMAND ${SWIG_EXECUTABLE} "<SOURCE>"
                               "<OUTFILE_DIR>/<BASE_NAME>${OUTFILE_EXT}"
                               ${SWIG_LANGUAGE}
                               <COMPILE_OPTIONS>
    CONFIGURATOR FOR_FILE_SET fileset_configurator FOR_SOURCE source_configurator)

And, by defining a file set of type ``swig``, we can compile swig sources:

.. code-block:: cmake

  add_library(swig_example)

  target_sources(swig_example PRIVATE FILE_SET swig_srcs TYPE swig
                                      FILES file1.i file2.i)
  # define the target language
  set_property(FILE_SET swig_srcs TARGET swig_example PROPERTY LANGUAGE python)
  # define swig c++ mode
  set_property(SOURCE file1.i file2.i PROPERTY CPLUSPLUS ON)

Derived Rule
^^^^^^^^^^^^

.. signature::
  add_custom_rule(<name> FROM_RULE <rule>
                  [...])
  :target:
    FROM_RULE

  Create a new template rule ``<name>`` inheriting a snapshot of all the
  characteristics of the ``<rule>``, including the properties except the
  ``GLOBAL`` one. Rule names defined in all uppercase are reserved for CMake's
  own built-in rules.

  .. code-block:: cmake

    add_custom_rule(<name> FROM_RULE <rule>
                           [CONFIGURATOR [FOR_FILE_SET <configurator> [CHAIN|OVERRIDE]]
                                         [FOR_SOURCE <configurator> [CHAIN|OVERRIDE]]]
                           [GLOBAL])

  Properties attached to this new rule can be freely customized, independently
  of the rule we inherited from.

  The options are:

  ``FROM_RULE``
    Specify the rule from which this new rule will inherit.

  ``CONFIGURATOR``
    Specify one or two CMake functions which will be called at the generation
    step before the effective instantiation and custom commands definition.

    .. note::

       The rule properties are all read-only during the execution of the
       configurators. Moreover, it is strongly discouraged to change the
       target properties.

    ``FOR_FILE_SET``
      The specified function will be called once per file set.
      The expected signature is the following:

      .. signature::
        configurator(rule target fileset outputFileset patterns)

      The arguments provide the names of the effective artifacts involved in
      the current rule instantiation.

      The ``patterns`` argument holds the name of the variable which can be
      used to enrich the list of patterns.

    ``FOR_SOURCE``
      The specified function will be called for each file of the file set.
      The expected signature is the following:

      .. signature::
        configurator(rule target fileset outputFileset source patterns)

      The arguments provide the names of the effective artifacts involved in
      the current rule instantiation.

      The ``patterns`` argument holds the name of the variable which can be
      used to enrich the list of patterns.

    For these two sub-options, there are two possible configurations:

    ``CHAIN``
      This ``<configurator>`` will be added to the already specified
      configurators of inherited rules. Configurators will be called in order
      of their rules' definition.

    ``OVERRIDE``
      The specified ``<configurator>`` will override any other already defined
      configurators. This is the default.

  ``GLOBAL``
    Make the rule name globally visible. Without this keyword, the rule will
    only be visible in the directory where it was created as well as the
    sub-directories.

Example
=======

By reusing the previous defined rule ``swig``, we can provide a more simple way
to compile swig sources by creating a more specialized rule:


.. code-block:: cmake

  function(python_configurator rule target fileset outputFileset patterns)
    # define target language
    set_property(FILE_SET ${fileset} TARGET ${target} PROPERTY LANGUAGE python)
  endfunction()

  function(cxx_configurator rule target fileset outputFileset source patterns)
    # define swig c++ mode
    set_property(SOURCE "${source}" TARGET_DIRECTORY ${target} PROPERTY CPLUSPLUS ON)
    set_property(SOURCE "${source}" TARGET_DIRECTORY ${target}
                 APPEND PROPERTY COMPILE_OPTIONS -c++)
  endfunction()

  add_custom_rule(swig_python FROM_RULE swig
                              CONFIGURATOR FOR_FILE_SET python_configurator CHAIN
                                           FOR_SOURCE cxx_configurator OVERRIDE)

Now, we can define a file set which does not need any specific settings. And
because the ``CHAIN`` option was specified for the file set configurator, the
pattern ``<OUTFILE_DIR>`` will be defined as well.

.. code-block:: cmake

  add_library(swig_example)

  target_sources(swig_example PRIVATE FILE_SET swig_srcs TYPE swig_python
                                      FILES file1.i file2.i)

See Also
^^^^^^^^

* :command:`set_property(RULE)`
* :command:`get_property(RULE)`
* :command:`target_sources`
* :command:`add_custom_command`
