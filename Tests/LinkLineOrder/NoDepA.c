/* depends on NoDepB */
void NoDepB_func(void);

void NoDepA_func(void)
{
  NoDepB_func();
}
