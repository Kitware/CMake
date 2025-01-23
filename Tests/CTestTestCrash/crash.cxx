// causes a segfault
int main()
{
  int volatile* ptr = 0;
  *ptr = 1;
}
