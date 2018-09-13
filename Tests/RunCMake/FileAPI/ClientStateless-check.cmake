set(expect
  query
  query/client-foo
  query/client-foo/__test-v1
  query/client-foo/__test-v2
  query/client-foo/__test-v3
  query/client-foo/unknown
  reply
  reply/__test-v1-[0-9a-f]+.json
  reply/__test-v2-[0-9a-f]+.json
  reply/index-[0-9.T-]+.json
  )
check_api("^${expect}$")

check_python(ClientStateless)
