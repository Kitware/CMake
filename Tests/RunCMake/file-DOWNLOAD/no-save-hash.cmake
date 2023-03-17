include(common.cmake)

# Test downloading without saving to a file.
set(file "")
file_download(EXPECTED_HASH MD5=55555555555555555555555555555555)
