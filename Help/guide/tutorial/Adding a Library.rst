Step 2: Adding a Library
========================

At this point, we have seen how to create a basic project using CMake. In this
step, we will learn how to create and use a library in our project. We will
also see how to make the use of our library optional.

Exercise 1 - Creating a Library
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To add a library in CMake, use the :command:`add_library` command and specify
which source files should make up the library.

Rather than placing all of the source files in one directory, we can organize
our project with one or more subdirectories. In this case, we will create a
subdirectory specifically for our library. Here, we can add a new
``CMakeLists.txt`` file and one or more source files. In the top level
``CMakeLists.txt`` file, we will use the :command:`add_subdirectory` command
to add the subdirectory to the build.

Once the library is created, it is connected to our executable target with
:command:`target_include_directories` and :command:`target_link_libraries`.

Goal
----

Add and use a library.

Helpful Resources
-----------------

* :command:`add_library`
* :command:`add_subdirectory`
* :command:`target_include_directories`
* :command:`target_link_libraries`
* :variable:`PROJECT_SOURCE_DIR`

Files to Edit
-------------

* ``CMakeLists.txt``
* ``tutorial.cxx``
* ``MathFunctions/CMakeLists.txt``

Getting Started
---------------

In this exercise, we will add a library to our project that contains our own
implementation for computing the square root of a number. The executable can
then use this library instead of the standard square root function provided by
the compiler.

For this tutorial we will put the library into a subdirectory called
``MathFunctions``. This directory already contains a header file,
``MathFunctions.h``, and a source file ``mysqrt.cxx``. We will not need to
modify either of these files. The source file has one function called
``mysqrt`` that provides similar functionality to the compiler's ``sqrt``
function.

From the ``Help/guide/tutorial/Step2`` directory, start with ``TODO 1`` and
complete through ``TODO 6``.

First, fill in the one line ``CMakeLists.txt`` in the ``MathFunctions``
subdirectory.

Next, edit the top level ``CMakeLists.txt``.

Finally, use the newly created ``MathFunctions`` library in ``tutorial.cxx``

Build and Run
-------------

Run the :manual:`cmake  <cmake(1)>` executable or the
:manual:`cmake-gui <cmake-gui(1)>` to configure the project and then build it
with your chosen build tool.

Below is a refresher of what that looks like from the command line:

.. code-block:: console

  mkdir Step2_build
  cd Step2_build
  cmake ../Step2
  cmake --build .

Try to use the newly built ``Tutorial`` and ensure that it is still
producing accurate square root values.

Solution
--------

In the ``CMakeLists.txt`` file in the ``MathFunctions`` directory, we create
a library target called ``MathFunctions`` with :command:`add_library`. The
source file for the library is passed as an argument to
:command:`add_library`. This looks like the following line:

.. raw:: html

  <details><summary>TODO 1: Click to show/hide answer</summary>

.. literalinclude:: Step3/MathFunctions/CMakeLists.txt
  :caption: TODO 1: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-add_library
  :language: cmake
  :end-before: # TODO 1

.. raw:: html

  </details>

To make use of the new library we will add an :command:`add_subdirectory`
call in the top-level ``CMakeLists.txt`` file so that the library will get
built.

.. raw:: html

  <details><summary>TODO 2: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 2: CMakeLists.txt
  :name: CMakeLists.txt-add_subdirectory

  add_subdirectory(MathFunctions)

.. raw:: html

  </details>

Next, the new library target is linked to the executable target using
:command:`target_link_libraries`.

.. raw:: html

  <details><summary>TODO 3: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 3: CMakeLists.txt
  :name: CMakeLists.txt-target_link_libraries

  target_link_libraries(Tutorial PUBLIC MathFunctions)

.. raw:: html

  </details>

Finally we need to specify the library's header file location. Modify
:command:`target_include_directories` to add the ``MathFunctions`` subdirectory
as an include directory so that the ``MathFunctions.h`` header file can be
found.

.. raw:: html

  <details><summary>TODO 4: Click to show/hide answer</summary>

.. code-block:: cmake
  :caption: TODO 4: CMakeLists.txt
  :name: CMakeLists.txt-target_include_directories-step2

  target_include_directories(Tutorial PUBLIC
                            "${PROJECT_BINARY_DIR}"
                            "${PROJECT_SOURCE_DIR}/MathFunctions"
                            )

.. raw:: html

  </details>

Now let's use our library. In ``tutorial.cxx``, include ``MathFunctions.h``:

.. raw:: html

  <details><summary>TODO 5: Click to show/hide answer</summary>

.. code-block:: c++
  :caption: TODO 5 : tutorial.cxx
  :name: tutorial.cxx-include_MathFunctions.h

  #include "MathFunctions.h"

.. raw:: html

  </details>

Lastly, replace ``sqrt`` with our library function ``mysqrt``.

.. raw:: html

  <details><summary>TODO 6: Click to show/hide answer</summary>

.. code-block:: c++
  :caption: TODO 6 : tutorial.cxx
  :name: tutorial.cxx-call_mysqrt

  const double outputValue = mysqrt(inputValue);

.. raw:: html

  </details>

Exercise 2 - Making Our Library Optional
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Now let us make the MathFunctions library optional. While for the tutorial
there really isn't any need to do so, for larger projects this is a common
occurrence.

CMake can do this using the :command:`option` command. This gives users a
variable which they can change when configuring their cmake build. This
setting will be stored in the cache so that the user does not need to set
the value each time they run CMake on a build directory.

Goal
----

Add the option to build without ``MathFunctions``.


Helpful Resources
-----------------

* :command:`if`
* :command:`list`
* :command:`option`
* :command:`cmakedefine <configure_file>`

Files to Edit
-------------

* ``CMakeLists.txt``
* ``tutorial.cxx``
* ``TutorialConfig.h.in``

Getting Started
---------------

Start with the resulting files from Exercise 1. Complete ``TODO 7`` through
``TODO 13``.

First create a variable ``USE_MYMATH`` using the :command:`option` command
in the top-level ``CMakeLists.txt`` file. In that same file, use that option
to determine whether to build and use the ``MathFunctions`` library.

Then, update ``tutorial.cxx`` and ``TutorialConfig.h.in`` to use
``USE_MYMATH``.

Build and Run
-------------

Since we have our build directory already configured from Exercise 1, we can
rebuild by simply calling the following:

.. code-block:: console

  cd ../Step2_build
  cmake --build .

Next, run the ``Tutorial`` executable on a few numbers to verify that it's
still correct.

Now let's update the value of ``USE_MYMATH`` to ``OFF``. The easiest way is to
use the :manual:`cmake-gui <cmake-gui(1)>` or  :manual:`ccmake <ccmake(1)>`
if you're in the terminal. Or, alternatively, if you want to change the
option from the command-line, try:

.. code-block:: console

  cmake ../Step2 -DUSE_MYMATH=OFF

Now, rebuild the code with the following:

.. code-block:: console

  cmake --build .

Then, run the executable again to ensure that it still works with
``USE_MYMATH`` set to ``OFF``. Which function gives better results, ``sqrt``
or ``mysqrt``?

Solution
--------

The first step is to add an option to the top-level ``CMakeLists.txt`` file.
This option will be displayed in the :manual:`cmake-gui <cmake-gui(1)>` and
:manual:`ccmake <ccmake(1)>` with a default value of ``ON`` that can be
changed by the user.

.. raw:: html

  <details><summary>TODO 7: Click to show/hide answer</summary>

.. literalinclude:: Step3/CMakeLists.txt
  :caption: TODO 7: CMakeLists.txt
  :name: CMakeLists.txt-option
  :language: cmake
  :start-after: # should we use our own math functions
  :end-before: # configure a header file to pass some of the CMake settings

.. raw:: html

  </details>

Next, make building and linking the ``MathFunctions`` library
conditional.

Start by creating a :command:`list` of the optional library targets for our
project. At the moment, it is just ``MathFunctions``. Let's name our list
``EXTRA_LIBS``.

Similarly, we need to make a :command:`list` for the optional includes which
we will call ``EXTRA_INCLUDES``. In this list, we will ``APPEND`` the path of
the header file needed for our library.

Next, create an :command:`if` statement which checks the value of
``USE_MYMATH``. Inside the :command:`if` block, put the
:command:`add_subdirectory` command from Exercise 1 with the additional
:command:`list` commands.

When ``USE_MYMATH`` is ``ON``, the lists will be generated and will be added to
our project. When ``USE_MYMATH`` is ``OFF``, the lists stay empty. With this
strategy, we allow users to toggle ``USE_MYMATH`` to manipulate what library is
used in the build.

The top-level CMakeLists.txt file will now look like the following:

.. raw:: html

  <details><summary>TODO 8: Click to show/hide answer</summary>

.. literalinclude:: Step3/CMakeLists.txt
  :caption: TODO 8: CMakeLists.txt
  :name: CMakeLists.txt-USE_MYMATH
  :language: cmake
  :start-after: # add the MathFunctions library
  :end-before: # add the executable

.. raw:: html

  </details>

Now that we have these two lists, we need to update
:command:`target_link_libraries` and :command:`target_include_directories` to
use them. Changing them is fairly straightforward.

For :command:`target_link_libraries`, we replace the written out
library names with ``EXTRA_LIBS``. This looks like the following:

.. raw:: html

  <details><summary>TODO 9: Click to show/hide answer</summary>

.. literalinclude:: Step3/CMakeLists.txt
  :caption: TODO 9: CMakeLists.txt
  :name: CMakeLists.txt-target_link_libraries-EXTRA_LIBS
  :language: cmake
  :start-after: add_executable(Tutorial tutorial.cxx)
  :end-before: # TODO 3

.. raw:: html

  </details>

Then, we do the same thing with :command:`target_include_directories` and
``EXTRA_INCLUDES``.

.. raw:: html

  <details><summary>TODO 10: Click to show/hide answer</summary>

.. literalinclude:: Step3/CMakeLists.txt
  :caption: TODO 10 : CMakeLists.txt
  :name: CMakeLists.txt-target_link_libraries-EXTRA_INCLUDES
  :language: cmake
  :start-after: # so that we will find TutorialConfig.h

.. raw:: html

  </details>

Note that this is a classic approach when dealing with many components. We
will cover the modern approach in the Step 3 of the tutorial.

The corresponding changes to the source code are fairly straightforward.
First, in ``tutorial.cxx``, we include the ``MathFunctions.h`` header if
``USE_MYMATH`` is defined.

.. raw:: html

  <details><summary>TODO 11: Click to show/hide answer</summary>

.. literalinclude:: Step3/tutorial.cxx
  :caption: TODO 11 : tutorial.cxx
  :name: tutorial.cxx-ifdef-include
  :language: c++
  :start-after: // should we include the MathFunctions header
  :end-before: int main

.. raw:: html

  </details>

Then, in the same file, we make ``USE_MYMATH`` control which square root
function is used:

.. raw:: html

  <details><summary>TODO 12: Click to show/hide answer</summary>

.. literalinclude:: Step3/tutorial.cxx
  :caption: TODO 12 : tutorial.cxx
  :name: tutorial.cxx-ifdef-const
  :language: c++
  :start-after: // which square root function should we use?
  :end-before: std::cout << "The square root of

.. raw:: html

  </details>

Since the source code now requires ``USE_MYMATH`` we can add it to
``TutorialConfig.h.in`` with the following line:

.. raw:: html

  <details><summary>TODO 13: Click to show/hide answer</summary>

.. literalinclude:: Step3/TutorialConfig.h.in
  :caption: TODO 13 : TutorialConfig.h.in
  :name: TutorialConfig.h.in-cmakedefine
  :language: c++
  :lines: 4

.. raw:: html

  </details>

With these changes, our library is now completely optional to whoever is
building and using it.

Bonus Question
--------------

Why is it important that we configure ``TutorialConfig.h.in``
after the option for ``USE_MYMATH``? What would happen if we inverted the two?

Answer
------

.. raw:: html

  <details><summary>Click to show/hide answer</summary>

We configure after because ``TutorialConfig.h.in`` uses the value of
``USE_MYMATH``. If we configure the file before
calling :command:`option`, we won't be using the expected value of
``USE_MYMATH``.

.. raw:: html

  </details>
