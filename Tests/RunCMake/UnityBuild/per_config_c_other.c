#ifdef CFG_DEBUG
#  error "per_config_c_other built with CFG_DEBUG"
#endif
#ifndef CFG_OTHER
#  error "per_config_c_other built without CFG_OTHER"
#endif
void per_config_c_other(void)
{
}
