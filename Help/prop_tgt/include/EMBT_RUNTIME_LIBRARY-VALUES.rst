``Static``
  Compile without additional flags to use the statically-linked runtime
  library.
``Dynamic``
  Compile with ``-tR`` and define ``USEPACKAGES`` to use the
  dynamically-linked (shared) runtime library.

The value is ignored on non-Embarcadero compilers but an unsupported value
will be rejected as an error when using the ``bcc64x`` toolchain.

If this property is not set then no runtime library selection flag is added
explicitly by CMake, which is equivalent to ``Static``.
