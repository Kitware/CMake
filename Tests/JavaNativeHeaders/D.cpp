
#include "D.h"

#include <jni.h>
#include <stdio.h>

JNIEXPORT void JNICALL Java_D_printName(JNIEnv*, jobject)
{
  printf("D\n");
}
