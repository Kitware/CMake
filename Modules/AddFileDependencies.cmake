# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
AddFileDependencies
-------------------

.. deprecated:: 3.20

Add dependencies to a source file.

.. code-block:: cmake

  add_file_dependencies(<source> <files>...)

Adds the given ``<files>`` to the dependencies of file ``<source>``.

Do not use this command in new code.  It is just a wrapper around:

.. code-block:: cmake

  set_property(SOURCE <source> APPEND PROPERTY OBJECT_DEPENDS <files>...)

Instead use the :command:`set_property` command to append to the
:prop_sf:`OBJECT_DEPENDS` source file property directly.

#]=======================================================================]

function(add_file_dependencies _file)

  set_property(SOURCE "${_file}" APPEND PROPERTY OBJECT_DEPENDS "${ARGN}")

endfunction()
