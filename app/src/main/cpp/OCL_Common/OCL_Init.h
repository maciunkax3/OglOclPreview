#pragma once
#include <CL/cl.h>
#include <memory>
#include "Queue.h"

namespace OCL{
    class Context;
    class Runtime{
    public:
        Runtime(int &status);
        uint64_t getKernelExecutionTime(cl_event event);
        cl_platform_id cpPlatform;
        cl_device_id device_id;
        std::unique_ptr<Context> context;
    };
}