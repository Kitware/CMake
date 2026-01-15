set(expect
  reply
  reply/codemodel-v2-[0-9a-f]+.json
  .*reply/index-[0-9.T-]+.json.*
)

# Only need to check for existence. Other tests check the reply contents.
check_api("^${expect}$")
