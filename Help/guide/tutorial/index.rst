CMake Tutorial
**************

.. only:: html

   .. contents::

This tutorial provides a step-by-step guide that covers common build
system issues that CMake helps address. Seeing how various topics all
work together in an example project can be very helpful. This tutorial
can be found in the ``Help/guide/tutorial`` directory of the CMake
source code tree. Each topic has its own subdirectory containing code
that may be used as a starting point for that step. The tutorial
examples are progressive so that each step provides the complete
solution for the previous step.

A Basic Starting Point (Step 1)
===============================

The most basic project is an executable built from source code files.
For simple projects, a two line CMakeLists file is all that is required.
This will be the starting point for our tutorial. The CMakeLists file
looks like:

.. literalinclude:: Step1/CMakeLists.txt
  :language: cmake

Note that this example uses lower case commands in the CMakeLists file.
Upper, lower, and mixed case commands are supported by CMake. The source
code for ``tutorial.cxx`` will compute the square root of a number and
the first version of it is very simple, as follows:

.. literalinclude:: Step1/tutorial.cxx
  :language: c++

Adding a Version Number and Configured Header File
--------------------------------------------------

The first feature we will add is to provide our executable and project with a
version number. While we could do this exclusively in the source code, using
CMakeLists provides more flexibility.

To add a version number we modify the CMakeLists file as follows:

.. literalinclude:: Step2/CMakeLists.txt
  :language: cmake
  :start-after: # set the version number
  :end-before: # configure a header file

Since the configured file will be written into the binary tree, we
must add that directory to the list of paths to search for include
files.

.. literalinclude:: Step2/CMakeLists.txt
  :language: cmake
  :start-after: # so that we will find TutorialConfig.h

We then create a ``TutorialConfig.h.in`` file in the source tree with the
following contents:

.. literalinclude:: Step1/TutorialConfig.h.in
  :language: cmake

When CMake configures this header file the values for
``@Tutorial_VERSION_MAJOR@`` and ``@Tutorial_VERSION_MINOR@`` will be
replaced by the values from the CMakeLists file. Next we modify
``tutorial.cxx`` to include the configured header file and to make use of the
version numbers. The updated source code is listed below.

.. literalinclude:: Step2/tutorial.cxx
  :language: c++
  :start-after: // report version
  :end-before: return 1;

The main changes are the inclusion of the ``TutorialConfig.h`` header
file and printing out a version number as part of the usage message.

Specify the C++ Standard
-------------------------

Next let's add some C++11 features to our project. We will need to explicitly
state in the CMake code that it should use the correct flags. The easiest way
to enable C++11 support for CMake is by using the ``CMAKE_CXX_STANDARD``
variable.

First, replace ``atof`` with ``std::stod`` in ``tutorial.cxx``.

Then, set the ``CMAKE_CXX_STANDARD`` variable in the CMakeLists file.

Which variable can we set in the CMakeLists file to treat the
``CMAKE_CXX_STANDARD`` value as a requirement?

Build and Test
--------------

Run **cmake** or **cmake-gui** to configure the project and then build it
with your chosen build tool.

cd to the directory where Tutorial was built (likely the make directory or
a Debug or Release build configuration subdirectory) and run these commands:

.. code-block:: console

  Tutorial 4294967296
  Tutorial 10
  Tutorial

Adding a Library (Step 2)
=========================

Now we will add a library to our project. This library will contain our own
implementation for computing the square root of a number. The executable can
then use this library instead of the standard square root function provided by
the compiler.

For this tutorial we will put the library into a subdirectory
called MathFunctions. It will have the following one line CMakeLists file:

.. literalinclude:: Step2/MathFunctions/CMakeLists.txt
  :language: cmake

The source file ``mysqrt.cxx`` has one function called ``mysqrt`` that
provides similar functionality to the compiler’s ``sqrt`` function. To make use
of the new library we add an ``add_subdirectory`` call in the top-level
CMakeLists file so that the library will get built. We add the new library to
the executable, and add MathFunctions as an include directory so that the
``mqsqrt.h`` header file can be found. The last few lines of the top-level
CMakeLists file now look like:

.. code-block:: cmake

        # add the MathFunctions library
        add_subdirectory(MathFunctions)

        # add the executable
        add_executable(Tutorial tutorial.cxx)

        target_link_libraries(Tutorial MathFunctions)

        # add the binary tree to the search path for include files
        # so that we will find TutorialConfig.h
        target_include_directories(Tutorial PUBLIC
                                  "${PROJECT_BINARY_DIR}"
                                  "${PROJECT_SOURCE_DIR}/MathFunctions"
                                  )

Now let us make the MathFunctions library optional. While for the tutorial
there really isn’t any need to do so, for larger projects this is a common
occurrence. The first step is to add an option to the top-level CMakeLists
file.

.. literalinclude:: Step3/CMakeLists.txt
  :language: cmake
  :start-after: # should we use our own math functions
  :end-before: # set the version number

This will show up in the CMake GUI and ccmake with a default value of ON
that can be changed by the user. This setting will be stored in the cache so
that the user does not need to set the value each time they run CMake on this
build directory.

The next change is to make building and linking the MathFunctions library
conditional. To do this we change the end of the top-level CMakeLists file to
look like the following:

.. literalinclude:: Step3/CMakeLists.txt
  :language: cmake
  :start-after: # add the MathFunctions library?

Note the use of the variables ``EXTRA_LIBS`` and ``EXTRA_INCLUDES`` to collect
up any optional libraries to later be linked into the executable. This is a
classic approach when dealing with many optional components, we will cover the
modern approach in the next step.

The corresponding changes to the source code are fairly straightforward. First,
include the MathFunctions header if we need it:

.. literalinclude:: Step3/tutorial.cxx
  :language: c++
  :start-after: // should we include the MathFunctions header
  :end-before: int main

Then make which square root function is used dependent on ``USE_MYMATH``:

.. literalinclude:: Step3/tutorial.cxx
  :language: c++
  :start-after: // which square root function should we use?
  :end-before: std::cout << "The square root of

Since the source code now requires ``USE_MYMATH`` we can add it to
``TutorialConfig.h.in`` with the following line:

.. literalinclude:: Step3/TutorialConfig.h.in
  :language: c
  :lines: 4

Run **cmake** or **cmake-gui** to configure the project and then build it
with your chosen build tool. Then run the built Tutorial executable.

Which function gives better results, Step1’s sqrt or Step2’s mysqrt?

Adding Usage Requirements for Library (Step 3)
==============================================

Usage requirements allow for far better control over a library or executable's
link and include line while also giving more control over the transitive
property of targets inside CMake. The primary commands that leverage usage
requirements are:

  - ``target_compile_definitions``
  - ``target_compile_options``
  - ``target_include_directories``
  - ``target_link_libraries``

First up is MathFunctions. We first state that anybody linking to MathFunctions
needs to include the current source directory, while MathFunctions itself
doesn't. So this can become an ``INTERFACE`` usage requirement.

Remember ``INTERFACE`` means things that consumers require but the producer
doesn't. Update ``MathFunctions/CMakeLists.txt`` with:

.. literalinclude:: Step4/MathFunctions/CMakeLists.txt
  :language: cmake
  :start-after: # to find MathFunctions.h

Now that we've specified usage requirements for MathFunctions we can safely
remove our uses of the ``EXTRA_INCLUDES`` variable from the top-level
CMakeLists.

Once this is done, run **cmake** or **cmake-gui** to configure the project
and then build it with your chosen build tool.

Installing and Testing (Step 4)
===============================

Now we can start adding install rules and testing support to our project.

Install Rules
-------------

The install rules are fairly simple for MathFunctions we want to install the
library and header file and for the application we want to install the
executable and configured header.

So to ``MathFunctions/CMakeLists.txt`` we add:

.. literalinclude:: Step5/MathFunctions/CMakeLists.txt
  :language: cmake
  :start-after: # install rules

And the to top-level ``CMakeLists.txt`` we add:

.. literalinclude:: Step5/CMakeLists.txt
  :language: cmake
  :start-after: # add the install targets
  :end-before: # enable testing

That is all that is needed to create a basic local install of the tutorial.

Run **cmake** or **cmake-gui** to configure the project and then build it
with your chosen build tool. Build the ``install`` target by typing
``make install`` from the command line or build the ``INSTALL`` target from
an IDE. This will install the appropriate header files, libraries, and
executables.

Verify that the installed Tutorial runs. Note: The CMake variable
``CMAKE_INSTALL_PREFIX`` is used to determine the root of where the files will
be installed.

Testing Support
---------------

Next let's test our application. At the end of the top-level CMakeLists file we
can add a number of basic tests to verify that the application is
working correctly.

.. literalinclude:: Step5/CMakeLists.txt
  :language: cmake
  :start-after: # enable testing

The first test simply verifies that the application runs, does not segfault or
otherwise crash, and has a zero return value. This is the basic form of a CTest
test.

The next test makes use of the ``PASS_REGULAR_EXPRESSION`` test property to
verify that the output of the test contains certain strings, in this case:
verifying that the the usage message is printed when an incorrect number of
arguments are provided.

Lastly, we have a function called ``do_test`` that runs the application and
verifies that the computed square root is correct for given input. For each
invocation of ``do_test``, another test is added to the project with a name,
input, and expected results based on the passed arguments.

Rebuild the application and then cd to the binary directory and run
``ctest -N`` and ``ctest -VV``.

Adding System Introspection (Step 5)
====================================

Let us consider adding some code to our project that depends on features the
target platform may not have. For this example, we will add some code that
depends on whether or not the target platform has the ``log`` and ``exp``
functions. Of course almost every platform has these functions but for this
tutorial assume that they are not common.

If the platform has ``log`` and ``exp`` then we will use them to compute the
square root in the ``mysqrt`` function. We first test for the availability of
these functions using the ``CheckSymbolExists.cmake`` macro in the top-level
CMakeLists file as follows:

.. literalinclude:: Step6/CMakeLists.txt
  :language: cmake
  :start-after: # does this system provide the log and exp functions?
  :end-before: # should we use our own math functions

Now let's add these defines to ``TutorialConfig.h.in`` so that we can use them
from ``mysqrt.cxx``:

.. literalinclude:: Step6/TutorialConfig.h.in
  :language: c
  :start-after: // does the platform provide exp and log functions?

Finally, in the ``mysqrt`` function we can provide an alternate implementation
based on ``log`` and ``exp`` if they are available on the system using the
following code:

.. literalinclude:: Step6/MathFunctions/mysqrt.cxx
  :language: c++
  :start-after: // if we have both log and exp then use them
  :end-before: #else

Run **cmake** or **cmake-gui** to configure the project and then build it
with your chosen build tool.

You will notice that even though ``HAVE_LOG`` and ``HAVE_EXP`` are both
defined ``mysqrt`` isn't using them. We should realize quickly that we have
forgotten to include ``TutorialConfig.h`` in ``mysqrt.cxx``.

After making this update, go ahead and build the project again.

Run the built Tutorial executable. Which function gives better results now,
Step1’s sqrt or Step5’s mysqrt?

**Exercise**: Why is it important that we configure ``TutorialConfig.h.in``
after the checks for ``HAVE_LOG`` and ``HAVE_EXP``? What would happen if we
inverted the two?

**Exercise**: Is there a better place for us to save the ``HAVE_LOG`` and
``HAVE_EXP`` values other than in ``TutorialConfig.h``?

Adding a Custom Command and Generated File (Step 6)
===================================================

In this section, we will add a generated source file into the build process
of an application. For this example, we will create a table of precomputed
square roots as part of the build process, and then compile that
table into our application.

To accomplish this, we first need a program that will generate the table. In
the MathFunctions subdirectory a new source file named ``MakeTable.cxx`` will
do just that.

.. literalinclude:: Step7/MathFunctions/MakeTable.cxx
  :language: c++

Note that the table is produced as valid C++ code and that the output filename
is passed in as an argument.

The next step is to add the appropriate commands to MathFunctions' CMakeLists
file to build the MakeTable executable and then run it as part of the build
process. A few commands are needed to accomplish this.

First, the executable for ``MakeTable`` is added as any other executable would
be added.

.. literalinclude:: Step7/MathFunctions/CMakeLists.txt
  :language: cmake
  :start-after: # first we add the executable that generates the table
  :end-before: # add the command to generate the source code

Then we add a custom command that specifies how to produce ``Table.h``
by running MakeTable.

.. literalinclude:: Step7/MathFunctions/CMakeLists.txt
  :language: cmake
  :start-after: # add the command to generate the source code
  :end-before: # add the main library

Next we have to let CMake know that ``mysqrt.cxx`` depends on the generated
file ``Table.h``. This is done by adding the generated ``Table.h`` to the list
of sources for the library MathFunctions.

.. literalinclude:: Step7/MathFunctions/CMakeLists.txt
  :language: cmake
  :start-after: # add the main library
  :end-before: # state that anybody linking

We also have to add the current binary directory to the list of include
directories so that ``Table.h`` can be found and included by ``mysqrt.cxx``.

.. literalinclude:: Step7/MathFunctions/CMakeLists.txt
  :start-after: # state that we depend on our bin
  :end-before: # install rules

Now let's use the generated table. First, modify ``mysqrt.cxx`` to include
``Table.h``. Next, we can rewrite the mysqrt function to use the table:

.. literalinclude:: Step7/MathFunctions/mysqrt.cxx
  :language: c++
  :start-after: // a hack square root calculation using simple operations

Run **cmake** or **cmake-gui** to configure the project and then build it
with your chosen build tool. When this project is built it will first build
the ``MakeTable`` executable. It will then run ``MakeTable`` to produce
``Table.h``. Finally, it will compile ``mysqrt.cxx`` which includes
``Table.h`` to produce the MathFunctions library.

Building an Installer (Step 7)
==============================

Next suppose that we want to distribute our project to other people so that
they can use it. We want to provide both binary and source distributions on a
variety of platforms. This is a little different from the install we did
previously in `Installing and Testing (Step 4)`_ , where we were
installing the binaries that we had built from the source code. In this
example we will be building installation packages that support binary
installations and package management features. To accomplish this we will use
CPack to create platform specific installers. Specifically we need to add
a few lines to the bottom of our top-level ``CMakeLists.txt`` file.

.. literalinclude:: Step8/CMakeLists.txt
  :language: cmake
  :start-after: # setup installer

That is all there is to it. We start by including
``InstallRequiredSystemLibraries``. This module will include any runtime
libraries that are needed by the project for the current platform. Next we
set some CPack variables to where we have stored the license and version
information for this project. The version information makes use of the
variables we set earlier in this tutorial. Finally we include the CPack
module which will use these variables and some other properties of the system
you are on to setup an installer.

The next step is to build the project in the usual manner and then run
CPack on it. To build a binary distribution you would run:

.. code-block:: console

  cpack

To create a source distribution you would type:

.. code-block:: console

  cpack -C CPackSourceConfig.cmake

Alternatively, run ``make package`` or right click the ``Package`` target and
``Build Project`` from an IDE.

Run the installer executable found in the binary directory. Then run the
installed executable and verify that it works.

Adding Support for a Dashboard (Step 8)
=======================================

Adding support for submitting our test results to a dashboard is very easy. We
already defined a number of tests for our project in the earlier steps of this
tutorial. We just have to run those tests and submit them to a dashboard. To
include support for dashboards we include the CTest module in our top-level
``CMakeLists.txt``.

Replace:

.. code-block:: cmake

  # enable testing
  enable_testing()

With:

.. code-block:: cmake

  # enable dashboard scripting
  include(CTest)

The CTest module will automatically call ``enable_testing()``, so
we can remove it from our CMake files.

We will also need to create a ``CTestConfig.cmake`` file where we can specify
the name of the project and where to submit the dashboard.

.. literalinclude:: Step9/CTestConfig.cmake
  :language: cmake

CTest will read in this file when it runs. To create a simple dashboard you can
run **cmake** or **cmake-gui** to configure the project, but do not build it
yet. Instead, change directory to the binary tree, and then run:

.. code-block:: console

 'ctest [-VV] –D Experimental'

On Windows, build the EXPERIMENTAL target.

Ctest will build and test the project and submit the results to the Kitware
public dashboard. The results of your dashboard will be uploaded to Kitware's
public dashboard here: https://my.cdash.org/index.php?project=CMakeTutorial.

Mixing Static and Shared (Step 9)
=================================

In this section we will show how by using the ``BUILD_SHARED_LIBS`` variable
we can control the default behavior of ``add_library``, and allow control
over how libraries without an explicit type (STATIC/SHARED/MODULE/OBJECT) are
built.

To accomplish this we need to add ``BUILD_SHARED_LIBS`` to the top-level
``CMakeLists.txt``. We use the ``option`` command as it allows users to
optionally select if the value should be On or Off.

Next we are going to refactor MathFunctions to become a real library that
encapsulates using ``mysqrt`` or ``sqrt``, instead of requiring the calling
code to do this logic. This will also mean that ``USE_MYMATH`` will not control
building MathFuctions, but instead will control the behavior of this library.

The first step is to update the starting section of the top-level
``CMakeLists.txt`` to look like:

.. literalinclude:: Step10/CMakeLists.txt
  :language: cmake
  :start-after: set(Tutorial_VERSION_MINOR
  :end-before: # add the binary tree

Now that we have made MathFunctions always be used, we will need to update
the logic of that library. So, in ``MathFunctions/CMakeLists.txt`` we need to
create a SqrtLibrary that will conditionally be built when ``USE_MYMATH`` is
enabled. Now, since this is a tutorial, we are going to explicitly require
that SqrtLibrary is built statically.

The end result is that ``MathFunctions/CMakeLists.txt`` should look like:

.. literalinclude:: Step10/MathFunctions/CMakeLists.txt
  :language: cmake
  :lines: 1-40,46-

Next, update ``MathFunctions/mysqrt.cxx`` to use the ``mathfunctions`` and
``detail`` namespaces:

.. literalinclude:: Step10/MathFunctions/mysqrt.cxx
  :language: c++

We also need to make some changes in ``tutorial.cxx``, so that it no longer
uses ``USE_MYMATH``:

#. Always include ``MathFunctions.h``
#. Always use ``mathfunctions::sqrt``

Finally, update ``MathFunctions/MathFunctions.h`` to use dll export defines:

.. literalinclude:: Step10/MathFunctions/MathFunctions.h
  :language: c++

At this point, if you build everything, you will notice that linking fails
as we are combining a static library without position enabled code with a
library that has position enabled code. The solution to this is to explicitly
set the ``POSITION_INDEPENDENT_CODE`` target property of SqrtLibrary to be
True no matter the build type.

**Exercise**: We modified ``MathFunctions.h`` to use dll export defines.
Using CMake documentation can you find a helper module to simplify this?

Adding Generator Expressions (Step 10)
======================================

Generator expressions are evaluated during build system generation to produce
information specific to each build configuration.

Generator expressions are allowed in the context of many target properties,
such as ``LINK_LIBRARIES``, ``INCLUDE_DIRECTORIES``, ``COMPILE_DEFINITIONS``
and others. They may also be used when using commands to populate those
properties, such as ``target_link_libraries()``,
``target_include_directories()``,
``target_compile_definitions()`` and others.

Generator expressions may be used to enable conditional linking, conditional
definitions used when compiling, conditional include directories and more.
The conditions may be based on the build configuration, target properties,
platform information or any other queryable information.

There are different types of generator expressions including Logical,
Informational, and Output expressions.

Logical expressions are used to create conditional output. The basic
expressions are the 0 and 1 expressions. A ``$<0:...>`` results in the empty
string, and ``<1:...>`` results in the content of "...".  They can also be
nested.

For example:

.. code-block:: cmake

  if(HAVE_LOG AND HAVE_EXP)
    target_compile_definitions(SqrtLibrary
                               PRIVATE "HAVE_LOG" "HAVE_EXP")
  endif()

Can be rewritten with generator expressions:

.. code-block:: cmake

  target_compile_definitions(SqrtLibrary PRIVATE
                             "$<$<BOOL:${HAVE_LOG}>:HAVE_LOG>"
                             "$<$<BOOL:${HAVE_EXP}>:HAVE_EXP>"
                            )

Note that ``${HAVE_LOG}`` is evaluated at CMake configure time while
``$<$<BOOL:${HAVE_LOG}>:HAVE_LOG>`` is evaluated at build system generation
time.

Adding Export Configuration (Step 11)
=====================================

During `Installing and Testing (Step 4)`_ of the tutorial we added the ability
for CMake to install the library and headers of the project. During
`Building an Installer (Step 7)`_ we added the ability to package up this
information so it could be distributed to other people.

The next step is to add the necessary information so that other CMake projects
can use our project, be it from a build directory, a local install or when
packaged.

The first step is to update our ``install(TARGETS)`` commands to not only
specify a ``DESTINATION`` but also an ``EXPORT``. The ``EXPORT`` keyword
generates and installs a CMake file containing code to import all targets
listed in the install command from the installation tree. So let's go ahead
and explicitly ``EXPORT`` the MathFunctions library by updating the
``install`` command in ``MathFunctions/CMakeLists.txt`` to look like:

.. literalinclude:: Complete/MathFunctions/CMakeLists.txt
  :language: cmake
  :start-after: # install rules

Now that we have MathFunctions being exported, we also need to explicitly
install the generated ``MathFunctionsTargets.cmake`` file. This is done by
adding the following to the bottom of the top-level ``CMakeLists.txt``:

.. literalinclude:: Complete/CMakeLists.txt
  :language: cmake
  :start-after: # install the configuration targets
  :end-before: include(CMakePackageConfigHelpers)

At this point you should try and run CMake. If everything is setup properly
you will see that CMake will generate an error that looks like:

.. code-block:: console

  Target "MathFunctions" INTERFACE_INCLUDE_DIRECTORIES property contains
  path:

    "/Users/robert/Documents/CMakeClass/Tutorial/Step11/MathFunctions"

  which is prefixed in the source directory.

What CMake is trying to say is that during generating the export information
it will export a path that is intrinsically tied to the current machine and
will not be valid on other machines. The solution to this is to update the
MathFunctions ``target_include_directories`` to understand that it needs
different ``INTERFACE`` locations when being used from within the build
directory and from an install / package. This means converting the
``target_include_directories`` call for MathFunctions to look like:

.. literalinclude:: Complete/MathFunctions/CMakeLists.txt
  :language: cmake
  :start-after: # to find MathFunctions.h, while we don't.
  :end-before: # should we use our own math functions

Once this has been updated, we can re-run CMake and see verify that it doesn't
warn anymore.

At this point, we have CMake properly packaging the target information that is
required but we will still need to generate a ``MathFunctionsConfig.cmake`` so
that the CMake ``find_package command`` can find our project. So let's go
ahead and add a new file to the top-level of the project called
``Config.cmake.in`` with the following contents:

.. literalinclude:: Complete/Config.cmake.in

Then, to properly configure and install that file, add the following to the
bottom of the top-level CMakeLists:

.. literalinclude:: Complete/CMakeLists.txt
  :language: cmake
  :start-after: # install the configuration targets
  :end-before: # generate the export

At this point, we have generated a relocatable CMake Configuration for our
project that can be used after the project has been installed or packaged. If
we want our project to also be used from a build directory we only have to add
the following to the bottom of the top level CMakeLists:

.. literalinclude:: Complete/CMakeLists.txt
  :language: cmake
  :start-after: # needs to be after the install(TARGETS ) command

With this export call we now generate a ``Targets.cmake``, allowing the
configured ``MathFunctionsConfig.cmake`` in the build directory to be used by
other projects, without needing it to be installed.

Import a CMake Project (Consumer)
=================================

This examples shows how a project can find other CMake packages that
generate ``Config.cmake`` files.

It also shows how to state a project's external dependencies when generating
a ``Config.cmake``.

Packaging Debug and Release (MultiPackage)
==========================================

By default CMake is model is that a build directory only contains a single
configuration, be it Debug, Release, MinSizeRel, or RelWithDebInfo.

But it is possible to setup CPack to bundle multiple build directories at the
same time to build a package that contains multiple configurations of the
same project.

First we need to ahead and construct a directory called ``multi_config`` this
will contain all the builds that we want to package together.

Second create a ``debug`` and ``release`` directory underneath
``multi_config``. At the end you should have a layout that looks like:

─ multi_config
    ├── debug
    └── release

Now we need to setup debug and release builds, which would roughly entail
the following:

.. code-block:: console

  cd debug
  cmake -DCMAKE_BUILD_TYPE=Debug ../../MultiPackage/
  cmake --build .
  cd ../release
  cmake -DCMAKE_BUILD_TYPE=Release ../../MultiPackage/
  cmake --build .
  cd ..


Now that both the debug and release builds are complete we can now use
the custom MultiCPackConfig to package both builds into a single release.

.. code-block:: console

  cpack --config ../../MultiPackage/MultiCPackConfig.cmake
