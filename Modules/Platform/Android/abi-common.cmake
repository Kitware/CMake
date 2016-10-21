string(APPEND _ANDROID_ABI_INIT_CFLAGS
  " -funwind-tables"
  " -no-canonical-prefixes"
  )

string(APPEND _ANDROID_ABI_INIT_EXE_LDFLAGS " -Wl,--gc-sections")

if(NOT _ANDROID_ABI_INIT_EXE_LDFLAGS_NO_nocopyreloc)
  string(APPEND _ANDROID_ABI_INIT_EXE_LDFLAGS " -Wl,-z,nocopyreloc")
endif()
