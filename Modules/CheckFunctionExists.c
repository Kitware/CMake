#ifdef CHECK_FUNCTION_EXISTS

char CHECK_FUNCTION_EXISTS();

int main()
{
  CHECK_FUNCTION_EXISTS();
  return 0;
}

#else  /* CHECK_FUNCTION_EXISTS */

#  error "CHECK_FUNCTION_EXISTS has to specify the function"

#endif /* CHECK_FUNCTION_EXISTS */
