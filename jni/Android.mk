LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_MODULE := libsmart_remote
LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c))
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)

LOCAL_SHARED_LIBRARIES += liblog libcutils	

include $(BUILD_SHARED_LIBRARY)