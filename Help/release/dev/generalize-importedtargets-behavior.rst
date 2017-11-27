generalize-importedtargets-behavior
-----------------------------------

* The :command:`target_compile_definitions` command learned to set the
  :prop_tgt:`INTERFACE_COMPILE_DEFINITIONS` property on
  :ref:`Imported Targets`.

* The :command:`target_compile_features` command learned to set the
  :prop_tgt:`INTERFACE_COMPILE_FEATURES` property on :ref:`Imported Targets`.

* The :command:`target_compile_options` command learned to set the
  :prop_tgt:`INTERFACE_COMPILE_OPTIONS` property on :ref:`Imported Targets`.

* The :command:`target_include_directories` command learned to set the
  :prop_tgt:`INTERFACE_INCLUDE_DIRECTORIES` property on
  :ref:`Imported Targets`.

* The :command:`target_sources` command learned to set the
  :prop_tgt:`INTERFACE_SOURCES` property on :ref:`Imported Targets`.

* The :command:`target_link_libraries` command learned to set the
  :prop_tgt:`INTERFACE_LINK_LIBRARIES` property on :ref:`Imported Targets`.

* :ref:`Alias Targets` may now alias :ref:`Imported Targets` that are
  created with the ``GLOBAL`` option to :command:`add_library`.
