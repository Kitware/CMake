// causes a segfault
int main()
{
  volatile int* ptr = 0;
  *ptr = 1;
}
