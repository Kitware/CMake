/* depends on NoDepA */
void NoDepA_func(void);

void NoDepC_func(void)
{
  NoDepA_func();
}
