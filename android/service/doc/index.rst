*************************************************
OpenCV Engine. Android service for OpenCV
*************************************************

OpenCV Engine is an Android service targeted to manage OpenCV library binaries on end users devices. It allows sharing the OpenCV dynamic libraries of different versions between applications on the same device. The Engine provides the following benefits\:

- Less memory usage. All apps use the same binaries from service and do not keep native libs inside themself;
- Hardware specific optimisations for all supported platfroms;
- Trusted OpenCV library source. All packages with OpenCV are publiched on Google Play service;
- Regular updates and bug fixes;

Contents:

.. toctree::
   :maxdepth: 2

   Intro
   UseCases
   JavaHelper
   OpenCVEngineInterface
   AsyncServiceHelper
   NativeHelper
   IOpenCVEngine
