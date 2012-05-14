LOCAL_PATH := $(call my-dir)

#---------------------------------------------------------------------
#        Binder component library
#---------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
    BinderComponent/OpenCVEngine.cpp \
    BinderComponent/BnOpenCVEngine.cpp \
    BinderComponent/BpOpenCVEngine.cpp \
    BinderComponent/ProcReader.cpp \
    BinderComponent/TegraDetector.cpp \
    BinderComponent/StringUtils.cpp \
    BinderComponent/HardwareDetector.cpp

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/BinderComponent \
    $(TOP)/frameworks/base/include \
    $(TOP)/system/core/include

LOCAL_CFLAGS += -DPLATFORM_ANDROID

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libOpenCVEngine

LOCAL_LDLIBS += -lz -lbinder -llog -lutils

LOCAL_LDFLAGS += -Wl,-allow-shlib-undefine

include $(BUILD_SHARED_LIBRARY)

#---------------------------------------------------------------------
#        JNI library for Java service
#---------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
    JNIWrapper/OpenCVEngine_jni.cpp \
    NativeService/CommonPackageManager.cpp \
    JNIWrapper/JavaBasedPackageManager.cpp \
    NativeService/PackageInfo.cpp

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/JNIWrapper \
    $(LOCAL_PATH)/NativeService \
    $(LOCAL_PATH)/BinderComponent \
    $(TOP)/frameworks/base/include \
    $(TOP)/system/core/include \
    $(TOP)/frameworks/base/core/jni

LOCAL_PRELINK_MODULE := false

LOCAL_CFLAGS += -DPLATFORM_ANDROID

LOCAL_MODULE := libOpenCVEngine_jni

LOCAL_LDLIBS += -lz -lbinder -llog -lutils -landroid_runtime

LOCAL_SHARED_LIBRARIES = libOpenCVEngine

include $(BUILD_SHARED_LIBRARY)

# #---------------------------------------------------------------------
# #        Native test application
# #---------------------------------------------------------------------
# 
# include $(CLEAR_VARS)
# 
# LOCAL_MODULE_TAGS := optional
# 
# LOCAL_CPP_EXTENSION := .cc
# 
# LOCAL_SRC_FILES := \
#     Tests/gtest/gtest-all.cpp \
#     Tests/TestMain.cpp
# # 
# #     BinderComponent/ProcReader.cpp \
# #     BinderComponent/TegraDetector.cpp \
# #     BinderComponent/HardwareDetector_itseez.cpp \
# #     Tests/PackageManagerStub.cpp \
# #     NativeService/CommonPackageManager.cpp \
# #     NativeService/PackageInfo.cpp \
# #     Tests/PackageManagmentTest.cpp \
# #     Tests/PackageInfoTest.cpp \
# #     Tests/OpenCVEngineTest.cpp \
# #     Tests/HardwareDetectionTest.cpp \
# 
# LOCAL_C_INCLUDES := \
#     $(LOCAL_PATH)/Tests \
#     $(LOCAL_PATH)/Tests/gtest \
#     $(LOCAL_PATH)/include \
#     $(LOCAL_PATH)/BinderComponent \
#     $(LOCAL_PATH)/NativeService \
#     $(LOCAL_PATH)/Tests/gtest/include \
#     $(TOP)/frameworks/base/include \
#     $(TOP)/system/core/include
# 
# LOCAL_CFLAGS += -O0 -DGTEST_HAS_TR1_TUPLE=0
# 
# LOCAL_LDFLAGS = -Wl,-allow-shlib-undefined
# 
# LOCAL_MODULE := OpenCVEngineTestApp
# 
# LOCAL_LDLIBS += -lz -lbinder -llog
# 
# LOCAL_SHARED_LIBRARIES += libOpenCVEngine
# 
# include $(BUILD_EXECUTABLE)