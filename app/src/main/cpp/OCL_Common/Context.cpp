#include "Context.h"
#include "log.h"

namespace OCL{
    Context::Context(cl_platform_id &cpPlatform, cl_device_id &device_id) {
        cl_int err = -1;
        context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
        LOGI("ref of clCreateContext:%d\n",err);
    }
    Context::Context(cl_platform_id &cpPlatform, cl_device_id &device_id, cl_context_properties *props) {
        cl_int err = -1;
        context = clCreateContext(props, 1, &device_id, NULL, NULL, &err);
        LOGI("ref of clCreateContext:%d\n",err);
    }
    Context::~Context(){
        clReleaseContext(context);
    }
}