
int curand_main();
int nppif_main();

int mixed_version()
{
  return curand_main() == 0 && nppif_main() == 0;
}
