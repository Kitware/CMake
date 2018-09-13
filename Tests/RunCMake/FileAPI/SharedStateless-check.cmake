set(expect
  query
  query/__test-v1
  query/__test-v2
  query/__test-v3
  query/query.json
  query/unknown
  reply
  reply/__test-v1-[0-9a-f]+.json
  reply/__test-v2-[0-9a-f]+.json
  reply/index-[0-9.T-]+.json
  )
check_api("^${expect}$")

check_python(SharedStateless)
