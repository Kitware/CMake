CMAKE_SYCL_KNOWN_FEATURES
-------------------------

.. versionadded:: 4.5

.. note::
   Experimental. Gated by ``CMAKE_EXPERIMENTAL_SYCL``.

List of SYCL features known to this version of CMake.

The features listed in this global property may be known to be available to the
SYCL compiler.  If a feature is available with the SYCL compiler, it will be
listed in the :variable:`CMAKE_SYCL_COMPILE_FEATURES` variable.

The features listed here may be used with the
:command:`target_compile_features` command.  See the
:manual:`cmake-compile-features(7)` manual for information on compile features
and a list of supported compilers.

The features known to this version of CMake are the ``cxx_*`` granular features
listed by :prop_gbl:`CMAKE_CXX_KNOWN_FEATURES`, plus:

``sycl_cxx_std_98``
  C++ compiler mode for SYCL is at least C++98.

``sycl_cxx_std_11``
  C++ compiler mode for SYCL is at least C++11.

``sycl_cxx_std_14``
  C++ compiler mode for SYCL is at least C++14.

``sycl_cxx_std_17``
  C++ compiler mode for SYCL is at least C++17.

``sycl_cxx_std_20``
  C++ compiler mode for SYCL is at least C++20.

``sycl_cxx_std_23``
  C++ compiler mode for SYCL is at least C++23.

``sycl_cxx_std_26``
  C++ compiler mode for SYCL is at least C++26.

.. include:: include/CMAKE_LANG_STD_FLAGS.rst
