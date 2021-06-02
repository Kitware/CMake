#ifdef _WIN32
#  define DLLIMPORT __declspec(dllimport)
#else
#  define DLLIMPORT
#endif

DLLIMPORT extern void dep1(void);
DLLIMPORT extern void dep2(void);
DLLIMPORT extern void dep3(void);
DLLIMPORT extern void dep4(void);
DLLIMPORT extern void dep5(void);
DLLIMPORT extern void dep10(void);
DLLIMPORT extern void dep11(void);
DLLIMPORT extern void dep12(void);
DLLIMPORT extern void sublib1(void);
DLLIMPORT extern void sublib2(void);

int main(void)
{
  dep1();
  dep2();
  dep3();
  dep4();
  dep5();
  dep10();
  dep11();
  dep12();
  sublib1();
  sublib2();
  return 0;
}
