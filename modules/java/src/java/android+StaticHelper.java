package org.opencv.android;

import java.util.StringTokenizer;

import android.util.Log;

public class StaticHelper {

    public static int initOpenCV()
    {
        int result;
        String libs = "";

        Log.d(TAG, "Trying to get library list");

        try
        {
            System.loadLibrary("opencvinfo");
            libs = GetLibraryList();
        }
        catch(UnsatisfiedLinkError e)
        {
            Log.e(TAG, "OpenCV error: Cannot load info library for OpenCV");
        }

        Log.d(TAG, "Library list: \"" + libs + "\"");
        Log.d(TAG, "First attempt to load libs");
        if (initOpenCVLibs(libs))
        {
            Log.d(TAG, "First attempt to load libs is OK");
            result = OpenCVLoader.Success;
        }
        else
        {
            Log.d(TAG, "First attempt to load libs fails");
            result = OpenCVLoader.InitFailed;
        }

        return result;
    }

    private static boolean loadLibrary(String Name)
    {
        boolean result = true;

        Log.d(TAG, "Trying to load library " + Name);
        try
        {
            System.loadLibrary(Name);
            Log.d(TAG, "OpenCV libs init was ok!");
        }
        catch(UnsatisfiedLinkError e)
        {
            Log.d(TAG, "Cannot load library \"" + Name + "\"");
            e.printStackTrace();
            result &= false;
        }

        return result;
    }

    private static boolean initOpenCVLibs(String Libs)
    {
        Log.d(TAG, "Trying to init OpenCV libs");

        boolean result = true;

        if ((null != Libs) && (Libs.length() != 0))
        {
            Log.d(TAG, "Trying to load libs by dependency list");
            StringTokenizer splitter = new StringTokenizer(Libs, ";");
            while(splitter.hasMoreTokens())
            {
                result &= loadLibrary(splitter.nextToken());
            }
        }
        else
        {
            // If dependencies list is not defined or empty
            result &= loadLibrary("opencv_java");
        }

        return result;
    }

    private static final String TAG = "OpenCV/StaticHelper";

    private static native String GetLibraryList();
}
