#include "rot13.h"

void rot13(char* in)
{
  char* end = in + strlen(in);
  for (char* c = in; c < end; c++) {
    if (*c >= 'a' && *c <= 'z') {
      *c += (*c < 'n') ? 13 : -13;
      continue;
    }
    if (*c >= 'A' && *c <= 'Z') {
      *c += (*c < 'N') ? 13 : -13;
    }
  }
}
