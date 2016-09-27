/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCryptoHash.h"

#include "cm_sha2.h"

#include <cmsys/FStream.hxx>
#include <cmsys/MD5.h>
#include <string.h>

CM_AUTO_PTR<cmCryptoHash> cmCryptoHash::New(const char* algo)
{
  if (strcmp(algo, "MD5") == 0) {
    return CM_AUTO_PTR<cmCryptoHash>(new cmCryptoHashMD5);
  }
  if (strcmp(algo, "SHA1") == 0) {
    return CM_AUTO_PTR<cmCryptoHash>(new cmCryptoHashSHA1);
  }
  if (strcmp(algo, "SHA224") == 0) {
    return CM_AUTO_PTR<cmCryptoHash>(new cmCryptoHashSHA224);
  }
  if (strcmp(algo, "SHA256") == 0) {
    return CM_AUTO_PTR<cmCryptoHash>(new cmCryptoHashSHA256);
  }
  if (strcmp(algo, "SHA384") == 0) {
    return CM_AUTO_PTR<cmCryptoHash>(new cmCryptoHashSHA384);
  }
  if (strcmp(algo, "SHA512") == 0) {
    return CM_AUTO_PTR<cmCryptoHash>(new cmCryptoHashSHA512);
  }
  return CM_AUTO_PTR<cmCryptoHash>(CM_NULLPTR);
}

bool cmCryptoHash::IntFromHexDigit(char input, char& output)
{
  if (input >= '0' && input <= '9') {
    output = char(input - '0');
    return true;
  }
  if (input >= 'a' && input <= 'f') {
    output = char(input - 'a' + 0xA);
    return true;
  }
  if (input >= 'A' && input <= 'F') {
    output = char(input - 'A' + 0xA);
    return true;
  }
  return false;
}

std::string cmCryptoHash::ByteHashToString(
  const std::vector<unsigned char>& hash)
{
  // Map from 4-bit index to hexadecimal representation.
  static char const hex[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

  std::string res;
  for (std::vector<unsigned char>::const_iterator vit = hash.begin();
       vit != hash.end(); ++vit) {
    res.push_back(hex[(*vit) >> 4]);
    res.push_back(hex[(*vit) & 0xF]);
  }
  return res;
}

std::vector<unsigned char> cmCryptoHash::ByteHashString(
  const std::string& input)
{
  this->Initialize();
  this->Append(reinterpret_cast<unsigned char const*>(input.c_str()),
               static_cast<int>(input.size()));
  return this->Finalize();
}

std::vector<unsigned char> cmCryptoHash::ByteHashFile(const std::string& file)
{
  cmsys::ifstream fin(file.c_str(), std::ios::in | std::ios::binary);
  if (fin) {
    this->Initialize();
    {
      // Should be efficient enough on most system:
      cm_sha2_uint64_t buffer[512];
      char* buffer_c = reinterpret_cast<char*>(buffer);
      unsigned char const* buffer_uc =
        reinterpret_cast<unsigned char const*>(buffer);
      // This copy loop is very sensitive on certain platforms with
      // slightly broken stream libraries (like HPUX).  Normally, it is
      // incorrect to not check the error condition on the fin.read()
      // before using the data, but the fin.gcount() will be zero if an
      // error occurred.  Therefore, the loop should be safe everywhere.
      while (fin) {
        fin.read(buffer_c, sizeof(buffer));
        if (int gcount = static_cast<int>(fin.gcount())) {
          this->Append(buffer_uc, gcount);
        }
      }
    }
    if (fin.eof()) {
      // Success
      return this->Finalize();
    }
    // Finalize anyway
    this->Finalize();
  }
  // Return without success
  return std::vector<unsigned char>();
}

std::string cmCryptoHash::HashString(const std::string& input)
{
  return ByteHashToString(this->ByteHashString(input));
}

std::string cmCryptoHash::HashFile(const std::string& file)
{
  return ByteHashToString(this->ByteHashFile(file));
}

cmCryptoHashMD5::cmCryptoHashMD5()
  : MD5(cmsysMD5_New())
{
}

cmCryptoHashMD5::~cmCryptoHashMD5()
{
  cmsysMD5_Delete(this->MD5);
}

void cmCryptoHashMD5::Initialize()
{
  cmsysMD5_Initialize(this->MD5);
}

void cmCryptoHashMD5::Append(unsigned char const* buf, int sz)
{
  cmsysMD5_Append(this->MD5, buf, sz);
}

std::vector<unsigned char> cmCryptoHashMD5::Finalize()
{
  std::vector<unsigned char> hash(16, 0);
  cmsysMD5_Finalize(this->MD5, &hash[0]);
  return hash;
}

#define cmCryptoHash_SHA_CLASS_IMPL(SHA)                                      \
  cmCryptoHash##SHA::cmCryptoHash##SHA()                                      \
    : SHA(new SHA_CTX)                                                        \
  {                                                                           \
  }                                                                           \
  cmCryptoHash##SHA::~cmCryptoHash##SHA() { delete this->SHA; }               \
  void cmCryptoHash##SHA::Initialize() { SHA##_Init(this->SHA); }             \
  void cmCryptoHash##SHA::Append(unsigned char const* buf, int sz)            \
  {                                                                           \
    SHA##_Update(this->SHA, buf, sz);                                         \
  }                                                                           \
  std::vector<unsigned char> cmCryptoHash##SHA::Finalize()                    \
  {                                                                           \
    std::vector<unsigned char> hash(SHA##_DIGEST_LENGTH, 0);                  \
    SHA##_Final(&hash[0], this->SHA);                                         \
    return hash;                                                              \
  }

cmCryptoHash_SHA_CLASS_IMPL(SHA1) cmCryptoHash_SHA_CLASS_IMPL(SHA224)
  cmCryptoHash_SHA_CLASS_IMPL(SHA256) cmCryptoHash_SHA_CLASS_IMPL(SHA384)
    cmCryptoHash_SHA_CLASS_IMPL(SHA512)
