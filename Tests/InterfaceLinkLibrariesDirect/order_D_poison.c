extern void order_D_poison(void);

void order_A(void)
{
  order_D_poison();
}

void order_B(void)
{
  order_D_poison();
}

void order_C(void)
{
  order_D_poison();
}
