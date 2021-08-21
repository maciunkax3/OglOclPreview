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
        err = clGetDeviceInfo(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(numComputeUnits), &numComputeUnits, nullptr);
        LOGI("ref of clGetDeviceInfo:%d\n",err);
        if(err != 0){
            status = -1;
            return;
        }
        err = clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(maxWG), &maxWG, nullptr);
        LOGI("ref of clGetDeviceInfo:%d\n",err);
        if(err != 0){
            status = -1;
            return;
        }
        size_t extSize;
        err = clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, 0, nullptr, &extSize);
        auto extArray = std::make_unique<char[]>(extSize);
        err = clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, extSize, extArray.get(), nullptr);
        LOGI("ref of clGetDeviceInfo:%d\n",err);
        if(err != 0){
            status = -1;
            return;
        }
        std::string extensionsString(extArray.get());
        supportDP = extensionsString.find("cl_khr_fp64") != std::string::npos;
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