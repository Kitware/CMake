#include <cctype>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <thread>

std::string getStdin()
{
  char buffer[1024];
  std::ostringstream str;
  do {
    std::cin.read(buffer, 1024);
    str.write(buffer, std::cin.gcount());
  } while (std::cin.gcount() > 0);
  return str.str();
}

int main(int argc, char** argv)
{
  if (argc < 2) {
    return -1;
  }

  std::string command = argv[1];
  if (command == "echo") {
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    std::cout << "HELLO world!" << std::flush;
    std::cerr << "1" << std::flush;
    return 0;
  }
  if (command == "capitalize") {
    std::this_thread::sleep_for(std::chrono::milliseconds(9000));
    std::string input = getStdin();
    for (auto& c : input) {
      c = static_cast<char>(std::toupper(c));
    }
    std::cout << input << std::flush;
    std::cerr << "2" << std::flush;
    return 1;
  }
  if (command == "dedup") {
    // Use a nested scope to free all resources before aborting below.
    try {
      std::string input = getStdin();
      std::set<char> seen;
      std::string output;
      for (auto c : input) {
        if (!seen.count(c)) {
          seen.insert(c);
          output += c;
        }
      }
      std::cout << output << std::flush;
      std::cerr << "3" << std::flush;
    } catch (...) {
    }

    // On Windows, the exit code of abort() is different between debug and
    // release builds, and does not yield a term_signal in libuv in either
    // case. For the sake of simplicity, we just return another non-zero code.
#ifdef _WIN32
    return 2;
#else
    std::abort();
#endif
  }

  return -1;
}
