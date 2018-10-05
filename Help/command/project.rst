project
-------

Set the name of the project.

Synopsis
^^^^^^^^

.. code-block:: cmake

 project(<PROJECT-NAME> [<language-name>...])
 project(<PROJECT-NAME>
         [VERSION <major>[.<minor>[.<patch>[.<tweak>]]]]
         [DESCRIPTION <project-description-string>]
         [HOMEPAGE_URL <url-string>]
         [LANGUAGES <language-name>...])

Sets the name of the project, and stores it in the variable
:variable:`PROJECT_NAME`.  Also sets the variables

* :variable:`PROJECT_SOURCE_DIR`,
  :variable:`<PROJECT-NAME>_SOURCE_DIR`
* :variable:`PROJECT_BINARY_DIR`,
  :variable:`<PROJECT-NAME>_BINARY_DIR`

Further variables are set by the optional arguments described in the following.
If any of these arguments is not used, then the corresponding variables are
set to the empty string.

If the variable :variable:`CMAKE_PROJECT_<PROJECT-NAME>_INCLUDE` exists,
the file pointed to by that variable will be included as the last step of the
project command.

Options
^^^^^^^

The options are:

``VERSION <version>``
  Optional; may not be used unless policy :policy:`CMP0048` is
  set to ``NEW``.

  Takes a ``<version>`` argument composed of non-negative integer components,
  i.e. ``<major>[.<minor>[.<patch>[.<tweak>]]]``,
  and sets the variables

  * :variable:`PROJECT_VERSION`,
    :variable:`<PROJECT-NAME>_VERSION`
  * :variable:`PROJECT_VERSION_MAJOR`,
    :variable:`<PROJECT-NAME>_VERSION_MAJOR`
  * :variable:`PROJECT_VERSION_MINOR`,
    :variable:`<PROJECT-NAME>_VERSION_MINOR`
  * :variable:`PROJECT_VERSION_PATCH`,
    :variable:`<PROJECT-NAME>_VERSION_PATCH`
  * :variable:`PROJECT_VERSION_TWEAK`,
    :variable:`<PROJECT-NAME>_VERSION_TWEAK`.

``DESCRIPTION <project-description-string>``
  Optional.
  Sets the variables

  * :variable:`PROJECT_DESCRIPTION`, :variable:`<PROJECT-NAME>_DESCRIPTION`

  to ``<project-description-string>``.
  It is recommended that this description is a relatively short string,
  usually no more than a few words.

``HOMEPAGE_URL <url-string>``
  Optional.
  Sets the variables

  * :variable:`PROJECT_HOMEPAGE_URL`, :variable:`<PROJECT-NAME>_HOMEPAGE_URL`

  to ``<url-string>``, which should be the canonical home URL for the project.

``LANGUAGES <language-name>...``
  Optional.
  Can also be specified without ``LANGUAGES`` keyword per the first, short signature.

  Selects which programming languages are needed to build the project.
  Supported languages include ``C``, ``CXX`` (i.e.  C++), ``CUDA``, ``Fortran``, and ``ASM``.
  By default ``C`` and ``CXX`` are enabled if no language options are given.
  Specify language ``NONE``, or use the ``LANGUAGES`` keyword and list no languages,
  to skip enabling any languages.

  If enabling ``ASM``, list it last so that CMake can check whether
  compilers for other languages like ``C`` work for assembly too.

The variables set through the ``VERSION``, ``DESCRIPTION`` and ``HOMEPAGE_URL``
options are intended for use as default values in package metadata and documentation.

Usage
^^^^^

The top-level ``CMakeLists.txt`` file for a project must contain a
literal, direct call to the :command:`project` command; loading one
through the :command:`include` command is not sufficient.  If no such
call exists CMake will implicitly add one to the top that enables the
default languages (``C`` and ``CXX``).  The name of the project set in
the top level ``CMakeLists.txt`` file is available from the
:variable:`CMAKE_PROJECT_NAME` variable, its description from
:variable:`CMAKE_PROJECT_DESCRIPTION`, its homepage URL from
:variable:`CMAKE_PROJECT_HOMEPAGE_URL` and its version from
:variable:`CMAKE_PROJECT_VERSION`.

.. note::
  Call the :command:`cmake_minimum_required` command at the beginning
  of the top-level ``CMakeLists.txt`` file even before calling the
  ``project()`` command.  It is important to establish version and
  policy settings before invoking other commands whose behavior they
  may affect.  See also policy :policy:`CMP0000`.
