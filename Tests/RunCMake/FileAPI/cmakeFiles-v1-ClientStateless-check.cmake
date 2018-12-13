set(expect
  query
  query/client-foo
  query/client-foo/cmakeFiles-v1
  reply
  reply/cmakeFiles-v1-[0-9a-f]+.json
  reply/index-[0-9.T-]+.json
  )
check_api("^${expect}$")

check_python(cmakeFiles-v1)
