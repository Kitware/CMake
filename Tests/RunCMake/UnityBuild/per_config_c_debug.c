#ifndef CFG_DEBUG
#  error "per_config_c_debug built without CFG_DEBUG"
#endif
#ifdef CFG_OTHER
#  error "per_config_c_debug built with CFG_OTHER"
#endif
void per_config_c_debug(void)
{
}
