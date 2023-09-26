FetchContent-exclude-from-all
-----------------------------

* The :module:`FetchContent` module's :command:`FetchContent_Declare` command
  gained an ``EXCLUDE_FROM_ALL`` option, which propagates through to the
  :command:`add_subdirectory` call made by
  :command:`FetchContent_MakeAvailable` for the dependency.
