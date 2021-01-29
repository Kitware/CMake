cmake_minimum_required (VERSION 3.19...3.20)

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

## Specify a range including current OpenSSL version
string (REGEX MATCH "^([0-9]+)" upper_version "${version}")
math (EXPR upper_version "${upper_version} + 1")

find_package (OpenSSL 0.9...${upper_version}.0 COMPONENTS Crypto)
if (NOT OPENSSL_FOUND)
  message (FATAL_ERROR "Failed to find OpenSSL with version range 0.9...${upper_version}.0")
endif()

# clean-up OpenSSL variables
unset (OPENSSL_INCLUDE_DIR)
unset (OPENSSL_CRYPTO_LIBRARY)
unset (OPENSSL_CRYPTO_LIBRARIES)
unset (OPENSSL_LIBRARIES)
unset (OPENSSL_VERSION)
unset (OPENSSL_FOUND)

## Specify a range excluding current OpenSSL version
set (range 0.9...<${version})
find_package (OpenSSL ${range} COMPONENTS Crypto)
if (OPENSSL_FOUND)
  message (FATAL_ERROR "Unexpectedly find OpenSSL with version range ${range}")
endif()
