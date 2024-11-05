CMAKE_TYPE_LINKER_FLAGS-LINKER-prefix-support
---------------------------------------------

* The :variable:`CMAKE_EXE_LINKER_FLAGS`,
  :variable:`CMAKE_EXE_LINKER_FLAGS_<CONFIG>`,
  :variable:`CMAKE_SHARED_LINKER_FLAGS`,
  :variable:`CMAKE_SHARED_LINKER_FLAGS_<CONFIG>`,
  :variable:`CMAKE_MODULE_LINKER_FLAGS`,
  and :variable:`CMAKE_MODULE_LINKER_FLAGS_<CONFIG>` variables learned to
  support the ``LINKER:`` prefix.

  This support implies to parse and re-quote the content of these variables.
  This parsing is controlled by :policy:`CMP0181` policy.
