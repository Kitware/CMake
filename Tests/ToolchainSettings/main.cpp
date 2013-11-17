
#include <string>
#include <fstream>
#include <iostream>

int main(void)
{
  std::ifstream f;
  f.open(MAKE_OUTPUT);
  if (!f.is_open())
    {
    return -1;
    }
  std::string content;

  bool gotCompilation = false;
  bool gotLink = false;
  while (!f.eof())
    {
    std::string output;
    getline(f,output);
    if (output.find(COMPILER) != std::string::npos)
      {
      if (output.find("-sysroot") == std::string::npos)
        {
        return -1;
        }
      if (output.find(" -c ") != std::string::npos)
        {
        gotCompilation = true;
        if (output.find(" -I") != std::string::npos)
          {
          return -1;
          }
        if (output.find(" -isystem") != std::string::npos)
          {
          return -1;
          }
        }
      else
        {
        gotLink = true;
        if (output.find("zlib.so") != std::string::npos)
          {
          return -1;
          }
        if (output.find("-lz") == std::string::npos)
          {
          return -1;
          }
        }
      }
    }
  if (!gotCompilation || !gotLink)
    {
    return -1;
    }
  f.close();

  return 0;
}
