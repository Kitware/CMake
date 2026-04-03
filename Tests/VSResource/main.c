#include <windows.h>

#include <stdio.h>

extern int lib(void);

struct x
{
  char const* txt;
};

int main(int argc, char** argv)
{
  int ret = 1;
  HRSRC hello = FindResource(NULL, MAKEINTRESOURCE(1025), "TEXTFILE");

  fprintf(stdout, "CTEST_FULL_OUTPUT (Avoid ctest truncation of output)\n");

#ifdef CMAKE_RCDEFINE
  fprintf(stdout, "CMAKE_RCDEFINE defined\n");
#endif

#ifdef CMAKE_RCDEFINE_NO_QUOTED_STRINGS
  {
    // Expect CMAKE_RCDEFINE to preprocess to exactly test.txt
    struct x test;
    test.txt = "*exactly* test.txt";
    fprintf(stdout, "CMAKE_RCDEFINE_NO_QUOTED_STRINGS defined\n");
    fprintf(stdout, "CMAKE_RCDEFINE is %s, and is *not* a string constant\n",
            CMAKE_RCDEFINE);
  }
#else
  // Expect CMAKE_RCDEFINE to be a string:
  fprintf(stdout, "CMAKE_RCDEFINE='%s', and is a string constant\n",
          CMAKE_RCDEFINE);
#endif

  if (hello) {
    HGLOBAL hgbl = LoadResource(NULL, hello);
    int datasize = (int)SizeofResource(NULL, hello);
    fprintf(stdout, "FindResource worked\n");
    if (hgbl && datasize > 0) {
      void* data = LockResource(hgbl);
      fprintf(stdout, "LoadResource worked\n");
      fprintf(stdout, "SizeofResource returned datasize='%d'\n", datasize);
      if (data) {
        char* str = (char*)malloc(datasize + 4);
        fprintf(stdout, "LockResource worked\n");
        if (str) {
          memcpy(str, data, datasize);
          str[datasize] = 'E';
          str[datasize + 1] = 'O';
          str[datasize + 2] = 'R';
          str[datasize + 3] = 0;
          fprintf(stdout, "str='%s'\n", str);
          free(str);

          ret = 0;

#ifdef CMAKE_RCDEFINE_NO_QUOTED_STRINGS
          fprintf(stdout, "LoadString skipped\n");
#else
          {
            char buf[256];
            if (LoadString(NULL, 1026, buf, sizeof(buf)) > 0) {
              fprintf(stdout, "LoadString worked\n");
              fprintf(stdout, "buf='%s'\n", buf);
            } else {
              fprintf(stdout, "LoadString failed\n");
              ret = 1;
            }
          }
#endif
        }
      }
    }
  }

  return ret + lib();
}
