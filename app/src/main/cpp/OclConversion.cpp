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
const char *rgbMax = R"===(
__kernel void imageRgbMax(__write_only image2d_t dst ,__read_only image2d_t src){
    int gidX = get_global_id(0);
    int gidY = get_global_id(1);
    const int2 coord = (int2)(gidX, gidY);
    const float4 pixel = read_imagef(src, coord);

    float max = pixel.x;
    if(max < pixel.y){
        max = pixel.y;
    }
    if(max < pixel.z){
        max = pixel.z;
    }

    if(pixel.x < max)
        pixel.x = 0.0;
    if(pixel.y < max)
        pixel.y = 0.0;
    if(pixel.z < max)
        pixel.z = 0.0;
    write_imagef(dst, coord, pixel);
}
)===";
const char *blackWhite = R"===(
__kernel void blackWhite(__write_only image2d_t dst ,__read_only image2d_t src){
    int gidX = get_global_id(0);
    int gidY = get_global_id(1);
    const int2 coord = (int2)(gidX, gidY);
    const float4 pixel = read_imagef(src, coord);
    float value = (pixel.x + pixel.y + pixel.z) / 3;
    write_imagef(dst, coord, (float4)(value, value, value, 1.0));
}
)===";
const char *avgFilter = R"===(
__kernel void avgFilter(__write_only image2d_t dst ,__read_only image2d_t src){
    int gidX = get_global_id(0);
    int gidY = get_global_id(1);
    int width = get_image_width(src);
    int height = get_image_height(src);
    float4 value = (float4)(0.0, 0.0, 0.0, 0.0);
    for (int i = -2; i< 3;i++) {
        for(int j = -2;j < 3; j++){
            if(gidX + j < 0 || gidX + j > width)
                continue;
            if(gidY + i < 0 || gidY + i > height)
                continue;
            value += read_imagef(src, (int2)(gidX + j, gidY + i));
        }
    }
    write_imagef(dst, (int2)(gidX, gidY), (float4)(value.x/25, value.y/25, value.z/25, 1.0));
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
    imageObj = clCreateFromGLTexture(runtime->context->context,
                                     CL_MEM_READ_WRITE,
                                     GL_TEXTURE_2D,
                                     0,
                                     texture_id,
                                     &status);
    LOGI("Image From Texture: %d", status);
    queue = std::make_unique<OCL::Queue>(runtime.get());
    kernel = std::make_unique<OCL::Kernel>(runtime->context.get(), convertNV21ToRGBImage,
                                           "convertNV21ToRGBImage",
                                           nullptr);
    kernelGrayScale = std::make_unique<OCL::Kernel>(runtime->context.get(), blackWhite,
                                                                                               "blackWhite",
                                                                                               nullptr);
    kernelMaxRgb = std::make_unique<OCL::Kernel>(runtime->context.get(), rgbMax,
                                           "imageRgbMax",
                                          nullptr);
    kernelAvg = std::make_unique<OCL::Kernel>(runtime->context.get(), avgFilter,
                                                 "avgFilter",
                                                 nullptr);
    size_t width = 0;
    size_t height = 0;
    clGetImageInfo(
            imageObj,
            CL_IMAGE_WIDTH,
            sizeof(size_t),
            &width,
            nullptr);
    clGetImageInfo(
            imageObj,
            CL_IMAGE_HEIGHT,
            sizeof(size_t),
            &height,
            nullptr);
    // Create OpenCL image
    cl_image_format clImageFormat;
    clImageFormat.image_channel_order = CL_RGBA;
    clImageFormat.image_channel_data_type = CL_UNORM_INT8;

    cl_int errNum;
    cl_mem clImage;

    // New in OpenCL 1.2, need to create image descriptor.
    cl_image_desc clImageDesc;
    clImageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    clImageDesc.image_width = width;
    clImageDesc.image_height = height;
    clImageDesc.image_row_pitch = 0;
    clImageDesc.image_slice_pitch = 0;
    clImageDesc.num_mip_levels = 0;
    clImageDesc.num_samples = 0;
    clImageDesc.buffer = NULL;

    imageTmp = clCreateImage(runtime->context->context, CL_MEM_READ_WRITE,&clImageFormat, &clImageDesc, nullptr, &status);
}

void OclConversion::initialize(size_t sizeSrc, int width, int height) {
    srcBuffer = std::make_unique<OCL::Buffer>(runtime->context.get(), sizeSrc, nullptr);
    srcBuffer->toDevice(queue.get(), src);
    kernel->setArg<cl_mem>(0, &imageObj);
    kernel->setArg<cl_mem>(1, &srcBuffer->memObj);
    kernel->setArg<cl_int>(2, &width);
    kernel->setArg<cl_int>(3, &height);
    kernelGrayScale->setArg<cl_mem>(0, &imageObj);
    kernelGrayScale->setArg<cl_mem>(1, &imageTmp);
    kernelMaxRgb->setArg<cl_mem>(0, &imageObj);
    kernelMaxRgb->setArg<cl_mem>(1, &imageTmp);
    kernelAvg->setArg<cl_mem>(0, &imageObj);
    kernelAvg->setArg<cl_mem>(1, &imageTmp);
    kernel->gws[0] = width;
    kernel->gws[1] = height;
    kernel->lws[0] = runtime->maxWG;
    kernel->dims = 2;


    kernelGrayScale->gws[0] = width;
    kernelGrayScale->gws[1] = height;
    kernelGrayScale->lws[0] = runtime->maxWG;
    kernelGrayScale->dims = 2;


    kernelMaxRgb->gws[0] = width;
    kernelMaxRgb->gws[1] = height;
    kernelMaxRgb->lws[0] = runtime->maxWG;
    kernelMaxRgb->dims = 2;


    kernelAvg->gws[0] = width;
    kernelAvg->gws[1] = height;
    kernelAvg->lws[0] = runtime->maxWG;
    kernelAvg->dims = 2;
    initialized = true;
}