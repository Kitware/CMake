CMake Experimental Features Guide
*********************************

The following is a guide to CMake experimental features that are
under development and not yet included in official documentation.
See documentation on `CMake Development`_ for more information.

.. _`CMake Development`: README.rst

Features are gated behind ``CMAKE_EXPERIMENTAL_`` variables which must be set
to specific values in order to enable their gated behaviors. Note that the
specific values will change over time to reinforce their experimental nature.
When used, a warning will be generated to indicate that an experimental
feature is in use and that the affected behavior in the project is not part of
CMake's stability guarantees.
