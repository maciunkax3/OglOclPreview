package com.example.myapplication;

import android.graphics.Point;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.opengl.EGL14;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.SurfaceHolder;

import java.io.IOException;
import java.nio.ByteBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MyGL20Renderer implements GLSurfaceView.Renderer {

    DirectVideo mDirectVideo;
    int textureRGB;
    int texture;
    private SurfaceTexture surface;
    private SurfaceHolder mHolder;
    private static byte[] texture_data;
    private static Point size;
    private long time;
    MainActivity delegate;
    private FPSCounter counter;

    public MyGL20Renderer(MainActivity _delegate) {
        delegate = _delegate;
    }

    public void onSurfaceCreated(GL10 unused, EGLConfig config) {
        size = delegate.getOpenCameraAndGetResolution();

        long time = System.currentTimeMillis();
        texture_data = new byte[size.x*size.y * 4];
        textureRGB = createTextureRGB();
        try {
            delegate.startCamera(texture_data, textureRGB);
        } catch (IOException e) {
            e.printStackTrace();
        }
        for(int i=0;i<size.x*size.y * 4;i++){
            texture_data[i] = (byte)(i%256);
        }
        mDirectVideo = new DirectVideo(textureRGB);
        GLES20.glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        counter = new FPSCounter();
    }

    private int createTextureRGB() {
        int[] texture = new int[1];

        GLES20.glGenTextures(1,texture, 0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture[0]);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_MIN_FILTER,GLES20.GL_LINEAR);
        GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA, size.x, size.y, 0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, ByteBuffer.wrap(texture_data));
        return texture[0];
    }

    public void onDrawFrame(GL10 unused) {
        float[] mtx = new float[16];
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
        surface.updateTexImage();
        updateTexture();
        surface.getTransformMatrix(mtx);
        counter.logFrame();
        mDirectVideo.draw();
    }

    public void onSurfaceChanged(GL10 unused, int width, int height) {
        GLES20.glViewport(0, 0, width, height);
    }

    static public int loadShader(int type, String shaderCode) {
        int shader = GLES20.glCreateShader(type);

        GLES20.glShaderSource(shader, shaderCode);
        GLES20.glCompileShader(shader);

        return shader;
    }
    private void updateTexture(){
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureRGB);
        GLES20.glTexSubImage2D(GLES20.GL_TEXTURE_2D, 0,
                0, 0,
                size.x, size.y,
                GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE,
                ByteBuffer.wrap(texture_data));
    }
    static private int createTexture() {
        int[] texture = new int[1];

        GLES20.glGenTextures(1, texture, 0);
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, texture[0]);
        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
                GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_LINEAR);
        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
                GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_LINEAR);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
                GL10.GL_TEXTURE_WRAP_S, GL10.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
                GL10.GL_TEXTURE_WRAP_T, GL10.GL_CLAMP_TO_EDGE);

        return texture[0];
    }

    public void setSurface(SurfaceTexture _surface) {
        surface = _surface;
    }
}