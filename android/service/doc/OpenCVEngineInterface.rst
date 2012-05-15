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

    Gets OpenCV Engine version.

    :rtype: int;
    :return: Returns OpenCV Engine version.

String getLibPathByVersion()
----------------------------

.. method:: String GetLibPathByVersion(String version)

    Gets path to native OpenCV libraries. 

    :param version: OpenCV Library version;
    :rtype: String;
    :return: Returns path to OpenCV native libs or empty string if OpenCV was not found.

String getLibraryList()
-----------------------

.. method:: String GetLibraryList(String version)

    Gets list of OpenCV native libraries in loading order.

    :param version: OpenCV Library version;
    :rtype: String;
    :return: Returns OpenCV libraries names seporated by semicolumn symbol in loading order.

boolean installVersion()
------------------------

.. method:: boolean InstallVersion(String version)

    Trys to install defined version of OpenCV.

    :param version: OpenCV Library version;
    :rtype: String;
    :return: Returns true if installation successfull or package has been already installed.
 