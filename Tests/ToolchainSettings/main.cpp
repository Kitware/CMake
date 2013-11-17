
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
  bool gotInstallation = false;
  while (!f.eof())
    {
    std::string output;
    getline(f,output);
    if (output.find(COMPILER) != std::string::npos)
      {
      if (output.find("-sysroot") == std::string::npos)
        {
        std::cout << "Sysroot not used by compiler: " << output << std::endl;
        return -1;
        }
      if (output.find(" -c ") != std::string::npos)
        {
        gotCompilation = true;
        if (output.find(" -I") != std::string::npos)
          {
          std::cout << "Unexpected include: " << output << std::endl;
          return -1;
          }
        if (output.find(" -isystem") != std::string::npos)
          {
          std::cout << "Unexpected include: " << output << std::endl;
          return -1;
          }
        }
      else
        {
        gotLink = true;
        if (output.find("zlib.so") != std::string::npos)
          {
          std::cout << "Unexpected library name: " << output << std::endl;
          return -1;
          }
        if (output.find("-lz") == std::string::npos)
          {
          std::cout << "Expected -lz: " << output << std::endl;
          return -1;
          }
        }
      }
    else if (output.find("-- Installing: ") != std::string::npos)
      {
      gotInstallation = true;
      if (output.find("/stage/") == std::string::npos)
        {
        std::cout << "Install outside stage: " << output << std::endl;
        return -1;
        }
      if (output.find("InstallationPrefix") != std::string::npos)
        {
        std::cout << "Install to prefix, not stage: " << output << std::endl;
        return -1;
        }
      }
    }
  if (!gotCompilation)
    {
    std::cout << "Compilation step missing." << std::endl;
    return -1;
    }
  if (!gotLink)
    {
    std::cout << "Link step missing." << std::endl;
    return -1;
    }
  if (!gotInstallation)
    {
    std::cout << "Installation step missing." << std::endl;
    return -1;
    }
  f.close();

  return 0;
}
