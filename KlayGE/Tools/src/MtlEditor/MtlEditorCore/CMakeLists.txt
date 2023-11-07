SET(SOURCE_FILES 
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditorCore/MtlEditorCore.cpp
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditorCore/Model.cpp
)

SET(HEADER_FILES 
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditorCore/MtlEditorCore.hpp
	${KLAYGE_PROJECT_DIR}/Tools/src/MtlEditor/MtlEditorCore/Model.hpp
)

SET(EFFECT_FILES 
	${KLAYGE_PROJECT_DIR}/Tools/media/MtlEditor/MtlEditor.fxml
	${KLAYGE_PROJECT_DIR}/Tools/media/MtlEditor/MVUtil.fxml
)

SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
SOURCE_GROUP("Resource Files" FILES ${RESOURCE_FILES})
SOURCE_GROUP("Effect Files" FILES ${EFFECT_FILES})
SOURCE_GROUP("Post Processors" FILES ${POST_PROCESSORS})
SOURCE_GROUP("UI Files" FILES ${UI_FILES})

ADD_LIBRARY(MtlEditorCore SHARED ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES} ${EFFECT_FILES} ${POST_PROCESSORS} ${UI_FILES})

ADD_DEPENDENCIES(MtlEditorCore AllInEngine)

SET_TARGET_PROPERTIES(MtlEditorCore PROPERTIES
	OUTPUT_NAME MtlEditorCore
	FOLDER "KlayGE/Tools/MtlEditor"
)

target_link_libraries(MtlEditorCore
	PUBLIC
		KlayGE_Core
)
