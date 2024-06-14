/* depends on NoDepY*/
void NoDepY_func(void);

void NoDepX_func(void)
{
  NoDepY_func();
}
