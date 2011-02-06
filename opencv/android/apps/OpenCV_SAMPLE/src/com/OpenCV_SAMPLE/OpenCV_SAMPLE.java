package com.OpenCV_SAMPLE;

import java.util.LinkedList;

import android.os.Bundle;

import com.OpenCV_SAMPLE.jni.CVSample;
import com.opencv.camera.CameraActivity;
import com.opencv.camera.NativeProcessor;
import com.opencv.camera.NativeProcessor.PoolCallback;
import com.opencv.jni.Mat;
import com.opencv.jni.image_pool;

public class OpenCV_SAMPLE extends CameraActivity {

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
	}

	@Override
	protected LinkedList<PoolCallback> getCallBackStack() {
		LinkedList<PoolCallback> list = new LinkedList<NativeProcessor.PoolCallback>();
		list.add(samplePoolCallback);
		return list;
	}
	
	CVSample cvsample = new CVSample();
	Mat canny = new Mat();
	PoolCallback samplePoolCallback = new PoolCallback() {
		
		@Override
		public void process(int idx, image_pool pool, long timestamp,
				NativeProcessor nativeProcessor) {
			cvsample.canny(pool.getGrey(idx),canny,10);
			pool.addImage(idx+1,canny);
			glview.getDrawCallback().process(idx+1, pool, timestamp, nativeProcessor);
		}
	};

}