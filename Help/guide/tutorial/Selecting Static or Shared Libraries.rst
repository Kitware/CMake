Step 9: Selecting Static or Shared Libraries
============================================

In this section we will show how the :variable:`BUILD_SHARED_LIBS` variable can
be used to control the default behavior of :command:`add_library`,
and allow control over how libraries without an explicit type (``STATIC``,
``SHARED``, ``MODULE`` or ``OBJECT``) are built.

To accomplish this we need to add :variable:`BUILD_SHARED_LIBS` to the
top-level ``CMakeLists.txt``. We use the :command:`option` command as it allows
users to optionally select if the value should be ``ON`` or ``OFF``.

Next we are going to refactor ``MathFunctions`` to become a real library that
encapsulates using ``mysqrt`` or ``sqrt``, instead of requiring the calling
code to do this logic. This will also mean that ``USE_MYMATH`` will not control
building ``MathFunctions``, but instead will control the behavior of this
library.

The first step is to update the starting section of the top-level
``CMakeLists.txt`` to look like:

.. literalinclude:: Step10/CMakeLists.txt
  :caption: CMakeLists.txt
  :name: CMakeLists.txt-option-BUILD_SHARED_LIBS
  :language: cmake
  :end-before: # add the binary tree

Now that we have made ``MathFunctions`` always be used, we will need to update
the logic of that library. So, in ``MathFunctions/CMakeLists.txt`` we need to
create a SqrtLibrary that will conditionally be built and installed when
``USE_MYMATH`` is enabled. Now, since this is a tutorial, we are going to
explicitly require that SqrtLibrary is built statically.

The end result is that ``MathFunctions/CMakeLists.txt`` should look like:

.. literalinclude:: Step10/MathFunctions/CMakeLists.txt
  :caption: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-add_library-STATIC
  :language: cmake
  :lines: 1-36,42-

Next, update ``MathFunctions/mysqrt.cxx`` to use the ``mathfunctions`` and
``detail`` namespaces:

.. literalinclude:: Step10/MathFunctions/mysqrt.cxx
  :caption: MathFunctions/mysqrt.cxx
  :name: MathFunctions/mysqrt.cxx-namespace
  :language: c++

We also need to make some changes in ``tutorial.cxx``, so that it no longer
uses ``USE_MYMATH``:

#. Always include ``MathFunctions.h``
#. Always use ``mathfunctions::sqrt``
#. Don't include ``cmath``

Finally, update ``MathFunctions/MathFunctions.h`` to use dll export defines:

.. literalinclude:: Step10/MathFunctions/MathFunctions.h
  :caption: MathFunctions/MathFunctions.h
  :name: MathFunctions/MathFunctions.h
  :language: c++

At this point, if you build everything, you may notice that linking fails
as we are combining a static library without position independent code with a
library that has position independent code. The solution to this is to
explicitly set the :prop_tgt:`POSITION_INDEPENDENT_CODE` target property of
SqrtLibrary to be ``True`` no matter the build type.

.. literalinclude:: Step10/MathFunctions/CMakeLists.txt
  :caption: MathFunctions/CMakeLists.txt
  :name: MathFunctions/CMakeLists.txt-POSITION_INDEPENDENT_CODE
  :language: cmake
  :lines: 37-42

**Exercise**: We modified ``MathFunctions.h`` to use dll export defines.
Using CMake documentation can you find a helper module to simplify this?
