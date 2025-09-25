set(expect
  query
  query/client-bar
  query/client-bar/query.json
  query/client-foo
  query/client-foo/toolchains-v1
  query/toolchains-v1
  reply
  reply/error-[0-9.T-]+.json
  reply/index-[0-9.T-]+.json
  reply/toolchains-v1-[0-9a-f]+\\.json
  )
check_api("^${expect}$")

check_python(toolchains-v1-FailConfigure error)
check_python(toolchains-v1 index) # Last-good index is intact.
