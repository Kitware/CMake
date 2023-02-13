find_package (OpenSSL REQUIRED COMPONENTS Crypto)
# Store version without a possibly trailing letter.
string (REGEX MATCH "^([0-9.]+)" version "${OPENSSL_VERSION}")

# clean-up OpenSSL variables
unset (OPENSSL_INCLUDE_DIR)
unset (OPENSSL_CRYPTO_LIBRARY)
unset (OPENSSL_CRYPTO_LIBRARIES)
unset (OPENSSL_LIBRARIES)
unset (OPENSSL_VERSION)
unset (OPENSSL_FOUND)


find_package (OpenSSL ${version} EXACT COMPONENTS Crypto)
if (NOT OPENSSL_FOUND)
  message (FATAL_ERROR "Failed to find OpenSSL with version ${version} EXACT")
endif()
