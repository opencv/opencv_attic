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

.. method:: static boolean initStatic()
    
    Load and initialize OpenCV library from current application package. Roughly it is analog of system.loadLibrary("opencv_java")

    :rtype: boolean
    :return: Return true if initialization of OpenCV was successful

.. note:: This way is depricated for production code. It is designed for experimantal and local development purposes only. If you want to publish your app use aproach with async initialisation

boolean initAsync()
-------------------

.. method:: static boolean initAsync(String Version, Context AppContext, LoaderCallbackInterface Callback)

    Load and initialize OpenCV library using OpenCV Engine service.

    :param Version: OpenCV Library version
    :param AppContext: Application context for connecting to service
    :param Callback: Object, that implements LoaderCallbackInterface for handling Connection status. See BaseLoaderCallback.
    :rtype: boolean
    :return: Return true if initialization of OpenCV starts successfully
