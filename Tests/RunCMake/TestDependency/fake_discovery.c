#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
  if (argc >= 2 && strcmp(argv[1], "--list_tests") == 0) {
    puts("case_foo,label_one");
    puts("case_bar,label_two");
    return 0;
  }

  return 0;
}
