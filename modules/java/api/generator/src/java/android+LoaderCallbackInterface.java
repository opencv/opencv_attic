package org.opencv.android;

public interface LoaderCallbackInterface
{
    static final int SUCCESS = 0;
    static final int NO_SERVICE = 1;
    static final int RESTART_REQUIRED = 2;
    static final int MARKET_ERROR = 3;
    static final int INIT_FAILED = 0xff;

    public void onEngineConnected(int status);
};
