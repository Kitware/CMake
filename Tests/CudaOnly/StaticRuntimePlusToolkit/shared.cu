
int curand_main();
int nppif_main();

int shared_version()
{
  return curand_main() == 0 && nppif_main() == 0;
}
