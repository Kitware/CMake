include(common.cmake)

file(DOWNLOAD ${url} ${file}
  EXPECTED_HASH SHA1=0123456789abcdef0123456789abcdef01234567
  TIMEOUT 30
  STATUS status
  )
message("status='${status}'")
