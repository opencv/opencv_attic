************************************
Java Static OpenCV Helper (internal)
************************************

.. highlight:: java
.. module:: org.opencv.android
    :platform: Android
    :synopsis: Implements Android dependent Java classes
.. Class:: AsyncServiceHelper

Helper class provides implementation of static OpenCV initialization. All OpenCV libraries must be included to application package.

.. note:: This is imternal class. Does not use it directly. Use OpenCVLoader.initStatic() instead!

int initOpenCV()
----------------

.. method:: int initOpenCV()

    Tries to init OpenCV library using libraries from application package. Method uses libopencv_info.so library for getting 
    list of libraries in loading order. Method loads libopencv_java.so, if info library is not present.

    :rtype: boolean
    :return: Return true if initialization was successful