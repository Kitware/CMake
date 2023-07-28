CMake
*****

Introduction
============

CMake is a cross-platform, open-source build system generator.
For full documentation visit the `CMake Home Page`_ and the
`CMake Documentation Page`_. The `CMake Community Wiki`_ also
references useful guides and recipes.

.. _`CMake Home Page`: https://cmake.org
.. _`CMake Documentation Page`: https://cmake.org/documentation
.. _`CMake Community Wiki`: https://gitlab.kitware.com/cmake/community/-/wikis/home

CMake is maintained and supported by `Kitware`_ and developed in
collaboration with a productive community of contributors.

.. _`Kitware`: http://www.kitware.com/cmake

License
=======

CMake is distributed under the OSI-approved BSD 3-clause License.
See `Copyright.txt`_ for details.

.. _`Copyright.txt`: Copyright.txt

Building CMake
==============

Supported Platforms
-------------------

* Microsoft Windows
* Apple macOS
* Linux
* FreeBSD
* OpenBSD
* Solaris
* AIX

Other UNIX-like operating systems may work too out of the box, if not
it should not be a major problem to port CMake to this platform.
Please post to the `CMake Discourse Forum`_ to ask if others have
had experience with the platform.

.. _`CMake Discourse Forum`: https://discourse.cmake.org

Building CMake from Scratch
---------------------------

UNIX/Mac OSX/MinGW/MSYS/Cygwin
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You need to have a C++ compiler (supporting C++11) and a ``make`` installed.
Run the ``bootstrap`` script you find in the source directory of CMake.
You can use the ``--help`` option to see the supported options.
You may use the ``--prefix=<install_prefix>`` option to specify a custom
installation directory for CMake.  Once this has finished successfully,
run ``make`` and ``make install``.

For example, if you simply want to build and install CMake from source,
you can build directly in the source tree::

  $ ./bootstrap && make && sudo make install

Or, if you plan to develop CMake or otherwise run the test suite, create
a separate build tree::

  $ mkdir build && cd build
  $ ../bootstrap && make

Windows
^^^^^^^

There are two ways for building CMake under Windows:

1. Compile with MSVC from VS 2015 or later.
   You need to download and install a binary release of CMake.  You can get
   these releases from the `CMake Download Page`_.  Then proceed with the
   instructions below for `Building CMake with CMake`_.

2. Bootstrap with MinGW under MSYS2.
   Download and install `MSYS2`_.  Then install the required build tools::

     $ pacman -S --needed git base-devel mingw-w64-x86_64-gcc

   and bootstrap as above.

.. _`CMake Download Page`: https://cmake.org/download
.. _`MSYS2`: https://www.msys2.org/

Building CMake with CMake
-------------------------

You can build CMake as any other project with a CMake-based build system:
run the installed CMake on the sources of this CMake with your preferred
options and generators. Then build it and install it.
For instructions how to do this, see documentation on `Running CMake`_.

.. _`Running CMake`: https://cmake.org/runningcmake

To build the documentation, install `Sphinx`_ and configure CMake with
``-DSPHINX_HTML=ON`` and/or ``-DSPHINX_MAN=ON`` to enable the "html" or
"man" builder.  Add ``-DSPHINX_EXECUTABLE=/path/to/sphinx-build`` if the
tool is not found automatically.

.. _`Sphinx`: http://sphinx-doc.org

Reporting Bugs
==============

If you have found a bug:

1. If you have a patch, please read the `CONTRIBUTING.rst`_ document.

2. Otherwise, please post to the `CMake Discourse Forum`_ and ask about
   the expected and observed behaviors to determine if it is really
   a bug.

3. Finally, if the issue is not resolved by the above steps, open
   an entry in the `CMake Issue Tracker`_.

.. _`CMake Issue Tracker`: https://gitlab.kitware.com/cmake/cmake/-/issues

Contributing
============

See `CONTRIBUTING.rst`_ for instructions to contribute.

.. _`CONTRIBUTING.rst`: CONTRIBUTING.rst
