This is CMake, the cross-platform, open-source make system.
CMake is distributed under the BSD License, see Copyright.txt.
For documentation see the Docs/ directory once you have built CMake
or visit http://www.cmake.org.


Building CMake
==============


Supported Platforms
-------------------

MS Windows, Mac OS X, Linux, FreeBSD, Solaris, HP-UX, IRIX, BeOS, QNX

Other UNIX-like operating systems may work too out of the box, if not
it shouldn't be a major problem to port CMake to this platform. Contact the
CMake mailing list in this case: http://www.cmake.org/mailman/listinfo/cmake


If you don't have any previous version of CMake already installed
--------------------------------------------------------------

* UNIX/Mac OSX/MinGW/MSYS/Cygwin:

You need to have a compiler and a make installed.
Run the bootstrap script you find the in the source directory of CMake.
You can use the --help option to see the supported options.
You may want to use the --prefix=<install_prefix> option to specify a custom
installation directory for CMake. You can run the bootstrap script from
within the CMake source directory or any other build directory of your
choice. Once this has finished successfully, run make and make install.
So basically it's the same as you may be used to from autotools-based
projects:

$ ./bootstrap; make; make install


* Other Windows:

You need to download and install a binary release of CMake in order to build
CMake.  You can get these releases from
http://www.cmake.org/HTML/Download.html .  Then proceed with the instructions
below.


You already have a version of CMake installed
---------------------------------------------

You can build CMake as any other project with a CMake-based build system:
run the installed CMake on the sources of this CMake with your preferred
options and generators. Then build it and install it.
For instructions how to do this, see http://www.cmake.org/HTML/RunningCMake.html
