# The environment variable provides a default.
set(ENV{CMAKE_TLS_VERSION} bad-env)
file(DOWNLOAD "" TLS_VERIFY 1 STATUS status LOG log)

# The cmake variable overrides the environment variable.
set(CMAKE_TLS_VERSION bad-var)
file(DOWNLOAD "" TLS_VERIFY 1 STATUS status LOG log)

# The explicit argument overrides the cmake variable.
file(DOWNLOAD "" TLS_VERSION bad-arg TLS_VERIFY 1 STATUS status LOG log)
