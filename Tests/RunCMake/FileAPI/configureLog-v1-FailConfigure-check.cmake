set(expect
  query
  query/client-bar
  query/client-bar/query.json
  query/client-foo
  query/client-foo/configureLog-v1
  query/configureLog-v1
  reply
  reply/configureLog-v1-[0-9a-f]+.json
  reply/error-[0-9.T-]+.json
  reply/index-[0-9.T-]+.json
  )
check_api("^${expect}$")

check_python(configureLog-v1-FailConfigure error)
check_python(configureLog-v1 index) # Last-good index is intact.
