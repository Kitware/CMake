int main () {
  /* Are we little or big endian?  From Harbison&Steele.  */
  union
  {
    long l;
    char c[sizeof (long)];
  } u;
  u.l = 1;
  return (u.c[sizeof (long) - 1] == 1)?1:0;
}
