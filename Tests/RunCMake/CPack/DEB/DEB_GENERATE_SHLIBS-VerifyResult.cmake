set(shlibs_shlibs "^libtest_lib 0\\.8 generate_shlibs \\(\\= 0\\.1\\.1\\)\n$")
set(shlibs_shlibs_permissions_regex "-rw-r--r-- .*")
verifyDebControl("${FOUND_FILE_1}" "shlibs" "shlibs")
