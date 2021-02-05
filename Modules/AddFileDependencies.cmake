# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
AddFileDependencies
-------------------

Add dependencies to a source file.

.. code-block:: cmake

  add_file_dependencies(<source> <files>)

Adds the given ``<files>`` to the dependencies of file ``<source>``.
#]=======================================================================]

function(add_file_dependencies _file)

  set_property(SOURCE "${_file}" APPEND PROPERTY OBJECT_DEPENDS "${ARGN}")

endfunction()
