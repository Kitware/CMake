set(expect
  query
  reply
  reply/index-[0-9.T-]+.json
  )
check_api("^${expect}$")

check_python(Empty)
