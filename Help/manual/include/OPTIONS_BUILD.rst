.. option:: -S <path-to-source>

 Path to root directory of the CMake project to build.

.. option:: -B <path-to-build>

 Path to directory which CMake will use as the root of build directory.

 If the directory doesn't already exist CMake will make it.

.. option:: -C <initial-cache>

 Pre-load a script to populate the cache.

 When CMake is first run in an empty build tree, it creates a
 ``CMakeCache.txt`` file and populates it with customizable settings for
 the project.  This option may be used to specify a file from which
 to load cache entries before the first pass through the project's
 CMake listfiles.  The loaded entries take priority over the
 project's default values.  The given file should be a CMake script
 containing :command:`set` commands that use the ``CACHE`` option, not a
 cache-format file.

 References to :variable:`CMAKE_SOURCE_DIR` and :variable:`CMAKE_BINARY_DIR`
 within the script evaluate to the top-level source and build tree.

.. option:: -D <var>:<type>=<value>, -D <var>=<value>

 Create or update a CMake ``CACHE`` entry.

 When CMake is first run in an empty build tree, it creates a
 ``CMakeCache.txt`` file and populates it with customizable settings for
 the project.  This option may be used to specify a setting that
 takes priority over the project's default value.  The option may be
 repeated for as many ``CACHE`` entries as desired.

 If the ``:<type>`` portion is given it must be one of the types
 specified by the :command:`set` command documentation for its
 ``CACHE`` signature.
 If the ``:<type>`` portion is omitted the entry will be created
 with no type if it does not exist with a type already.  If a
 command in the project sets the type to ``PATH`` or ``FILEPATH``
 then the ``<value>`` will be converted to an absolute path.

 This option may also be given as a single argument:
 ``-D<var>:<type>=<value>`` or ``-D<var>=<value>``.

 It's important to note that the order of ``-C`` and ``-D`` arguments is
 significant. They will be carried out in the order they are listed, with the
 last argument taking precedence over the previous ones. For example, if you
 specify ``-DCMAKE_BUILD_TYPE=Debug``, followed by a ``-C`` argument with a
 file that calls:

 .. code-block:: cmake

   set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)

 then the ``-C`` argument will take precedence, and ``CMAKE_BUILD_TYPE`` will
 be set to ``Release``. However, if the ``-D`` argument comes after the ``-C``
 argument, it will be set to ``Debug``.

 If a ``set(... CACHE ...)`` call in the ``-C`` file does not use ``FORCE``,
 and a ``-D`` argument sets the same variable, the ``-D`` argument will take
 precedence regardless of order because of the nature of non-``FORCE``
 ``set(... CACHE ...)`` calls.

.. option:: -U <globbing_expr>

 Remove matching entries from CMake ``CACHE``.

 This option may be used to remove one or more variables from the
 ``CMakeCache.txt`` file, globbing expressions using ``*`` and ``?`` are
 supported.  The option may be repeated for as many ``CACHE`` entries as
 desired.

 Use with care, you can make your ``CMakeCache.txt`` non-working.

.. option:: -G <generator-name>

 Specify a build system generator.

 CMake may support multiple native build systems on certain
 platforms.  A generator is responsible for generating a particular
 build system.  Possible generator names are specified in the
 :manual:`cmake-generators(7)` manual.

 If not specified, CMake checks the :envvar:`CMAKE_GENERATOR` environment
 variable and otherwise falls back to a builtin default selection.

.. option:: -T <toolset-spec>

 Toolset specification for the generator, if supported.

 Some CMake generators support a toolset specification to tell
 the native build system how to choose a compiler.  See the
 :variable:`CMAKE_GENERATOR_TOOLSET` variable for details.

.. option:: -A <platform-name>

 Specify platform name if supported by generator.

 Some CMake generators support a platform name to be given to the
 native build system to choose a compiler or SDK.  See the
 :variable:`CMAKE_GENERATOR_PLATFORM` variable for details.

.. option:: --toolchain <path-to-file>

 .. versionadded:: 3.21

 Specify the cross compiling toolchain file, equivalent to setting
 :variable:`CMAKE_TOOLCHAIN_FILE` variable. Relative paths are interpreted as
 relative to the build directory, and if not found, relative to the source
 directory.

.. option:: --install-prefix <directory>

 .. versionadded:: 3.21

 Specify the installation directory, used by the
 :variable:`CMAKE_INSTALL_PREFIX` variable. Must be an absolute path.

.. option:: --project-file <project-file-name>

 .. versionadded:: 4.0

 Specify an alternate project file name.

 This determines the top-level file processed by CMake when configuring a
 project, and the file processed by :command:`add_subdirectory`.

 By default, this is ``CMakeLists.txt``. If set to anything else,
 ``CMakeLists.txt`` will be used as a fallback whenever the specified file
 cannot be found within a project subdirectory.

 .. note::

  This feature is intended for temporary use by developers during an incremental
  transition and not for publication of a final product. CMake will always emit
  a warning when the project file is anything other than ``CMakeLists.txt``.

.. option:: -W<category>

 Promote the specified diagnostic category (and any children) to at least
 ``WARN`` severity.  Note that this has no effect on diagnostics already set to
 a higher severity.

 See the :manual:`cmake-diagnostics(7)` manual for a list of accepted values
 for ``<category>``.

.. option:: -Wno-<category>

 Disable (ignore) all diagnostics of the specified category, including any
 child categories.

.. option:: -Werror=<category>

 Promote the specified diagnostic category (and any children) to at least
 ``SEND_ERROR`` severity.  Note that this has no effect on diagnostics already
 set to a higher severity.

.. option:: -Wno-error=<category>

 Demote the specified diagnostic category (and any children) to at most
 ``WARN`` severity.  Note that this has no effect on diagnostics already
 set to a lower severity.

.. option:: -Wdev, -Wno-dev

 .. deprecated:: 4.4

 Compatibility synonyms for ``-Wauthor`` / ``-Wno-author``.
