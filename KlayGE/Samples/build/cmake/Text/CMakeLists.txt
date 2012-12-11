SET(SOURCE_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Text/Text.cpp
)
IF(KLAYGE_WITH_WINRT)
	SET(SOURCE_FILES
		${SOURCE_FILES}
		${KLAYGE_PROJECT_DIR}/Samples/media/Text/Package.appxmanifest
	)
ENDIF()

SET(HEADER_FILES
	${KLAYGE_PROJECT_DIR}/Samples/src/Text/Text.hpp
)

SET(UI_FILES
	${KLAYGE_PROJECT_DIR}/Samples/media/Text/Text.uiml
)

IF(KLAYGE_WITH_WINRT)
	SET(RESOURCE_FILES
		${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/Logo.png
		${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/SmallLogo.png
		${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/SplashScreen.png
		${KLAYGE_PROJECT_DIR}/Samples/media/Common/Assets/StoreLogo.png
	)
ELSE()
	SET(RESOURCE_FILES
		${KLAYGE_PROJECT_DIR}/Samples/media/Common/KlayGESample.rc
		${KLAYGE_PROJECT_DIR}/klayge_logo.ico
	)
ENDIF()

SETUP_SAMPLE(Text)
