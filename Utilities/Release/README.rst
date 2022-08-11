CMake Release Utilities
***********************

This directory contains scripts used to package CMake itself for distribution
on ``cmake.org``.  See also the `CMake Source Code Guide`_.

.. _`CMake Source Code Guide`: ../../Help/dev/source.rst

File Table
----------

The set of package files distributed on ``cmake.org`` varies by CMake version.
Clients providing automatic download functionality may query the set of
package files available using a special file that lists them:

* `File Table v1`_ Documentation

.. _`File Table v1`: files-v1.rst

Docker
------

The ``<os>/<arch>/`` directories contain Docker specifications that anyone
may use to produce binaries for CMake on the following platforms:

* ``linux/x86_64/``: Linux on ``x86_64`` architectures.
* ``linux/aarch64/``: Linux on ``aarch64`` architectures.

Each ``<os>/<arch>/`` directory contains the following:

* ``<os>/<arch>/base/Dockerfile``:
  Produces a base image with a build environment for portable CMake binaries.
  This image is published in the `kitware/cmake Docker Hub Repository`_
  with tag ``build-<os>-<arch>-base-<date>``.

* ``<os>/<arch>/deps/Dockerfile``:
  Produces an image with custom-built dependencies for portable CMake binaries.
  This image is published in the `kitware/cmake Docker Hub Repository`_
  with tag ``build-<os>-<arch>-deps-<date>``.

* ``<os>/<arch>/Dockerfile``:
  Produce an image containing a portable CMake binary package.
  Build this image using the CMake source directory as the build context.
  The resulting image will have an ``/out`` directory
  containing the package.  For example, on Linux ``x86_64``:

  .. code-block:: console

    $ docker build --tag=cmake:build --network none \
        -f cmake-src/Utilities/Release/linux/x86_64/Dockerfile cmake-src
    $ docker container create --name cmake-build cmake:build
    $ docker cp cmake-build:/out .
    $ ls out/cmake-*-linux-x86_64.*

* ``<os>/<arch>/test/Dockerfile``:
  Produces a base image with a test environment for packaged CMake binaries.
  For example, on Linux ``x86_64``, one may build the test base image:

  .. code-block:: console

    $ docker build --tag=cmake:test-base \
        cmake-src/Utilities/Release/linux/x86_64/test

  Then create a local ``test/Dockerfile`` to prepare an image with both the
  CMake source tree and the above-built package::

    FROM cmake:test-base
    COPY cmake-src /opt/cmake/src/cmake
    ADD out/cmake-<ver>-linux-x86_64.tar.gz /opt/
    ENV PATH=/opt/cmake-<ver>-linux-x86_64/bin:$PATH

  Build the test image and run it to drive testing:

  .. code-block:: console

    $ docker build --tag cmake:test --network none -f test/Dockerfile .
    $ docker run --network none cmake:test bash test-make.bash
    $ docker run --network none cmake:test bash test-ninja.bash

.. _`kitware/cmake Docker Hub Repository`: https://hub.docker.com/r/kitware/cmake

macOS
-----

The ``macos/`` directory contains scripts used to produce dependencies
for building CMake binaries on macOS.

Windows
-------

The ``win/`` directory contains scripts used to produce dependencies
for building CMake binaries on Windows.
