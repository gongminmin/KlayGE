if (NOT KLAYGE_PLATFORM_WINDOWS)
	return()
endif()

ADD_LIBRARY(KlayGE_RenderEngine_D3D11 SHARED
	Source/D3D11Adapter.cpp
	Source/D3D11Adapter.hpp
	Source/D3D11AdapterList.cpp
	Source/D3D11AdapterList.hpp
	Source/D3D11Fence.cpp
	Source/D3D11Fence.hpp
	Source/D3D11FrameBuffer.cpp
	Source/D3D11FrameBuffer.hpp
	Source/D3D11GraphicsBuffer.cpp
	Source/D3D11GraphicsBuffer.hpp
	Source/D3D11Query.cpp
	Source/D3D11Query.hpp
	Source/D3D11RenderEngine.cpp
	Source/D3D11RenderEngine.hpp
	Source/D3D11RenderFactory.cpp
	Source/D3D11RenderFactory.hpp
	Source/D3D11RenderLayout.cpp
	Source/D3D11RenderLayout.hpp
	Source/D3D11RenderStateObject.cpp
	Source/D3D11RenderStateObject.hpp
	Source/D3D11RenderView.cpp
	Source/D3D11RenderView.hpp
	Source/D3D11RenderWindow.cpp
	Source/D3D11RenderWindow.hpp
	Source/D3D11ShaderObject.cpp
	Source/D3D11ShaderObject.hpp
	Source/D3D11Texture.cpp
	Source/D3D11Texture.hpp
	Source/D3D11Texture1D.cpp
	Source/D3D11Texture2D.cpp
	Source/D3D11Texture3D.cpp
	Source/D3D11TextureCube.cpp
	Source/D3D11Util.cpp
	Source/D3D11Util.hpp
	Source/D3D11VideoMode.cpp
	Source/D3D11VideoMode.hpp
)

if(KLAYGE_COMPILER_GCC)
	# GCC warnings emitted by preprocessor can't be suppressed with pragma GCC diagnostic ignored
	target_compile_options(KlayGE_RenderEngine_D3D11
		PRIVATE
			-Wno-unknown-pragmas
	)
endif()

SET_TARGET_PROPERTIES(KlayGE_RenderEngine_D3D11 PROPERTIES
	OUTPUT_NAME KlayGE_RenderEngine_D3D11${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Render"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(KlayGE_RenderEngine_D3D11 PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(KlayGE_RenderEngine_D3D11
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(KlayGE_RenderEngine_D3D11
	PRIVATE
		KlayGE_Core
		DirectX-Headers
		dxsdk
)
IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	target_link_libraries(KlayGE_RenderEngine_D3D11
		PRIVATE
			d3d11 dxgi dxguid
	)
ENDIF()

ADD_DEPENDENCIES(AllInEngine KlayGE_RenderEngine_D3D11)
