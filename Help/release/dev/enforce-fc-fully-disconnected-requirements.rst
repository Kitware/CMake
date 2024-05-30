enforce-fc-fully-disconnected-requirements
------------------------------------------

* When :variable:`FETCHCONTENT_FULLY_DISCONNECTED` is set to true,
  :command:`FetchContent_MakeAvailable` and the single-argument form of
  :command:`FetchContent_Populate` require that the dependency's source
  directory has already been populated. CMake 3.29 and earlier did not
  check this requirement, but it is now enforced, subject to policy
  :policy:`CMP0170`.
