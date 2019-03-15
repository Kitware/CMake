set(expect
  query
  query/client-foo
  reply
  reply/index-[0-9.T-]+.json
  )
check_api("^${expect}$")

check_python(EmptyClient)
