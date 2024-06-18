#ifdef _WIN32
__declspec(dllimport)
#endif
  int sla(void);

int main(void)
{
  return sla();
}
