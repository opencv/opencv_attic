**************************************
Native OpenCV Engine service interface
**************************************

.. highlight:: cpp
.. module:: IOpenCVEngine.h
    :platform: Android
    :synopsis: Defines OpenCV Engine interface for Android Binder component.
.. Class:: OpenCVEngine

OpenCVEngine class provides Binder interface to OpenCV Engine Service.

int getEngineVersion()
----------------------

.. method:: int GetEngineVersion()

    Gets OpenCV Engine version.

    :rtype: int;
    :return: Returns OpenCV Engine version.

android::String16 getLibPathByVersion()
---------------------------------------

.. method:: android::String16 GetLibPathByVersion(android::String16 version)

    Gets path to native OpenCV libraries. 

    :param version: OpenCV Library version;
    :rtype: String;
    :return: Returns path to OpenCV native libs or empty string if OpenCV was not found.

android::String16 getLibraryList()
----------------------------------

.. method:: android::String16 GetLibraryList(android::String16 version)

    Gets list of OpenCV native libraries in loading order.

    :param version: OpenCV Library version;
    :rtype: String;
    :return: Returns OpenCV libraries names seporated by semicolumn symbol in loading order.

boolean installVersion()
------------------------

.. method:: boolean InstallVersion(android::String16 version)

    Trys to install defined version of OpenCV.

    :param version: OpenCV Library version;
    :rtype: String;
    :return: Returns true if installation successfull or package has been already installed.
