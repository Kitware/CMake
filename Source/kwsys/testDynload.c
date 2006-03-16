#ifdef _WIN32
#define DL_EXPORT __declspec( dllexport )
#else
#define DL_EXPORT
#endif

DL_EXPORT int TestDynamicLoaderData = 0;

DL_EXPORT void TestDynamicLoaderSymbolPointer()
{
}
