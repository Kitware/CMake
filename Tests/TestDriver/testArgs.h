void testProccessArgs(int* ac, char***av)
{
  if(*ac < 2)
    {
    return;
    }
  char** argv = *av;
  if(strcmp(argv[1], "--with-threads") == 0)
    {
    printf("number of threads is %s\n", argv[2]);
    *av +=2;
    *ac -=2;
    }
  else if (strcmp(argv[1], "--without-threads") == 0)
    {
    printf("no threads\n");
    *av += 1;
    *ac -= 1;
    }
}
