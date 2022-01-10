Step 5: Adding System Introspection
===================================

Let us consider adding some code to our project that depends on features the
target platform may not have. For this example, we will add some code that
depends on whether or not the target platform has the ``log`` and ``exp``
functions. Of course almost every platform has these functions but for this
tutorial assume that they are not common.

If the platform has ``log`` and ``exp`` then we will use them to compute the
square root in the ``mysqrt`` function. We first test for the availability of
these functions using the :module:`CheckSymbolExists` module in
``MathFunctions/CMakeLists.txt``. On some platforms, we will need to link to
the ``m`` library. If ``log`` and ``exp`` are not initially found, require the
``m`` library and try again.

Add the checks for ``log`` and ``exp`` to ``MathFunctions/CMakeLists.txt``,
after the call to :command:`target_include_directories`:

.. literalinclude:: Step6/MathFunctions/CMakeLists.txt
  :caption: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-check_symbol_exists
  :language: cmake
  :start-after: # to find MathFunctions.h, while we don't.
  :end-before: # add compile definitions

If available, use :command:`target_compile_definitions` to specify
``HAVE_LOG`` and ``HAVE_EXP`` as ``PRIVATE`` compile definitions.

.. literalinclude:: Step6/MathFunctions/CMakeLists.txt
  :caption: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-target_compile_definitions
  :language: cmake
  :start-after: # add compile definitions
  :end-before: # install rules

If ``log`` and ``exp`` are available on the system, then we will use them to
compute the square root in the ``mysqrt`` function. Add the following code to
the ``mysqrt`` function in ``MathFunctions/mysqrt.cxx`` (don't forget the
``#endif`` before returning the result!):

.. literalinclude:: Step6/MathFunctions/mysqrt.cxx
  :caption: MathFunctions/mysqrt.cxx
  :name: MathFunctions/mysqrt.cxx-ifdef
  :language: c++
  :start-after: // if we have both log and exp then use them
  :end-before: // do ten iterations

We will also need to modify ``mysqrt.cxx`` to include ``cmath``.

.. literalinclude:: Step6/MathFunctions/mysqrt.cxx
  :caption: MathFunctions/mysqrt.cxx
  :name: MathFunctions/mysqrt.cxx-include-cmath
  :language: c++
  :end-before: #include <iostream>

Run the :manual:`cmake  <cmake(1)>` executable or the
:manual:`cmake-gui <cmake-gui(1)>` to configure the project and then build it
with your chosen build tool and run the Tutorial executable.

Which function gives better results now, ``sqrt`` or ``mysqrt``?
