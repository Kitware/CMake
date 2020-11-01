
#include "C.h"

#include <jni.h>
#include <stdio.h>

JNIEXPORT void JNICALL Java_C_printName(JNIEnv*, jobject)
{
  printf("C\n");
}
