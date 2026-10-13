CMAKE_SKIP_INSTALL_PREP_TARGETS
-------------------------------

.. versionadded:: 4.5

Do not generate ``install_prep/<component>`` and ``install_prep/all`` build
targets.

When this variable is set to a true value, CMake does not generate the
preparation targets that build the known prerequisites of a subsequent
component-specific or full installation, even when using a generator
that supports them.  See :ref:`Install Preparation Targets`.  The
preparation targets are also not generated if the project has no install
rules or :variable:`CMAKE_SKIP_INSTALL_RULES` is enabled.

Set this variable in the top-level directory.  It is off by default, in
which case the preparation targets are generated.
