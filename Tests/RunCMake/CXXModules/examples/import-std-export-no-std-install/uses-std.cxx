module uses_std;
import std;

int f()
{
  std::string str = "hello!";
  std::cout << "program: " << str << std::endl;
  return 0;
}
