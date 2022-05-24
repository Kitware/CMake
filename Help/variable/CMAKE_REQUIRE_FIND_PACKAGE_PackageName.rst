CMAKE_REQUIRE_FIND_PACKAGE_<PackageName>
----------------------------------------

.. versionadded:: 3.22

Variable for making :command:`find_package` call ``REQUIRED``.

Every non-``REQUIRED`` :command:`find_package` call in a project can be
turned into ``REQUIRED`` by setting the variable
``CMAKE_REQUIRE_FIND_PACKAGE_<PackageName>`` to ``TRUE``.
This can be used to assert assumptions about build environment and to
ensure the build will fail early if they do not hold.

See also the :variable:`CMAKE_DISABLE_FIND_PACKAGE_<PackageName>` variable.
