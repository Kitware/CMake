
#include "libshared.h"

#include "libstatic.h"

#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>

void compare(const char* refName, const char* testName)
{
  std::ifstream ref;
  ref.open(refName);
  if (!ref.is_open()) {
    std::cout << "Could not open \"" << refName << "\"." << std::endl;
    exit(1);
  }
  std::ifstream test;
  test.open(testName);
  if (!test.is_open()) {
    std::cout << "Could not open \"" << testName << "\"." << std::endl;
    exit(1);
  }

  while (!ref.eof() && !test.eof()) {
    std::string refLine;
    std::string testLine;
    std::getline(ref, refLine);
    std::getline(test, testLine);
    // Some very old Borland runtimes (C++ Builder 5 WITHOUT Update 1) add a
    // trailing null to the string that we need to strip before testing for a
    // trailing space.
    if (refLine.size() && refLine[refLine.size() - 1] == 0) {
      refLine = refLine.substr(0, refLine.size() - 1);
    }
    if (testLine.size() && testLine[testLine.size() - 1] == 0) {
      testLine = testLine.substr(0, testLine.size() - 1);
    }
    // The reference files never have trailing spaces:
    if (testLine.size() && testLine[testLine.size() - 1] == ' ') {
      testLine = testLine.substr(0, testLine.size() - 1);
    }
    if (refLine != testLine) {
      std::cout << "Ref and test are not the same:\n  Ref:  \"" << refLine
                << "\"\n  Test: \"" << testLine << "\"\n";
      exit(1);
    }
  }
  if (!ref.eof() || !test.eof()) {
    std::cout << "Ref and test have differing numbers of lines.";
    exit(1);
  }
}

int main()
{
  {
    Libshared l;
    l.libshared();
    l.libshared_exported();
    l.libshared_deprecated();
    l.libshared_not_exported();
#if defined(_WIN32) || defined(__CYGWIN__)
    l.libshared_excluded();
#else
// l.libshared_excluded(); LINK ERROR (NOT WIN32 AND NOT CYGWIN)
#endif
  }

  {
    LibsharedNotExported l;
    // l.libshared(); LINK ERROR
    l.libshared_exported();
    l.libshared_deprecated();
    // l.libshared_not_exported(); LINK ERROR
    // l.libshared_excluded(); LINK ERROR
  }

  {
    LibsharedExcluded l;
    // l.libshared(); LINK ERROR
    l.libshared_exported();
    l.libshared_deprecated();
    // l.libshared_not_exported(); LINK ERROR
    // l.libshared_excluded(); LINK ERROR
  }

  libshared_exported();
  libshared_deprecated();
  // libshared_not_exported(); LINK ERROR
  // libshared_excluded(); LINK ERROR

  {
    Libstatic l;
    l.libstatic();
    l.libstatic_exported();
    l.libstatic_deprecated();
    l.libstatic_not_exported();
    l.libstatic_excluded();
  }

  {
    LibstaticNotExported l;
    l.libstatic();
    l.libstatic_exported();
    l.libstatic_deprecated();
    l.libstatic_not_exported();
    l.libstatic_excluded();
  }

  {
    LibstaticExcluded l;
    l.libstatic();
    l.libstatic_exported();
    l.libstatic_deprecated();
    l.libstatic_not_exported();
    l.libstatic_excluded();
  }

  libstatic_exported();
  libstatic_deprecated();
  libstatic_not_exported();
  libstatic_excluded();

#if defined(SRC_DIR) && defined(BIN_DIR)
  compare(SRC_DIR "/libshared_export.h",
          BIN_DIR "/libshared/libshared_export.h");
  compare(SRC_DIR "/libstatic_export.h",
          BIN_DIR "/libstatic/libstatic_export.h");
#endif

  return 0;
}
