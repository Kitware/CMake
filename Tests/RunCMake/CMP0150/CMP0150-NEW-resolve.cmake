include(ExternalProject/shared_internal_commands)

function(test_resolve parentUrl relativeUrl expectedResult)
  _ep_resolve_relative_git_remote(result "${parentUrl}" "${relativeUrl}")
  if(NOT result STREQUAL expectedResult)
    message(SEND_ERROR "URL resolved to unexpected result:\n"
      "  Expected: ${expectedResult}\n"
      "  Actual  : ${result}"
    )
  endif()
endfunction()

test_resolve(
  "https://example.com/group/parent"
  "../other"
  "https://example.com/group/other"
)
test_resolve(
  "https://example.com/group/parent"
  "../../alt/other"
  "https://example.com/alt/other"
)

test_resolve(
  "git@example.com:group/parent"
  "../other"
  "git@example.com:group/other"
)
test_resolve(
  "git@example.com:group/parent"
  "../../alt/other"
  "git@example.com:alt/other"
)
test_resolve(
  "git@example.com:/group/parent"
  "../other"
  "git@example.com:/group/other"
)
test_resolve(
  "git@example.com:/group/parent"
  "../../alt/other"
  "git@example.com:/alt/other"
)
test_resolve(
  "git+ssh://git@example.com:group/parent"
  "../other"
  "git+ssh://git@example.com:group/other"
)
test_resolve(
  "ssh://git@example.com:1234/group/parent"
  "../../alt/other"
  "ssh://git@example.com:1234/alt/other"
)

test_resolve(
  "file:///group/parent"
  "../other"
  "file:///group/other"
)
test_resolve(
  "file:///group/parent"
  "../../alt/other"
  "file:///alt/other"
)
test_resolve(
  "file:///~/group/parent"
  "../../other"
  "file:///~/other"
)
test_resolve(
  "/group/parent"
  "../other"
  "/group/other"
)
test_resolve(
  "/group/parent"
  "../../alt/other"
  "/alt/other"
)
test_resolve(
  "C:/group/parent"
  "../other"
  "C:/group/other"
)
test_resolve(
  "C:/group/parent"
  "../../alt/other"
  "C:/alt/other"
)

test_resolve(
  "x-Test+v1.0://example.com/group/parent"
  "../other"
  "x-Test+v1.0://example.com/group/other"
)

# IPv6 literals
test_resolve(
  "http://[::1]/group/parent"
  "../../alt/other"
  "http://[::1]/alt/other"
)
test_resolve(
  "git@[::1]:group/parent"
  "../../alt/other"
  "git@[::1]:alt/other"
)
