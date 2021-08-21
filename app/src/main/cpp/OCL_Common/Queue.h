#pragma once
#include <CL/cl.h>
#include "Buffer.h"

namespace OCL{
    class Runtime;
    class Kernel;
    class Queue {
    public:
        Queue(Runtime *runtime);
        ~Queue();
        void runKernel(Kernel *kernel);
        void flushKernels();
        void waitForExecutionFinish();
        void copyBufferToBuffer(Buffer *src, Buffer *dst, size_t size, cl_event *event);
        cl_command_queue queue;
    };
}