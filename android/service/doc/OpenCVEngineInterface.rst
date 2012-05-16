*********************************
Java OpenCV OpenCVEngineInterface
*********************************

.. highlight:: java
.. module:: org.opencv.engine
    :platform: Android
    :synopsis: Defines OpenCV Engine interface for Android.
.. Class:: OpenCVEngineInterface

OpenCVEngineInterface class provides Java interface to OpenCV Engine Service. Is synchronious with native OpenCVEngine class.

int getEngineVersion()
----------------------

.. method:: int GetEngineVersion()

    Get OpenCV Engine version

    :rtype: int
    :return: Return OpenCV Engine version

String getLibPathByVersion()
----------------------------

.. method:: String GetLibPathByVersion(String version)

    Find already installed OpenCV library 

    :param version: OpenCV library version
    :rtype: String
    :return: Return path to OpenCV native libs or empty string if OpenCV was not found

String getLibraryList()
-----------------------

.. method:: String GetLibraryList(String version)

    Get list of OpenCV native libraries in loading order seporated by ";" symbol

    :param version: OpenCV library version
    :rtype: String
    :return: Return OpenCV libraries names seporated by symbol ";" in loading order

boolean installVersion()
------------------------

.. method:: boolean InstallVersion(String version)

    Try to install defined version of OpenCV from Google Play (Android Market).

    :param version: OpenCV library version
    :rtype: String
    :return: Return true if installation was successful or OpenCV package has been already installed
 