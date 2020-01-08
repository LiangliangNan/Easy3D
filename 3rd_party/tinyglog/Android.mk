LOCAL_PATH := $(call my-dir)
# Mini glog so
include $(CLEAR_VARS)
LOCAL_MODULE           := miniglog
$(warning "Build miniglog.so/miniglog.a")
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES :=  $(LOCAL_PATH)
$(warning $(LOCAL_PATH))
LOCAL_SRC_FILES  := glog/logging.cc

# glog start
ifeq ($(MINIGLOG_LIB_TYPE),SHARED)
	include $(BUILD_SHARED_LIBRARY)
else
	include $(BUILD_STATIC_LIBRARY)
endif
# glog end

