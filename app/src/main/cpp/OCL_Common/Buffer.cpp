#include "Buffer.h"
#include "Context.h"
#include "Queue.h"
#include "log.h"

constexpr size_t alignment = 0x1000;

namespace OCL {
    Buffer::Buffer(Context *context, size_t size, void * userHostPtr) {
        ctx = context->context;
        this->size = size;
        cl_int err= -1;
        if(userHostPtr){
            hostPtr = userHostPtr;
            memObj = clCreateBuffer(context->context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, size, hostPtr, &err);
        } else{
            memObj = clCreateBuffer(context->context, CL_MEM_READ_WRITE , size, nullptr, &err);
        }
        LOGI("ref of clBufferCreate:%d\n", err);
    }

    Buffer::~Buffer() {
        clReleaseMemObject(memObj);
    }

    void *Buffer::getPtr() {
        return hostPtr;
    }

    void Buffer::toDevice(Queue *queue, void* userHostPtr) {
        clEnqueueWriteBuffer(queue->queue, memObj,CL_FALSE, 0, size, userHostPtr == nullptr ? hostPtr : userHostPtr, 0, NULL, NULL);
    }

    void Buffer::toHost(Queue *queue, void* userHostPtr) {
        clEnqueueReadBuffer(queue->queue, memObj, CL_FALSE, 0, size, userHostPtr == nullptr ? hostPtr : userHostPtr, 0, NULL, NULL);
    }
}