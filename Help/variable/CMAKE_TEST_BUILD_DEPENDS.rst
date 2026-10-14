CMAKE_TEST_BUILD_DEPENDS
------------------------

.. deprecated:: 4.5

.. versionadded:: 4.4

Enabling this variable previously enabled generation of ``test_prep/<name>``
build targets for tests when using :ref:`Ninja Generators`.  Since CMake 4.5,
these targets are now enabled by default.  Since CMake 4.5, the
:variable:`CMAKE_SKIP_TEST_PREP_TARGETS` may be used to disable generation of
these targets.

See Also
^^^^^^^^

* :ref:`Test Preparation Targets`
