/* depends on NoDepC and NoDepE (and hence on NoDepA, NoDepB and */
/*  NoDepF) */
void NoDepC_func(void);
void NoDepE_func(void);

void OneFunc(void)
{
  NoDepC_func();
  NoDepE_func();
}
