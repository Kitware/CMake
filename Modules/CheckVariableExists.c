#ifdef CHECK_VARIABLE_EXISTS

extern int CHECK_VARIABLE_EXISTS;

int main()
{
  int* p;
  p = &CHECK_VARIABLE_EXISTS;
  return 0;
}

#else  /* CHECK_VARIABLE_EXISTS */

#  error "CHECK_VARIABLE_EXISTS has to specify the variable"

#endif /* CHECK_VARIABLE_EXISTS */
