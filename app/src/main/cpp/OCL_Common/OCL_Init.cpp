#include "OCL_Init.h"
#include "log.h"
#include "Context.h"
#include "Kernel.h"

namespace OCL{
    Runtime::Runtime(int &status){
        cl_int err = -1;
        cl_uint platformsCount;

        err = clGetPlatformIDs(1, &cpPlatform, NULL);
        LOGI("ref of clGetPlatformIDs:%d\n",err);
        if(err != 0){
            status = -1;
            return;
        }
        // Get ID for the device
        err = clGetDeviceIDs(cpPlatform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
        LOGI("ref of clGetDeviceIDs:%d\n",err);
        if(err != 0){
            status = -1;
            return;
        }
        context = std::make_unique<Context>(cpPlatform, device_id);
    }

    uint64_t Runtime::getKernelExecutionTime(cl_event event) {
        uint64_t start = 0;
        uint64_t end = 0;
        size_t copySize;
        clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(start), &start, &copySize);
        LOGI("ref of Profiling Info start:%d\n",start);
        clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(end), &end, &copySize);
        LOGI("ref of Profiling Info end:%d\n",end);

        return end - start;
    }
}