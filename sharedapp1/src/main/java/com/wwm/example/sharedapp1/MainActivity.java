package com.wwm.example.sharedapp1;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("unique_fd_demo");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        testUniqueFd();

        testCloseAnUnownedFd();
    }

    static native void testUniqueFd();
    static native void testCloseAnUnownedFd();
}
