message(CHECK_START "Find `libfoo`")
message(CHECK_START "Looking for `libfoo.h`")
message(CHECK_PASS "found [/usr/include]")
message(CHECK_START "Looking for `libfoo.so`")
message(CHECK_PASS "found [/usr/lib/libfoo.so]")
message(CHECK_START "Getting `libfoo` version")
message(CHECK_START "Looking for `libfoo/version.h`")
message(CHECK_PASS "found")
message(CHECK_PASS "1.2.3")
message(CHECK_FAIL "required version 4.5.6 but found 1.2.3")

# Should generate an error, no associated CHECK_START
message(CHECK_FAIL "unmatched check fail case")
