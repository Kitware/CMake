fetchcontent-CMAKE-vars-passthrough.rst
---------------------------------------

* The :module:`FetchContent` module now passes through the
  :variable:`CMAKE_TLS_VERIFY`, :variable:`CMAKE_TLS_CAINFO`,
  :variable:`CMAKE_NETRC` and :variable:`CMAKE_NETRC_FILE` variables (when
  defined) to the underlying :module:`ExternalProject` sub-build.
  Previously, those variables were silently ignored by :module:`FetchContent`.
