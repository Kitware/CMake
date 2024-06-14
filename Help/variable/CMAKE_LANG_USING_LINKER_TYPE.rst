CMAKE_<LANG>_USING_LINKER_<TYPE>
--------------------------------

.. versionadded:: 3.29

This variable defines how to specify the ``<TYPE>`` linker for the link step,
as controlled by the :variable:`CMAKE_LINKER_TYPE` variable or the
:prop_tgt:`LINKER_TYPE` target property. Depending on the value of the
:variable:`CMAKE_<LANG>_USING_LINKER_MODE` variable,
``CMAKE_<LANG>_USING_LINKER_<TYPE>`` can hold compiler flags for the link step,
or flags to be given directly to the linker tool.

.. note::

  The specified linker tool is generally expected to be accessible through
  the ``PATH`` environment variable.

For example, the ``LLD`` linker for ``GNU`` compilers is defined like so:

.. code-block:: cmake

  set(CMAKE_C_USING_LINKER_LLD "-fuse-ld=lld")

On the ``Windows`` platform with ``Clang`` compilers simulating ``MSVC``:

.. code-block:: cmake

  set(CMAKE_C_USING_LINKER_LLD "-fuse-ld=lld-link")

And for the ``MSVC`` compiler, the linker is invoked directly, not via the
compiler frontend:

.. code-block:: cmake

  set(CMAKE_C_USING_LINKER_LLD "/path/to/lld-link.exe")
  set(CMAKE_C_USING_LINKER_MODE TOOL)

A custom linker type can also be defined, usually in a toolchain file:

.. code-block:: cmake

  set(CMAKE_LINKER_TYPE lld_launcher)
  set(CMAKE_C_USING_LINKER_lld_launcher "-fuse-ld=/path/to/lld-launcher.sh")
  set(CMAKE_C_USING_LINKER_MODE FLAG)
