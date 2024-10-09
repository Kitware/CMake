include(ExternalProject)
set(ENV{CMAKE_TLS_VERSION} bad-env)
set(CMAKE_TLS_VERSION bad-var)
ExternalProject_Add(MyProj GIT_REPOSITORY "fake")
