#include <stdio.h>

int main(int argc, char* argv[])
{
  int ret = 0;
  char const* fname = TARGET_DIRECTORY "/PdbCompileFileName.pdb";
  FILE* f = fopen(fname, "r");
  if (f) {
    fclose(f);
  } else {
    printf("Failed to open PDB file '%s'\n", fname);
    ret = 1;
  }
  return ret;
}
