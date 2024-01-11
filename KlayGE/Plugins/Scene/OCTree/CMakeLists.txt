option(KLAYGE_BUILD_PLUGIN_OCTREE_SCENE "Build Octree scene plugin" ON)
if(NOT KLAYGE_BUILD_PLUGIN_OCTREE_SCENE)
	return()
endif()

ADD_LIBRARY(KlayGE_Scene_OCTree ${KLAYGE_PREFERRED_LIB_TYPE}
	Source/OCTree.cpp
	Source/OCTree.hpp
	Source/OCTreeFactory.cpp
)

SET_TARGET_PROPERTIES(KlayGE_Scene_OCTree PROPERTIES
	OUTPUT_NAME KlayGE_Scene_OCTree${KLAYGE_OUTPUT_SUFFIX}
	FOLDER "KlayGE/Engine/Plugins/Scene Management"
)
if(KLAYGE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
	set_target_properties(KlayGE_Scene_OCTree PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)
endif()

target_precompile_headers(KlayGE_Scene_OCTree
	PRIVATE
		"${KLAYGE_PROJECT_DIR}/Core/Include/KlayGE/KlayGE.hpp"
)

target_link_libraries(KlayGE_Scene_OCTree
	PRIVATE
		KlayGE_Core
)

ADD_DEPENDENCIES(AllInEngine KlayGE_Scene_OCTree)
