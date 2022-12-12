set(expect
  query
  query/configureLog-v1
  reply
  reply/configureLog-v1-[0-9a-f]+.json
  reply/index-[0-9.T-]+.json
  )
check_api("^${expect}$")

check_python(configureLog-v1)
