project
-------

Set a name and enable languages for the entire project.

.. code-block:: cmake

 project(<PROJECT-NAME> [LANGUAGES] [<language-name>...])

Sets the name of the project and stores the name in the
:variable:`PROJECT_NAME` variable.  Additionally this sets variables

* :variable:`PROJECT_SOURCE_DIR`,
  :variable:`<PROJECT-NAME>_SOURCE_DIR`
* :variable:`PROJECT_BINARY_DIR`,
  :variable:`<PROJECT-NAME>_BINARY_DIR`

Optionally you can specify which languages your project supports.
Example languages are ``C``, ``CXX`` (i.e.  C++), ``Fortran``, etc.
By default ``C`` and ``CXX`` are enabled if no language options are
given.  Specify language ``NONE``, or use the ``LANGUAGES`` keyword
and list no languages, to skip enabling any languages.

If a variable exists called :variable:`CMAKE_PROJECT_<PROJECT-NAME>_INCLUDE`,
the file pointed to by that variable will be included as the last step of the
project command.

The top-level ``CMakeLists.txt`` file for a project must contain a
literal, direct call to the :command:`project` command; loading one
through the :command:`include` command is not sufficient.  If no such
call exists CMake will implicitly add one to the top that enables the
default languages (``C`` and ``CXX``).
