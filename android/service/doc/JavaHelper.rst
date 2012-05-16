******************
Java OpenCV Loader
******************

.. highlight:: java
.. module:: org.opencv.android
    :platform: Android
    :synopsis: Implements Android dependent Java classes.
.. Class:: OpenCVLoader

Helper class provides common methods for OpenCV library and return values constants.

int initStatic()
----------------

.. method:: int initStatic()
    
    Loads and initializes OpenCV library from current application package. Roughly it is analog of system.loadLibrary("opencv_java").

    :rtype: int;
    :return: Function returns initialisation status.

.. note:: This way is depricated for production code. It is designed for experimantal and local development purposes only. If you want to publish your app use aproach with async initialisation.

int initAsync()
---------------

.. method:: int initAsync(String Version, Context AppContext, LoaderCallbackInterface Callback)

    Loads and inits OpenCV library using OpenCV Engine service.

    :param Version: OpenCV Library version;
    :param AppContext: Application context for connecting to service;
    :param Callback: Object, that implements LoaderCallbackInterface for handling Connection status;
    :rtype: int;
    :return: Function returns initialisation status.

Initialisation status constants
-------------------------------

.. data:: SUCCESS

    OpenCV initialisation finished successfully.

.. data:: NO_SERVICE

    OpenCV Engine service is not installed on the device. App need to notify user about it.

.. data:: RESTART_REQUIRED

    OpenCV library installation via Google Play service was initialized. Application restart is required.

.. data:: MARKET_ERROR

    Google Play (Android Market) cannot be invoked.

.. data:: INSTALL_CANCELED

    OpenCV library installation was canceled by user.

.. data:: INIT_FAILED

    OpenCV library initialization failed.

Loader callback interface
-------------------------

.. class:: LoaderCallbackInterface

    Interface for callback object in case of asynchronious initialization of OpenCV.

.. method:: void onEngineConnected(int status)

    Callback method for Async OpenCV intialization;
 
    :param status: Result of initialization. See Initialisation status constants.