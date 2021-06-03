CMAKE_<LANG>_FLAGS_<CONFIG>
---------------------------

Flags for language ``<LANG>`` when building for the ``<CONFIG>`` configuration.

The flags in this variable will be passed to the compiler after those
in the :variable:`CMAKE_<LANG>_FLAGS` variable, but before flags added
by the :command:`add_compile_options` or :command:`target_compile_options`
commands.
