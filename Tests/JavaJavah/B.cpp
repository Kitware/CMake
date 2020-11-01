
#include "B.h"

#include <jni.h>
#include <stdio.h>

JNIEXPORT void JNICALL Java_B_printName(JNIEnv*, jobject)
{
  printf("B\n");
}
