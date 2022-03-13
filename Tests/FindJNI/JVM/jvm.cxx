#include <jni.h>

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
  jsize n = 0;
  if (JNI_GetCreatedJavaVMs(nullptr, 0, &n) != JNI_OK || n <= 0) {
    return -1;
  }

  return JNI_VERSION_1_2;
}

extern "C" JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved)
{
}
