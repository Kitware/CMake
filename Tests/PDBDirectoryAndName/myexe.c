extern int mylibA(void);
extern int mylibB(void);
extern int mylibC(void);
extern int mylibD(void);
int main(void)
{
  return mylibA() + mylibB() + mylibC() + mylibD();
}
