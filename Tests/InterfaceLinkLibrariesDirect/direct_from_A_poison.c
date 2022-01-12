extern void poison_not_direct_from_A(void);
void not_direct_from_A(void)
{
  poison_not_direct_from_A();
}
