package com.example.depthai_android_jni_example;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.Window;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {
    // Load the JNI library
    static { System.loadLibrary("depthai_android_jni_example"); }

    // Camera and UI
    private static final int WIDTH = 416;
    private static final int HEIGHT = 416;

    private ImageView imageView;
    private Bitmap bitmap;
    private final Handler mainHandler = new Handler(Looper.getMainLooper());
    private boolean isRunning = true;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // Fullscreen, keep alive, and UI setup
        supportRequestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Bind UI
        imageView = findViewById(R.id.imageView);
        bitmap = Bitmap.createBitmap(WIDTH, HEIGHT, Bitmap.Config.ARGB_8888);

        // Start device and frame update loop
        new Thread(this::startDevice).start();
        mainHandler.post(updateFrameLoop);
    }

    private void startDevice() {
        int result = nativeStartDevice("", WIDTH, HEIGHT);
        runOnUiThread(() -> {
            if (result == 0) {
                Toast.makeText(this, "Device connected", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(this, "Failed to connect to device", Toast.LENGTH_LONG).show();
            }
        });
    }

    private final Runnable updateFrameLoop = new Runnable() {
        @Override
        public void run() {
            int[] rgb = imageFromJNI();
            if (rgb != null && rgb.length > 0) {
                bitmap.setPixels(rgb, 0, WIDTH, 0, 0, WIDTH, HEIGHT);
                imageView.setImageBitmap(bitmap);
            }
            if (isRunning) {
                mainHandler.postDelayed(this, 33); // ~30 fps
            }
        }
    };

    @Override
    protected void onDestroy() {
        isRunning = false;
        mainHandler.removeCallbacks(updateFrameLoop);
        nativeStopDevice();
        super.onDestroy();
    }

    // Native methods (must match C++ exactly!)
    public native int nativeStartDevice(String modelPath, int width, int height);
    public native int[] imageFromJNI();
    public native void nativeStopDevice();
}
