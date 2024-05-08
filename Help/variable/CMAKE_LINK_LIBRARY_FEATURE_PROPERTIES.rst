CMAKE_LINK_LIBRARY_<FEATURE>_PROPERTIES
---------------------------------------

.. versionadded:: 3.30

This variable defines the semantics of the specified ``<FEATURE>`` (as
described by the :variable:`CMAKE_<LANG>_LINK_LIBRARY_USING_<FEATURE>` or
:variable:`CMAKE_LINK_LIBRARY_USING_<FEATURE>` variables) used for the link
command generation.

This variable will be considered only if the
 :variable:`CMAKE_<LANG>_LINK_LIBRARY_<FEATURE>_PROPERTIES` is not defined.

.. include:: CMAKE_LINK_LIBRARY_FEATURE_PROPERTIES.txt
