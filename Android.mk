LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.biometrics.fingerprint@2.1-service.sony
LOCAL_INIT_RC := android.hardware.biometrics.fingerprint@2.1-service.sony.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := \
    $(call all-subdir-cpp-files) \
    QSEEComFunc.c \
    common.c

LOCAL_CFLAGS += \
    -DEGISTEC_SAVE_TEMPLATE_RETURNS_SIZE \
    -DEGIS_QSEE_APP_NAME=\"egista\"

LOCAL_CFLAGS += \
    -DHAS_DYNAMIC_POWER_MANAGEMENT

LOCAL_SHARED_LIBRARIES := \
    android.hardware.biometrics.fingerprint@2.1 \
    libcutils \
    libdl \
    libdmabufheap \
    libhardware \
    libhidlbase \
    liblog \
    libutils

LOCAL_CFLAGS += \
    -DPLATFORM_SDK_VERSION=$(PLATFORM_SDK_VERSION) \
    -fexceptions

include $(BUILD_EXECUTABLE)
endif
endif # PRODUCT_PLATFORM_SOD
