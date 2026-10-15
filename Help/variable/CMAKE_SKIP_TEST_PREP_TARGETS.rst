CMAKE_SKIP_TEST_PREP_TARGETS
----------------------------

.. versionadded:: 4.5

Do not generate ``test_prep/<name>`` and ``test_prep/all`` build targets.

When this variable is set to a true value, CMake does not generate the
preparation targets that build the known prerequisites of tests.  See
:ref:`Test Preparation Targets`.

Set this variable in the top-level directory.  It is off by default, in
which case the preparation targets are generated.
