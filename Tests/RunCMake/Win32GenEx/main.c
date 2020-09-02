#ifdef USE_WIN32_MAIN
#  include <windows.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nShowCmd)
{
  return 0;
}
#else
int main(void)
{
  return 0;
}
#endif
