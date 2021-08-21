#include <string>

namespace Kernels{
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
}