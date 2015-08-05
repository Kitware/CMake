
#include <jni.h>
#include <iostream>

#include "B.h"

JNIEXPORT void JNICALL Java_B_printName
(JNIEnv *, jobject)
{
    std::cout << "B" << std::endl;
}
