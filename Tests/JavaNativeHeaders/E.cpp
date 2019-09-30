
#include "E.h"

#include <jni.h>
#include <stdio.h>

JNIEXPORT void JNICALL Java_E_printName(JNIEnv*, jobject)
{
  printf("E\n");
}
