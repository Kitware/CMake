CXX_SCAN_FOR_MODULES
--------------------

.. versionadded:: 4.4

``CXX_SCAN_FOR_MODULES`` is a boolean specifying whether CMake will scan the
file set's sources for C++ module dependencies. See also the
:prop_sf:`CXX_SCAN_FOR_MODULES` source file property and the
:prop_tgt:`CXX_SCAN_FOR_MODULES` target property settings.

When this property is set ``ON``, CMake will scan the file set's sources at
build time and add module dependency information to the compile line as
necessary.  When this property is set ``OFF``, CMake will not scan the sources
at build time.  When this property is unset, the
:prop_sf:`CXX_SCAN_FOR_MODULES` source file property and
:prop_tgt:`CXX_SCAN_FOR_MODULES` target property are consulted, in that order.

Note that scanning is only performed if C++20 or higher is enabled for the
target and the source uses the ``CXX`` language.  Scanning for modules in
sources belonging to file sets of type ``CXX_MODULES`` is always performed.
