// A static library is neither a DLL nor a console target.
#if defined(__DLL__) || defined(__CONSOLE__)
#  error "static library compiled with __DLL__ or __CONSOLE__"
#endif
std::string lib_value()
{
  return "lib";
}
