#pragma once
#include <CL/cl.h>

namespace OCL{
    class Context{
    public:
       Context(cl_platform_id &cpPlatform, cl_device_id &device_id);
       Context(cl_platform_id &cpPlatform, cl_device_id &device_id, cl_context_properties *props);
       ~Context();
        cl_context context;
    };
}
