#ifdef _WIN32
#error bla
#define DL_EXPORT __declspec( dllexport )
#else
#define DL_EXPORT
#endif

DL_EXPORT int TestDynamicLoaderData;

DL_EXPORT void TestDynamicLoaderFunction()
{
}
