#include <jawt.h>

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
  void* tmp;

  if (vm->GetEnv(&tmp, JNI_VERSION_1_2) != JNI_OK) {
    return -1;
  }

  JAWT awt;
  awt.version = JAWT_VERSION_1_3;

  if (JAWT_GetAWT(static_cast<JNIEnv*>(tmp), &awt) == JNI_FALSE) {
    return -1;
  }

  return JNI_VERSION_1_2;
}

extern "C" JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved)
{
}
