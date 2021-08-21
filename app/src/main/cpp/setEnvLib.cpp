#include <jni.h>
#include <string>

extern "C" JNIEXPORT void JNICALL
Java_com_example_oclbenchark_MainActivity_setenv(JNIEnv *env, jclass clazz, jstring key, jstring value) {
    setenv((char *)env->GetStringUTFChars(key, 0),
           (char *)env->GetStringUTFChars(value, 0), 1);
}