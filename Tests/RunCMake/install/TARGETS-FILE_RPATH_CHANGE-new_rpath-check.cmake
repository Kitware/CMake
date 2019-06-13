include(${RunCMake_SOURCE_DIR}/TARGETS-FILE_RPATH_CHANGE-check-common.cmake)
skip_without_rpath_change_rule()
string(APPEND prefix "${wsnl}" [[FILE "[^"]*/]])

set(target "exe1_cmp0095_old")
string(CONCAT regex "${prefix}${target}\"${wssl}"
              [[NEW_RPATH "/foo/bar]])
check()

set(target "exe1_cmp0095_warn")
string(CONCAT regex "${prefix}${target}\"${wssl}"
              [[NEW_RPATH "/foo/bar]])
check()

set(target "exe1_cmp0095_new")
string(CONCAT regex "${prefix}${target}\"${wssl}"
              [[NEW_RPATH "/foo/bar]])
check()

set(target "exe2_cmp0095_old")
string(CONCAT regex "${prefix}${target}\"${wssl}"
              [[NEW_RPATH "\$ORIGIN/../lib]])
check()

set(target "exe2_cmp0095_warn")
string(CONCAT regex "${prefix}${target}\"${wssl}"
              [[NEW_RPATH "\$ORIGIN/../lib]])
check()

set(target "exe2_cmp0095_new")
string(CONCAT regex "${prefix}${target}\"${wssl}"
              [[NEW_RPATH "\\\$ORIGIN/../lib]])
check()

set(target "exe3_cmp0095_old")
string(CONCAT regex "${prefix}${target}\"${wssl}"
              [[NEW_RPATH "\${ORIGIN}/../lib]])
check()

set(target "exe3_cmp0095_warn")
string(CONCAT regex "${prefix}${target}\"${wssl}"
              [[NEW_RPATH "\${ORIGIN}/../lib]])
check()

set(target "exe3_cmp0095_new")
string(CONCAT regex "${prefix}${target}\"${wssl}"
              [[NEW_RPATH "\\\${ORIGIN}/../lib]])
check()

set(target "exe4_cmp0095_old")
string(CONCAT regex "${prefix}${target}\"${wssl}"
              [[NEW_RPATH "/foo/bar/\${PLATFORM}]])
check()

set(target "exe4_cmp0095_warn")
string(CONCAT regex "${prefix}${target}\"${wssl}"
              [[NEW_RPATH "/foo/bar/\${PLATFORM}]])
check()

set(target "exe4_cmp0095_new")
string(CONCAT regex "${prefix}${target}\"${wssl}"
              [[NEW_RPATH "/foo/bar/\\\${PLATFORM}]])
check()
