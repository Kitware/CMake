import importable;

extern int unrelated();

int main(int argc, char* argv[])
{
  return from_import() + unrelated();
}
