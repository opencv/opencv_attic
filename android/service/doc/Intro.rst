************
Introduction
************

.. highlight:: java

OpenCV Engine is an Android service targeted to manage OpenCV library binaries on end users devices. It allows sharing the OpenCV dynamic libraries of different versions between applications on the same device. The Engine provides the following benefits\:

- Less memory usage. All apps use the same binaries from service and do not keep native libs inside themself;
- Hardware specific optimisations for all supported platfroms;
- Trusted OpenCV library source. All packages with OpenCV are publiched on Google Play service;
- Regular updates and bug fixes;

Usage model for target user
---------------------------

.. image:: img/AndroidAppUsageModel.dia.png

First OpenCV app\:

- User downloads app dependent from OpenCV from Google Play or installs it manually;
- User starts application. Application asks user to install OpenCV Engine;
- User installs OpenCV Engine from Google Play Service;
- User starts application. Application proposes to user to install OpenCV library for target device and runs Google Play;
- User runs app in the third time  and gets what he or she wants.

Next OpenCV app:

- User downloads app dependent from OpenCV from Google Play or installs it manually;
- User starts application.
- If selected version is not installed Engine asks user to install OpenCV library package and runs Google Play;
- User runs app in the second time and gets what he or she wants.