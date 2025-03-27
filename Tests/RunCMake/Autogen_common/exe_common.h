#ifndef EXE_COMMON_H
#define EXE_COMMON_H

#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

inline int runRealExe(const int argc, char** argv)
{
  std::vector<std::string> args;
  std::string realMocPath;
  std::string const pathArg = "EXE_PATH=";
  std::string cmd;
  if (argc > 1) {
    for (int i = 1; i < argc; ++i) {
      std::string const arg = argv[i];
      if (arg.find(pathArg) != std::string::npos) {
        realMocPath = arg.substr(pathArg.length());
        // if EXE_PATH contains spaces, wrap it in quotes
        if (realMocPath.find(" ") != std::string::npos) {
          realMocPath = "\"" + realMocPath + "\"";
        }
      } else {
        args.push_back(arg);
      }
    }
  }
#ifdef _WIN32
  cmd += "cmd /C \"";
#endif
  cmd += realMocPath + " ";
  for (auto arg : args) {
    // if arg contains spaces, wrap it in quotes
    if (arg.find(' ') != std::string::npos) {
      cmd += " \"" + arg + "\"";
    } else {
      cmd += " " + arg;
    }
  }
#ifdef _WIN32
  cmd += "\"";
#endif
  std::cout << "Running real exe:" << cmd << std::endl;
  return std::system(cmd.c_str());
}

#endif
