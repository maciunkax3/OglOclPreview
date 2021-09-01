#pragma once

#include "OCL_Common/Kernel.h"
#include "OCL_Common/OCL_Init.h"
#include <memory>
#include "OCL_Common/Queue.h"
#include "log.h"
#include <jni.h>
class OclConversion {
public:
    OclConversion();
    void initialize(size_t sizeSrc, size_t sizeDst, int width, int height);
    bool initialized = false;
    std::unique_ptr<OCL::Kernel> kernel;
    jbyte *dst;
    jbyte *src;
    std::unique_ptr<OCL::Buffer> dstBuffer;
    std::unique_ptr<OCL::Buffer> srcBuffer;
    std::unique_ptr<OCL::Queue> queue;
    std::unique_ptr<OCL::Runtime> runtime;

};