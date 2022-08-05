extern void poison_not_direct_from_A_for_exe(void);
void not_direct_from_A_for_exe(void)
{
  poison_not_direct_from_A_for_exe();
}
