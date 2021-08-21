package com.example.myapplication;

public class NativeOgl {
    static
    {
        System.loadLibrary("NativeOGL");
    }
    public static native void init(int width, int height, byte[] data);
    public static native void step();
}
