package com.example.myapplication;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Point;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.opengl.EGL14;
import android.os.Bundle;
import android.util.Log;

import com.example.myapplication.databinding.ActivityMainBinding;

import java.io.IOException;

public class MainActivity extends AppCompatActivity implements SurfaceTexture.OnFrameAvailableListener {
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private Camera mCamera;
    private MyGLSurfaceView glSurfaceView;
    private SurfaceTexture dummySurface;
    private SurfaceTexture mainSurface;
    private static int a = 0;
    private int dummyTexture;
    private int mainTexture;
    byte[] texture_data;
    MyGL20Renderer renderer;
    private ActivityMainBinding binding;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        initOCL();
        binding = ActivityMainBinding.inflate(getLayoutInflater());
        glSurfaceView = new MyGLSurfaceView(this);
        renderer = glSurfaceView.getRenderer();
        setContentView(glSurfaceView);
    }

    public void startCamera(byte[] tex_data, int mainTexId) throws IOException {
        texture_data = tex_data;
        dummySurface = new SurfaceTexture(dummyTexture);
        mainSurface = new SurfaceTexture(mainTexId);
        mainSurface.setOnFrameAvailableListener(this);
        renderer.setSurface(mainSurface);
        //mainTexture = mainTexId;

        Camera.PreviewCallback callback = new Camera.PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] data, Camera camera) {
                Camera.Parameters mParameters = camera.getParameters();
                Camera.Size mSize = mParameters.getPreviewSize();
                int mWidth = mSize.width;
                int mHeight = mSize.height;
                //yuv2rgb(texture_data, data, mWidth, mHeight);
                convertToRGBOCL(texture_data, data, mWidth, mHeight);
                onFrameAvailable(mainSurface);
            }
        };
        mCamera.setPreviewCallback(callback);
        mCamera.setPreviewTexture(dummySurface);
        mCamera.startPreview();
    }

    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        glSurfaceView.requestRender();
    }

    @Override
    public void onPause() {
        super.onPause();
        mCamera.stopPreview();
        mCamera.release();
        System.exit(0);
    }

    @Override
    public void onStop() {
        super.onStop();
        closeOCL();
    }

    public Point getOpenCameraAndGetResolution() {
        mCamera = Camera.open();
        return new Point(mCamera.getParameters().getPreviewSize().width,mCamera.getParameters().getPreviewSize().height);
        //return new Point(2048, 1080);
    }

    public static void yuv2rgb(byte[] rgba, byte[] yuv, int width, int height) {
        int total = width * height;
        int Y, Cb = 0, Cr = 0, index = 0;
        int R = 0, G = 0, B = 0;

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Y = yuv[y * width + x];
                if (Y < 0) Y += 255;

                if ((x & 1) == 0) {
                    Cr = yuv[(y >> 1) * (width) + x + total];
                    Cb = yuv[(y >> 1) * (width) + x + total + 1];

                    if (Cb < 0) Cb += 127;
                    else Cb -= 128;
                    if (Cr < 0) Cr += 127;
                    else Cr -= 128;
                }

                R = Y + Cr + (Cr >> 2) + (Cr >> 3) + (Cr >> 5);
                G = Y - (Cb >> 2) + (Cb >> 4) + (Cb >> 5) - (Cr >> 1) + (Cr >> 3) + (Cr >> 4) + (Cr >> 5);
                B = Y + Cb + (Cb >> 1) + (Cb >> 2) + (Cb >> 6);

                if (R < 0) R = 0;
                else if (R > 255) R = 255;
                if (G < 0) G = 0;
                else if (G > 255) G = 255;
                if (B < 0) B = 0;
                else if (B > 255) B = 255;
                rgba[4 * index + 0] = ((byte) (R));
                rgba[4 * index + 1] = ((byte) (G));
                rgba[4 * index + 2] = ((byte) (B));
                rgba[4 * index + 3] = ((byte) (255));
                index++;
            }
        }
    }

    public static native void convertToRGBOCL(byte[] rgb, byte[] yuv, int width, int height);

    public static native void initOCL();

    public static native void closeOCL();
}