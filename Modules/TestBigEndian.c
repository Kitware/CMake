int main (int ac, char*av[]) {
  int ret = 1;
  /* Are we little or big endian?  From Harbison&Steele.  */
  union
  {
    long l;
    char c[sizeof (long)];
  } u;
  u.l = 1;
  if(ac > 100)
    {
    ret = *av[0];
    }
  return (u.c[sizeof (long) - 1] == 1)?ret:0;
}
