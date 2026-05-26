SKIP_UNITY_BUILD_INCLUSION
--------------------------

.. versionadded:: 4.4

Setting this property to true ensures the source files of the file set will be
skipped by unity builds when its associated target has its
:prop_tgt:`UNITY_BUILD` property set to true.  The source files of the file set
will instead be compiled on their own in the same way as it would with unity
builds disabled.

This property helps with "ODR (One definition rule)" problems where combining
a particular source file with others might lead to build errors or other
unintended side effects.

Note that :ref:`file sets <File Sets>`  of type ``HEADERS`` and ``CXX_MODULES``
as well as the sources which are scanned for C++ modules (see
:manual:`cmake-cxxmodules(7)`) are not eligible for unity build inclusion and
will automatically be excluded.

See Also
^^^^^^^^

* :prop_sf:`SKIP_UNITY_BUILD_INCLUSION` source file property
