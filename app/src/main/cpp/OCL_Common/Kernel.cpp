#include "Kernel.h"
#include "Context.h"
#include "log.h"

namespace OCL{

    Kernel::Kernel(Context *context, const char* source, const char* kernelName, const char* options) {
        cl_int err =-1;
        ctx = context->context;
        program = clCreateProgramWithSource(context->context, 1, (const char **) &source, NULL, &err);
        LOGI("ref of clCreateProgramWithSource:%d\n",err);

        // Build the program executable
        err = clBuildProgram(program, 0, NULL, options, NULL, NULL);
        LOGI("ref of clBuildProgram:%d\n",err);
        // Create the compute kernel in the program we wish to run
        kernel = clCreateKernel(program, kernelName, &err);
        LOGI("ref of clCreateKernel:%d\n",err);

    }

    Kernel::~Kernel() {
        clReleaseProgram(program);
        clReleaseKernel(kernel);
        if(event){
            clReleaseEvent(event);
        }
    }

    void Kernel::createEvent() {
        event = clCreateUserEvent(ctx, NULL);
    }
}