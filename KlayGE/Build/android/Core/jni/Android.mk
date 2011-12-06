LOCAL_PATH := $(call my-dir)
KLAYGE_PATH := $(LOCAL_PATH)
KLAYGE_SRC_PATH := $(LOCAL_PATH)/../../../../Core/Src

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(KLAYGE_SRC_PATH)/../../../External/boost \
		$(KLAYGE_SRC_PATH)/../../../External/Python/include \
		$(KLAYGE_SRC_PATH)/../../../External/rapidxml \
		$(KLAYGE_SRC_PATH)/../../../glloader/include \
		$(KLAYGE_SRC_PATH)/../Include \
		
LOCAL_MODULE := KlayGE_Core_gcc
LOCAL_PATH := $(KLAYGE_SRC_PATH)
LOCAL_SRC_FILES := \
		AppLayer/App3D.cpp \
		AppLayer/Window.cpp \
		Audio/AudioBuffer.cpp \
		Audio/AudioDataSource.cpp \
		Audio/AudioEngine.cpp \
		Audio/AudioFactory.cpp \
		Audio/MusicBuffer.cpp \
		Audio/SoundBuffer.cpp \
		Input/InputActionMap.cpp \
		Input/InputDevice.cpp \
		Input/InputEngine.cpp \
		Input/InputFactory.cpp \
		Input/Joystick.cpp \
		Input/Keyboard.cpp \
		Input/Mouse.cpp \
		Kernel/Context.cpp \
		Kernel/CpuInfo.cpp \
		Kernel/DllLoader.cpp \
		Kernel/KlayGE.cpp \
		Kernel/ResLoader.cpp \
		Kernel/ThrowErr.cpp \
		Kernel/Timer.cpp \
		Kernel/Util.cpp \
		Kernel/XMLDom.cpp \
		Math/Frustum.cpp \
		Math/Math.cpp \
		Net/Lobby.cpp \
		Net/Player.cpp \
		Net/Socket.cpp \
		Pack/ArchiveExtractCallback.cpp \
		Pack/ArchiveOpenCallback.cpp \
		Pack/BSTR.cpp \
		Pack/Extract7z.cpp \
		Pack/LzFind.cpp \
		Pack/LZMACodec.cpp \
		Pack/LzmaDec.cpp \
		Pack/LzmaEnc.cpp \
		Pack/Streams.cpp \
		Render/BlockCompression.cpp \
		Render/Camera.cpp \
		Render/CameraController.cpp \
		Render/DeferredRenderingLayer.cpp \
		Render/Font.cpp \
		Render/FrameBuffer.cpp \
		Render/FXAAPostProcess.cpp \
		Render/GraphicsBuffer.cpp \
		Render/HDRPostProcess.cpp \
		Render/HeightMap.cpp \
		Render/InfTerrain.cpp \
		Render/JudaTexture.cpp \
		Render/LensFlare.cpp \
		Render/Light.cpp \
		Render/Mesh.cpp \
		Render/PostProcess.cpp \
		Render/Query.cpp \
		Render/Renderable.cpp \
		Render/RenderableHelper.cpp \
		Render/RenderEffect.cpp \
		Render/RenderEngine.cpp \
		Render/RenderFactory.cpp \
		Render/RenderLayout.cpp \
		Render/RenderStateObject.cpp \
		Render/RenderView.cpp \
		Render/SATPostProcess.cpp \
		Render/ShaderObject.cpp \
		Render/SSVOPostProcess.cpp \
		Render/TAAPostProcess.cpp \
		Render/Texture.cpp \
		Render/Viewport.cpp \
		Script/Script.cpp \
		Show/ShowEngine.cpp \
		Show/ShowFactory.cpp \
		Scene/SceneManager.cpp \
		Scene/SceneObject.cpp \
		Scene/SceneObjectHelper.cpp \
		UI/UI.cpp \
		UI/UIButton.cpp \
		UI/UICheckBox.cpp \
		UI/UIComboBox.cpp \
		UI/UIEditBox.cpp \
		UI/UIListBox.cpp \
		UI/UIPolylineEditBox.cpp \
		UI/UIProgressBar.cpp \
		UI/UIRadioButton.cpp \
		UI/UIScrollBar.cpp \
		UI/UISlider.cpp \
		UI/UIStatic.cpp \
		UI/UITexButton.cpp \

		
LOCAL_CFLAGS := -DKLAYGE_BUILD_DLL -DKLAYGE_CORE_SOURCE -DGLLOADER_GLES_SUPPORT

include $(BUILD_STATIC_LIBRARY)
