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

SET(RESOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
	${KLAYGE_PROJECT_DIR}/klayge_logo.ico
)

SET(EXTRA_INCLUDE_DIRS
	${KLAYGE_PROJECT_DIR}/../External/Python/Include
)

IF(CMAKE_GENERATOR MATCHES "Win64")
	SET(EXTRA_LINKED_DIRS
		${KLAYGE_PROJECT_DIR}/../External/Python/libs/amd64
	)
ELSE()
	SET(EXTRA_LINKED_DIRS
		${KLAYGE_PROJECT_DIR}/../External/Python/libs
	)
ENDIF()

SET(EXTRA_LINKED_LIBRARIES
	debug python32_d optimized python32
)

SETUP_SAMPLE(ScenePlayer)

ADD_CUSTOM_COMMAND(TARGET ScenePlayer
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/ScenePlayer.py ${KLAYGE_BIN_DIR})


INSTALL(FILES ${KLAYGE_PROJECT_DIR}/Samples/media/ScenePlayer/ScenePlayer.py
	DESTINATION ${KLAYGE_BIN_DIR}
)
