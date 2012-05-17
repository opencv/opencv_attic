******************
Java OpenCV Loader
******************

.. highlight:: java
.. module:: org.opencv.android
    :platform: Android
    :synopsis: Implements Android dependent Java classes.
.. Class:: OpenCVLoader

Helper class provides common initialization methods for OpenCV library

boolean initStatic()
--------------------

.. method:: boolean initStatic()
    
    Load and initialize OpenCV library from current application package. Roughly it is analog of system.loadLibrary("opencv_java")

    :rtype: boolean
    :return: Return true if initialization of OpenCV was successful

.. note:: This way is depricated for production code. It is designed for experimantal and local development purposes only. If you want to publish your app use aproach with async initialisation

boolean initAsync()
-------------------

.. method:: int initAsync(String Version, Context AppContext, LoaderCallbackInterface Callback)

    Load and initialize OpenCV library using OpenCV Engine service.

    :param Version: OpenCV Library version
    :param AppContext: Application context for connecting to service
    :param Callback: Object, that implements LoaderCallbackInterface for handling Connection status
    :rtype: boolean
    :return: Return true if initialization of OpenCV starts successfully

Loader callback interface
-------------------------

.. class:: LoaderCallbackInterface

    Interface for callback object in case of asynchronous initialization of OpenCV

.. method:: void onEngineConnected(int status)

    Callback method for Async OpenCV initialization
 
    :param status: Status of initialization. See Initialization status constants

Initialisation status constants
-------------------------------

.. data:: SUCCESS

    OpenCV initialization finished successfully

.. data:: NO_SERVICE

    OpenCV Engine service is not installed on the device. App need to notify user about it

.. data:: RESTART_REQUIRED

    OpenCV library installation via Google Play service was initialized. Application restart is required

.. data:: MARKET_ERROR

    Google Play (Android Market) cannot be invoked

.. data:: INSTALL_CANCELED

    OpenCV library installation was canceled by user

.. data:: INCOMPATIBLE_ENGINE_VERSION

    Version of OpenCV Engine Service is incompatible with this app. Service update is needed

.. data:: INIT_FAILED

    OpenCV library initialization failed

OpenCV version constatnts
-------------------------

.. data:: OPEN_CV_VERSION_2_4

    OpenCV Library version 2.4.x

Other constatnts
----------------

.. data:: OPEN_CV_SERVICE_URL

    Url for OpenCV Engine on Google Play (Android Market)