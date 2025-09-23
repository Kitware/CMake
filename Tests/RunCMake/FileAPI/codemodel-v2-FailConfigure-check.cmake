set(expect
  query
  query/client-bar
  query/client-bar/query.json
  query/client-foo
  query/client-foo/codemodel-v2
  query/codemodel-v2
  reply
  reply/codemodel-v2-[0-9a-f]+\\.json
  .*
  reply/error-[0-9.T-]+.json
  reply/index-[0-9.T-]+.json
  .*
  )
check_api("^${expect}$")

check_python(codemodel-v2-FailConfigure error)
check_python(codemodel-v2 index) # Last-good index is intact.
