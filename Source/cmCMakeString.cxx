/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmCMakeString.hxx"

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <vector>

#include "cmsys/RegularExpression.hxx"

#include "cmCryptoHash.h"
#include "cmGeneratorExpression.h"
#include "cmMakefile.h"
#include "cmPolicies.h"
#include "cmStringAlgorithms.h"
#include "cmStringReplaceHelper.h"
#include "cmSystemTools.h"
#include "cmTimestamp.h"
#include "cmUuid.h"

namespace cm {
bool CMakeString::Compare(CompOperator op, cm::string_view other)
{
  switch (op) {
    case CompOperator::EQUAL:
      return this->String_ == other;
    case CompOperator::LESS:
      return this->String_ < other;
    case CompOperator::LESS_EQUAL:
      return this->String_ <= other;
    case CompOperator::GREATER:
      return this->String_ > other;
    case CompOperator::GREATER_EQUAL:
      return this->String_ >= other;
    default:
      return false;
  }
}

CMakeString& CMakeString::Replace(std::string const& matchExpression,
                                  std::string const& replaceExpression,
                                  Regex regex, cmMakefile* makefile)
{
  if (regex == Regex::Yes) {
    if (makefile) {
      makefile->ClearMatches();
    }

    cmStringReplaceHelper replaceHelper(matchExpression, replaceExpression,
                                        makefile);
    if (!replaceHelper.IsReplaceExpressionValid()) {
      throw std::invalid_argument(replaceHelper.GetError());
    }
    if (!replaceHelper.IsRegularExpressionValid()) {
      throw std::invalid_argument(
        cmStrCat("Failed to compile regex \"", matchExpression, '"'));
    }

    std ::string output;
    if (!replaceHelper.Replace(this->String_, output)) {
      throw std::runtime_error(replaceHelper.GetError());
    }
    this->String_ = std::move(output);
  } else {
    std::string output = this->String_.str();
    cmsys::SystemTools::ReplaceString(output, matchExpression,
                                      replaceExpression);
    this->String_ = std::move(output);
  }

  return *this;
};

cmList CMakeString::Match(std::string const& matchExpression,
                          MatchItems matchItems, cmMakefile* makefile) const
{
  if (makefile) {
    makefile->ClearMatches();
  }

  // Compile the regular expression.
  cmsys::RegularExpression re;
  if (!re.compile(matchExpression)) {
    throw std::invalid_argument(
      cmStrCat("Failed to compile regex \"", matchExpression, '"'));
  }

  cmList output;
  if (matchItems == MatchItems::Once) {
    if (re.find(this->String_.data())) {
      if (makefile) {
        makefile->StoreMatches(re);
      }
      output = re.match();
    }
  } else {
    unsigned optAnchor = 0;
    if (makefile &&
        makefile->GetPolicyStatus(cmPolicies::CMP0186) != cmPolicies::NEW) {
      optAnchor = cmsys::RegularExpression::BOL_AT_OFFSET;
    }

    // Scan through the input for all matches.
    std::string::size_type base = 0;
    unsigned optNonEmpty = 0;
    while (re.find(this->String_.data(), base, optAnchor | optNonEmpty)) {
      if (makefile) {
        makefile->ClearMatches();
        makefile->StoreMatches(re);
      }

      output.push_back(re.match());
      base = re.end();

      if (re.start() == this->String_.length()) {
        break;
      }
      if (re.start() == re.end()) {
        optNonEmpty = cmsys::RegularExpression::NONEMPTY_AT_OFFSET;
      } else {
        optNonEmpty = 0;
      }
    }
  }

  return output;
}

CMakeString CMakeString::Substring(long begin, long count) const
{
  if (begin < 0 || static_cast<size_type>(begin) > this->String_.size()) {
    throw std::out_of_range(cmStrCat(
      "begin index: ", begin, " is out of range 0 - ", this->String_.size()));
  }
  if (count < -1) {
    throw std::out_of_range(
      cmStrCat("end index: ", count, " should be -1 or greater"));
  }

  return this->String_.substr(static_cast<size_type>(begin),
                              count == -1 ? npos
                                          : static_cast<size_type>(count));
}

CMakeString& CMakeString::Strip(StripItems stripItems)
{
  if (stripItems == StripItems::Space) {
    this->String_ = cmTrimWhitespace(this->String_);
  } else {
    this->String_ = cmGeneratorExpression::Preprocess(
      this->String_, cmGeneratorExpression::StripAllGeneratorExpressions);
  }

  return *this;
}

CMakeString& CMakeString::Repeat(size_type count)
{
  switch (this->Size()) {
    case 0u:
      // Nothing to do for zero length input strings
      break;
    case 1u:
      // NOTE If the string to repeat consists of the only character,
      // use the appropriate constructor.
      this->String_ = std::string(count, this->String_[0]);
      break;
    default:
      std::string result;
      auto size = this->Size();

      result.reserve(size * count);
      for (auto i = 0u; i < count; ++i) {
        result.insert(i * size, this->String_.data(), size);
      }
      this->String_ = std::move(result);
      break;
  }
  return *this;
}

CMakeString& CMakeString::Quote(QuoteItems)
{
  std ::string output;
  // Escape all regex special characters
  cmStringReplaceHelper replaceHelper("([][()+*^.$?|\\\\])", R"(\\\1)");
  if (!replaceHelper.Replace(this->String_, output)) {
    throw std::runtime_error(replaceHelper.GetError());
  }

  this->String_ = std::move(output);
  return *this;
}

CMakeString& CMakeString::Hash(cm::string_view hashAlgorithm)
{
  std::unique_ptr<cmCryptoHash> hash(cmCryptoHash::New(hashAlgorithm));
  if (hash) {
    this->String_ = hash->HashString(this->String_);
    return *this;
  }

  throw std::invalid_argument(
    cmStrCat(hashAlgorithm, ": invalid hash algorithm."));
}

CMakeString& CMakeString::FromASCII(string_range codes)
{
  std::string output;
  output.reserve(codes.size());

  for (auto const& code : codes) {
    try {
      auto ch = std::stoi(code);
      if (ch > 0 && ch < 256) {
        output += static_cast<char>(ch);
      } else {
        throw std::invalid_argument(
          cmStrCat("Character with code ", code, " does not exist."));
      }
    } catch (...) {
      throw std::invalid_argument(
        cmStrCat("Character with code ", code, " does not exist."));
    }
  }
  this->String_ = std::move(output);
  return *this;
}

CMakeString& CMakeString::ToHexadecimal(cm::string_view str)
{
  std::string output(str.size() * 2, ' ');
  std::string::size_type hexIndex = 0;

  for (auto const& c : str) {
    std::snprintf(&output[hexIndex], 3, "%.2x", c & 0xFFu);
    hexIndex += 2;
  }
  this->String_ = output;
  return *this;
}

cm::string_view const CMakeString::RandomDefaultAlphabet{
  "qwertyuiopasdfghjklzxcvbnm"
  "QWERTYUIOPASDFGHJKLZXCVBNM"
  "0123456789"
};
bool CMakeString::Seeded = false;

CMakeString& CMakeString::Random(unsigned int seed, std::size_t length,
                                 cm::string_view alphabet)
{
  if (alphabet.empty()) {
    alphabet = RandomDefaultAlphabet;
  }

  if (length < 1) {
    throw std::out_of_range("Invoked with bad length.");
  }

  if (!this->Seeded) {
    this->Seeded = true;
    std::srand(seed);
  }

  double alphabetSize = static_cast<double>(alphabet.size());
  std::vector<char> result;
  result.reserve(length + 1);

  for (std::size_t i = 0; i < length; i++) {
    auto index = static_cast<std::string::size_type>(
      alphabetSize * std::rand() / (RAND_MAX + 1.0));
    result.push_back(alphabet[index]);
  }
  result.push_back(0);

  this->String_ = result.data();
  return *this;
}

CMakeString& CMakeString::Timestamp(cm::string_view format, UTC utc)
{
  cmTimestamp timestamp;
  this->String_ = timestamp.CurrentTime(format, utc == UTC::Yes);
  return *this;
}

CMakeString& CMakeString::UUID(cm::string_view nameSpace, cm::string_view name,
                               UUIDType type, Case uuidCase)
{
#if !defined(CMAKE_BOOTSTRAP)
  cmUuid uuidGenerator;
  std::vector<unsigned char> uuidNamespace;
  std::string uuid;

  if (!uuidGenerator.StringToBinary(nameSpace, uuidNamespace)) {
    throw std::invalid_argument("malformed NAMESPACE UUID");
  }

  if (type == UUIDType::MD5) {
    uuid = uuidGenerator.FromMd5(uuidNamespace, name);
  } else if (type == UUIDType::SHA1) {
    uuid = uuidGenerator.FromSha1(uuidNamespace, name);
  }

  if (uuid.empty()) {
    throw std::runtime_error("generation failed");
  }

  if (uuidCase == Case::Upper) {
    uuid = cmSystemTools::UpperCase(uuid);
  }

  this->String_ = std::move(uuid);
  return *this;
#else
  throw std::runtime_error("not available during bootstrap");
#endif
}

}
