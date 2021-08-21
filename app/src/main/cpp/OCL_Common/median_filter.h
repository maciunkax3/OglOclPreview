#include <string>

namespace Kernels{
    const char *medianFilter = R"===(
#define WINDOW_SIZE 3
#define WINDOW_ARRAY_SIZE 9

void bubbleSort(float *v,int size)
{
    int greater = 0;
    int less = 0;
    for(int i = 0; i < size ; i++){
        for(int j = 0; j < size; i++){
            if(v[i] > v[j]){
                greater++;
            } else if (v[i] < v[j]){
                less++;
            }
            if(greater > size/2 || less > size/2){
                break;
            }
        }
        if (greater == less){
            v[size/2] = v[i];
            break;
        } else {
            greater = 0;
            less = 0;
        }
    }
}

__kernel void medianFilter(__write_only image2d_t img, __global float4 *rgbData, int width, int height){
    int gidX = get_global_id(0);
    int gidY = get_global_id(1);
    if(gidY>height || gidX>width)
       return;
    int index = gidX + (width * gidY);
    int filter_offset = WINDOW_SIZE / 2;
    float window[3][WINDOW_ARRAY_SIZE];
    for(int j=0 ; j<3;j++) {
        for (int i=0; i < WINDOW_ARRAY_SIZE; i++) {
            window[j][i]=0.0;
        }
    }
//
    int count=0;
    for( int k=gidY-filter_offset; k<=gidY+filter_offset; k++) {
        for (int l=gidX-filter_offset; l<=gidX+filter_offset; l++) {
            if(k>=0 && l>=0 && k<height && l<width)
                window[0][count]=rgbData[(k)*width+(l)].x;
                window[1][count]=rgbData[(k)*width+(l)].y;
                window[2][count]=rgbData[(k)*width+(l)].z;
                count++;
        }
    }
    bubbleSort(window[0],WINDOW_ARRAY_SIZE);
    bubbleSort(window[1],WINDOW_ARRAY_SIZE);
    bubbleSort(window[2],WINDOW_ARRAY_SIZE);
    //write_imagef(img, (int2)(gidX, gidY), (float4)(window[0][WINDOW_SIZE/2], window[1][WINDOW_SIZE/2], window[2][WINDOW_SIZE/2], 1.0));

    write_imagef(img, (int2)(gidX, gidY), rgbData[index]);

}
)===";
}