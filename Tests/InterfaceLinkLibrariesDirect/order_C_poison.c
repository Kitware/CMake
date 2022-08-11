extern void order_C_poison(void);

void order_A(void)
{
  order_C_poison();
}

void order_B(void)
{
  order_C_poison();
}
