LOCAL_PATH := $(call my-dir)
MAIN_LOCAL_PATH := $(LOCAL_PATH)

# 预编译静态库 - libdobby
include $(CLEAR_VARS)
LOCAL_MODULE := libdobby
LOCAL_SRC_FILES := Dobby/libraries/$(TARGET_ARCH_ABI)/libdobby.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/dobby/
include $(PREBUILT_STATIC_LIBRARY)

# 预编译静态库 - libcurl
include $(CLEAR_VARS)
LOCAL_MODULE := libcurl
LOCAL_SRC_FILES := Dobby/libraries/$(TARGET_ARCH_ABI)/libcurl.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/dobby/
include $(PREBUILT_STATIC_LIBRARY)

# 主共享库
include $(CLEAR_VARS)

# 基础配置
LOCAL_MODULE := Hook
LOCAL_CPP_EXTENSION := .cpp .cc
LOCAL_ARM_MODE := arm

# 编译选项
LOCAL_CFLAGS := -std=c17 -fvisibility=hidden -Wno-error=format-security -w -fno-rtti -fno-exceptions -fpermissive
LOCAL_CPPFLAGS := -std=c++17 -fvisibility=hidden -Wno-error=format-security -fpermissive -w -Werror -s -fno-rtti -fno-exceptions -fms-extensions -Wno-error=c++11-narrowing

# 链接选项
LOCAL_LDFLAGS := -Wl,--gc-sections,--strip-all -llog
LOCAL_LDLIBS := -llog -landroid -lz -ldl -lEGL -lGLESv2 -lGLESv3 -lGLESv1_CM

# 头文件路径
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Dobby
LOCAL_C_INCLUDES += $(LOCAL_PATH)/Imgui

# 源文件列表
FILE_LIST += $(wildcard $(LOCAL_PATH)/*.c*)
FILE_LIST += $(wildcard $(LOCAL_PATH)/Imgui/*.c*)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += Dobby/libraries/$(TARGET_ARCH_ABI)/syscall.s

# 静态库依赖 & 特性
LOCAL_STATIC_LIBRARIES += libdobby
LOCAL_CPP_FEATURES := exceptions

include $(BUILD_SHARED_LIBRARY)
