#include <fstream>
#include <iostream>
#include <stdio.h>

int main ()
{
  int result = 0;

  // parse the dart test file
  std::ifstream fin("UndefinedProperties.txt");
  if(!fin)
    {
    fprintf(stderr,"failed to find undefined properties file");
    return 1;
    }

  char buffer[1024];
  while ( fin )
    {
    buffer[0] = 0;
    fin.getline(buffer, 1023);
    buffer[1023] = 0;
    std::string line = buffer;
    if(line.size() && line.find("with scope VARIABLE") == std::string::npos)
      {
      fprintf(stderr, "%s\n", line.c_str());
      result = 1;
      }
    }
  fin.close();

  return result;
}
