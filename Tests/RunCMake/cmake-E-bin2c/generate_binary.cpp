#include <fstream>
#include <ios>
#include <string>

static unsigned char const long_contents[] = {
#include "long.c.txt"
};

int main(int argc, char** argv)
{
  if (argc != 3) {
    return 1;
  }
  std::string name = argv[1];
  std::ofstream fout(argv[2], std::ios::out | std::ios::binary);

  if (name == "basic") {
    fout.write("\xFC\xFD\xFE\xFF\x00\x01\x02\x03", 8);
  } else if (name == "empty") {
    // Write nothing
  } else if (name == "text_lf") {
    fout << "All work and no play makes Jack a dull boy.\n";
  } else if (name == "text_crlf") {
    fout << "All work and no play makes Jack a dull boy.\r\n";
  } else if (name == "text_align") {
    fout << "This exactly 32 characters long!";
  } else if (name == "long") {
    fout.write(reinterpret_cast<char const*>(long_contents),
               sizeof(long_contents));
  }

  fout.flush();
  fout.close();
  return 0;
}
