IMPORTED_CXX_MODULES_COMPILE_DEFINITIONS
----------------------------------------

.. versionadded:: 3.28

.. note ::

  Experimental. Gated by ``CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API``

Preprocessor definitions for compiling an ``IMPORTED`` target's C++ module
sources.

CMake will automatically drop some definitions that are not supported
by the native build tool.
