CMAKE_<LANG>_BYTE_ORDER
-----------------------

.. versionadded:: 3.20

Byte order of ``<LANG>`` compiler target architecture, if known.
If defined and not empty, the value is one of:

``BIG_ENDIAN``
  The target architecture is Big Endian.

``LITTLE_ENDIAN``
  The target architecture is Little Endian.

This is defined for languages ``C``, ``CXX``, ``OBJC``, ``OBJCXX``,
and ``CUDA``.

If :variable:`CMAKE_OSX_ARCHITECTURES` specifies multiple architectures, the
value of ``CMAKE_<LANG>_BYTE_ORDER`` is non-empty only if all architectures
share the same byte order.
