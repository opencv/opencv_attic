package org.opencv.android;

public interface LoaderCallbackInterface
{
    static final int Success = 0;
    static final int NoService = 1;
    static final int RestartRequired = 2;
    static final int MarketError = 3;
    static final int InitFailed = 0xff;
    
    public void onEngineConnected(int status);
};
