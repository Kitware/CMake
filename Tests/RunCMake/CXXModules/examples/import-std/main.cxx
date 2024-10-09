import std;

int main(int argc, char* argv[])
{
  if (argc > 0 && argv[0]) {
    std::string argv0 = argv[0];
    std::cout << "program: " << argv0 << std::endl;
  }
  return 0;
}
