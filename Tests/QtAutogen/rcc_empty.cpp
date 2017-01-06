
extern int qInitResources_rcc_empty_resource();

int main(int, char**)
{
  // Fails to link if the symbol is not present.
  qInitResources_rcc_empty_resource();
  return 0;
}
