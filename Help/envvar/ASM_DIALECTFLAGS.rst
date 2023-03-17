ASM<DIALECT>FLAGS
-----------------

.. include:: ENV_VAR.txt

Add default compilation flags to be used when compiling a specific dialect
of an assembly language.  ``ASM<DIALECT>FLAGS`` can be one of:

* ``ASMFLAGS``
* ``ASM_NASMFLAGS``
* ``ASM_MASMFLAGS``
* ``ASM_MARMASMFLAGS``
* ``ASM-ATTFLAGS``

.. |CMAKE_LANG_FLAGS| replace:: :variable:`CMAKE_ASM<DIALECT>_FLAGS <CMAKE_<LANG>_FLAGS>`
.. |LANG| replace:: ``ASM<DIALECT>``
.. include:: LANG_FLAGS.txt

See also :variable:`CMAKE_ASM<DIALECT>_FLAGS_INIT <CMAKE_<LANG>_FLAGS_INIT>`.
