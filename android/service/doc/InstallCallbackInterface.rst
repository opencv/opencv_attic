**************************
Install Callback Interface
**************************
.. highlight:: java
.. module:: org.opencv.android
    :platform: Android
    :synopsis: Defines callback interface for package managment.

.. class:: InstallCallbackInterface

    Callback interface for package installation or update.

String getPackageName()
-----------------------

.. method:: String getPackageName()

    Get name of a package to be installed

    :rtype: String
    :return: Return package name, i.e. "OpenCV Manager Service" or "OpenCV library"

void install()
--------------

.. method:: void install()

    Installation of package is approved

void cancel()
-------------

.. method:: void cancel()

    Installation canceled
