set(expect
  reply
  reply/cache-v2-[0-9a-f]+.json
  reply/cmakeFiles-v1-[0-9a-f]+.json
  reply/codemodel-v2-[0-9a-f]+.json
  .*reply/index-[0-9.T-]+.json
  .*reply/toolchains-v1-[0-9a-f]+.json
)

# Only need to check for existence. Other tests check the reply contents.
check_api("^${expect}$")
