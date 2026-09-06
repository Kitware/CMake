cmake_policy(SET CMP0222 NEW)

# The operator is an if() spelling of cmake_path(IS_PREFIX), so assert that
# the two agree rather than repeating a table of expected values here.  What
# each input should evaluate to is pinned by the cmake_path(IS_PREFIX) test;
# this asserts only that the two surfaces cannot drift apart.  Neither
# normalizes, so the comparison is against the plain form of the command.
#
# Because nothing below states an expected value, the host path model needs
# no branching: parity has to hold on every platform whatever the answer is.
function(assert_parity prefix path)
  set(prefix_var "${prefix}")
  cmake_path(IS_PREFIX prefix_var "${path}" expected)
  if(expected)
    set(expected TRUE)
  else()
    set(expected FALSE)
  endif()
  if(prefix PATH_IS_PREFIX path)
    set(actual TRUE)
  else()
    set(actual FALSE)
  endif()
  if(NOT actual STREQUAL expected)
    message(SEND_ERROR
      "if('${prefix}' PATH_IS_PREFIX '${path}') is ${actual}, but "
      "cmake_path(IS_PREFIX) says ${expected}")
  endif()
endfunction()

# Each pair appears in both roles, so a reversed call cannot pass.
assert_parity("/a/b" "/a/b/c")
assert_parity("/a/b/c" "/a/b")
assert_parity("/a/b" "/a/b")

# Trailing separators, which are significant and asymmetric.
assert_parity("/a/b" "/a/b/")
assert_parity("/a/b/" "/a/b")
assert_parity("/a/b/" "/a/b/c")

# Without NORMALIZE, '.' and '..' are ordinary components.
assert_parity("/a/b" "/a/b/../../etc")
assert_parity("/a/b" "/a/./b//c")
assert_parity("." "./a/b")
assert_parity("." "a/b")

# Duplicate separators are not components.
assert_parity("a/b" "a///b")

# Sibling sharing a textual prefix.
assert_parity("/a/b" "/a/bc")

# Relative and absolute paths are not reconciled.
assert_parity("/a" "b/c")
assert_parity("b" "b/c")
assert_parity("/" "b/c")
assert_parity("/" "/a/b")

# Empty operands in each position.
assert_parity("" "/a/b")
assert_parity("/a" "")
assert_parity("" "")

# Host path model: backslashes, case, drive-relative paths, UNC root-names.
assert_parity("C:\\a" "C:\\a\\b")
assert_parity("c:/a" "C:/A/b")
assert_parity("C:/" "C:foo")
assert_parity("//host/share" "//host/share/a")
assert_parity("/a" "//a/b")

# Literal operands, not just variables.
if(NOT "/a/b" PATH_IS_PREFIX "/a/b/c")
  message(SEND_ERROR "if(PATH_IS_PREFIX): literal operands rejected")
endif()

# NOT composes with the operator, which is the motivating guard idiom.
set(guard_ok FALSE)
if(NOT "/a/b" PATH_IS_PREFIX "/a/bc")
  set(guard_ok TRUE)
endif()
if(NOT guard_ok)
  message(SEND_ERROR "if(NOT <prefix> PATH_IS_PREFIX <path>) did not compose")
endif()

# Works in an elseif() chain, which a helper function could not.
if("/a" PATH_IS_PREFIX "/x/y")
  message(SEND_ERROR "if(PATH_IS_PREFIX): '/a' matched '/x/y'")
elseif("/a/b" PATH_IS_PREFIX "/a/b/c")
else()
  message(SEND_ERROR "elseif(<prefix> PATH_IS_PREFIX <path>) did not match")
endif()

# The motivating case: a prefix that if(MATCHES) cannot express.
set(prefix "/proj/lib+ssl(v2)/inc.d")
if(NOT "${prefix}" PATH_IS_PREFIX "${prefix}/f.h")
  message(SEND_ERROR "if(PATH_IS_PREFIX): regex metacharacters in prefix")
endif()

# The keyword is consumed as an operator only with both a preceding and a
# following token, so it stays usable as a variable otherwise.  Neither
# condition may be written with a leading NOT: that would put the keyword in
# the operator slot and make these a hard error.
set(PATH_IS_PREFIX "yes")
if(PATH_IS_PREFIX)
else()
  message(SEND_ERROR "if(PATH_IS_PREFIX): unary use broken")
endif()
if(PATH_IS_PREFIX STREQUAL "yes")
else()
  message(SEND_ERROR "if(PATH_IS_PREFIX STREQUAL ...): left-operand use broken")
endif()
