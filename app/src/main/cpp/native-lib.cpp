#include <jni.h>
#include <string>
#include <CL/cl_ext.h>
#include "OCL_Common/OCL_Init.h"
#include "OCL_Common/Context.h"
#include "OCL_Common/Kernel.h"
#include "OCL_Common/Queue.h"
#include "OCL_Common/Buffer.h"
#include "OclConversion.h"
#include "log.h"

OclConversion *oclConv = nullptr;

const char *program = R"===(
__constant char hello[24] = {'H','e','l','l','o',' ','w','o','r','l','d',' ','f','r','o','m',' ','O','p','e','n','C','l','\0'};
__kernel void hello_opencl(__global char *dst){
    uint gid = get_global_id(0);
    if(gid <= 24){
        dst[gid] = hello[gid];
    }
}
)===";

extern "C"
JNIEXPORT void JNICALL
Java_com_example_myapplication_MainActivity_closeOCL(JNIEnv *env, jobject thiz) {
    if(oclConv != nullptr){
        delete oclConv;
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_myapplication_MainActivity_initOCL(JNIEnv *env, jobject thiz) {
    if(oclConv == nullptr){
        oclConv = new OclConversion();
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_myapplication_MainActivity_convertToRGBOCL(JNIEnv *env, jclass clazz,
                                                            jbyteArray rgb, jbyteArray yuv, jint width,
                                                            jint height) {
    oclConv->dst = env->GetByteArrayElements(rgb, 0);
    oclConv->src = env->GetByteArrayElements(yuv, 0);
    if(!oclConv->initialized){
        int sizeDst = env->GetArrayLength(rgb);
        int sizeSrc = env->GetArrayLength(yuv);
        oclConv->initialize(sizeSrc, sizeDst, width, height);
    }
    oclConv->srcBuffer->toDevice(oclConv->queue.get(),oclConv->src);
    oclConv->queue->runKernel(oclConv->kernel.get());
    oclConv->dstBuffer->toHost(oclConv->queue.get(), oclConv->dst);
    oclConv->queue->waitForExecutionFinish();
    env->ReleaseByteArrayElements(rgb, oclConv->dst, 0);
    env->ReleaseByteArrayElements(yuv, oclConv->src, 0);
}