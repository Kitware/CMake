.. index::
   single: directive ignored

title_text
----------

.. comment ignored
..
   comment ignored

Command :cmake:command:`some_cmd` explicit cmake domain.
Command :command:`some_cmd` without target.
Command :command:`some_cmd <some_cmd>` with target.
Command :command:`some_cmd_<cmd>` placeholder without target.
Command :command:`some_cmd_<cmd> <some_cmd>` placeholder with target.
Command :command:`some_cmd()` with parens.
Command :command:`some_cmd(SUB)` with subcommand.
Command :command:`some_cmd(SUB) <some_cmd>` with subcommand and target.
Command :command:`some_cmd (SUB) <some_cmd>` with space and subcommand and target.
Command :command:`some command <some_cmd>` with space and target.
Variable :variable:`some variable <some_var>` space and target.
Variable :variable:`<PLACEHOLDER>_VARIABLE` with leading placeholder.
Variable :variable:`VARIABLE_<PLACEHOLDER>` with trailing placeholder.
Variable :variable:`<PLACEHOLDER>_VARIABLE <target>` with leading placeholder and target.
Variable :variable:`VARIABLE_<PLACEHOLDER> <target>` with trailing placeholder and target.
Environment variable :envvar:`SOME_ENV_VAR`.
Environment variable :envvar:`some env var <SOME_ENV_VAR>` with space and target.
Generator :generator:`Some Generator` with space.
Generator :cpack_gen:`Some Generator` with space.
Generator expression :genex:`SOME_GENEX`.
Generator expression :genex:`$<SOME_GENEX>` with brackets.
Generator expression :genex:`$<SOME_GENEX:...>` with brackets and parameter.
Generator expression :genex:`some genex <SOME_GENEX>` with space and target.
Generator expression :genex:`$<SOME_GENEX> <SOME_GENEX>` with brackets, space, and target.
Generator expression :genex:`$<SOME_GENEX:...> <SOME_GENEX>` with brackets, parameter, space, and target.
Inline cref :cref:`Link Dest`.
Inline cref :cref:`Link_Dest_<Placeholder>`.
Inline cref :cref:`Link Text <ExternalDest>`.
Inline cref :cref:`Link_Text_<Placeholder> <ExternalDest>`.
Inline link `Link Dest`_.
Inline link `Link Text <ExternalDest>`_.
Inline link `Link Text \<With \\-escaped Brackets\> <ExternalDest>`_.
Inline literal ``__`` followed by inline link `Link Text <InternalDest_>`_.
Inline literal ``~!@#$%^&*( )_+-=\\[]{}'":;,<>.?/``.

Try running :program:`cmake`.
Enable :diagnostic:`CMD_AUTHOR` warnings with :cmake-option:`-Wauthor`,
or with the :preset:`configurePresets.warnings.author` preset field.
Read the :manual:`cmake(1)` manual.
Learn about the :module:`GNUInstallDirs` module.
Policy :policy:`CMP9999` doesn't exist. (Yet?)

Some properties that don't exist:
- :prop_cache:`NO_SUCH_CACHE_PROPERTY`
- :prop_dir:`NO_SUCH_DIRECTORY_PROPERTY`
- :prop_fs:`NO_SUCH_FILESET_PROPERTY`
- :prop_gbl:`NO_SUCH_GLOBAL_PROPERTY`
- :prop_inst:`NO_SUCH_INSTALL_PROPERTY`
- :prop_sf:`NO_SUCH_SOURCE_FILE_PROPERTY`
- :prop_test:`NO_SUCH_TEST_PROPERTY`
- :prop_tgt:`NO_SUCH_TARGET_PROPERTY`

Various ways to reference command-line options:
- :option:`cmake --build \<dir\> <cmake --build>`
- :cmake-option:`--log-level`
- :cmake-build-option:`--parallel`
- :cmake-install-option:`--config`
- :cmake-workflow-option:`--preset`
- :cpack-option:`-G`
- :ctest-option:`--parallel`
- :ctest-dashboard-option:`--build-dir`

.. |not replaced| replace:: not replaced through toctree
.. |not replaced in literal| replace:: replaced in parsed literal

.. toctree::
   :maxdepth: 2

   testRSTtoc1
   /testRSTtoc2

.. cmake-module:: testRSTmod.cmake

.. cmake:command::
   some_cmd

   Command some_cmd description.

.. command:: other_cmd

   Command other_cmd description.

.. cmake:envvar::
   some_var

   Environment variable some_var description.

.. envvar:: other_var

   Environment variable other_var description.

.. cmake:genex::
   SOME_GENEX

   Generator expression SOME_GENEX description.

.. genex:: $<OTHER_GENEX>

   Generator expression $<OTHER_GENEX> description.

.. cmake:signature::
   some_command(SOME_SIGNATURE)

   Command some_command SOME_SIGNATURE description.

.. signature:: other_command(OTHER_SIGNATURE)

   Command other_command OTHER_SIGNATURE description.

.. cmake:variable::
   some_var

   Variable some_var description.

.. variable:: other_var

   Variable other_var description.

.. parsed-literal::

    Parsed-literal included without directive.
   Common Indentation Removed
   # |not replaced in literal|

.. code-block:: cmake

   # Sample CMake code block
   if(condition)
     message(indented)
   endif()
   # |not replaced in literal|

A literal block starts after a line consisting of two colons

::

    Literal block.
   Common Indentation Removed
   # |not replaced in literal|

or after a paragraph ending in two colons::

    Literal block.
   Common Indentation Removed
   # |not replaced in literal|

but not after a line ending in two colons::
in the middle of a paragraph.

A literal block can be empty::



.. productionlist::
 grammar: `production`
 production: "content rendered"

.. note::
 Notes are called out.

.. versionadded:: 1.2
 Version blocks are preserved.

.. versionchanged:: 2.3
 Version blocks are preserved.

.. |substitution| replace::
   |nested substitution|
   with multiple lines becomes one line
.. |nested substitution| replace:: substituted text

.. include:: testRSTinclude1.rst
.. include:: /testRSTinclude2.rst

Testing a |delayed substitution|.

.. |delayed substitution| replace:: replacement defined after use

|header substitution|
^^^^^^^^^^^^^^^^^^^^^

.. |header substitution| replace:: this text is a different length

Substitutions in headers should cause the header line's length to be updated.
