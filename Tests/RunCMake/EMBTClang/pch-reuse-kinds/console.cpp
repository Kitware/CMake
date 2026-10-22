// A console executable must be compiled with __CONSOLE__ and without __DLL__.
#if !defined(__CONSOLE__) || defined(__DLL__)
#  error "console target compiled with the wrong __CONSOLE__/__DLL__ state"
#endif
int main()
{
  return static_cast<int>(std::string().size());
}
