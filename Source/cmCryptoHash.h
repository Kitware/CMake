/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <cm/string_view>

/**
 * @brief Abstract base class for cryptographic hash generators
 */
class cmCryptoHash
{
public:
  enum Algo
  {
    AlgoMD5,
    AlgoSHA1,
    AlgoSHA224,
    AlgoSHA256,
    AlgoSHA384,
    AlgoSHA512,
    AlgoSHA3_224,
    AlgoSHA3_256,
    AlgoSHA3_384,
    AlgoSHA3_512
  };

  cmCryptoHash(Algo algo);
  ~cmCryptoHash();

  cmCryptoHash(cmCryptoHash const&) = delete;
  cmCryptoHash& operator=(cmCryptoHash const&) = delete;

  /// @brief Returns a new hash generator of the requested type
  /// @arg algo Hash type name. Supported hash types are
  ///      MD5, SHA1, SHA224, SHA256, SHA384, SHA512,
  ///      SHA3_224, SHA3_256, SHA3_384, SHA3_512
  /// @return A valid auto pointer if algo is supported or
  ///         an invalid/NULL pointer otherwise
  static std::unique_ptr<cmCryptoHash> New(cm::string_view algo);

  /// @brief Converts a hex character to its binary value (4 bits)
  /// @arg input Hex character [0-9a-fA-F].
  /// @arg output Binary value of the input character (4 bits)
  /// @return True if input was a valid hex character
  static bool IntFromHexDigit(char input, char& output);

  /// @brief Converts a byte hash to a sequence of hex character pairs
  static std::string ByteHashToString(const std::vector<unsigned char>& hash);

  /// @brief Calculates a binary hash from string input data
  /// @return Binary hash vector
  std::vector<unsigned char> ByteHashString(cm::string_view input);

  /// @brief Calculates a binary hash from file content
  /// @see ByteHashString()
  /// @return Non empty binary hash vector if the file was read successfully.
  ///         An empty vector otherwise.
  std::vector<unsigned char> ByteHashFile(const std::string& file);

  /// @brief Calculates a hash string from string input data
  /// @return Sequence of hex characters pairs for each byte of the binary hash
  std::string HashString(cm::string_view input);

  /// @brief Calculates a hash string from file content
  /// @see HashString()
  /// @return Non empty hash string if the file was read successfully.
  ///         An empty string otherwise.
  std::string HashFile(const std::string& file);

  void Initialize();
  void Append(void const*, size_t);
  void Append(cm::string_view input);
  std::vector<unsigned char> Finalize();
  std::string FinalizeHex();

private:
  unsigned int Id;
  struct rhash_context* CTX;
};
