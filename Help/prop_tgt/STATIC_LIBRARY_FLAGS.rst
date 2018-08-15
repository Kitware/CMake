STATIC_LIBRARY_FLAGS
--------------------

Archiver (or MSVC librarian) flags for a static library target.
Targets that are shared libraries, modules, or executables can use
the :prop_tgt:`LINK_OPTIONS` or :prop_tgt:`LINK_FLAGS` target property.

The STATIC_LIBRARY_FLAGS property, managed as a string, can be used to add
extra flags to the link step of a static library target.
:prop_tgt:`STATIC_LIBRARY_FLAGS_<CONFIG>` will add to the configuration
``<CONFIG>``, for example, ``DEBUG``, ``RELEASE``, ``MINSIZEREL``,
``RELWITHDEBINFO``, ...
