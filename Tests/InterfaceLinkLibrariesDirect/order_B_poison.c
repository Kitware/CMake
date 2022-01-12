extern void order_B_poison(void);

void order_A(void)
{
  order_B_poison();
}
