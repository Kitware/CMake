CMake
*****

Daemon Mode
-----------

This branch implements a daemon mode for CMake. It is intended for
collaborators wishing to upstream this into CMake proper.  This
branch is not something for end users.

This will allow advanced IDE features such as

* Semantic highlighting
* Code completion
* Content debugging
* Contextual help

and more. See https://steveire.wordpress.com/2016/01/24/cmake-daemon-for-user-tools/ for details

See a Qt-based client as a KDE Kate plugin here: https://quickgit.kde.org/?p=scratch%2Fskelly%2Fcmakekate.git.

Building
--------

Prerequisites are:

* `libuv`_
* `dtl`_

.. _`libuv`: https://github.com/libuv/libuv
.. _`dtl`: https://github.com/cubicdaiya/dtl

Build with::

    mkdir build
    cd build
    cmake .. "-DLIBUV_INCLUDE_DIR=C:/Program Files/libuv/include"
        "-DLIBUV_LIBRARY=C:/Program Files/libuv/libuv.lib"
        "-DDTL_INCLUDE_DIR=C:/software/dtl"
    cmake --build .

Then run it with::

    cmake -E daemon $PWD

or::

    cmake -E daemon %CD%

See Help/manual/cmake-metadata-daemon.7.rst for more.

Run the unit test with

    ctest -R Server

in the build dir.

License
=======

CMake is distributed under the OSI-approved BSD 3-clause License.
See `Copyright.txt`_ for details.

.. _`Copyright.txt`: Copyright.txt
