include(common.cmake)

file(DOWNLOAD ${url}_invalid ${file}
  EXPECTED_HASH SHA1=0123456789abcdef0123456789abcdef01234567
  STATUS status
)
