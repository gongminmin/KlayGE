SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/ScenePlayer/ScenePlayer.cpp
	${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/ScenePlayer.py
	${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/DeferredRendering.scene
	${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/GlobalIllumination.scene
	${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/ShadowCubemap.scene
)

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/ScenePlayer/ScenePlayer.hpp
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/ScenePlayer.uiml
)

SETUP_SAMPLE(ScenePlayer)

ADD_CUSTOM_COMMAND(TARGET ScenePlayer
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/ScenePlayer.py ${KLAYGE_BIN_DIR})

INSTALL(FILES ${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/ScenePlayer.py
	DESTINATION ${KLAYGE_BIN_DIR}
)
