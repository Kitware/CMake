/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCryptoHash_h
#define cmCryptoHash_h

#include <cmConfigure.h>

#include <cm_auto_ptr.hxx>
#include <string>
#include <vector>

/**
 * @brief Abstract base class for cryptographic hash generators
 */
class cmCryptoHash
{
public:
  virtual ~cmCryptoHash() {}

  /// @brief Returns a new hash generator of the requested type
  /// @arg algo Hash type name. Supported hash types are
  ///      MD5, SHA1, SHA224, SHA256, SHA384, SHA512
  /// @return A valid auto pointer if algo is supported or
  ///         an invalid/NULL pointer otherwise
  static CM_AUTO_PTR<cmCryptoHash> New(const char* algo);

  /// @brief Converts a hex character to its binary value (4 bits)
  /// @arg input Hex character [0-9a-fA-F].
  /// @arg output Binary value of the input character (4 bits)
  /// @return True if input was a valid hex character
  static bool IntFromHexDigit(char input, char& output);

  /// @brief Converts a byte hash to a sequence of hex character pairs
  static std::string ByteHashToString(const std::vector<unsigned char>& hash);

  /// @brief Calculates a binary hash from string input data
  /// @return Binary hash vector
  std::vector<unsigned char> ByteHashString(const std::string& input);

  /// @brief Calculates a binary hash from file content
  /// @see ByteHashString()
  /// @return Non empty binary hash vector if the file was read successfully.
  ///         An empty vector otherwise.
  std::vector<unsigned char> ByteHashFile(const std::string& file);

  /// @brief Calculates a hash string from string input data
  /// @return Sequence of hex characters pairs for each byte of the binary hash
  std::string HashString(const std::string& input);

  /// @brief Calculates a hash string from file content
  /// @see HashString()
  /// @return Non empty hash string if the file was read successfully.
  ///         An empty string otherwise.
  std::string HashFile(const std::string& file);

protected:
  virtual void Initialize() = 0;
  virtual void Append(unsigned char const*, int) = 0;
  virtual std::vector<unsigned char> Finalize() = 0;
};

class cmCryptoHashMD5 : public cmCryptoHash
{
  struct cmsysMD5_s* MD5;

public:
  cmCryptoHashMD5();
  ~cmCryptoHashMD5() CM_OVERRIDE;

protected:
  void Initialize() CM_OVERRIDE;
  void Append(unsigned char const* buf, int sz) CM_OVERRIDE;
  std::vector<unsigned char> Finalize() CM_OVERRIDE;
};

#define cmCryptoHash_SHA_CLASS_DECL(SHA)                                      \
  class cmCryptoHash##SHA : public cmCryptoHash                               \
  {                                                                           \
    union _SHA_CTX* SHA;                                                      \
                                                                              \
  public:                                                                     \
    cmCryptoHash##SHA();                                                      \
    ~cmCryptoHash##SHA();                                                     \
                                                                              \
  protected:                                                                  \
    virtual void Initialize();                                                \
    virtual void Append(unsigned char const* buf, int sz);                    \
    virtual std::vector<unsigned char> Finalize();                            \
  }

cmCryptoHash_SHA_CLASS_DECL(SHA1);
cmCryptoHash_SHA_CLASS_DECL(SHA224);
cmCryptoHash_SHA_CLASS_DECL(SHA256);
cmCryptoHash_SHA_CLASS_DECL(SHA384);
cmCryptoHash_SHA_CLASS_DECL(SHA512);

#undef cmCryptoHash_SHA_CLASS_DECL

#endif
