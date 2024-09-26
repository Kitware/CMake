extern int BasicB(void);
extern int BasicC(void);

/* Use a symbol provided by a dedicated object file in A, B, and C.
   The first library linked will determine the return value.  */
extern int BasicX(void);

int main(void)
{
  return BasicB() + BasicC() + BasicX();
}
