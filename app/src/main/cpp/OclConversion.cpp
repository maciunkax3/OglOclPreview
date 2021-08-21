#include "OclConversion.h"
#include "OCL_Common/OCL_Init.h"
#include "OCL_Common/Context.h"
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <libopencl-stub/include/CL/cl_gl.h>

const char *convertNV21ToRGBImage = R"===(
__kernel void convertNV21ToRGBImage(__write_only image2d_t img , __global char *yuv, int width, int height){
    int gidX = get_global_id(0);
    int gidY = get_global_id(1);
    int index = gidX + (width * gidY);
    int R, G, B;
    int total = width * height;
    int Y, Cb = 0, Cr = 0;
    Y = (int)yuv[gidY * width + gidX];
    if (Y < 0) Y += 255;
    if ((gidX & 1) == 0) {
        Cr = (int)yuv[(gidY >> 1) * (width) + gidX + total];
        Cb = (int)yuv[(gidY >> 1) * (width) + gidX + total + 1];
        if (Cb < 0) Cb += 127; else Cb -= 128;
        if (Cr < 0) Cr += 127; else Cr -= 128;
    }
    R = Y + Cr + (Cr >> 2) + (Cr >> 3) + (Cr >> 5);
    G = Y - (Cb >> 2) + (Cb >> 4) + (Cb >> 5) - (Cr >> 1) + (Cr >> 3) + (Cr >> 4) + (Cr >> 5);
    B = Y + Cb + (Cb >> 1) + (Cb >> 2) + (Cb >> 6);
    if (R < 0){
        R = 0;
    } else if (R > 255){
        R = 255;
    }
    if (G < 0){
        G = 0;
    } else if (G > 255){
        G = 255;
    }
    if (B < 0){
        B = 0;
    } else if (B > 255){
        B = 255;
    }

    if((R > 0 && R <125) && (G > 125 && G <255) && (B > 0 && B <125)){
        R = 0; G = 0; B = 0;
    }
    write_imagef(img, (int2)(gidX, gidY), (float4)(((float)(R)/ (float)(255)), ((float)(G)/ (float)(255)), ((float)(B)/ (float)(255)), 1.0));
    //write_imagef(img, (int2)(gidX, gidY), (float4)(0.1, 0.2, 0.3, 0.4));

}
)===";
const char *programNV21ToRgb = R"===(
__kernel void convertNV21ToRGB(__global char *rgb, __global char *yuv, int width, int height){
    int gidX = get_global_id(0);
    int gidY = get_global_id(1);
    int index = gidX + (width * gidY);
    int R, G, B;
    int total = width * height;
    int Y, Cb = 0, Cr = 0;
    Y = (int)yuv[gidY * width + gidX];
    if (Y < 0) Y += 255;
    if ((gidX & 1) == 0) {
        Cr = (int)yuv[(gidY >> 1) * (width) + gidX + total];
        Cb = (int)yuv[(gidY >> 1) * (width) + gidX + total + 1];
        if (Cb < 0) Cb += 127; else Cb -= 128;
        if (Cr < 0) Cr += 127; else Cr -= 128;
    }
    R = Y + Cr + (Cr >> 2) + (Cr >> 3) + (Cr >> 5);
    G = Y - (Cb >> 2) + (Cb >> 4) + (Cb >> 5) - (Cr >> 1) + (Cr >> 3) + (Cr >> 4) + (Cr >> 5);
    B = Y + Cb + (Cb >> 1) + (Cb >> 2) + (Cb >> 6);
    if (R < 0){
        R = 0;
    } else if (R > 255){
        R = 255;
    }
    if (G < 0){
        G = 0;
    } else if (G > 255){
        G = 255;
    }
    if (B < 0){
        B = 0;
    } else if (B > 255){
        B = 255;
    }
    rgb[(4 * index)+0] = ((char)(R));
    rgb[(4 * index)+1] = ((char)(G));
    rgb[(4 * index)+2] = ((char)(B));
    rgb[(4 * index)+3] = ((char)(255));

}
)===";

OclConversion::OclConversion(jint texture_id, jlong dis, jlong ctx) {
    int status = 0;
    runtime = std::make_unique<OCL::Runtime>(status);
    cl_context_properties props[] =
            {CL_GL_CONTEXT_KHR, (cl_context_properties) ctx,
             CL_EGL_DISPLAY_KHR, (cl_context_properties) dis,
             CL_CONTEXT_PLATFORM, 0,
             0};
    runtime->context.reset(new OCL::Context(runtime->cpPlatform, runtime->device_id, props));
    imageObj = clCreateFromGLTexture(	runtime->context->context,
                                         CL_MEM_WRITE_ONLY,
                                         GL_TEXTURE_2D,
                                         0,
                                         texture_id,
                                         &status);
    LOGI("Image From Texture: %d", status);
    queue = std::make_unique<OCL::Queue>(runtime.get());
    kernel = std::make_unique<OCL::Kernel>(runtime->context.get(), convertNV21ToRGBImage,
                                           "convertNV21ToRGBImage",
                                           nullptr);
}

void OclConversion::initialize(size_t sizeSrc, int width, int height) {
    srcBuffer = std::make_unique<OCL::Buffer>(runtime->context.get(), sizeSrc, nullptr);
    srcBuffer->toDevice(queue.get(), src);
    kernel->setArg<cl_mem>(0, &imageObj);
    kernel->setArg<cl_mem>(1, &srcBuffer->memObj);
    kernel->setArg<cl_int>(2, &width);
    kernel->setArg<cl_int>(3, &height);
    kernel->gws[0] = width;
    kernel->gws[1] = height;
    kernel->lws[0] = runtime->maxWG;
    kernel->dims = 2;
    initialized = true;
}