#ifdef CHECK_FUNCTION_EXISTS

char CHECK_FUNCTION_EXISTS();

int main(int ac, char*av[])
{
  int ret = 0;
  CHECK_FUNCTION_EXISTS();
  if(ac > 100)
    {
    ret = *av[0];
    }
  return ret;
}

#else  /* CHECK_FUNCTION_EXISTS */

#  error "CHECK_FUNCTION_EXISTS has to specify the function"

#endif /* CHECK_FUNCTION_EXISTS */
