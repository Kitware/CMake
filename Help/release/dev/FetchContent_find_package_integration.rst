FetchContent_find_package_integration
-------------------------------------

* Integration has been added between the :module:`FetchContent` module and the
  :command:`find_package` command, enabling the following new capabilities:

  * :command:`FetchContent_MakeAvailable` can now try to satisfy a dependency
    by calling :command:`find_package` first.  A new
    :variable:`FETCHCONTENT_TRY_FIND_PACKAGE_MODE` variable controls whether
    this is done by default for all dependencies, is opt-in per dependency,
    or is disabled entirely.

  * :command:`find_package` can be re-routed to call
    :command:`FetchContent_MakeAvailable` instead.  A new read-only
    :variable:`CMAKE_FIND_PACKAGE_REDIRECTS_DIR` variable points to a
    directory where config package files can be located to facilitate these
    re-routed calls.
