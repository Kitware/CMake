include(ExternalProject)
set(ENV{CMAKE_TLS_VERSION} bad-env)
ExternalProject_Add(MyProj GIT_REPOSITORY "fake")
