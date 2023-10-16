CMAKE_<LANG>_USING_LINKER_<TYPE>
--------------------------------

.. versionadded:: 3.29

This variable defines how to specify the linker for the link step for the type
as specified by the variable :variable:`CMAKE_LINKER_TYPE` or the target
property :prop_tgt:`LINKER_TYPE`. It can hold compiler flags for the link step
or directly the linker tool. The type of data is given by the variable
:variable:`CMAKE_<LANG>_USING_LINKER_MODE`.

For example, to specify the ``LLVM`` linker for ``GNU`` compilers, we have:

.. code-block:: cmake

  set(CMAKE_C_USING_LINKER_LLD "-fuse-ld=lld")

Or on ``Windows`` platform, for ``Clang`` compilers simulating ``MSVC``, we
have:

.. code-block:: cmake

  set(CMAKE_C_USING_LINKER_LLD "-fuse-ld=lld-link")

And for the ``MSVC`` compiler, linker is directly used, so we have:

.. code-block:: cmake

  set(CMAKE_C_USING_LINKER_LLD "/path/to/lld-link.exe")
  set(CMAKE_C_USING_LINKER_MODE TOOL)
