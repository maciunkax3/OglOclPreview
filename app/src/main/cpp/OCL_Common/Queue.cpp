#include "Queue.h"
#include "Context.h"
#include "Kernel.h"
#include "OCL_Init.h"
#include "log.h"

namespace OCL{
    Queue::Queue(Runtime *runtime){
        cl_int err = -1;
        queue = clCreateCommandQueue(runtime->context->context, runtime->device_id, CL_QUEUE_PROFILING_ENABLE, &err);
        LOGI("ref of clCreateCommandQueue:%d\n",err);
    }

    Queue::~Queue() {
        clReleaseCommandQueue(queue);
    }

    void Queue::runKernel(Kernel *kernel) {
        clEnqueueNDRangeKernel(queue, kernel->kernel, kernel->dims, NULL, kernel->gws, nullptr, 0, nullptr, kernel->event ? &kernel->event :nullptr);
    }

    void Queue::flushKernels() {
        clFlush(queue);
    }

    void Queue::waitForExecutionFinish() {
        clFinish(queue);
    }
    void Queue::copyBufferToBuffer(Buffer *src, Buffer *dst, size_t size, cl_event *event) {
        clEnqueueCopyBuffer(queue, src->memObj, dst->memObj, 0, 0, size, 0, nullptr, event);
    }
}