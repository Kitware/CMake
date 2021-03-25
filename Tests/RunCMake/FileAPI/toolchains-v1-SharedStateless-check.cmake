set(expect
  query
  query/toolchains-v1
  reply
  reply/index-[0-9.T-]+.json
  reply/toolchains-v1-[0-9a-f]+.json
  )
check_api("^${expect}$")

check_python(toolchains-v1)
