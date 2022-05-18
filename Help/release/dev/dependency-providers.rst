dependency-providers
--------------------

* The :command:`cmake_language` command gained a new
  ``SET_DEPENDENCY_PROVIDER`` sub-command.  When a dependency provider is set,
  calls to :command:`find_package` and :command:`FetchContent_MakeAvailable`
  can be redirected through a custom command, which can choose to fulfill the
  request directly, modify how the request is processed, or leave it to be
  fulfilled by the built-in implementation.  See :ref:`dependency_providers`.
