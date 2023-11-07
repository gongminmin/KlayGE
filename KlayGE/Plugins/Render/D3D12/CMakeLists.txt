if (NOT KLAYGE_PLATFORM_WINDOWS)
	return()
endif()

ADD_LIBRARY(KlayGE_RenderEngine_D3D12 SHARED
	Source/D3D12Adapter.cpp
	Source/D3D12Adapter.hpp
	Source/D3D12AdapterList.cpp
	Source/D3D12AdapterList.hpp
	Source/D3D12Fence.cpp
	Source/D3D12Fence.hpp
	Source/D3D12FrameBuffer.cpp
	Source/D3D12FrameBuffer.hpp
	Source/D3D12GpuDescriptorAllocator.cpp
	Source/D3D12GpuDescriptorAllocator.hpp
	Source/D3D12GpuMemoryAllocator.cpp
	Source/D3D12GpuMemoryAllocator.hpp
	Source/D3D12GraphicsBuffer.cpp
	Source/D3D12GraphicsBuffer.hpp
	Source/D3D12InterfaceLoader.cpp
	Source/D3D12InterfaceLoader.hpp
	Source/D3D12Query.cpp
	Source/D3D12Query.hpp
	Source/D3D12RenderEngine.cpp
	Source/D3D12RenderEngine.hpp
	Source/D3D12RenderFactory.cpp
	Source/D3D12RenderFactory.hpp
	Source/D3D12RenderLayout.cpp
	Source/D3D12RenderLayout.hpp
	Source/D3D12RenderStateObject.cpp
	Source/D3D12RenderStateObject.hpp
	Source/D3D12RenderView.cpp
	Source/D3D12RenderView.hpp
	Source/D3D12RenderWindow.cpp
	Source/D3D12RenderWindow.hpp
	Source/D3D12Resource.cpp
	Source/D3D12Resource.hpp
	Source/D3D12ShaderObject.cpp
	Source/D3D12ShaderObject.hpp
	Source/D3D12Texture.cpp
	Source/D3D12Texture.hpp
	Source/D3D12Texture1D.cpp
	Source/D3D12Texture2D.cpp
	Source/D3D12Texture3D.cpp
	Source/D3D12TextureCube.cpp
	Source/D3D12Util.cpp
	Source/D3D12Util.hpp
	Source/D3D12VideoMode.cpp
	Source/D3D12VideoMode.hpp
)

if(KLAYGE_COMPILER_GCC)
	# GCC warnings emitted by preprocessor can't be suppressed with pragma GCC diagnostic ignored
	target_compile_options(KlayGE_RenderEngine_D3D12
		PRIVATE
			-Wno-unknown-pragmas
	)
endif()

SET_TARGET_PROPERTIES(KlayGE_RenderEngine_D3D12 PROPERTIES
	OUTPUT_NAME KlayGE_RenderEngine_D3D12${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Render"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(KlayGE_RenderEngine_D3D12 PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(KlayGE_RenderEngine_D3D12
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(KlayGE_RenderEngine_D3D12
	PRIVATE
		KlayGE_Core
		DirectX-Headers
		dxsdk
)
IF(KLAYGE_PLATFORM_WINDOWS_STORE)
	target_link_libraries(KlayGE_RenderEngine_D3D12
		PRIVATE
			d3d12 dxgi dxguid
	)
ENDIF()

ADD_DEPENDENCIES(AllInEngine KlayGE_RenderEngine_D3D12)
