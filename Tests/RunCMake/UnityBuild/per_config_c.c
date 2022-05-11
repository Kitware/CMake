#ifdef CFG_DEBUG
extern void per_config_c_debug(void);
#endif
#ifdef CFG_OTHER
extern void per_config_c_other(void);
#endif
int main(void)
{
#ifdef CFG_DEBUG
  per_config_c_debug();
#endif
#ifdef CFG_OTHER
  per_config_c_other();
#endif
  return 0;
}
