fix-bison-flex-command-escaping
-------------------------------

* The :module:`FindBISON` module ``BISON_TARGET`` macro now supports
  special characters by passing the ``VERBATIM`` option to internal
  :command:`add_custom_command` calls.  This may break clients that
  added escaping manually to work around the bug.

* The :module:`FindFLEX` module ``FLEX_TARGET`` macro now supports
  special characters by passing the ``VERBATIM`` option to internal
  :command:`add_custom_command` calls.  This may break clients that
  added escaping manually to work around the bug.
