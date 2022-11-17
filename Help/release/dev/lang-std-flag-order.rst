lang-std-flag-order
-------------------

* :ref:`Language Standard Flags`, such as ``-std=c++11``, when generated due
  to :command:`target_compile_features` or :variable:`CMAKE_<LANG>_STANDARD`,
  are now placed before flags added by :command:`target_compile_options`,
  rather than after them.
