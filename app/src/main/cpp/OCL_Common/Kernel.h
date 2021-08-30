#pragma once
#include <CL/cl.h>
#include <string>

namespace OCL{
    class Context;
    class Kernel {
    public:
        Kernel(Context *context, const char *source, const char *kernelName, const char* options);
        ~Kernel();
        template <typename T>
        void setArg(uint32_t index, void* argVal){
            clSetKernelArg(kernel, index, sizeof(T), argVal);
        }
        void createEvent();
        cl_program program = nullptr;
        cl_event event = nullptr;
        cl_kernel kernel = nullptr;
        cl_context ctx = nullptr;
        size_t gws[3]={1,1,1};
        size_t lws[3]={1,1,1};
        cl_uint dims = 1;
        size_t maxWG = 0;
    };
}