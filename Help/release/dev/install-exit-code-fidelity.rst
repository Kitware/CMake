install-exit-code-fidelity
---------------------------

* The :option:`cmake --install` command now reports a non-zero exit code
  when an install script fails.  Previously, a parallel install (enabled by
  the :prop_gbl:`INSTALL_PARALLEL` global property) always reported success,
  and a serial install could lose the failure of an earlier component that
  exited via :command:`cmake_language(EXIT) <cmake_language>` when a later
  component succeeded.
