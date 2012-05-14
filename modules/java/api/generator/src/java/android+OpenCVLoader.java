package org.opencv.android;

import android.content.Context;

public class OpenCVLoader
{
	public static final String OPEN_CV_VERSION_2_4 = "2.4";
	
	public static boolean initStatic()
    {
        if (!mIsInitialised)
        {
            mIsInitialised = StaticHelper.initOpenCV();
        }

        return mIsInitialised;
    }

    public static boolean initAsync(String Version, Context AppContext,
            LoaderCallbackInterface Callback)
    {
        if (!mIsInitialised)
        {
            mIsInitialised = AsyncServiceHelper.initOpenCV(Version, AppContext, Callback);
        }

        return mIsInitialised;
    }

    protected static boolean mIsInitialised = false;
}
