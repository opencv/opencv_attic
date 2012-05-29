package org.opencv.samples.puzzle15;

import org.opencv.android.InstallCallbackInterface;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Window;

/** Activity class implements LoaderCallbackInterface to handle OpenCV initialization status **/
public class puzzle15Activity extends Activity implements LoaderCallbackInterface
{
	private static final String TAG = "Sample::Activity";

    private MenuItem            mItemNewGame;
    private MenuItem            mItemToggleNumbers;
    private puzzle15View        mView = null;
    
    public puzzle15Activity()
    {
        Log.i(TAG, "Instantiated new " + this.getClass());
    }

    @Override
	protected void onPause() {
        Log.i(TAG, "onPause");
		super.onPause();
		if (null != mView)
			mView.releaseCamera();
	}

	@Override
	protected void onResume() {
        Log.i(TAG, "onResume");
		super.onResume();
		if( mView!=null && !mView.openCamera() ) {
			AlertDialog ad = new AlertDialog.Builder(this).create();  
			ad.setCancelable(false); // This blocks the 'BACK' button  
			ad.setMessage("Fatal error: can't open camera!");  
			ad.setButton("OK", new DialogInterface.OnClickListener() {  
			    public void onClick(DialogInterface dialog, int which) {  
			        dialog.dismiss();                      
					finish();
			    }  
			});  
			ad.show();
		}
	}

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        Log.i(TAG, "onCreate");
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        
        Log.i(TAG, "Trying to load OpenCV library");
        if (!OpenCVLoader.initAsync(OpenCVLoader.OPEN_CV_VERSION_2_4, this, this))
        {
        	Log.e(TAG, "Cannot connect to OpenCVEngine");
        	//finish();
        }
    }
    
    /** LoaderCallbackInterface method. Called when OpenCV initialization will be finished **/
	public void onEngineConnected(int status)
	{
		switch (status)
		{
			/** OpenCV initialization was successful. **/
			case LoaderCallbackInterface.SUCCESS:
			{
				Log.i(TAG, "OpenCV loaded successfully");
				// Create and set View
				mView = new puzzle15View(this);
				setContentView(mView);
				// Check native OpenCV camera
				if( !mView.openCamera() ) {
					AlertDialog ad = new AlertDialog.Builder(this).create();  
					ad.setCancelable(false); // This blocks the 'BACK' button  
					ad.setMessage("Fatal error: can't open camera!");  
					ad.setButton("OK", new DialogInterface.OnClickListener() {  
					    public void onClick(DialogInterface dialog, int which) {  
					        dialog.dismiss();                      
							finish();
					    }  
					});  
					ad.show();
				}

			} break;
			/** OpenCV engine or library package installation is in progress. Restart of application is required **/
			case LoaderCallbackInterface.RESTART_REQUIRED:
			{
				Log.d(TAG, "OpenCV downloading. App restart is needed!");
				AlertDialog ResartMessage = new AlertDialog.Builder(this).create();
				ResartMessage.setTitle("App restart is required");
				ResartMessage.setMessage("Application will be closed now. Start it when installtion will be finished!");
				ResartMessage.setButton("OK", new OnClickListener() {	
					public void onClick(DialogInterface dialog, int which) {
						finish();
					}
				});

				ResartMessage.show();
			} break;
			/** OpenCV loader cannot find OpenCV Engine Service on the device **/
			case LoaderCallbackInterface.NO_SERVICE:
			{
				Log.d(TAG, "OpenCVEngine Service is not installed!");
				AlertDialog NoServiceMessage = new AlertDialog.Builder(this).create();
				NoServiceMessage.setTitle("OpenCV Engine");
				NoServiceMessage.setMessage("OpenCV Engine service was not found");
				NoServiceMessage.setButton("Ok", new OnClickListener() {
					
					public void onClick(DialogInterface dialog, int which) {
				        finish();
					}
				});				
				NoServiceMessage.show();
			} break;
			/** OpenCV loader cannot start Google Play **/
			case LoaderCallbackInterface.MARKET_ERROR:
			{
				Log.d(TAG, "Google Play service is not installed! You can get it here");
				AlertDialog MarketErrorMessage = new AlertDialog.Builder(this).create();
				MarketErrorMessage.setTitle("OpenCV Engine");
				MarketErrorMessage.setMessage("Package instalation failed!");
				MarketErrorMessage.setButton("OK", new OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						finish();
					}
				});
				MarketErrorMessage.show();
			} break;
			/** Package installation was canceled **/
			case LoaderCallbackInterface.INSTALL_CANCELED:
			{
				Log.d(TAG, "OpenCV library instalation was canceled by user");
				finish();
			} break;
			/** Application is incompatible with OpenCV Engine. Possible Service update is needed **/
			case LoaderCallbackInterface.INCOMPATIBLE_ENGINE_VERSION:
			{
				Log.d(TAG, "OpenCVEngine Service is uncompatible with this app!");
				AlertDialog IncomatibilityMessage = new AlertDialog.Builder(this).create();
				IncomatibilityMessage.setTitle("OpenCV Engine");
				IncomatibilityMessage.setMessage("OpenCV Eingine service is incompatible with this app. Update it!");
				IncomatibilityMessage.setButton("OK", new OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						finish();
					}
				});
				IncomatibilityMessage.show();				
			}
			/** Other status, i.e. INIT_FAILED **/
			default:
			{
				Log.e(TAG, "OpenCV loading failed!");
				AlertDialog InitFailedDialog = new AlertDialog.Builder(this).create();
				InitFailedDialog.setTitle("OpenCV error");
				InitFailedDialog.setMessage("OpenCV was not initialised correctly. Application will be shut down");
				InitFailedDialog.setButton("OK", new OnClickListener() {
					
					public void onClick(DialogInterface dialog, int which) {
						finish();
					}
				});

				InitFailedDialog.show();
			} break;
		}
	}

	/** LoaderCallbackInterface method. Called when package installation approve is required **/
	public void onPackageInstall(final InstallCallbackInterface Callback)
	{
        AlertDialog InstallMessage = new AlertDialog.Builder(this).create();
        InstallMessage.setTitle("Package not found");
        InstallMessage.setMessage(Callback.getPackageName() + " package was not found! Try to install it?");
        InstallMessage.setButton("Yes", new OnClickListener()
        {
            public void onClick(DialogInterface dialog, int which)
            {
            	Callback.install();
            }
        });

        InstallMessage.setButton2("No", new OnClickListener() {

        	public void onClick(DialogInterface dialog, int which)
        	{
        		Callback.cancel();
            }
        });

        InstallMessage.show();
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