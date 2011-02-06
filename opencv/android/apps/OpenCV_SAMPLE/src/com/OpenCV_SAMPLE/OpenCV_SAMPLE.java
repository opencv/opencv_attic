package com.OpenCV_SAMPLE;

import java.util.LinkedList;

import android.os.Bundle;

import com.opencv.camera.CameraActivity;
import com.opencv.camera.NativeProcessor.PoolCallback;

public class OpenCV_SAMPLE extends CameraActivity {

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
	}

	@Override
	protected LinkedList<PoolCallback> getCallBackStack() {
		// TODO Auto-generated method stub
		return null;
	}

}