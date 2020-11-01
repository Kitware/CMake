
// Forward declaration
bool libStrict();
bool libRelaxed();

int main(int argv, char** args)
{
  return (libStrict() && libRelaxed()) ? 0 : -1;
}
