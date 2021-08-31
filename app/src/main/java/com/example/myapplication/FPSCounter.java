package com.example.myapplication;

import android.util.Log;

public class FPSCounter {
    long startTime = System.nanoTime();
    int frames = 0;
    int framesAvg = 0;
    int cycles = 0;

    public void logFrame() {
        frames++;
        framesAvg++;
        if(System.nanoTime() - startTime >= 1000000000) {
            cycles++;
            //Log.d("FPSCounter", "fps: " + frames);
            if(cycles == 15) {
                Log.d("FPSCounter AVG", "fps: " + framesAvg / cycles);
                cycles = 0;framesAvg = 0;
            }
            frames = 0;
            startTime = System.nanoTime();
        }
    }
}