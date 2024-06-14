/* depends on NoDepX */
void NoDepX_func(void);

void NoDepZ_func(void)
{
  NoDepX_func();
}
