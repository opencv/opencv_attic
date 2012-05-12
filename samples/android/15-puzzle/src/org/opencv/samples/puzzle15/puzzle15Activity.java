package org.opencv.samples.puzzle15;

import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Window;

public class puzzle15Activity extends Activity implements LoaderCallbackInterface
{
	private static final String TAG = "Sample::Activity";

    private MenuItem            mItemNewGame;
    private MenuItem            mItemToggleNumbers;
    private puzzle15View        mView;
    
    public puzzle15Activity()
    {
        Log.i(TAG, "Instantiated new " + this.getClass());
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        Log.i(TAG, "onCreate");
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        
        Log.i(TAG, "Trying to load OpenCV library");
        if (!OpenCVLoader.initAsync("2.4", this, this))
        {
        	Log.e(TAG, "Cannot connect to OpenCVEngine");
        	finish();
        }
    }
    
	public void onEngineConnected(int status)
	{
		switch (status)
		{
			case LoaderCallbackInterface.Success:
			{
				Log.i(TAG, "OpenCV loaded successfully");
				mView = new puzzle15View(this);
				setContentView(mView);
			} break;
			case LoaderCallbackInterface.RestartRequired:
			{
				Log.d(TAG, "OpenCV downloading. App restart is needed!");
				finish();
			} break;
			default:
			{
				Log.e(TAG, "OpenCV loading failed!");
				finish();
			} break;
		}
	}

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        Log.i(TAG, "onCreateOptionsMenu");
        mItemNewGame = menu.add("Start new game");
        mItemToggleNumbers = menu.add("Show/hide tile numbers");
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        Log.i(TAG, "Menu Item selected " + item);
        if (item == mItemNewGame) {
            synchronized (mView) {
                mView.startNewGame();
            }
        } else if (item == mItemToggleNumbers)
            mView.tolggleTileNumbers();
        return true;
    }
}