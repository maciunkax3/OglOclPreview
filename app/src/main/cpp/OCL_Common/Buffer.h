#pragma once
#include <CL/cl.h>
#include <string>

namespace OCL{
    class Context;
    class Queue;
    class Buffer {
    public:
        Buffer(Context *context, size_t size, void* userHostPtr);
        ~Buffer();
        void* getPtr();
        void toHost(Queue *queue, void* userHostPtr);
        void toDevice(Queue *queue, void* userHostPtr);
        void createEvent();
        
        cl_mem memObj;
        void* hostPtr;
        std::unique_ptr<uint8_t[]> hostMem;
        cl_context ctx;
        size_t size;
        cl_event event = nullptr;
    };
}