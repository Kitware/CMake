Step 7: Packaging an Installer
==============================

Next suppose that we want to distribute our project to other people so that
they can use it. We want to provide both binary and source distributions on a
variety of platforms. This is a little different from the install we did
previously in :guide:`tutorial/Installing and Testing`, where we were
installing the binaries that we had built from the source code. In this
example we will be building installation packages that support binary
installations and package management features. To accomplish this we will use
CPack to create platform specific installers. Specifically we need to add a
few lines to the bottom of our top-level ``CMakeLists.txt`` file.

.. literalinclude:: Step8/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-include-CPack
  :language: cmake
  :start-after: # setup installer

That is all there is to it. We start by including
:module:`InstallRequiredSystemLibraries`. This module will include any runtime
libraries that are needed by the project for the current platform. Next we set
some CPack variables to where we have stored the license and version
information for this project. The version information was set earlier in this
tutorial and the ``license.txt`` has been included in the top-level source
directory for this step.

Finally we include the :module:`CPack module <CPack>` which will use these
variables and some other properties of the current system to setup an
installer.

The next step is to build the project in the usual manner and then run the
:manual:`cpack <cpack(1)>` executable. To build a binary distribution, from the
binary directory run:

.. code-block:: console

  cpack

To specify the generator, use the ``-G`` option. For multi-config builds, use
``-C`` to specify the configuration. For example:

.. code-block:: console

  cpack -G ZIP -C Debug

To create a source distribution you would type:

.. code-block:: console

  cpack --config CPackSourceConfig.cmake

Alternatively, run ``make package`` or right click the ``Package`` target and
``Build Project`` from an IDE.

Run the installer found in the binary directory. Then run the installed
executable and verify that it works.
