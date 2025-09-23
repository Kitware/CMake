set(expect
  query
  query/cache-v2
  query/client-bar
  query/client-bar/query.json
  query/client-foo
  query/client-foo/cache-v2
  query/client-foo/cmakeFiles-v1
  query/client-foo/codemodel-v2
  query/client-foo/configureLog-v1
  query/client-foo/toolchains-v1
  query/cmakeFiles-v1
  query/codemodel-v2
  query/configureLog-v1
  query/toolchains-v1
  reply
  reply/configureLog-v1-[0-9a-f]+\\.json
  reply/error-[0-9.T-]+\\.json
  )
check_api("^${expect}$")

check_python(FailConfigure error)
