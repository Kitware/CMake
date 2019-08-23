CMake Release Utilities
***********************

This directory contains scripts used to package CMake itself for distribution
on ``cmake.org``.  See also the `CMake Source Code Guide`_.

.. _`CMake Source Code Guide`: ../../Help/dev/source.rst

Scripts for Kitware
-------------------

Kitware uses the following scripts to produce binaries for ``cmake.org``.
They work only on specific machines Kitware uses for such builds.

* ``create-cmake-release.cmake``:
  Run ``cmake -DCMAKE_CREATE_VERSION=$ver -P ../create-cmake-release.cmake``
  to generate ``create-$ver-*.sh`` release scripts.  It also displays
  instructions to run them.

* ``*_release.cmake``:
  Platform-specific settings used in corresponding scripts generated above.

* ``release_cmake.cmake``:
  Code shared by all ``*_release.cmake`` scripts.

* ``release_cmake.sh.in``:
  Template for script that runs on the actual build machines.
