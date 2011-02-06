package com.OpenCV_SAMPLE;

import java.util.LinkedList;

import android.os.Bundle;
import android.util.Log;

import com.OpenCV_SAMPLE.jni.CVSample;
import com.opencv.camera.CameraActivity;
import com.opencv.camera.NativeProcessor;
import com.opencv.camera.NativeProcessor.PoolCallback;
import com.opencv.jni.*;


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
	Mat canny; 
	PoolCallback samplePoolCallback = new PoolCallback() {
		
		@Override
		public void process(int idx, image_pool pool, long timestamp,
				NativeProcessor nativeProcessor) {
			Mat grey = pool.getImage(idx);
			canny = pool.getImage(idx+1);
			//if(canny == null) canny = new Mat();
			if(Mat.getCPtr(canny) == 0) Log.e("opencv","bad ptr");
//			if(grey == null || canny == null) return;
			//cvsample.invert(grey);
			cvsample.canny(grey, canny, 10);
			pool.addImage(idx+1,canny);
			glview.getDrawCallback().process(idx+1, pool, timestamp, nativeProcessor);
		}
	};

}