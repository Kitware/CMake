#include <fstream>
#include <ios>

static unsigned char const contents[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
};

int main(int argc, char** argv)
{
  if (argc != 2) {
    return 1;
  }

  std::ofstream fout(argv[1], std::ios::out | std::ios::binary);
  fout.write(reinterpret_cast<char const*>(contents), sizeof(contents));

  return 0;
}
