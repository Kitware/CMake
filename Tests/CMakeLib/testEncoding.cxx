#include <iostream>
#include <iterator>
#include <string>

#include "cmsys/FStream.hxx"

#ifdef _WIN32
#  include <windows.h>
#endif

int main(int argc, char* argv[])
{
  if (argc <= 2) {
    std::cout << "Usage: testEncoding <encoding> <file>" << std::endl;
    return 1;
  }
  std::string const encoding(argv[1]);
#ifdef _WIN32
  unsigned int codePage = 0;
  if ((encoding == "UTF8") || (encoding == "UTF-8")) {
    codePage = CP_UTF8;
  } else if (encoding == "ANSI") {
    codePage = CP_ACP;
  } else if (encoding == "OEM") {
    codePage = CP_OEMCP;
  } else if (unsigned int consoleOutputCP = GetConsoleOutputCP()) {
    codePage = consoleOutputCP;
  } else if (unsigned int ansiCP = GetACP()) {
    codePage = ansiCP;
  }
#endif
  cmsys::ifstream file(argv[2]);
  if (!file.is_open()) {
    std::cout << "Failed to open file: " << argv[2] << std::endl;
    return 2;
  }
  std::string text((std::istreambuf_iterator<char>(file)),
                   std::istreambuf_iterator<char>());
#ifdef _WIN32
  if (codePage) {
    if (int wlen = MultiByteToWideChar(CP_UTF8, 0, text.data(),
                                       int(text.size()), nullptr, 0)) {
      std::vector<wchar_t> w(wlen);
      if (MultiByteToWideChar(CP_UTF8, 0, text.data(), int(text.size()),
                              w.data(), w.size())) {
        if (int nlen =
              WideCharToMultiByte(codePage, 0, w.data(), int(w.size()),
                                  nullptr, 0, nullptr, nullptr)) {
          std::vector<char> n(nlen);
          if (WideCharToMultiByte(codePage, 0, w.data(), int(w.size()),
                                  n.data(), int(n.size()), nullptr, nullptr)) {
            text = std::string(n.data(), n.size());
          }
        }
      }
    }
  }
#endif
  std::cout << text;
  return 0;
}
