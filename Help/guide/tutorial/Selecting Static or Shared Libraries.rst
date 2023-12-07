Step 10: Selecting Static or Shared Libraries
=============================================

In this section we will show how the :variable:`BUILD_SHARED_LIBS` variable can
be used to control the default behavior of :command:`add_library`,
and allow control over how libraries without an explicit type (``STATIC``,
``SHARED``, ``MODULE`` or ``OBJECT``) are built.

To accomplish this we need to add :variable:`BUILD_SHARED_LIBS` to the
top-level ``CMakeLists.txt``. We use the :command:`option` command as it allows
users to optionally select if the value should be ``ON`` or ``OFF``.

.. literalinclude:: Step11/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-option-BUILD_SHARED_LIBS
  :language: cmake
  :start-after: set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
  :end-before: # configure a header file to pass the version number only

Next, we need to specify output directories for our static and shared
libraries.

.. literalinclude:: Step11/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-cmake-output-directories
  :language: cmake
  :start-after: # we don't need to tinker with the path to run the executable
  :end-before: # configure a header file to pass the version number only

Finally, update ``MathFunctions/MathFunctions.h`` to use dll export defines:

.. literalinclude:: Step11/MathFunctions/MathFunctions.h
  :caption: MathFunctions/MathFunctions.h
  :name: MathFunctions/MathFunctions.h
  :language: c++

At this point, if you build everything, you may notice that linking fails
as we are combining a static library without position independent code with a
library that has position independent code. The solution to this is to
explicitly set the :prop_tgt:`POSITION_INDEPENDENT_CODE` target property of
SqrtLibrary to be ``True`` when building shared libraries.

.. literalinclude:: Step11/MathFunctions/CMakeLists.txt
  :caption: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-POSITION_INDEPENDENT_CODE
  :language: cmake
  :start-at: # state that SqrtLibrary need PIC when the default is shared libraries
  :end-at:  )

Define ``EXPORTING_MYMATH`` stating we are using ``declspec(dllexport)`` when
building on Windows.

.. literalinclude:: Step11/MathFunctions/CMakeLists.txt
  :caption: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-dll-export
  :language: cmake
  :start-at: # define the symbol stating we are using the declspec(dllexport) when
  :end-at: target_compile_definitions(MathFunctions PRIVATE "EXPORTING_MYMATH")

**Exercise**: We modified ``MathFunctions.h`` to use dll export defines.
Using CMake documentation can you find a helper module to simplify this?
