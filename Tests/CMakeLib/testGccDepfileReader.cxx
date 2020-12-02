#include <cstddef> // IWYU pragma: keep
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <cm/optional>

#include "cmsys/FStream.hxx"

#include "cmGccDepfileReader.h"
#include "cmGccDepfileReaderTypes.h" // for cmGccDepfileContent, cmGccStyle...
#include "cmSystemTools.h"

namespace {

cmGccDepfileContent readPlainDepfile(const char* filePath)
{
  cmGccDepfileContent result;
  cmsys::ifstream is(filePath);
  if (!is.is_open())
    return result;
  std::string line;

  cmGccStyleDependency dep;
  bool readingRules = true;
  while (cmSystemTools::GetLineFromStream(is, line)) {
    if (line == "--RULES--") {
      if (!dep.rules.empty()) {
        result.push_back(std::move(dep));
        dep = cmGccStyleDependency();
      }
      readingRules = true;
    } else if (line == "--DEPENDENCIES--") {
      readingRules = false;
    } else {
      std::vector<std::string>& dst = readingRules ? dep.rules : dep.paths;
      dst.push_back(std::move(line));
      line = std::string();
    }
  }

  if (!dep.rules.empty()) {
    result.push_back(std::move(dep));
  }

  return result;
}

bool compare(const std::vector<std::string>& actual,
             const std::vector<std::string>& expected, const char* msg)
{
  if (actual.size() != expected.size()) {
    std::cerr << msg << "expected " << expected.size() << " entries."
              << std::endl
              << "Actual number of entries: " << actual.size() << std::endl;
    return false;
  }
  for (std::size_t i = 0; i < actual.size(); ++i) {
    if (actual[i] != expected[i]) {
      std::cerr << msg << std::endl
                << "expected: " << expected[i] << std::endl
                << "actual: " << actual[i] << std::endl;
      return false;
    }
  }
  return true;
}

bool compare(const cmGccDepfileContent& actual,
             const cmGccDepfileContent& expected)
{
  if (actual.size() != expected.size()) {
    std::cerr << "Expected " << expected.size() << " entries." << std::endl
              << "Actual number of entries: " << actual.size() << std::endl;
    return false;
  }
  for (std::size_t i = 0; i < actual.size(); ++i) {
    if (!compare(actual[i].rules, expected[i].rules, "Rules differ: ") ||
        !compare(actual[i].paths, expected[i].paths, "Paths differ: ")) {
      return false;
    }
  }
  return true;
}

void dump(const char* label, const cmGccDepfileContent& dfc)
{
  std::cerr << label << std::endl;
  for (const auto& entry : dfc) {
    auto rit = entry.rules.cbegin();
    if (rit != entry.rules.cend()) {
      std::cerr << *rit;
      for (++rit; rit != entry.rules.cend(); ++rit) {
        std::cerr << "  " << *rit;
      }
      std::cerr << ": " << std::endl;
    }
    for (const auto& path : entry.paths) {
      std::cerr << "    " << path << std::endl;
    }
  }
}

} // anonymous namespace

int testGccDepfileReader(int argc, char* argv[])
{
  if (argc < 2) {
    std::cout << "Invalid arguments.\n";
    return -1;
  }

  std::string dataDirPath = argv[1];
  dataDirPath += "/testGccDepfileReader_data";
  const int numberOfTestFiles = 7; // 6th file doesn't exist
  for (int i = 1; i <= numberOfTestFiles; ++i) {
    const std::string base = dataDirPath + "/deps" + std::to_string(i);
    const std::string depfile = base + ".d";
    const std::string plainDepfile = base + ".txt";
    std::cout << "Comparing " << base << " with " << plainDepfile << std::endl;
    const auto actual = cmReadGccDepfile(depfile.c_str());
    if (cmSystemTools::FileExists(plainDepfile)) {
      if (!actual) {
        std::cerr << "Reading " << depfile << " should have succeeded\n";
        return 1;
      }
      const auto expected = readPlainDepfile(plainDepfile.c_str());
      if (!compare(*actual, expected)) {
        dump("actual", *actual);
        dump("expected", expected);
        return 1;
      }
    } else if (actual) {
      std::cerr << "Reading " << depfile << " should have failed\n";
      return 1;
    }
  }

  return 0;
}
