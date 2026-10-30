.. _`CTest Dashboard Step 8`:

Step 8: Using CMake Presets
===========================

.. versionadded:: 4.4

This step extends :ref:`Step 3 <CTest Dashboard Step 3>` by moving the
coverage settings into a :manual:`preset <cmake-presets(7)>`, instead of
passing them with the ``OPTIONS`` argument of :command:`ctest_configure`.

.. literalinclude:: Step8/CMakePresets.json
  :caption: Step8/CMakePresets.json
  :language: json

.. literalinclude:: Step8/submit.cmake
  :caption: Step8/submit.cmake
  :language: cmake

Setting :variable:`CTEST_PRESET` selects a preset with that name for the
:command:`ctest_configure`, :command:`ctest_build`, and :command:`ctest_test`
commands.  The commands read the variable themselves, so they need no
arguments.  Setting :variable:`CTEST_PRESETS_FILE` reads the presets from
a file other than the ``CMakePresets.json`` file in the source tree.  Here,
the presets file is in the ``Step8`` directory.

Presets keep the build settings in a standard file, instead of in the
dashboard script.  Other scripts, :option:`cmake --preset`, and IDEs can use
the same presets.

Requirements
^^^^^^^^^^^^

* GCC or Clang with ``--coverage``.
* A ``gcov`` tool that matches the compiler version, found in the ``PATH``.

Running the Script
^^^^^^^^^^^^^^^^^^

.. code-block:: shell

  ctest -S Step8/submit.cmake -V
