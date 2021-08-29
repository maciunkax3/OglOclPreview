#pragma once

#include "OCL_Common/Kernel.h"
#include "OCL_Common/OCL_Init.h"
#include "../../../../../../Android/Sdk/ndk/21.4.7075529/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/include/c++/v1/memory"
#include "OCL_Common/Queue.h"
#include "log.h"
#include <jni.h>
class OclConversion {
public:
    OclConversion(jint texture_id, jlong dis, jlong ctx);
    void initialize(size_t sizeSrc, int width, int height);
    bool initialized = false;
    std::unique_ptr<OCL::Kernel> kernel;
    std::unique_ptr<OCL::Kernel> kernelGrayScale;
    std::unique_ptr<OCL::Kernel> kernelMaxRgb;
    jbyte *dst;
    jbyte *src;
    std::unique_ptr<OCL::Buffer> dstBuffer;
    std::unique_ptr<OCL::Buffer> srcBuffer;
    std::unique_ptr<OCL::Queue> queue;
    std::unique_ptr<OCL::Runtime> runtime;
    cl_mem imageObj = nullptr;
    cl_mem imageTmp = nullptr;

};