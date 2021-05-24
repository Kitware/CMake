cmake-system-name-version
-------------------------

* :variable:`CMAKE_HOST_SYSTEM_NAME`'s undocumented version-stripping behavior
  has been moved earlier, before :command:`project` or
  :command:`enable_language` is called.
* :variable:`CMAKE_SYSTEM_NAME`'s undocumented version-stripping behavior has
  been removed entirely. If it is set by a ``-D`` flag or by a
  :manual:`toolchain file <cmake-toolchains(7)>`, it is left unaltered, even if
  it still contains a version number.
