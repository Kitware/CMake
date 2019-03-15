set(expect
  query
  query/cache-v2
  reply
  reply/cache-v2-[0-9a-f]+.json
  reply/index-[0-9.T-]+.json
  )
check_api("^${expect}$")

check_python(cache-v2)
