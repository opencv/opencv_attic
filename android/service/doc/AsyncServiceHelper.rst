*******************************************
Java Asynchronious OpenCV Helper (internal)
*******************************************

.. highlight:: java
.. module:: org.opencv.android
    :platform: Android
    :synopsis: Implements Android dependent Java classes.
.. Class:: AsyncServiceHelper

Helper class provides implementation of asynchronious OpenCV initialisation Android OpenCV Engine Service.

.. note:: This is imternal class. Does not use it directly. Use OpenCVLoader.InitOpenCVAsync() instead!

int initOpenCV()
----------------

.. method:: int initOpenCV(String Version, Context AppContext, LoaderCallbackInterface Callback)

    Trys to init OpenCV library using OpenCV Engine Service. Callback method will be called, when initialisation finishes.

    :param Version: Version of OpenCV;
    :param AppContext: Application context for service connection;
    :param CallBack: Object that implements LoaderCallbackInterface. See Helper callback interface;
    :rtype: int;
    :return: Base initialisation status. See Helper class constants.