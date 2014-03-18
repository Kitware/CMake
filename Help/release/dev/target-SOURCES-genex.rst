target-SOURCES-genex
--------------------

* The :prop_tgt:`SOURCES` target property now contains
  :manual:`generator expression <cmake-generator-expressions(7)>`
  such as ``TARGET_OBJECTS`` when read at configure time, if
  policy :policy:`CMP0051` is ``NEW``.

* The :prop_tgt:`SOURCES` target property now generally supports
  :manual:`generator expression <cmake-generator-expressions(7)>`.  The
  generator expressions may be used in the :command:`add_library` and
  :command:`add_executable` commands.
