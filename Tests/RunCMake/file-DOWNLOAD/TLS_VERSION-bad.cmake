set(CMAKE_TLS_VERSION bad-var)
file(DOWNLOAD "" TLS_VERIFY 1 STATUS status LOG log)

# The explicit argument overrides the cmake variable.
file(DOWNLOAD "" TLS_VERSION bad-arg TLS_VERIFY 1 STATUS status LOG log)
