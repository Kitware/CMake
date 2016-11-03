project(librhash C)

set(librhash_sources
  librhash/algorithms.c
  librhash/algorithms.h
  librhash/byte_order.c
  librhash/byte_order.h
  librhash/hex.c
  librhash/hex.h
  librhash/md5.c
  librhash/md5.h
  librhash/rhash.c
  librhash/rhash.h
  librhash/sha1.c
  librhash/sha1.h
  librhash/sha256.c
  librhash/sha256.h
  librhash/sha512.c
  librhash/sha512.h
  librhash/ustd.h
  librhash/util.h
  )

include_directories(
  ${KWSYS_HEADER_ROOT}
  )

add_library(cmlibrhash ${librhash_sources})
