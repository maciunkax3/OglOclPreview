
#define CL_TARGET_OPENCL_VERSION 200

#include <jni.h>
#include <string>
#include <OCL_Common/Queue.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include "OCL_Common/OCL_Init.h"
#include "OCL_Common/Context.h"
#include "OCL_Common/Kernel.h"
#include "OCL_Common/convert_to_rgb_image.h"
#include "OCL_Common/convert_to_rgb.h"
#include "OCL_Common/median_filter.h"
#include "log.h"
#include <thread>
#include <vector>
#include <libopencl-stub/include/CL/cl_gl.h>

class OclSharing {
public:
    OclSharing(jint texture_id, jlong dis, jlong ctx){
        textureID = texture_id;
        int status = -1;
        runtime = std::make_unique<OCL::Runtime>(status);

        cl_context_properties props[] =
                {   CL_GL_CONTEXT_KHR,   (cl_context_properties) ctx,
                    CL_EGL_DISPLAY_KHR,  (cl_context_properties) dis,
                    CL_CONTEXT_PLATFORM, 0,
                    0 };
        runtime->context.reset(new OCL::Context(runtime->cpPlatform, runtime->device_id, props));

        imageObj = clCreateFromGLTexture(	runtime->context->context,
                                  CL_MEM_WRITE_ONLY,
                                  GL_TEXTURE_2D,
                                  0,
                                  texture_id,
                                  &status);

        if(status == 0){
            textureLoaded = true;
        }
        queue = std::make_unique<OCL::Queue>(runtime.get());
        kernelConvertToRGBImage = std::make_unique<OCL::Kernel>(runtime->context.get(), Kernels::convertNV21ToRGBImage, "convertNV21ToRGBImage", nullptr);
        kernelMedianFilter = std::make_unique<OCL::Kernel>(runtime->context.get(), Kernels::medianFilter, "medianFilter", nullptr);
        kernelConvertToRGBData = std::make_unique<OCL::Kernel>(runtime->context.get(), Kernels::convertNV21ToRGBData, "convertNV21ToRGBData", nullptr);
    }
    void prepareKernel(jint width, jint height){
        kernelConvertToRGBImage->setArg<cl_mem>(0, &imageObj);
        kernelConvertToRGBImage->setArg<cl_mem>(1 ,&buffer->memObj);
        kernelConvertToRGBImage->setArg<cl_int>(2, &width);
        kernelConvertToRGBImage->setArg<cl_int>(3, &height);
        kernelConvertToRGBImage->gws[0] = width;
        kernelConvertToRGBImage->gws[1] = height;
        kernelConvertToRGBImage->lws[0] = 0x400;
        kernelConvertToRGBImage->dims = 2;
        kernelConvertImgInitialized = true;
    }
    void prepareKernelMedian(jint width, jint height){
        kernelMedianFilter->setArg<cl_mem>(0, &imageObj);
        kernelMedianFilter->setArg<cl_mem>(1 ,&tmpDataBuffer->memObj);
        kernelMedianFilter->setArg<cl_int>(2, &width);
        kernelMedianFilter->setArg<cl_int>(3, &height);
        kernelMedianFilter->gws[0] = width;
        kernelMedianFilter->gws[1] = height;
        kernelMedianFilter->lws[0] = 0x400;
        kernelMedianFilter->dims = 2;

        kernelConvertToRGBData->setArg<cl_mem>(0, &tmpDataBuffer->memObj);
        kernelConvertToRGBData->setArg<cl_mem>(1 ,&buffer->memObj);
        kernelConvertToRGBData->setArg<cl_int>(2, &width);
        kernelConvertToRGBData->setArg<cl_int>(3, &height);
        kernelConvertToRGBData->gws[0] = width;
        kernelConvertToRGBData->gws[1] = height;
        kernelConvertToRGBData->lws[0] = 0x400;
        kernelConvertToRGBData->dims = 2;

        kernelConvertMedianInitialized = true;
    }
    std::unique_ptr<OCL::Runtime> runtime;
    std::unique_ptr<OCL::Queue> queue;
    std::unique_ptr<OCL::Kernel> kernelConvertToRGBImage;
    std::unique_ptr<OCL::Kernel> kernelMedianFilter;
    std::unique_ptr<OCL::Kernel> kernelConvertToRGBData;
    std::unique_ptr<OCL::Buffer> buffer;
    std::unique_ptr<OCL::Buffer> tmpDataBuffer;
    cl_mem imageObj = nullptr;
    jbyte *src;
    bool kernelConvertImgInitialized = false;
    bool kernelConvertMedianInitialized = false;
    bool textureLoaded = false;
    jint textureID;

};
OclSharing *oclSharing;

void convertToRgb(jbyte *rgb, jbyte *yuv, int width, int height, int x, int y);

extern "C"
JNIEXPORT void JNICALL
Java_com_example_myapplication_MainActivity_convertToRGBOCL(JNIEnv *env, jclass clazz,
                                                            jbyteArray rgb, jbyteArray yuv, jint width,
                                                            jint height) {
    bool medianFilter = false;
    OCL::Kernel *kernelToRun = nullptr;
    oclSharing->src = env->GetByteArrayElements(yuv, 0);
    auto tmp = env->GetByteArrayElements(rgb, 0);
    if(!oclSharing->textureLoaded){
        int status;
        oclSharing->imageObj = clCreateFromGLTexture(	oclSharing->runtime->context->context,
                                             CL_MEM_WRITE_ONLY,
                                             GL_TEXTURE_2D,
                                             0,
                                             oclSharing->textureID,
                                             &status);
        if(status == 0){
            oclSharing->textureLoaded = true;
            oclSharing->kernelConvertToRGBImage->setArg<cl_mem>(0, &oclSharing->imageObj);

        }
    }
    if(medianFilter){
        if(!oclSharing->kernelConvertMedianInitialized){
            int sizeSrc = env->GetArrayLength(yuv);
            int sizeTmp = env->GetArrayLength(rgb);
            oclSharing->buffer = std::make_unique<OCL::Buffer>(oclSharing->runtime->context.get(), sizeSrc, nullptr);

            oclSharing->tmpDataBuffer = std::make_unique<OCL::Buffer>(oclSharing->runtime->context.get(), width * height * 4 * sizeof(float), nullptr);
            oclSharing->prepareKernelMedian(width, height);
        }
    }else{
        if(!oclSharing->kernelConvertImgInitialized){
            int sizeSrc = env->GetArrayLength(yuv);
            int sizeTmp = env->GetArrayLength(rgb);
            oclSharing->buffer = std::make_unique<OCL::Buffer>(oclSharing->runtime->context.get(), sizeSrc, nullptr);
            oclSharing->prepareKernel(width, height);
        }
    }
    oclSharing->buffer->toDevice(oclSharing->queue.get(), oclSharing->src);
    int status = clEnqueueAcquireGLObjects(
            oclSharing->queue->queue,
            1,
            &oclSharing->imageObj,
            0, 0, 0);

    if(medianFilter) {
        oclSharing->queue->runKernel(oclSharing->kernelConvertToRGBData.get());
        oclSharing->queue->runKernel(oclSharing->kernelMedianFilter.get());
    } else {
        oclSharing->queue->runKernel(oclSharing->kernelConvertToRGBImage.get());
    }
    oclSharing->queue->waitForExecutionFinish();
    status = clEnqueueReleaseGLObjects(oclSharing->queue->queue,
                                       1,
                                       &oclSharing->imageObj,
                                       0, NULL, NULL);
    env->ReleaseByteArrayElements(yuv, oclSharing->src, 0);
    env->ReleaseByteArrayElements(rgb, tmp, 0);
    //dst = env->GetByteArrayElements(rgb, 0);
    //if(!initialized){
    //    int sizeDst = env->GetArrayLength(rgb);
    //    int sizeSrc = env->GetArrayLength(yuv);
    //    dstBuffer = std::make_unique<OCL::Buffer>(runtime.context.get(), sizeDst, nullptr);
    //    srcBuffer = std::make_unique<OCL::Buffer>(runtime.context.get(), sizeSrc, nullptr);
    //    srcBuffer->toDevice(runtime.queue.get(),src);
    //    runtime.kernel->setArg<cl_mem>(0, &dstBuffer->memObj);
    //    runtime.kernel->setArg<cl_mem>(1, &srcBuffer->memObj);
    //    runtime.kernel->setArg<cl_int>(2, &width);
    //    runtime.kernel->setArg<cl_int>(3, &height);
    //    runtime.kernel->gws[0] = width;
    //    runtime.kernel->gws[1] = height;
    //    runtime.kernel->lws[0] = 0x400;
    //    runtime.kernel->dims = 2;
    //    initialized = true;
    //}
    //srcBuffer->toDevice(runtime.queue.get(),src);
    //runtime.queue->runKernel(runtime.kernel.get());
    //dstBuffer->toHost(runtime.queue.get(), dst);
    //runtime.queue->waitForExecutionFinish();
    //env->ReleaseByteArrayElements(rgb, dst, 0);
    //env->ReleaseByteArrayElements(yuv, src, 0);
}extern "C"
JNIEXPORT void JNICALL
Java_com_example_myapplication_MainActivity_convertToRGBMultiThread(JNIEnv *env, jclass clazz,
                                                                    jbyteArray rgb, jbyteArray yuv,
                                                                    jint width, jint height) {
    auto dst = env->GetByteArrayElements(rgb, 0);
    auto src = env->GetByteArrayElements(yuv, 0);
    std::vector<std::thread> threads;
    int threadCount =0;
    for(int i=0;i<height; i++){
        for(int j=0; j<width; j++){
            threads.push_back(std::thread(convertToRgb,dst, src, width, height, j, i));
            threadCount++;
            if(threadCount == 8){
                for(auto &thread : threads){
                    thread.join();
                }
                threadCount = 0;
                threads.clear();
            }
        }
    }
    for(auto &thread : threads){
        thread.join();
    }
    env->ReleaseByteArrayElements(rgb, dst, 0);
    env->ReleaseByteArrayElements(yuv, src, 0);
}

void convertToRgb(jbyte *rgb, jbyte *yuv, int width, int height, int x, int y){
    int index = x + (width * y);
    int R, G, B;
    int total = width * height;
    int Y, Cb = 0, Cr = 0;
    Y = (int)yuv[y * width + x];
    if (Y < 0) Y += 255;
    if ((x & 1) == 0) {
        Cr = (int) yuv[(y >> 1) * (width) + x + total];
        Cb = (int) yuv[(y >> 1) * (width) + x + total + 1];
        if (Cb < 0) Cb += 127; else Cb -= 128;
        if (Cr < 0) Cr += 127; else Cr -= 128;
    }
    R = Y + Cr + (Cr >> 2) + (Cr >> 3) + (Cr >> 5);
    G = Y - (Cb >> 2) + (Cb >> 4) + (Cb >> 5) - (Cr >> 1) + (Cr >> 3) + (Cr >> 4) + (Cr >> 5);
    B = Y + Cb + (Cb >> 1) + (Cb >> 2) + (Cb >> 6);
    if (R < 0) {
        R = 0;
    } else if (R > 255) {
        R = 255;
    }
    if (G < 0) {
        G = 0;
    }else if (G > 255) {
        G = 255;
    }
    if (B < 0) {
        B = 0;
    } else if (B > 255) {
        B = 255;
    }
    rgb[(3 * index)+0] = ((jbyte)(R));
    rgb[(3 * index)+1] = ((jbyte)(G));
    rgb[(3 * index)+2] = ((jbyte)(B));

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_myapplication_MyGL20Renderer_initOCL(JNIEnv *env, jclass clazz, jint texture_id,
                                                      jlong display_handle, jlong context_handle) {
    oclSharing = new OclSharing(texture_id, display_handle, context_handle);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_myapplication_MyGL20Renderer_modifyTextureOCL(JNIEnv *env, jclass clazz,
                                                            jint texture_id, jlong dis, jlong ctx) {
    int status = -1;
    LOGI("Modyfing!\n");oclSharing = new OclSharing(texture_id, dis, ctx);
    cl_context_properties props[] =
                  {   CL_GL_CONTEXT_KHR,   (cl_context_properties) ctx,
                                       CL_EGL_DISPLAY_KHR,  (cl_context_properties) dis,
                                        CL_CONTEXT_PLATFORM, 0,
                                        0 };
    auto g_clContext = clCreateContext(props, 1, &oclSharing->runtime->device_id, NULL, NULL, &status);if(status == 0)
    {
        LOGI("Successfully context!\n");
    }
    else
    {
        LOGI("context failed, %d\n", status);
    }
    auto g_SharedRGBAimageCLMemObject = clCreateFromGLTexture( g_clContext,
                                                                                                                            CL_MEM_WRITE_ONLY,
                                                                                                                            GL_TEXTURE_2D,
                                                                                                                          0,
                                                                                                                            texture_id,
                                                                                                                            &status);
    if(status == 0)
    {
        LOGI("Successfully shared!\n");
    }
    else
    {
        LOGI("Sharing failed, %d\n", status);
    }
    clReleaseMemObject(g_SharedRGBAimageCLMemObject);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_myapplication_MainActivity_readPixelOne(JNIEnv *env, jclass clazz) {
    int status = clEnqueueAcquireGLObjects(
            oclSharing->queue->queue,
            1,
            &oclSharing->imageObj,
            0, 0, 0);
    uint8_t srcImage[200*200*4] = {};
    memset(srcImage, 150, 200*200*4);
    size_t origin[3] = {0, 0, 0};
    size_t region[3]= {200,200,1};
    status = clEnqueueWriteImage(oclSharing->queue->queue, oclSharing->imageObj, false,origin, region,0, 0, srcImage, 0, nullptr, nullptr );
    status = clEnqueueReleaseGLObjects(oclSharing->queue->queue,
                                                                                                                        1,
                                                                                                                        &oclSharing->imageObj,
                                                                                                                        0, NULL, NULL);

    oclSharing->queue->waitForExecutionFinish();
}