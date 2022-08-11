#include <cassert>

#include <jni.h>

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
  void* tmp;
  if (vm->GetEnv(&tmp, JNI_VERSION_1_2) != JNI_OK) {
    return -1;
  }

  // The following lines do:
  //
  //    System.out.println("FindJNI");
  //
  JNIEnv* env = static_cast<JNIEnv*>(tmp);

  jclass clzS = env->FindClass("java/lang/System");
  jclass clzP = env->FindClass("java/io/PrintStream");

  jfieldID outF = env->GetStaticFieldID(clzS, "out", "Ljava/io/PrintStream;");
  jobject out = env->GetStaticObjectField(clzS, outF);
  jmethodID println =
    env->GetMethodID(clzP, "println", "(Ljava/lang/String;)V");

  env->CallVoidMethod(out, println, env->NewStringUTF("FindJNI"));

  return JNI_VERSION_1_2;
}

extern "C" JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved)
{
  void* env;
  jint result = vm->GetEnv(&env, JNI_VERSION_1_2);
  assert(result == JNI_OK);

  assert(static_cast<JNIEnv*>(env)->GetVersion() == JNI_VERSION_1_2);
}
