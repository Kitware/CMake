
include ("${RunCMake_SOURCE_DIR}/check_errors.cmake")
unset (errors)

set (path "a///b/c")
cmake_path(IS_PREFIX path "a/b/c/d" output)
if (NOT output)
  list (APPEND errors "'${path} is not prefix of 'a/b/c/d'")
endif()

set (path "a///b/c/../d")
cmake_path(IS_PREFIX path "a/b/d/e" output)
if (output)
  list (APPEND errors "'${path} is prefix of 'a/b/d/e'")
endif()
cmake_path(IS_PREFIX path "a/b/d/e" NORMALIZE output)
if (NOT output)
  list (APPEND errors "'${path} is not prefix of 'a/b/d/e'")
endif()

set(path "/a/b/..")
cmake_path(IS_PREFIX path "/a/c/../b" NORMALIZE output)
if (NOT output)
  list (APPEND errors "'${path} is not prefix of '/a/c/../b'")
endif()

# The test is not strict: a path is a prefix of itself.
set (prefix "/a/b")
cmake_path(IS_PREFIX prefix "/a/b" output)
if (NOT output)
  list (APPEND errors "'${prefix}' is not prefix of '/a/b'")
endif()

# A trailing separator is an empty component, so it is significant on the
# prefix, which then needs a further component to consume it.
cmake_path(IS_PREFIX prefix "/a/b/" output)
if (NOT output)
  list (APPEND errors "'${prefix}' is not prefix of '/a/b/'")
endif()
set (prefix "/a/b/")
cmake_path(IS_PREFIX prefix "/a/b" output)
if (output)
  list (APPEND errors "'${prefix}' is prefix of '/a/b'")
endif()
cmake_path(IS_PREFIX prefix "/a/b/c" output)
if (NOT output)
  list (APPEND errors "'${prefix}' is not prefix of '/a/b/c'")
endif()

# Without NORMALIZE, '.' and '..' are ordinary components, so a '..' escape
# is still a lexical prefix and a '.' component defeats the match.
set (prefix "/a/b")
cmake_path(IS_PREFIX prefix "/a/b/../../etc" output)
if (NOT output)
  list (APPEND errors "'${prefix}' is not prefix of '/a/b/../../etc'")
endif()
cmake_path(IS_PREFIX prefix "/a/./b//c" output)
if (output)
  list (APPEND errors "'${prefix}' is prefix of '/a/./b//c'")
endif()

# Components are compared whole, so a sibling sharing a textual prefix does
# not match.
cmake_path(IS_PREFIX prefix "/a/bc" output)
if (output)
  list (APPEND errors "'${prefix}' is prefix of '/a/bc'")
endif()

# Relative paths are not made absolute, and the root directory is not an
# exception to that rule.
set (prefix "/a")
cmake_path(IS_PREFIX prefix "b/c" output)
if (output)
  list (APPEND errors "'${prefix}' is prefix of 'b/c'")
endif()
set (prefix "b")
cmake_path(IS_PREFIX prefix "b/c" output)
if (NOT output)
  list (APPEND errors "'${prefix}' is not prefix of 'b/c'")
endif()
set (prefix "/")
cmake_path(IS_PREFIX prefix "b/c" output)
if (output)
  list (APPEND errors "'${prefix}' is prefix of 'b/c'")
endif()
cmake_path(IS_PREFIX prefix "/a/b" output)
if (NOT output)
  list (APPEND errors "'${prefix}' is not prefix of '/a/b'")
endif()

# A '.' prefix matches only a path that spells '.' too.
set (prefix ".")
cmake_path(IS_PREFIX prefix "a/b" output)
if (output)
  list (APPEND errors "'${prefix}' is prefix of 'a/b'")
endif()
cmake_path(IS_PREFIX prefix "./a/b" output)
if (NOT output)
  list (APPEND errors "'${prefix}' is not prefix of './a/b'")
endif()

# The empty path is a prefix of every path, including itself.
set (prefix "")
cmake_path(IS_PREFIX prefix "/a/b" output)
if (NOT output)
  list (APPEND errors "the empty path is not prefix of '/a/b'")
endif()
cmake_path(IS_PREFIX prefix "" output)
if (NOT output)
  list (APPEND errors "the empty path is not prefix of itself")
endif()
set (prefix "/a")
cmake_path(IS_PREFIX prefix "" output)
if (output)
  list (APPEND errors "'${prefix}' is prefix of the empty path")
endif()

# Comparison follows the host path model.
if(WIN32)
  # Backslashes are separators, and are not converted.
  set (prefix "C:\\a")
  cmake_path(IS_PREFIX prefix "C:\\a\\b" output)
  if (NOT output)
    list (APPEND errors "'${prefix}' is not prefix of 'C:\\a\\b'")
  endif()

  # Comparison is case-sensitive even where the filesystem is not.  The
  # second case isolates the root-name, since ordinary components are
  # case-sensitive under every path model.
  set (prefix "c:/a")
  cmake_path(IS_PREFIX prefix "C:/A/b" output)
  if (output)
    list (APPEND errors "'${prefix}' is prefix of 'C:/A/b'")
  endif()
  cmake_path(IS_PREFIX prefix "C:/a/b" output)
  if (output)
    list (APPEND errors "'${prefix}' is prefix of 'C:/a/b'")
  endif()

  # A drive-relative path has a root-name but no root-directory, so it is
  # relative and is not reconciled against an absolute prefix.
  set (prefix "C:/")
  cmake_path(IS_PREFIX prefix "C:foo" output)
  if (output)
    list (APPEND errors "'${prefix}' is prefix of 'C:foo'")
  endif()
else()
  # Backslashes are ordinary filename characters.
  set (prefix "C:\\a")
  cmake_path(IS_PREFIX prefix "C:\\a\\b" output)
  if (output)
    list (APPEND errors "'${prefix}' is prefix of 'C:\\a\\b'")
  endif()
endif()

if(WIN32 OR CYGWIN)
  # '//host/share' is a root-name, not a doubled separator.
  set (prefix "//host/share")
  cmake_path(IS_PREFIX prefix "//host/share/a" output)
  if (NOT output)
    list (APPEND errors "'${prefix}' is not prefix of '//host/share/a'")
  endif()
  set (prefix "/a")
  cmake_path(IS_PREFIX prefix "//a/b" output)
  if (output)
    list (APPEND errors "'${prefix}' is prefix of '//a/b'")
  endif()
else()
  # A doubled leading separator collapses.
  set (prefix "/a")
  cmake_path(IS_PREFIX prefix "//a/b" output)
  if (NOT output)
    list (APPEND errors "'${prefix}' is not prefix of '//a/b'")
  endif()
endif()

check_errors (IS_PREFIX ${errors})
