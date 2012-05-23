package org.opencv.android;

import java.io.File;
import java.util.StringTokenizer;

import org.opencv.engine.OpenCVEngineInterface;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

public class AsyncServiceHelper
{
    public static boolean initOpenCV(String Version, Context AppContext,
            LoaderCallbackInterface Callback)
    {
        AsyncServiceHelper helper = new AsyncServiceHelper(Version, AppContext, Callback);
        if (AppContext.bindService(new Intent("org.opencv.engine.BIND"),
                helper.mServiceConnection, Context.BIND_AUTO_CREATE))
        {
            return true;
        }
        else
        {
            AppContext.unbindService(helper.mServiceConnection);
            Callback.onEngineConnected(LoaderCallbackInterface.NO_SERVICE);
            return false;
        }
    }

    protected AsyncServiceHelper(String Version, Context AppContext,
            LoaderCallbackInterface Callback)
    {
        mOpenCVersion = Version;
        mUserAppCallback = Callback;
        mAppContext = AppContext;
    }

    protected static final String TAG = "OpenCvEngine/Helper";
    protected static final int MINIMUM_ENGINE_VERSION = 1;
    protected OpenCVEngineInterface mEngineService;
    protected LoaderCallbackInterface mUserAppCallback;
    protected String mOpenCVersion;
    protected Context mAppContext;
    protected int mStatus = LoaderCallbackInterface.SUCCESS;

    protected ServiceConnection mServiceConnection = new ServiceConnection()
    {
        public void onServiceConnected(ComponentName className, IBinder service)
        {
            Log.d(TAG, "Service connection created");
            mEngineService = OpenCVEngineInterface.Stub.asInterface(service);
            if (null == mEngineService)
            {
                Log.d(TAG, "Engine connection fails. May be service was not installed?");
                mStatus = LoaderCallbackInterface.NO_SERVICE;
            }
            else
            {
                try
                {
                    if (mEngineService.getEngineVersion() < MINIMUM_ENGINE_VERSION)
                    {
                        mStatus = LoaderCallbackInterface.INCOMPATIBLE_ENGINE_VERSION;
                        Log.d(TAG, "Init finished with status " + mStatus);
                        Log.d(TAG, "Unbind from service");
                        mAppContext.unbindService(mServiceConnection);
                        Log.d(TAG, "Calling using callback");
                        mUserAppCallback.onEngineConnected(mStatus);
                        return;
                    }

                    Log.d(TAG, "Trying to get library path");
                    String path = mEngineService.getLibPathByVersion(mOpenCVersion);
                    if ((null == path) || (path.length() == 0))
                    {
                        InstallCallbackInterface InstallQuery = new InstallCallbackInterface() {
							
							public void install() {
				                Log.d(TAG, "Trying to install OpenCV lib via Google Play");

				                try
				                {
				                    if (mEngineService.installVersion(mOpenCVersion))
				                    {
				                        mStatus = LoaderCallbackInterface.RESTART_REQUIRED;
				                    }
				                    else
				                    {
				                        Log.d(TAG, "OpenCV package was not installed!");
				                        mStatus = LoaderCallbackInterface.MARKET_ERROR;
				                    }
				                } catch (RemoteException e) {
				                    e.printStackTrace();
				                    mStatus = LoaderCallbackInterface.INIT_FAILED;
				                }

				                Log.d(TAG, "Init finished with status " + mStatus);
				                Log.d(TAG, "Unbind from service");
				                mAppContext.unbindService(mServiceConnection);
				                Log.d(TAG, "Calling using callback");
				                mUserAppCallback.onEngineConnected(mStatus);								
							}
							
							public void cancel() {
				                Log.d(TAG, "OpenCV library installation was canceled");
				                mStatus = LoaderCallbackInterface.INSTALL_CANCELED;
				                Log.d(TAG, "Init finished with status " + mStatus);
				                Log.d(TAG, "Unbind from service");
				                mAppContext.unbindService(mServiceConnection);
				                Log.d(TAG, "Calling using callback");
				                mUserAppCallback.onEngineConnected(mStatus);	
							}
						};
						
						mUserAppCallback.onLibraryInstall(InstallQuery);
						return;
                    }
                    else
                    {
                        Log.d(TAG, "Trying to get library list");
                        String libs = mEngineService.getLibraryList(mOpenCVersion);
                        Log.d(TAG, "Library list: \"" + libs + "\"");
                        Log.d(TAG, "First attempt to load libs");
                        if (initOpenCVLibs(path, libs))
                        {
                            Log.d(TAG, "First attempt to load libs is OK");
                            mStatus = LoaderCallbackInterface.SUCCESS;
                        }
                        else
                        {
                            Log.d(TAG, "First attempt to load libs fails");
                            mStatus = LoaderCallbackInterface.INIT_FAILED;
                        }

                        Log.d(TAG, "Init finished with status " + mStatus);
                        Log.d(TAG, "Unbind from service");
                        mAppContext.unbindService(mServiceConnection);
                        Log.d(TAG, "Calling using callback");
                        mUserAppCallback.onEngineConnected(mStatus);
                    }
                }
                catch (RemoteException e)
                {
                    e.printStackTrace();
                    mStatus = LoaderCallbackInterface.INIT_FAILED;
                    Log.d(TAG, "Init finished with status " + mStatus);
                    Log.d(TAG, "Unbind from service");
                    mAppContext.unbindService(mServiceConnection);
                    Log.d(TAG, "Calling using callback");
                    mUserAppCallback.onEngineConnected(mStatus);
                }

            }
        }

        private boolean loadLibrary(String AbsPath)
        {
            boolean result = true;

            Log.d(TAG, "Trying to load library " + AbsPath);
            try
            {
                System.load(AbsPath);
                Log.d(TAG, "OpenCV libs init was ok!");
            }
            catch(UnsatisfiedLinkError e)
            {
                Log.d(TAG, "Cannot load library \"" + AbsPath + "\"");
                e.printStackTrace();
                result &= false;
            }

            return result;
        }

        private boolean initOpenCVLibs(String Path, String Libs)
        {
            Log.d(TAG, "Trying to init OpenCV libs");
            if ((null != Path) && (Path.length() != 0))
            {
                boolean result = true;
                if ((null != Libs) && (Libs.length() != 0))
                {
                    Log.d(TAG, "Trying to load libs by dependency list");
                    StringTokenizer splitter = new StringTokenizer(Libs, ";");
                    while(splitter.hasMoreTokens())
                    {
                        String AbsLibraryPath = Path + File.separator + splitter.nextToken();
                        result &= loadLibrary(AbsLibraryPath);
                    }
                }
                else
                {
                    // If dependencies list is not defined or empty
                    String AbsLibraryPath = Path + File.separator + "libopencv_java.so";
                    result &= loadLibrary(AbsLibraryPath);
                }

                return result;
            }
            else
            {
                Log.d(TAG, "Library path \"" + Path + "\" is empty");
                return false;
            }
        }

        public void onServiceDisconnected(ComponentName className)
        {
            mEngineService = null;
        }
    };
}
