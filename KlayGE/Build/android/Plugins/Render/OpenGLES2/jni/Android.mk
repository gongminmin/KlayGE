LOCAL_PATH := $(call my-dir)

BOOST_PATH := ../../../../../../../External/boost
$(call import-module, boost)

KLAYGE_CORE_PATH := ../../../../../../
$(call import-module, KlayGE_Core)

GLLOADER_PATH := ../../../../../../../glloader
$(call import-module, glloader)

KLAYGE_PLUGIN_OPENGLES2_PATH := $(LOCAL_PATH)
KLAYGE_PLUGIN_OPENGLES2_SRC_PATH := $(LOCAL_PATH)/../../../../../../Plugins/Src/Render/OpenGLES2

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(KLAYGE_PLUGIN_OPENGLES2_SRC_PATH)/../../../../../External/boost \
		$(KLAYGE_PLUGIN_OPENGLES2_SRC_PATH)/../../../../Core/Include \
		$(KLAYGE_PLUGIN_OPENGLES2_SRC_PATH)/../../../Include \
		$(KLAYGE_PLUGIN_OPENGLES2_SRC_PATH)/../../../../../glloader/include \
		
LOCAL_MODULE := KlayGE_RenderEngine_OpenGLES2_gcc
LOCAL_PATH := $(KLAYGE_PLUGIN_OPENGLES2_SRC_PATH)
LOCAL_SRC_FILES := \
		OGLES2FrameBuffer.cpp \
		OGLES2GraphicsBuffer.cpp \
		OGLES2Mapping.cpp \
		OGLES2RenderEngine.cpp \
		OGLES2RenderFactory.cpp \
		OGLES2RenderLayout.cpp \
		OGLES2RenderStateObject.cpp \
		OGLES2RenderView.cpp \
		OGLES2RenderWindow.cpp \
		OGLES2ShaderObject.cpp \
		OGLES2Texture.cpp \
		OGLES2Texture1D.cpp \
		OGLES2Texture2D.cpp \
		OGLES2Texture3D.cpp \
		OGLES2TextureCube.cpp \

		
LOCAL_CFLAGS := -DKLAYGE_BUILD_DLL -DKLAYGE_OGL_RE_SOURCE -DGLLOADER_GLES_SUPPORT

LOCAL_LDLIBS := -llog -landroid
LOCAL_STATIC_LIBRARIES := KlayGE_Core glloader boost_date_time boost_filesystem boost_signals boost_system boost_thread

include $(BUILD_SHARED_LIBRARY)
