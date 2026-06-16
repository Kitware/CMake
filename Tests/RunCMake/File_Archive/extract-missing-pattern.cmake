# A PATTERNS entry that matches no entry in the archive must fail with a clear
# "Not found in archive" diagnostic.  This exercises the most user-reachable
# error path in extract_tar() (the one that previously leaked all three
# libarchive handles), so it guards that the error path still behaves correctly
# after the RAII cleanup refactor.

set(archive "${CMAKE_CURRENT_BINARY_DIR}/test.zip")
set(src "${CMAKE_CURRENT_BINARY_DIR}/src")
set(dest "${CMAKE_CURRENT_BINARY_DIR}/dest")

file(REMOVE "${archive}")
file(REMOVE_RECURSE "${src}" "${dest}")
file(MAKE_DIRECTORY "${src}")
file(WRITE "${src}/f1.txt" "content\n")
file(MAKE_DIRECTORY "${dest}")

file(ARCHIVE_CREATE
  OUTPUT "${archive}"
  FORMAT zip
  PATHS "${src}")

# "no_such_entry" matches nothing in the archive, so extraction must fail.
file(ARCHIVE_EXTRACT
  INPUT "${archive}"
  DESTINATION "${dest}"
  PATTERNS no_such_entry)
