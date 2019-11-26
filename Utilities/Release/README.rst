CMake Release Utilities
***********************

This directory contains scripts used to package CMake itself for distribution
on ``cmake.org``.  See also the `CMake Source Code Guide`_.

.. _`CMake Source Code Guide`: ../../Help/dev/source.rst

Docker
------

The ``linux/<arch>/`` directories contain Docker specifications that anyone
may use to produce Linux binaries for CMake:

* ``linux/<arch>/base/Dockerfile``:
  Produces a base image with a build environment for portable CMake binaries.
  This image is published in the `kitware/cmake Docker Hub Repository`_
  with tag ``build-linux-<arch>-base-<date>``.

* ``linux/<arch>/deps/Dockerfile``:
  Produces an image with custom-built dependencies for portable CMake binaries.
  This image is published in the `kitware/cmake Docker Hub Repository`_
  with tag ``build-linux-<arch>-deps-<date>``.

* ``linux/<arch>/Dockerfile``:
  Produce an image containing a portable CMake binary package for Linux.
  Build this image using the CMake source directory as the build context.
  The resulting image will have an ``/out`` directory containing the package.
  For example:

  .. code-block:: console

    $ docker build --tag=cmake:build --network none \
        -f cmake-src/Utilities/Release/linux/$arch/Dockerfile cmake-src
    $ docker container create --name cmake-build cmake:build
    $ docker cp cmake-build:/out .
    $ ls out/cmake-*-Linux-$arch.*

* ``linux/<arch>/test/Dockerfile``:
  Produces a base image with a test environment for packaged CMake binaries.
  For example, build the test base image:

  .. code-block:: console

    $ docker build --tag=cmake:test-base \
        cmake-src/Utilities/Release/linux/$arch/test

  Then create a local ``test/Dockerfile`` to prepare an image with both the
  CMake source tree and the above-built package::

    FROM cmake:test-base
    COPY cmake-src /opt/cmake/src/cmake
    ADD out/cmake-<ver>-Linux-<arch>.tar.gz /opt/
    ENV PATH=/opt/cmake-<ver>-Linux-<arch>/bin:$PATH

  Build the test image and run it to drive testing:

  .. code-block:: console

    $ docker build --tag cmake:test --network none -f test/Dockerfile .
    $ docker run --network none cmake:test bash test-make.bash
    $ docker run --network none cmake:test bash test-ninja.bash

.. _`kitware/cmake Docker Hub Repository`: https://hub.docker.com/r/kitware/cmake

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
