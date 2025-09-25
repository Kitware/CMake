set(expect
  query
  query/cache-v2
  query/client-bar
  query/client-bar/query.json
  query/client-foo
  query/client-foo/cache-v2
  reply
  reply/cache-v2-[0-9a-f]+\\.json
  reply/error-[0-9.T-]+.json
  reply/index-[0-9.T-]+.json
  )
check_api("^${expect}$")

check_python(cache-v2-FailConfigure error)
check_python(cache-v2 index) # Last-good index is intact.
